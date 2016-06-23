/*
* Copyright (c) 2000-2015 Samsung Electronics Co., Ltd All Rights Reserved
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
*/

#include "mp-ta.h"
#include "music.h"
#include "mp-setting-ctrl.h"
#include "mp-item.h"
#include "mp-player-control.h"
#include "mp-player-view.h"
#include "mp-playlist-mgr.h"
#include <system_settings.h>
#include "mp-file-util.h"
#include <efl_extension.h>

#include <signal.h>
// TEMP_BLOCK
//#include <power.h>
#include <glib.h>
#include <glib-object.h>
#include "mp-player-mgr.h"
#include "mp-player-debug.h"
#include "mp-lockscreenmini.h"
#include <pthread.h>
#include <media_key.h>
#include "mp-minicontroller.h"
#include "mp-play.h"
#include "mp-app.h"
#include "mp-ug-launch.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-all-view.h"
#ifdef MP_FEATURE_S_BEAM
#include "mp-s-beam.h"
#endif
#include "mp-volume.h"

#ifndef MP_SOUND_PLAYER
#include "mp-common.h"
#endif

#ifdef MP_FEATURE_APP_IN_APP
#include "mp-mini-player.h"
#include <math.h>
#endif

#ifdef MP_3D_FEATURE
#include "dali-music.h"
#endif

#ifdef MP_FEATURE_AVRCP_13
#include "mp-avrcp.h"
#endif

#ifdef MP_FEATURE_VOICE_CONTROL
#include "mp-voice-control-mgr.h"
#endif
#include <storage/storage.h>

void mp_play_next_and_updateview(void *data);

static Ecore_Pipe *gNotiPipe;
typedef enum {
	MP_APP_PIPE_CB_AVAILABLE_ROUTE_CHANGED,
	MP_APP_PIPE_CB_ACTIVE_DEVICE_CHANGED,
} mp_app_pipe_cb_type_e;

typedef struct {
	mp_app_pipe_cb_type_e type;
	void *user_data;
	//Replaced for _prod dependency
	sound_device_changed_info_e  out;
} mp_app_pipe_data_s;

#ifdef MP_FEATURE_USB_OTG
#define BUS_NAME                "org.tizen.usb.storage"
#define OBJECT_PATH             "/Org/Tizen/Usb/Storage"
#define INTERFACE_NAME          BUS_NAME
#define SIGNAL_NAME_USB_STORAGE "usbstorage"
#define USB_STORAGE_ADDED       "added"
#define USB_STORAGE_REMOVED     "removed"
#endif

#ifdef MP_ENABLE_INOTIFY
static void _mp_add_inofity_refresh_watch(struct appdata *ad);
#endif

static Eina_Bool
_mp_app_ear_key_timer_cb(void *data)
{
	EVENT_TRACE("");
	struct appdata *ad = (struct appdata *)data;
	if (ad->ear_key_press_cnt == 1) {
		DEBUG_TRACE("play/pause ctrl");
		if (ad->player_state == PLAY_STATE_PLAYING) {
			ad->paused_by_user = TRUE;
			mp_player_mgr_pause(ad);
		} else if (ad->player_state == PLAY_STATE_PAUSED) {
			ad->paused_by_user = FALSE;
			if (!mp_player_mgr_resume(ad)) {
				mp_setting_set_nowplaying_id(getpid());
				if (ad->player_state == PLAY_STATE_PAUSED) {
					mp_play_resume(ad);
				}
				ad->player_state = PLAY_STATE_PLAYING;
			}
		} else if (ad->player_state == PLAY_STATE_READY) {
			ad->paused_by_user = FALSE;
			mp_play_start_in_ready_state(ad);
		} else {
			ad->paused_by_user = FALSE;
			int ret = mp_play_new_file(ad, TRUE);
			if (ret) {
				ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
				if (ret == MP_PLAY_ERROR_NETWORK) {
					mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
				}
#endif
			}
		}
	} else if (ad->ear_key_press_cnt == 2) {
		DEBUG_TRACE("next ctrl");
		mp_play_next_file(data, TRUE);
	} else {
		DEBUG_TRACE("prev ctrl");
		mp_play_prev_file(data);
	}
	ad->ear_key_press_cnt = 0;
	ad->ear_key_timer = NULL;
	return EINA_FALSE;
}

void
_mp_app_noti_key_changed_cb(const char *key, void *data)
{
	EVENT_TRACE("");

	struct appdata *ad = (struct appdata *)data;
	MP_CHECK(ad);

	if (strcmp(key, MP_PREFKEY_PLAYING_PID) == 0) {
		int playing_pid = 0;
		preference_get_int(key, &playing_pid);
#ifndef MP_SOUND_PLAYER
		if (!playing_pid) {
			if (ad->player_state == PLAY_STATE_PAUSED) {
				DEBUG_TRACE("sound-player terminated.. show minicontroller");

				if (!ad->win_minicon) {
					mp_minicontroller_create(ad);
				}
				if (ad->win_minicon) {
					mp_minicontroller_show(ad);
				}
#ifdef MP_FEATURE_LOCKSCREEN
				if (!ad->win_lockmini) {
					mp_lockscreenmini_create(ad);
				}
				if (ad->win_lockmini) {
					mp_lockscreenmini_show(ad);
				}
#endif
				mp_setting_save_playing_info(ad);
			}
		} else
#endif
			if (playing_pid != getpid()) {
				DEBUG_TRACE("other player activated : [pid:%d]", playing_pid);
				if (ad->player_state == PLAY_STATE_PLAYING) {
					ad->paused_by_other_player = TRUE;
					mp_play_control_play_pause(ad, false);
				}
				mp_minicontroller_destroy(ad);
#ifdef MP_FEATURE_LOCKSCREEN
				mp_lockscreenmini_destroy(ad);
#endif
			}
	}

}

void
_mp_app_storage_state_changed_cb(int storage_id, storage_state_e state, void *user_data)
{
	EVENT_TRACE("");

	struct appdata *ad = (struct appdata *)user_data;
	MP_CHECK(ad);
	ad->is_sdcard_removed = true;
	if (state == STORAGE_STATE_REMOVED) {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_MMC_REMOVED);
#ifndef MP_SOUND_PLAYER
		mp_common_force_close_delete();
#endif
	} else if (state == STORAGE_STATE_UNMOUNTABLE) {
		if (strstr(ad->current_track_info->uri, MP_MMC_ROOT_PATH)
		        == ad->current_track_info->uri) {
			mp_play_next_and_updateview(ad);
		} else {
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_MMC_REMOVED);
#ifndef MP_SOUND_PLAYER
			mp_common_force_close_delete();
#endif
		}
	}
}

void
_mp_app_system_settings_changed_cb(system_settings_key_e key, void *user_data)
{
	EVENT_TRACE("");
}

#ifdef MP_FEATURE_PERSONAL_PAGE
void
_mp_app_personal_page_changed_cb(const char *key, void *data)
{
	DEBUG_TRACE("personal page case changed");

	bool personal_page = true;
	preference_get_boolean(key, &personal_page);
	if (personal_page == 0) {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_PERSONAL_PAGE_OFF);
	} else {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_PERSONAL_PAGE_ON);
	}
}
#endif

//Replaced for _prod dependency start
static void
_mp_add_available_route_changed_cb(sound_device_h  device, bool available, void *user_data)
{
	EVENT_TRACE("route: 0x%x, available: %d", device, available);
	MP_CHECK(gNotiPipe);

	mp_app_pipe_data_s pipe_data;
	memset(&pipe_data, 0, sizeof(mp_app_pipe_data_s));
	pipe_data.type = MP_APP_PIPE_CB_AVAILABLE_ROUTE_CHANGED;
	pipe_data.user_data = user_data;

	ecore_pipe_write(gNotiPipe, &pipe_data, sizeof(mp_app_pipe_data_s));

	if (!available) {
        struct appdata *ad = user_data;

        mp_player_mgr_pause(ad);
    }

}
static void
_mp_app_active_device_chaged_cb(sound_device_h  in, sound_device_changed_info_e  out, void *user_data)
{
	EVENT_TRACE("input=[0x%x], output=[0x%x]", in, out);
	MP_CHECK(gNotiPipe);

	mp_app_pipe_data_s pipe_data;
	memset(&pipe_data, 0, sizeof(mp_app_pipe_data_s));
	pipe_data.type = MP_APP_PIPE_CB_ACTIVE_DEVICE_CHANGED;
	pipe_data.out = out;
	pipe_data.user_data = user_data;

	ecore_pipe_write(gNotiPipe, &pipe_data, sizeof(mp_app_pipe_data_s));
}

static void
_mp_app_noti_pipe_handler(void *data, void *buffer, unsigned int nbyte)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	mp_app_pipe_data_s *pipe_data = buffer;
	MP_CHECK(pipe_data);

	switch (pipe_data->type) {
	case MP_APP_PIPE_CB_AVAILABLE_ROUTE_CHANGED:
		mp_setting_update_active_device();
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_ROUTE_CHANGED);
		break;

	case MP_APP_PIPE_CB_ACTIVE_DEVICE_CHANGED:
		mp_setting_update_active_device();
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_ROUTE_CHANGED);
#ifdef MP_FEATURE_AVRCP_13
		mp_avrcp_noti_track_position(mp_player_mgr_get_position());
#endif
		break;

	default:
		WARN_TRACE("Not defined.. [%d]", pipe_data->type);
	}

}

#ifdef MP_FEATURE_VOICE_CONTROL
static void
_mp_app_voice_control_volume_popup_show(void *data)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	MpView_t *playing_view = GET_PLAYER_VIEW;
	MpView_t *top_vew = mp_view_mgr_get_top_view(GET_VIEW_MGR);
	if ((playing_view == top_vew) && mp_volume_key_is_grabed()) {
		mp_player_view_volume_popup_control(playing_view, true);
	} else {
		mp_util_system_volume_popup_show();
	}
}

static void
_mp_app_voice_control_action_cb(mp_voice_ctrl_action_e action, void *data)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	mp_debug("voic control action = [%d]", action);
	switch (action) {
	case MP_VOICE_CTRL_ACTION_NEXT:
		mp_play_next_file(ad, true);
		break;

	case MP_VOICE_CTRL_ACTION_PREVIOUS:
		mp_play_prev_file(ad);
		break;

	case MP_VOICE_CTRL_ACTION_PAUSE:
		mp_play_control_play_pause(ad, false);
		break;

	case MP_VOICE_CTRL_ACTION_PLAY:
		if (ad->player_state != PLAY_STATE_PLAYING) {
			if (!mp_playlist_mgr_count(ad->playlist_mgr)) {
				mp_media_list_h media = NULL;
				int count = 0;

				mp_media_info_list_count(MP_TRACK_BY_PLAYED_TIME, NULL, NULL, NULL, 0, &count);
				if (count > 100) {
					count = 100;
				}
				if (count > 0) {
					mp_media_info_list_create(&media, MP_TRACK_BY_PLAYED_TIME, NULL, NULL, NULL, 0, 0, count);
				} else {
					mp_media_info_list_create(&media, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, 100);
				}

				mp_playlist_mgr_clear(ad->playlist_mgr);
				mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, media, count, 0, NULL);
				mp_media_info_list_destroy(media);

				int ret = mp_play_new_file(ad, TRUE);
				if (ret) {
					ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
					if (ret == MP_PLAY_ERROR_NETWORK) {
						mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
					}
#endif
				}
			} else {
				mp_play_control_play_pause(ad, true);
			}
		}
		break;

	case MP_VOICE_CTRL_ACTION_VOLUME_UP:
		_mp_app_voice_control_volume_popup_show(ad);
		mp_player_mgr_volume_up();
		break;

	case MP_VOICE_CTRL_ACTION_VOLUME_DOWN:
		_mp_app_voice_control_volume_popup_show(ad);
		mp_player_mgr_volume_down();
		break;

	default:
		mp_debug("Not defined");
		break;
	}

	power_wakeup(false);

}
#endif

void _mp_app_db_update_cb(void *data)
{
	EVENT_TRACE("Post DB Update event");

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	//mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);

	bool current_removed = false;
	bool next_play = false;

	if (mp_player_mgr_get_state() == PLAYER_STATE_PLAYING) {
		next_play = true;
	}

	mp_playlist_mgr_check_existance_and_refresh(ad->playlist_mgr, &current_removed);
	if (current_removed) {
		mp_play_destory(ad);
		if (mp_playlist_mgr_get_current(ad->playlist_mgr) == NULL) {
			if (ad->current_track_info) {
				//for lock-screen deleting thumbnail
				if (mp_setting_read_playing_status(ad->current_track_info->uri, "stop") != 1) {
					mp_setting_write_playing_status(ad->current_track_info->uri, "stop");
				}
				mp_util_free_track_info(ad->current_track_info);
				ad->current_track_info = NULL;
			}
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_UNSET_NOW_PLAYING);
			if (ad->b_minicontroller_show) {
				mp_minicontroller_hide(ad);
			}
#ifdef MP_FEATURE_LOCKSCREEN
			if (ad->b_lockmini_show) {
				mp_lockscreenmini_hide(ad);
			}
#endif

			/*as all the items are removed, remove now-playing.ini to avoid copy the same track but in DB, they are different*/
			char *data_path = app_get_data_path();
			char nowplaying_ini[1024] = {0};
			snprintf(nowplaying_ini, 1024, "%s%s", data_path, MP_NOWPLAYING_INI_FILE_NAME);
			mp_file_remove(nowplaying_ini);
			/* remove playing_track.ini to avoid lockscreen still using the file content*/
			char playing_ini[1024] = {0};
#ifndef MP_SOUND_PLAYER
			snprintf(playing_ini, 1024, "%s%s", data_path, MP_PLAYING_INI_FILE_NAME_MUSIC);
			free(data_path);
			mp_file_remove(playing_ini);
#else
			snprintf(playing_ini, 1024, "%s%s", data_path, MP_PLAYING_INI_FILE_NAME_SOUND);
			free(data_path);
			mp_file_remove(playing_ini);
#endif
		} else if (next_play) {
			mp_play_new_file(ad, true);
		}
	}
#ifndef MP_SOUND_PLAYER
	else {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE);
	}
#endif

#ifdef MP_ENABLE_INOTIFY
	_mp_add_inofity_refresh_watch(ad);
#endif
	MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYER);
	if (view) {
		mp_view_update_options((MpView_t *)view);
	}

}

#ifdef MP_FEATURE_AVRCP_13
void  _mp_app_privacy_popup_resp_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_popup_destroy(ad);

	int response = (int)event_info;
	if (response == MP_POPUP_YES) {
		//show settings
		mp_setting_privacy_launch();
	}
}
#endif



static void mp_app_now_playing_id_changed_cb(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK(ad);

	int playing_pid = mp_setting_get_nowplaying_id();
	ERROR_TRACE("wishjox playing_pid:%d", playing_pid);
#ifndef MP_SOUND_PLAYER
	if (!playing_pid) {
		if (ad->player_state == PLAY_STATE_PAUSED) {
			DEBUG_TRACE("sound-player terminated.. show minicontroller");

			if (!ad->win_minicon) {
				mp_minicontroller_create(ad);
			}
			if (ad->win_minicon) {
				mp_minicontroller_show(ad);
			}
#ifdef MP_FEATURE_LOCKSCREEN
			if (!ad->win_lockmini) {
				mp_lockscreenmini_create(ad);
			}
			if (ad->win_lockmini) {
				mp_lockscreenmini_show(ad);
			}
#endif
			mp_setting_save_playing_info(ad);
		}
	} else
#endif
		if (playing_pid != getpid()) {
			DEBUG_TRACE("other player activated : [pid:%d]", playing_pid);
			if (ad->player_state == PLAY_STATE_PLAYING) {
				ad->paused_by_other_player = TRUE;
				mp_play_control_play_pause(ad, false);
			}
			mp_minicontroller_destroy(ad);
#ifdef MP_FEATURE_LOCKSCREEN
			mp_lockscreenmini_destroy(ad);
#endif
		}

}

bool mp_app_get_supported_storages_callback(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	if (type == STORAGE_TYPE_EXTERNAL) {
		struct appdata *ad = (struct appdata *)userData;
		ad->externalStorageId = storageId;
		return false;
	}
	return true;
}

bool
mp_app_noti_init(void *data)
{
	startfunc;
	int retcode = -1;
	struct appdata *ad = data;
	int error = storage_foreach_device_supported(mp_app_get_supported_storages_callback, ad);
	if (error == STORAGE_ERROR_NONE) {
		storage_state_e state;
		storage_get_state(ad->externalStorageId, &state);
	}

	bool res = TRUE;
	retcode = system_settings_set_changed_cb(SYSTEM_SETTINGS_KEY_SOUND_SILENT_MODE, _mp_app_system_settings_changed_cb, ad);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ERROR_TRACE("Fail to register KEY_MUSIC_MENU_CHANGE key callback [%d]", retcode);
		res = FALSE;
	}
	char *path = app_get_data_path();
	char now_playing_id[1024] = {0};

	snprintf(now_playing_id, 1024, "%s%s", path, MP_NOW_PLAYING_ID_INI);
	free(path);
	ecore_file_monitor_add(now_playing_id, mp_app_now_playing_id_changed_cb, ad);

	if (storage_set_state_changed_cb(ad->externalStorageId, _mp_app_storage_state_changed_cb, ad) < 0) {
		ERROR_TRACE("Fail to register storage state changed callback");
		res = FALSE;
	}
#ifdef MP_FEATURE_PERSONAL_PAGE
	if (preference_set_changed_cb(KEY_MP_PERSONAL_PAGE, _mp_app_personal_page_changed_cb, ad) < 0) {
		ERROR_TRACE("Fail to register KEY_MP_PERSONAL_PAGE key callback");
		res = FALSE;
	}
#endif

	gNotiPipe = ecore_pipe_add(_mp_app_noti_pipe_handler, ad);
#ifdef MP_FEATURE_AVRCP_13
	int error = mp_avrcp_target_initialize();
	if (!error) {
		if (ad->current_track_info) {
			mp_track_info_t *info = ad->current_track_info;
			mp_avrcp_noti_track(info->title, info->artist, info->album, info->genre, info->duration);
		}
		error = _mp_app_set_avrcp_mode(ad);
	}

	if (error == MP_AVRCP_ERROR_PERMISSION_DENIED) {
		Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, NULL, _mp_app_privacy_popup_resp_cb, ad);
		mp_popup_desc_set(popup, GET_STR(STR_MP_THE_APP_CONTROL_IS_DISABLED));
		mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_CANCEL, MP_POPUP_NO);
		mp_popup_button_set(popup, MP_POPUP_BTN_2, STR_MP_SETTINGS, MP_POPUP_YES);
		evas_object_show(popup);
	}

	mp_avrcp_set_mode_change_cb(_mp_app_avrcp_connection_state_changed_cb, _mp_app_avrcp_shuffle_changed_cb,
	                            _mp_app_avrcp_repeat_changed_cb, _mp_app_avrcp_eq_changed_cb, NULL);
#endif
	//Replaced for _prod dependency start
	WARN_TRACE("Enter sound_manager_set_available_route_changed_cb");
	int ret = sound_manager_set_device_connected_cb(SOUND_DEVICE_ALL_MASK, _mp_add_available_route_changed_cb, ad);

	if (ret != SOUND_MANAGER_ERROR_NONE) {
		ERROR_TRACE("sound_manager_set_available_route_changed_cb().. [0x%x]", ret);
		res = FALSE;
	}
	WARN_TRACE("Leave sound_manager_set_available_route_changed_cb");

	WARN_TRACE("Enter sound_manager_set_active_device_changed_cb");
	ret = sound_manager_set_device_information_changed_cb(SOUND_DEVICE_ALL_MASK, _mp_app_active_device_chaged_cb, ad);

	if (ret != SOUND_MANAGER_ERROR_NONE) {
		ERROR_TRACE("sound_manager_set_active_device_changed_cb().. [0x%x]", ret);
		res = FALSE;
	}
	WARN_TRACE("Leave sound_manager_set_active_device_changed_cb");


#ifdef MP_FEATURE_VOICE_CONTROL
	mp_voice_ctrl_mgr_initialize((int)ad->xwin);
	mp_voice_ctrl_mgr_set_action_callback(_mp_app_voice_control_action_cb, ad);
	if (!ad->is_focus_out) {
		mp_voice_ctrl_mgr_start_listening();
	}
#endif

	ret = mp_media_info_set_db_update_cb(_mp_app_db_update_cb, NULL);
	if (ret) {
		ERROR_TRACE("Unable to set db update cb [0x%x]", ret);
	}

	return res;
}

bool
mp_app_noti_ignore(void *data)
{
	struct appdata *ad = (struct appdata *)data;
	int retcode = -1;
	// TEMP_BLOCK
	//sound_manager_unset_available_route_changed_cb();
	//sound_manager_unset_active_device_changed_cb();

	mp_media_info_unset_db_update_cb();
	retcode = system_settings_unset_changed_cb(SYSTEM_SETTINGS_KEY_SOUND_SILENT_MODE);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ERROR_TRACE("Error when ignore callback [%d]", retcode);
		return FALSE;
	}

	if (storage_unset_state_changed_cb(ad->externalStorageId, _mp_app_storage_state_changed_cb) != STORAGE_ERROR_NONE) {
		ERROR_TRACE("Error when ignore callback");
		return FALSE;
	}
	//mp_file_monitor_del(ECORE_FILE_MONITOR_TYPE_INOTIFY);

#ifdef MP_FEATURE_AVRCP_13
	mp_avrcp_target_finalize();
#endif

	if (gNotiPipe) {
		ecore_pipe_del(gNotiPipe);
		gNotiPipe = NULL;
	}

#ifdef MP_FEATURE_VOICE_CONTROL
	mp_voice_ctrl_mgr_set_action_callback(NULL, NULL);
	mp_voice_ctrl_mgr_finalize();
#endif

	mp_media_info_unset_db_update_cb();

	return TRUE;
}

#ifdef MP_FEATURE_LANDSCAPE
int
mp_app_rotate(app_device_orientation_e mode, void *data)
{
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK_VAL(ad, 0);

	int angle;
	DEBUG_TRACE("Enum Rotation  is %d", mode);
	DEBUG_TRACE("Rotation b is %d", elm_win_rotation_get(ad->win_main));

	switch (mode) {
	case APP_DEVICE_ORIENTATION_270:
		angle = -90;
		break;

	case APP_DEVICE_ORIENTATION_90:
		angle = 90;
		break;

	case APP_DEVICE_ORIENTATION_180:
		angle = 180;
		break;

	case APP_DEVICE_ORIENTATION_0:
	default:
		angle = 0;
		break;
	}
	DEBUG_TRACE("Angle  Rotation  is %d", angle);

	if (angle == ad->win_angle) {
		WARN_TRACE("Not rotated.. skip!!");
		return 0;
	}

	MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
	if (!mp_view_is_rotate_available(top_view)) {
		WARN_TRACE("top view is not rotatable");
		return 0;
	}
	ad->win_angle = angle;

	mp_view_set_nowplaying(top_view);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_VIEW_ROTATE);

	return 0;
}
#endif

Eina_Bool
mp_app_key_down_cb(void *data, int type, void *event)
{
	struct appdata *ad = data;
	MP_CHECK_VAL(ad, ECORE_CALLBACK_PASS_ON);

	Ecore_Event_Key *key = event;
	MP_CHECK_VAL(key, ECORE_CALLBACK_PASS_ON);
	EVENT_TRACE("%s", key->keyname);

	if (!g_strcmp0(key->keyname, "XF86AudioRaiseVolume")) {
		mp_volume_key_event_send(MP_VOLUME_KEY_UP, false);
	} else if (!g_strcmp0(key->keyname, "XF86AudioLowerVolume")) {
		mp_volume_key_event_send(MP_VOLUME_KEY_DOWN, false);
	} else if (!g_strcmp0(key->keyname, "XF86AudioMute")) {
		mp_volume_key_event_send(MP_VOLUME_KEY_MUTE, false);
	} else if (!g_strcmp0(key->keyname, "XF86AudioMedia")) {
		ad->press_time = key->timestamp;
	} else if (!g_strcmp0(key->keyname, "Down")) {
		DEBUG_TRACE("focused object is %s", elm_object_widget_type_get(elm_object_focused_object_get(ad->win_main)));
	} else if (!g_strcmp0(key->keyname, "Up")) {
		DEBUG_TRACE("focused object is %s", elm_object_widget_type_get(elm_object_focused_object_get(ad->win_main)));
	} else if (!g_strcmp0(key->keyname, "XF86AudioPrev")) {
		mp_play_control_prev();
	} else if (!g_strcmp0(key->keyname, "XF86AudioNext")) {
		mp_play_control_next();
	}

	return ECORE_CALLBACK_PASS_ON;
}

Eina_Bool
mp_app_key_up_cb(void *data, int type, void *event)
{
	struct appdata *ad = data;
	MP_CHECK_VAL(ad, ECORE_CALLBACK_PASS_ON);

	Ecore_Event_Key *key = event;
	MP_CHECK_VAL(key, ECORE_CALLBACK_PASS_ON);
	EVENT_TRACE("%s", key->keyname);

	if (!g_strcmp0(key->keyname, "XF86AudioRaiseVolume")) {
		mp_volume_key_event_send(MP_VOLUME_KEY_UP, true);
	} else if (!g_strcmp0(key->keyname, "XF86AudioLowerVolume")) {
		mp_volume_key_event_send(MP_VOLUME_KEY_DOWN, true);
	} else if (!g_strcmp0(key->keyname, "XF86AudioMute")) {
		mp_volume_key_event_send(MP_VOLUME_KEY_MUTE, true);
	} else if (!g_strcmp0(key->keyname, "XF86AudioMedia")) {
		if (ad->ear_key_press_cnt > 3) {
			DEBUG_TRACE("pressed more than 3times");
			return ECORE_CALLBACK_PASS_ON;
		}
		mp_ecore_timer_del(ad->ear_key_timer);
		if (key->timestamp - ad->press_time > 500) {
			DEBUG_TRACE("long pressed");
			app_control_h app_control = NULL;
			app_control_create(&app_control);
			app_control_set_app_id(app_control, "com.samsung.svoice");
			app_control_add_extra_data(app_control, "domain", "earjack");
			app_control_send_launch_request(app_control, NULL, NULL);
			app_control_destroy(app_control);
			ad->ear_key_press_cnt = 0;
		} else {
			ad->ear_key_timer = ecore_timer_add(0.5, _mp_app_ear_key_timer_cb, ad);
			ad->ear_key_press_cnt++;
		}
	}
#ifndef MP_SOUND_PLAYER
	else if (!g_strcmp0(key->keyname, "XF86Search")) {
		mp_common_create_search_view_cb(NULL, NULL, NULL);
	}
#endif

	return ECORE_CALLBACK_PASS_ON;
}


Eina_Bool
mp_app_mouse_event_cb(void *data, int type, void *event)
{
	struct appdata *ad = data;

	//static unsigned int buttons = 0;

	if (type == ECORE_EVENT_MOUSE_BUTTON_DOWN) {
		Ecore_Event_Mouse_Button *ev = event;
		if (!ad->mouse.downed) {
			ad->mouse.downed = TRUE;
			ad->mouse.sx = ev->root.x;
			ad->mouse.sy = ev->root.y;
			//	buttons = ev->buttons;
		}
	} else if (type == ECORE_EVENT_MOUSE_BUTTON_UP) {
		ad->mouse.sx = 0;
		ad->mouse.sy = 0;
		ad->mouse.downed = FALSE;
		ad->mouse.moving = FALSE;
	} else if (type == ECORE_EVENT_MOUSE_MOVE) {
#ifdef MP_FEATURE_APP_IN_APP
		Ecore_Event_Mouse_Move *ev = event;
		if (ad->mini_player_mode && ad->mouse.downed && !ad->mouse.moving) {
			double l = sqrt(pow((float)(ad->mouse.sx - ev->root.x), 2) + pow((float)(ad->mouse.sy - ev->root.y), 2));
			if (l >= 30.0f) {
				int x, y;
				ecore_x_pointer_last_xy_get(&x, &y);
				ecore_x_mouse_up_send(ad->xwin, x, y, buttons);
				ecore_x_pointer_ungrab();
				//mp_mini_player_window_drag_start(ad, x, y, buttons);

				ad->mouse.moving = TRUE;
			}
		}
#endif
	}

	return ECORE_CALLBACK_PASS_ON;
}

void mp_exit_job_cb(void *data)
{
	struct appdata *ad = data;
	ad->exit_job = NULL;
	elm_exit();
}

void
mp_app_exit(void *data)
{
	struct appdata *ad = data;
	mp_retm_if(ad == NULL, "appdata is NULL");
	DEBUG_TRACE("player_state [%d]", ad->player_state);

	if (!ad->exit_job) {
		ad->exit_job = ecore_job_add(mp_exit_job_cb, ad);
	}
}

#define CTR_EDJ_SIG_SRC "ctrl_edj"

static void
_mp_app_create_default_playing_list(struct appdata *ad, int index)
{
	startfunc;
	MP_CHECK(ad);

	int count;
	mp_media_list_h all = NULL;

	mp_common_create_playlist_mgr();
	MP_CHECK(ad->playlist_mgr);
	mp_playlist_mgr_clear(ad->playlist_mgr);
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	mp_media_info_list_create(&all, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, count);
	mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, all, count, index, NULL);
	mp_media_info_list_destroy(all);
}

void
_mp_app_media_key_event_cb(media_key_e key, media_key_event_e event, void *user_data)
{
	struct appdata *ad = (struct appdata *)user_data;
	MP_CHECK(ad);

	EVENT_TRACE("key [%d], event [%d]", key, event);
	bool released = false;
	if (event == MEDIA_KEY_STATUS_RELEASED) {
		released = true;
	}

	if (event == MEDIA_KEY_STATUS_UNKNOWN) {
		mp_debug("unknown key status");
		return;
	}

	switch (key) {
	case MEDIA_KEY_PLAY:
		if (released) {
			if (ad->player_state != PLAY_STATE_PLAYING) {
				if (ad->player_state == PLAY_STATE_PAUSED) {
					mp_play_control_resume_via_media_key(ad);    // workaround for Audi car-kit
				} else {
					if (ad->playlist_mgr == NULL || mp_playlist_mgr_count(ad->playlist_mgr) == 0) {
						_mp_app_create_default_playing_list(ad, 0);
					}
					mp_play_control_play_pause(ad, true);
				}
			}
		}
		break;
	case MEDIA_KEY_PAUSE:
		if (released) {
			if (ad->player_state == PLAY_STATE_PLAYING) {
				mp_play_control_play_pause(ad, false);
			}
		}
		break;
	case MEDIA_KEY_PLAYPAUSE:
		if (released) {
			if (ad->player_state == PLAY_STATE_PLAYING) {
				mp_play_control_play_pause(ad, false);
			} else if (ad->player_state == PLAY_STATE_PAUSED) {
				mp_play_control_resume_via_media_key(ad);    // workaround for Audi car-kit
			} else {
				if (ad->playlist_mgr == NULL || mp_playlist_mgr_count(ad->playlist_mgr) == 0) {
					_mp_app_create_default_playing_list(ad, 0);
				}
				mp_play_control_play_pause(ad, true);
			}
		}
		break;
	case MEDIA_KEY_PREVIOUS:
		DEBUG_TRACE("key pressed is previous");
		break;
	case MEDIA_KEY_NEXT:
		DEBUG_TRACE("key pressed is next");
		break;
	case MEDIA_KEY_REWIND:
		mp_play_control_rew(!released, true, true);
		break;

	case MEDIA_KEY_FASTFORWARD:
		mp_play_control_ff(!released, true, true);
		break;

	case MEDIA_KEY_STOP:
		if (ad->player_state == PLAY_STATE_PLAYING) {
			mp_play_control_play_pause(ad, false);
		}

		mp_player_mgr_set_position(0, NULL, NULL);

		ad->music_pos = 0;
		MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYER);
		if (view) {
			mp_player_view_update_progressbar(view);
		}

		break;
	default:
		mp_debug("Undefined key");
		break;
	}
}

bool
mp_app_grab_mm_keys(struct appdata *ad)
{
	WARN_TRACE("");
	Eina_Bool error = EINA_FALSE;
	error = eext_win_keygrab_set(ad->win_main, "XF86AudioMedia");
	if (error != EINA_TRUE) {
		ERROR_TRACE("Keygrab Failed");
	}
	int err = media_key_reserve(_mp_app_media_key_event_cb, ad);
	if (err != MEDIA_KEY_ERROR_NONE) {
		mp_error("media_key_reserve().. [0x%x]", err);
		return false;
	}

	return true;
}

void
mp_app_ungrab_mm_keys(struct appdata *ad)
{
	WARN_TRACE("");
	media_key_release();
	Eina_Bool error = EINA_FALSE;
	error = eext_win_keygrab_unset(ad->win_main, "XF86AudioMedia");
	if (error != EINA_TRUE) {
		ERROR_TRACE("Keygrab Failed");
	}

	mp_ecore_timer_del(ad->ear_key_timer);
}

#ifdef MP_FEATURE_AUTO_OFF
Eina_Bool
mp_app_auto_off_timer_expired_cb(void *data)
{
	struct appdata *ad = data;
	MP_CHECK_VAL(ad, ECORE_CALLBACK_CANCEL);

	mp_debug("#### auto off ####");

	ad->auto_off_timer = NULL;
	mp_setting_reset_auto_off_time();

	if (ad->player_state == PLAY_STATE_PLAYING) {
		mp_app_exit(ad);
	} else {
		mp_debug("auto off expired but not playing");
		MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
		if (top_view && top_view == mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_SETTING)) {
			mp_view_update(top_view);
		}
	}

	return ECORE_CALLBACK_DONE;
}

void
mp_app_auto_off_changed_cb(int min, void *data)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	mp_ecore_timer_del(ad->auto_off_timer);
	EVENT_TRACE("auto off time set [%d]", min);

	if (min <= 0) {
		mp_debug("disable auto off");
		return;
	}

	double timeout = min * 60;
	ad->auto_off_timer = ecore_timer_add(timeout, mp_app_auto_off_timer_expired_cb, ad);
}
#endif
#ifdef MP_FEATURE_PLAY_SPEED
void
mp_app_play_speed_changed_cb(double speed, void *data)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	EVENT_TRACE("playspeed: %f", speed);
	mp_player_mgr_set_play_speed(speed);
}
#endif

