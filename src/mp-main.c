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
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-setting-ctrl.h"
#include "mp-item.h"
#include "mp-player-control.h"
#include "mp-http-mgr.h"
#include "mp-playlist-mgr.h"
#include "mp-ug-launch.h"
#include "mp-popup.h"
/* TEMP_BLOCK */
/* #include "csc-feature.h" */
#include "mp-all-view.h"
#include <message_port.h>
#include "mp-player-view.h"

#include <signal.h>
#include <glib.h>
#include <glib-object.h>
#include "mp-player-mgr.h"
#include "mp-player-debug.h"
/* TEMP_BLOCK */
/* #include <power.h> */
#include <device/display.h>
#include <device/callback.h>
#include "mp-minicontroller.h"
#include "mp-lockscreenmini.h"
#include "mp-app.h"
#include "mp-play.h"
#include "mp-volume.h"
#include "mp-common-defs.h"
#ifdef MP_WATCH_DOG
#include "mp-watch-dog.h"
#endif

#ifndef MP_SOUND_PLAYER
#include "mp-common.h"
#endif
#include "mp-view-mgr.h"

#ifdef MP_FEATURE_APP_IN_APP
#include "mp-mini-player.h"
#endif

#ifdef MP_3D_FEATURE
#include "dali-music.h"
#endif

#ifdef MP_FEATURE_AVRCP_13
#include "mp-avrcp.h"
#endif

#ifdef MP_FEATURE_CONTEXT_ENGINE
#include "mp-context.h"
#endif

#ifdef MP_FEATURE_VOICE_CONTROL
#include "mp-voice-control-mgr.h"
#endif

#include <system_settings.h>
/* #include <appcore-common.h> */

static char *test_uri;

/* request type */
#define MP_REQ_TYPE_FILES	"files"
#define MP_REQ_TYPE_USER_PLAYLIST	"user-playlist"
#define MP_REQ_TYPE_AUTO_PLAYLIST	"auto-playlist"
#define MP_REQ_TYPE_RECENTLY_PLAYED	"recently-played"
#define MP_REQ_TYPE_PLAY_NEXT		"play-next"
#define MP_REQ_TYPE_PLAY_PREV		"play-prev"

/* extra data key */
#define MP_EXTRA_KEY_PLAY_FILES "files"
#define MP_EXTRA_KEY_PLAY_FILE_DELIM ";"
#define MP_EXTRA_KEY_PLAY_FILE_COUNT "filecount"
#define MP_EXTRA_KEY_PLAY_PLAYLIST "playlist"

/* playlist name for auto playlist type */
#define MP_EXTRA_VAL_RECENTLY_ADDED	"Recently added"
#define MP_EXTRA_VAL_RECENTLY_PLAYED	"Recently played"
#define MP_EXTRA_VAL_MOST_PLAYED		"Most played"
#define MP_EXTRA_VAL_FAVORITE		"Favorite"

#define MP_LIVEBOX "livebox"
#define MP_LIVEBOX_URI "uri"
#ifdef MP_FEATURE_EXIT_ON_BACK
#define MP_EXIT_ON_BACK "ExitOnBack"
#endif

#ifdef MP_FEATURE_DESKTOP_MODE
#define ELM_PROFILE_DESKTOP		"desktop"
#endif

struct appdata *g_ad;
static bool g_normal_launched = false;

#ifdef MP_FEATURE_SPLIT_WINDOW
static int __is_relaunch = 0;
#endif

static bool _mp_main_init(struct appdata *ad);
int app_control_to_bundle(app_control_h, bundle **);
static void _mp_main_win_visibility_withdrawn_cb(void *data, Evas_Object *obj, void *event);
static void _mp_main_win_visibility_normal_cb(void *data, Evas_Object *obj, void *event);
static void _mp_main_win_focus_in_cb(void *data, Evas_Object *obj, void *event);
static void _mp_main_win_focus_out_cb(void *data, Evas_Object *obj, void *event);
#ifdef MP_FEATURE_PALM_TOUCH
static Eina_Bool _mp_main_win_hold_cb(void *data, int type, void *event);
#endif
static Eina_Bool _mp_main_app_init_idler_cb(void *data);
void mp_device_orientation_cb(app_device_orientation_e orientation, void *user_data);

static void
_mp_main_exit_cb(void *data, Evas_Object * obj, void *event_info)
{
	mp_evas_object_del(obj);

	mp_app_exit(data);
}

static bool
_mp_main_init(struct appdata *ad)
{
	ad->music_setting_change_flag = false;
	ad->paused_by_user = true;

	mp_media_info_connect();

	/* window focus in/out event */
	evas_object_smart_callback_add(ad->win_main, "focused", _mp_main_win_focus_in_cb, ad);
	evas_object_smart_callback_add(ad->win_main, "unfocused", _mp_main_win_focus_out_cb, ad);

#ifdef MP_FEATURE_PALM_TOUCH
//	ad->hold = ecore_event_handler_add (ECORE_X_EVENT_GESTURE_NOTIFY_HOLD, _mp_main_win_hold_cb, ad);
//
//	Ecore_Evas *ee;
//	ee = ecore_evas_ecore_evas_get(evas_object_evas_get(ad->win_main));
//	if (!ecore_x_gesture_events_select((Ecore_X_Window)ecore_evas_window_get(ee), ECORE_X_GESTURE_EVENT_MASK_HOLD))
//		ERROR_TRACE("Fail to add gesture event");
#endif
	return TRUE;
}

static bool
_mp_main_is_launching_available(struct appdata *ad)
{
	if (!ad) {
		return false;
	}

	if (ad->low_battery_status == APP_EVENT_LOW_BATTERY_POWER_OFF) {
		Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, ad, _mp_main_exit_cb, ad);
		elm_object_text_set(popup, GET_SYS_STR("IDS_COM_BODY_LOW_BATTERY"));
		mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_SK_OK", MP_POPUP_YES);
		mp_popup_timeout_set(popup, MP_POPUP_TIMEOUT);
		evas_object_show(ad->win_main);
		evas_object_show(popup);
		return false;
	}

	if (mp_check_mass_storage_mode()) {
		Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, ad, _mp_main_exit_cb, ad);
		elm_object_text_set(popup, GET_SYS_STR("IDS_COM_POP_UNABLE_TO_USE_DURING_MASS_STORAGE_MODE"));
		mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_SK_OK", MP_POPUP_YES);
		mp_popup_timeout_set(popup, MP_POPUP_TIMEOUT);
		evas_object_show(ad->win_main);
		evas_object_show(popup);
		return false;
	}
	return true;
}

static void _mp_main_win_visibility_withdrawn_cb(void *data, Evas_Object *obj, void *event)
{
	struct appdata *ad = (struct appdata *)data;
	mp_retm_if(ad == NULL, "ad is null");
#if 0
	Ecore_X_Event_Window_Visibility_Change* ev = (Ecore_X_Event_Window_Visibility_Change *)event;

	if (ev->win == ad->xwin) {
		main window
		if (ev->fully_obscured == 1) {
			ad->app_is_foreground = false;
			mp_player_mgr_vol_type_unset();
			mp_player_mgr_safety_volume_set(ad->app_is_foreground);
			mp_view_view_pause(mp_view_mgr_get_top_view(GET_VIEW_MGR));
		} else {

			ad->app_is_foreground = true;
			mp_player_mgr_vol_type_set();
			mp_player_mgr_safety_volume_set(ad->app_is_foreground);
			mp_view_view_resume(mp_view_mgr_get_top_view(GET_VIEW_MGR));
		}
		DEBUG_TRACE("ad->app_is_foreground: %d", ad->app_is_foreground);
	}
#endif
	ad->app_is_foreground = false;
	mp_player_mgr_vol_type_unset();
	mp_player_mgr_safety_volume_set(ad->app_is_foreground);
	mp_view_view_pause(mp_view_mgr_get_top_view(GET_VIEW_MGR));
	DEBUG_TRACE("ad->app_is_foreground: %d", ad->app_is_foreground);

	/* return ECORE_CALLBACK_PASS_ON; */
}

static void _mp_main_win_visibility_normal_cb(void *data, Evas_Object *obj, void *event)
{
	struct appdata *ad = (struct appdata *)data;
	mp_retm_if(ad == NULL, "ad is null");

	ad->app_is_foreground = true;
	mp_player_mgr_vol_type_set();
	mp_player_mgr_safety_volume_set(ad->app_is_foreground);
	mp_view_view_resume(mp_view_mgr_get_top_view(GET_VIEW_MGR));
	DEBUG_TRACE("ad->app_is_foreground: %d", ad->app_is_foreground);

	/* return ECORE_CALLBACK_PASS_ON; */
}

static void _mp_main_win_focus_in_cb(void *data, Evas_Object *obj, void *event)
{
	struct appdata *ad = (struct appdata *)data;
	mp_retm_if(ad == NULL, "ad is null");

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode) {
		WARN_TRACE("floating window");
		return ECORE_CALLBACK_PASS_ON;
	}
#endif
	/* TODO Know about the event */
	/*Ecore_X_Event_Window_Focus_In *ev = (Ecore_X_Event_Window_Focus_In *)event;
	if (ev->win != ad->xwin)
		return ECORE_CALLBACK_PASS_ON;*/
	/* TODO */
	ad->is_focus_out = false;
	/*if (ad->win_minicon && ad->b_minicontroller_show)
		mp_minicontroller_destroy(ad); */

	DEBUG_TRACE("ad->is_focus_out: %d", ad->is_focus_out);

	MpPlayerView_t *player_view = (MpPlayerView_t *)GET_PLAYER_VIEW;
	if (player_view) {
		mp_player_view_refresh(player_view);
	}

	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_WINDOW_FOCUS, true);

#ifdef MP_FEATURE_VOICE_CONTROL
	mp_voice_ctrl_mgr_start_listening();
#endif

	/* return ECORE_CALLBACK_PASS_ON; */
}

static void _show_minicontroller(struct appdata *ad)
{
	ERROR_TRACE("");
	int playing_pid = 0;
	playing_pid = mp_setting_get_nowplaying_id();
	if (playing_pid != -1) {
		if (playing_pid && playing_pid != getpid()) {
#ifdef MP_SOUND_PLAYER
			DEBUG_TRACE("Music-player is may playing track");
#else
			DEBUG_TRACE("Sound-player is may playing track");
#endif
			return;
		}
	}

	if (ad->player_state == PLAY_STATE_PAUSED || ad->player_state == PLAY_STATE_PLAYING) {
		if (!ad->win_minicon) {
			mp_minicontroller_create(ad);
		} else {
			mp_minicontroller_show(ad);
		}

#ifdef MP_FEATURE_LOCKSCREEN
		if (!ad->win_lockmini) {
			mp_lockscreenmini_create(ad);
		} else {
			mp_lockscreenmini_show(ad);
		}
#endif
	}
}

static void _mp_main_win_focus_out_cb(void *data, Evas_Object *obj, void *event)
{
	startfunc;
	struct appdata *ad = (struct appdata *)data;
	mp_retm_if(ad == NULL, "ad is null");

	ad->is_focus_out = true;
	DEBUG_TRACE("ad->is_focus_out: %d", ad->is_focus_out);

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode) {
		WARN_TRACE("floating window");
		return ECORE_CALLBACK_PASS_ON;
	}
#endif
	/* TODO know about the event */
	/*Ecore_X_Event_Window_Focus_Out *ev = (Ecore_X_Event_Window_Focus_Out *)event;
	if (ev->win != ad->xwin)
		return ECORE_CALLBACK_PASS_ON;*/

	/* Testing Code. If a track is getting played or paused,
	   the MiniController should be displayed as soon as the main window goes to back ground.
	   When again the Music ICon in Main menu is pressed, the mini controller will be hidden and
	   The Main Screen of the Music Application will be displayed. */
	_show_minicontroller(ad);

	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_WINDOW_FOCUS, false);

#ifdef MP_FEATURE_VOICE_CONTROL
	mp_voice_ctrl_mgr_stop_listening();
#endif

	/* return ECORE_CALLBACK_PASS_ON; */
}

/*static Eina_Bool _mp_main_client_message_cb(void *data, int type, void *event)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	Ecore_X_Event_Client_Message *ev =
		(Ecore_X_Event_Client_Message *) event;

	if (ev->message_type == ECORE_X_ATOM_E_ILLUME_ROTATE_ROOT_ANGLE) {
		ad->quickpanel_angle = ev->data.l[0];
		DEBUG_TRACE("ROTATION: %d", ad->quickpanel_angle);
		mp_minicontroller_rotate(ad, ad->quickpanel_angle);
	} else if (ev->message_type == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_STATE) {
		bool visible = false;
		if (ev->data.l[0] == ECORE_X_ATOM_E_ILLUME_QUICKPANEL_ON) {
			mp_debug("quickpanel show");
			visible = true;
		} else {
			mp_debug("quickpanel hide");
			visible = false;
		}
//		mp_minicontroller_visible_set(ad, visible);
		if (visible)
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_QUICKPANNEL_SHOW);
		else
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_QUICKPANNEL_HIDE);

#ifdef MP_SOUND_PLAYER
		if (!visible && ad->player_state == PLAY_STATE_NONE) {
			mp_app_exit(ad);
		}
#endif

	}

	return ECORE_CALLBACK_PASS_ON;
}*/
/* TEMP_BLOCK #if 0 */
#if 1 /* start */
static void __mp_main_lcd_state_changed_cb(device_callback_e type, void *state_data, void *user_data)
{
	struct appdata *ad = user_data;
	MP_CHECK(ad);
	display_state_e state = (display_state_e)state_data;
	DEBUG_TRACE("power_state: %d", state);

	if (state == DISPLAY_STATE_SCREEN_OFF) {
		ad->is_lcd_off = true;
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_LCD_OFF);
		mp_view_view_pause(mp_view_mgr_get_top_view(GET_VIEW_MGR));

		if (ad->duration_change_timer) {
			ecore_timer_freeze(ad->duration_change_timer);
		}

#ifdef MP_FEATURE_LOCKSCREEN
		mp_lockscreenmini_on_lcd_event(ad, false);
#endif

	} else {
		/*POWER_STATE_SCREEN_DIM or POWER_STATE_NORMAL*/
		if (ad->is_lcd_off) {
			ad->is_lcd_off = false;
			/* for refresh progressbar */
			ad->music_pos = mp_player_mgr_get_position() / 1000.0;
			ad->music_length = mp_player_mgr_get_duration() / 1000.0;

			mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_PLAYING_LIST);
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_LCD_ON);
			mp_view_update(GET_PLAYER_VIEW);
			mp_minicontroller_update(ad, true);

#ifdef MP_FEATURE_LOCKSCREEN
			mp_lockscreenmini_update(ad);
#endif

#ifdef MP_FEATURE_APP_IN_APP
			if (ad->mini_player_mode) {
				mp_mini_player_refresh(ad);
			}
#endif

			if (ad->duration_change_timer && mp_player_mgr_get_state() == PLAYER_STATE_PLAYING) {
				MP_TIMER_THAW(ad->duration_change_timer);
			}

#ifdef MP_FEATURE_LOCKSCREEN
			mp_lockscreenmini_on_lcd_event(ad, true);
#endif

		}
	}
}
#endif /* end */

static bool _parse_widget_event(bundle *b, bool *activate_window)
{
	startfunc;
	MP_CHECK_FALSE(b);
	MP_CHECK_FALSE(activate_window);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	char *value = NULL;
	bundle_get_str(b, MP_LB_EVENT_KEY, &value);
	if (value && strlen(value) > 0) {
		*activate_window = false;
		EVENT_TRACE("event: %s", value);

		if (mp_playlist_mgr_count(ad->playlist_mgr) <= 0) {
			mp_common_create_default_playlist();
		}


		bool prepare_by_init = !mp_player_mgr_is_active();

		if (!g_strcmp0(value, MP_LB_EVENT_NEXT_PRESSED)) {
			mp_play_control_ff(1, 0, 1);
		} else if (!g_strcmp0(value, MP_LB_EVENT_NEXT_RELEASED)) {
			mp_play_control_ff(0, 0, 1);
			if (prepare_by_init) {
				ad->prepare_by_init = true;
			}
		} else if (!g_strcmp0(value, MP_LB_EVENT_PREV_PRESSED)) {
			mp_play_control_rew(1, 0, 1);
		} else if (!g_strcmp0(value, MP_LB_EVENT_PREV_RELEASED)) {
			mp_play_control_rew(0, 0, 1);
			if (prepare_by_init) {
				ad->prepare_by_init = true;
			}
		} else if (!g_strcmp0(value, MP_LB_EVENT_PLAY_CLICKED)) {
			ad->freeze_indicator_icon = false;
			ad->music_pos = mp_player_mgr_get_position() / 1000.0;
			mp_play_control_play_pause(ad, true);
		} else if (!g_strcmp0(value, MP_LB_EVENT_PAUSE_CLICKED)) {
			ad->freeze_indicator_icon = false;
			ad->music_pos = mp_player_mgr_get_position() / 1000.0;
			mp_play_control_play_pause(ad, false);
		}
		return true;
	}

	value = NULL;
	bundle_get_str(b, MP_NOWPLAYING_LIST_INDEX, &value);
	if (value && strlen(value) > 0) {
		EVENT_TRACE("index: %s", value);

		*activate_window = false;
		int index = 0;
		if (value) {
			index = atoi(value);
		}

		mp_play_destory(ad);

		mp_common_create_playlist_mgr();
		mp_playlist_mgr_clear(ad->playlist_mgr);

		mp_playlist_mgr_lazy_append_with_file(ad->playlist_mgr, MP_GROUP_LIST_DATA, NULL, index);
		return true;
	}

	return false;
}

static void
_mp_main_message_port_cb(int local_port_id, const char *remote_app_id, const char *remote_port, bool trusted_remote_port, bundle *message, void *user_data)
{
	eventfunc;
	MP_CHECK(message);

	bool ret = false;
	bool active_window = false;

	ret = _parse_widget_event(message, &active_window);
	if (ret) {
		DEBUG_TRACE("message port done");
		if (active_window) {
			struct appdata * ad = mp_util_get_appdata();
			if (ad && ad->win_main) {
				elm_win_inwin_activate(ad->win_main);
			}
		}
	}
}

static void _mp_main_conformant_sip_on_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = data;
	MP_CHECK(ad);
	MP_CHECK(ad->view_manager);

	ad->sip_state = true;
	mp_view_mgr_post_event(ad->view_manager, MP_SIP_STATE_CHANGED);
}

static void _mp_main_conformant_sip_off_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = data;
	MP_CHECK(ad);
	MP_CHECK(ad->view_manager);

	ad->sip_state = false;
	mp_view_mgr_post_event(ad->view_manager, MP_SIP_STATE_CHANGED);
}

static Eina_Bool
_mp_main_app_init_idler_cb(void *data)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	/* mp_setting_init(ad); */

	mp_player_mgr_vol_type_set();
	mp_player_mgr_safety_volume_set(ad->app_is_foreground);
	if (!mp_app_noti_init(ad)) {
		ERROR_TRACE("Error when noti init");
	}
	ad->key_down = ecore_event_handler_add(ECORE_EVENT_KEY_DOWN, mp_app_key_down_cb, ad);
	ad->key_up = ecore_event_handler_add(ECORE_EVENT_KEY_UP, mp_app_key_up_cb, ad);
	/*ad->mouse_button_down = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_DOWN, mp_app_mouse_event_cb, ad);
	ad->mouse_button_up = ecore_event_handler_add(ECORE_EVENT_MOUSE_BUTTON_UP, mp_app_mouse_event_cb, ad);
	ad->mouse_move = ecore_event_handler_add(ECORE_EVENT_MOUSE_MOVE, mp_app_mouse_event_cb, ad);*/
	/* window visibility change event */
	/* for event->fully_obscured==1, event is withdrawn otherwise normal */
	evas_object_smart_callback_add(ad->win_main, "withdrawn", _mp_main_win_visibility_withdrawn_cb, ad);
	evas_object_smart_callback_add(ad->win_main, "normal", _mp_main_win_visibility_normal_cb, ad);
//	ad->client_msg = ecore_event_handler_add(ECORE_X_EVENT_CLIENT_MESSAGE, _mp_main_client_message_cb, ad);

	int port_id = message_port_register_local_port(MP_MESSAGE_PORT_LIVEBOX, _mp_main_message_port_cb, NULL);
	DEBUG_TRACE("message port id = %d", port_id);

#ifdef MP_ENABLE_INOTIFY
	mp_app_inotify_init(ad);
#endif

#ifdef MP_FEATURE_AUTO_OFF
	mp_setting_auto_off_set_callback(mp_app_auto_off_changed_cb, ad);
#endif

#ifdef MP_FEATURE_PLAY_SPEED
	mp_setting_set_play_speed_change_callback(mp_app_play_speed_changed_cb, ad);
#endif

	/* TEMP_BLOCK
	power_set_changed_cb(__mp_main_lcd_state_changed_cb, ad);*/
	device_add_callback(DEVICE_CALLBACK_DISPLAY_STATE, __mp_main_lcd_state_changed_cb, ad);

#ifdef MP_FEATURE_CONTEXT_ENGINE
	mp_context_log_connect();
#endif

#ifdef MP_FEATURE_MUSIC_VIEW
	mp_music_view_mgr_init();
#endif

#ifdef MP_FEATURE_CLOUD
	mp_cloud_create();
#endif

	if (!ad->playlist_mgr) {
		mp_common_create_playlist_mgr();

		if (ad->playlist_mgr && ad->current_track_info) {
			if (ad->current_track_info->uri) {
				mp_playlist_mgr_lazy_append_with_file(ad->playlist_mgr, MP_NOWPLAYING_LIST_DATA, ad->current_track_info->uri, -1);
			}
		}
	}

	if (ad->conformant) {
		evas_object_smart_callback_add(ad->conformant, "virtualkeypad,state,on", _mp_main_conformant_sip_on_cb, ad);
		evas_object_smart_callback_add(ad->conformant, "virtualkeypad,state,off", _mp_main_conformant_sip_off_cb, ad);
	}

#ifdef MP_WATCH_DOG
	mp_watch_dog_init();
#endif

	/*initialize session type */
	/*
		if (!mp_player_mgr_session_init())
		{
			ERROR_TRACE("Error when set session");
		}
	*/
	mp_volume_init(ad->win_main);
#ifdef MP_SOUND_PLAYER
	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, true);
#endif

	mp_http_mgr_create(ad);

	if (mp_view_get_nowplaying_show_flag(GET_ALL_VIEW) && ad->player_state == PLAY_STATE_NONE) {
		ad->paused_by_user = true;
		mp_play_new_file(ad, true);
		ad->prepare_by_init = true;
	}

	ad->app_init_idler = NULL;

	endfunc;
	return ECORE_CALLBACK_CANCEL;
}

static bool __mp_main_app_control_extra_data_cb(app_control_h app_control, const char *key, void *user_data)
{
	MP_CHECK_FALSE(app_control);
	char *value = NULL;
	app_control_get_extra_data(app_control, key, &value);
	SECURE_DEBUG("key: %s, value: %s", key, value);
	IF_FREE(value);

	return true;
}

static void _mp_main_parse_get_playlist(struct appdata *ad)
{
	mp_media_list_h media = NULL;
	int count = 0;
	int track_type = MP_TRACK_BY_PLAYED_TIME;

	mp_media_info_list_count(track_type, NULL, NULL, NULL, 0, &count);

	if (count == 0) {
		DEBUG_TRACE("No recently played music.. Play All tracks");
		track_type = MP_TRACK_ALL;
		mp_media_info_list_count(track_type, NULL, NULL, NULL, 0, &count);
	} else if (count > 100) {
		count = 100;
	}

	mp_media_info_list_create(&media, track_type, NULL, NULL, NULL, 0, 0, count);
	mp_common_create_playlist_mgr();
	mp_playlist_mgr_clear(ad->playlist_mgr);
	mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, media, count, 0, NULL);
	mp_media_info_list_destroy(media);
}

static int
_mp_main_parse_request_type(struct appdata *ad, app_control_h app_control, const char *request, bool *start_playback)
{
	int ret = 0;

	char *val = NULL;
	char **path_array = NULL;
	int count = 0;

	if (!g_strcmp0(request, MP_REQ_TYPE_FILES)) {
		char *str = val;
		char *save_ptr = NULL;
		char *token = NULL;
		int i = 0;

		mp_common_create_playlist_mgr();

		if (!app_control_get_extra_data_array(app_control, MP_EXTRA_KEY_PLAY_FILES, &path_array, &count)) {
			if (path_array == NULL) {
				return -1;
			}
			if (count == 0) {
				free(path_array);
				return -1;
			}

			mp_playlist_mgr_clear(ad->playlist_mgr);
			for (i = 0; i < count; i++) {
				token = path_array[i];
				mp_playlist_mgr_item_append(ad->playlist_mgr, token, NULL, NULL, NULL, MP_TRACK_URI);
			}
			mp_playlist_mgr_set_current(ad->playlist_mgr, mp_playlist_mgr_get_nth(ad->playlist_mgr, 0));
			*start_playback = true;

			free(path_array);
		} else if (!app_control_get_extra_data(app_control, MP_EXTRA_KEY_PLAY_FILES, &val)) {

			str = val;
			char *title = NULL;
			if (!app_control_get_extra_data(app_control, MP_EXTRA_KEY_PLAY_FILE_COUNT, &val)) {
				if (val) {
					count = atoi(val);
				}
			}
			IF_FREE(val);

			if (!app_control_get_extra_data(app_control, APP_CONTROL_DATA_TITLE, &title)) {
				DEBUG_TRACE("Get title error");
			}

			mp_playlist_mgr_clear(ad->playlist_mgr);
			for (i = 0; i < count; i++, str = NULL) {
				token = strtok_r(str, MP_EXTRA_KEY_PLAY_FILE_DELIM, &save_ptr);
				if (token == NULL) {
					break;
				}
				if (count == 1) {
					mp_playlist_mgr_item_append(ad->playlist_mgr, token, NULL, title, NULL, MP_TRACK_URI);
				} else {
					mp_playlist_mgr_item_append(ad->playlist_mgr, token, NULL, NULL, NULL, MP_TRACK_URI);
				}
			}
			mp_playlist_mgr_set_current(ad->playlist_mgr, mp_playlist_mgr_get_nth(ad->playlist_mgr, 0));
			*start_playback = true;
		}
		IF_FREE(val);
	} else if (!g_strcmp0(request, MP_REQ_TYPE_AUTO_PLAYLIST)) {
		if (!app_control_get_extra_data(app_control, MP_EXTRA_KEY_PLAY_PLAYLIST, &val)) {
			mp_media_list_h media = NULL;
			int count = 0;
			mp_track_type_e type = 0;
			if (!g_strcmp0(val, MP_EXTRA_VAL_RECENTLY_ADDED)) {
				type = MP_TRACK_BY_ADDED_TIME;
			} else if (!g_strcmp0(val, MP_EXTRA_VAL_RECENTLY_PLAYED)) {
				type = MP_TRACK_BY_PLAYED_TIME;
			} else if (!g_strcmp0(val, MP_EXTRA_VAL_MOST_PLAYED)) {
				type = MP_TRACK_BY_PLAYED_COUNT;
			} else if (!g_strcmp0(val, MP_EXTRA_VAL_FAVORITE)) {
				type = MP_TRACK_BY_FAVORITE;
			} else {
				WARN_TRACE("invalid playlist : %s", val);
				return -1;
			}

			mp_media_info_list_count(type, NULL, NULL, NULL, 0, &count);
			if (count > 100) {
				count = 100;
			}
			mp_media_info_list_create(&media, type, NULL, NULL, NULL, 0, 0, count);
			mp_common_create_playlist_mgr();
			mp_playlist_mgr_clear(ad->playlist_mgr);
			mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, media, count, 0, NULL);
			mp_media_info_list_destroy(media);
			*start_playback = true;
		}
		IF_FREE(val);
	} else if (!g_strcmp0(request, MP_REQ_TYPE_USER_PLAYLIST)) {
		if (!app_control_get_extra_data(app_control, MP_EXTRA_KEY_PLAY_PLAYLIST, &val)) {
			mp_media_list_h media = NULL;
			int count = 0;
			int id = 0;

			mp_media_info_playlist_get_id_by_name(val, &id);
			mp_media_info_list_count(MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, id, &count);
			if (count > 100) {
				count = 100;
			}
			mp_media_info_list_create(&media, MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, id, 0, count);
			mp_common_create_playlist_mgr();
			mp_playlist_mgr_clear(ad->playlist_mgr);
			mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, media, count, 0, NULL);
			mp_media_info_list_destroy(media);
			*start_playback = true;
		}
		IF_FREE(val);
	} else if (!g_strcmp0(request, MP_REQ_TYPE_PLAY_NEXT)) {
		if (ad->playlist_mgr == NULL) {
			_mp_main_parse_get_playlist(ad);
		}
		if (mp_playlist_mgr_count(ad->playlist_mgr)) {
			mp_play_next_file(ad, true);
			/* when player view is popped, we need to re-create it */
			*start_playback = true;
		} else {
			ret = -1;
		}
	} else if (!g_strcmp0(request, MP_REQ_TYPE_PLAY_PREV)) {
		if (ad->playlist_mgr == NULL) {
			_mp_main_parse_get_playlist(ad);
		}
		if (mp_playlist_mgr_count(ad->playlist_mgr)) {
			mp_play_prev_file(ad);
			/* when player view is popped, we need to re-create it */
			*start_playback = true;
		} else {
			ret = -1;
		}
	} else if (!g_strcmp0(request, MP_REQ_TYPE_RECENTLY_PLAYED)) {
		_mp_main_parse_get_playlist(ad);
		*start_playback = true;
	} else if (!g_strcmp0(request, "livebox")) {
	} else {
		WARN_TRACE("Inavlid request: %s", request);
		ret = -1;
	}

	return ret;
}

/*
#ifdef MP_FEATURE_EXIT_ON_BACK
static Eina_Bool
_mp_main_caller_win_destroy_cb(void *data, int type, void *event)
{
	startfunc;
	Ecore_X_Event_Window_Hide *ev;
	struct appdata *ad = data;
	MP_CHECK_VAL(ad, ECORE_CALLBACK_RENEW);

	ev = event;
	if (ev == NULL) {
		DEBUG_TRACE("ev is NULL");
		return ECORE_CALLBACK_RENEW;
	}
	 DEBUG_TRACE("win: %d, caller_win: %d", ev->win ,ad->caller_win_id);
	if (ev->win == ad->caller_win_id) {
		elm_exit();
	}

	return ECORE_CALLBACK_RENEW;
}
#endif
*/

#ifdef MP_SOUND_PLAYER
static void
_mp_main_set_transient(struct appdata *ad, app_control_h app_control)
{
#ifdef MP_FEATURE_EXIT_ON_BACK
	if (ad->caller_win_id) {
		DEBUG_TRACE("unset transient for win: 0x%x", ad->caller_win_id);
//		ecore_x_icccm_transient_for_unset(elm_win_xwindow_get(ad->win_main));
//		ecore_event_handler_del(ad->callerWinEventHandler);
		ad->caller_win_id = 0;
		ad->callerWinEventHandler = NULL;
	}

#if 0
	unsigned int id = 0;
	app_control_get_window(app_control, &id);
	app_control_get_extra_data(app_control, MP_EXIT_ON_BACK, &value);

	if (id && value && !strcasecmp(value, "true")) {
//		ecore_x_icccm_transient_for_set(elm_win_xwindow_get(ad->win_main), id);
//		ecore_x_window_client_manage(id);

		/*ad->callerWinEventHandler =
			ecore_event_handler_add(ECORE_X_EVENT_WINDOW_DESTROY,
						_mp_main_caller_win_destroy_cb, ad);
		ad->caller_win_id = id;*/
	}
	IF_FREE(value);
#endif

#endif
}
#endif

#ifndef MP_SOUND_PLAYER
static void
_mp_main_parse_playback_control(struct appdata *ad, app_control_h app_control)
{
	MP_CHECK(ad);
	MP_CHECK(app_control);
	MP_CHECK(ad->playlist_mgr);

	char *control = NULL;
	app_control_get_extra_data(app_control, "control", &control);
	if (control) {
		WARN_TRACE("control = %s", control);
		mp_plst_item *current = NULL;
		if (!g_strcmp0(control, "PREV")) {
			current = mp_playlist_mgr_get_prev(ad->playlist_mgr);
		} else if (!g_strcmp0(control, "NEXT")) {
			current = mp_playlist_mgr_get_next(ad->playlist_mgr, true, false);
		}

		if (current) {
			mp_playlist_mgr_set_current(ad->playlist_mgr, current);
		}
	}
	IF_FREE(control);
}

static void
_mp_main_create_default_playing_list(struct appdata *ad, int index)
{
	startfunc;

	int count;
	mp_media_list_h all = NULL;
	char *path = NULL;

	char *last_played_path = NULL;
	mp_setting_get_now_playing_path_from_file(&last_played_path);
	SECURE_DEBUG("last played path = %s", last_played_path);

	mp_common_create_playlist_mgr();
	mp_playlist_mgr_clear(ad->playlist_mgr);

	mp_playlist_mgr_lazy_append_with_file(ad->playlist_mgr, MP_NOWPLAYING_LIST_DATA, last_played_path, -1);

	if (mp_playlist_mgr_count(ad->playlist_mgr) == 0) {
		mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
		mp_media_info_list_create(&all, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, count);
		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, all, count, index, path);
		mp_media_info_list_destroy(all);
	}
	IF_FREE(last_played_path);
}
#endif

static int
_mp_main_parse_livebox_event(app_control_h app_control, bool *activate_window, bool *start_playback)
{
#ifndef MP_SOUND_PLAYER
	startfunc;
	char *value = NULL;

	struct appdata *ad = mp_util_get_appdata();

	if (!app_control_get_extra_data(app_control, MP_LB_EVENT_KEY, &value)) {
		*activate_window = false;
		EVENT_TRACE("event: %s", value);
		if (mp_playlist_mgr_count(ad->playlist_mgr)) {
			*start_playback = false;
			if (!g_strcmp0(value, MP_LB_EVENT_NEXT_PRESSED)) {
				mp_play_control_ff(1, 0, 1);
			} else if (!g_strcmp0(value, MP_LB_EVENT_NEXT_RELEASED)) {
				mp_play_control_ff(0, 0, 1);
			} else if (!g_strcmp0(value, MP_LB_EVENT_PREV_PRESSED)) {
				mp_play_control_rew(1, 0, 1);
			} else if (!g_strcmp0(value, MP_LB_EVENT_PREV_RELEASED)) {
				mp_play_control_rew(0, 0, 1);
			} else if (!g_strcmp0(value, MP_LB_EVENT_PLAY_CLICKED)) {
				ad->music_pos = mp_player_mgr_get_position() / 1000.0;
				mp_play_control_play_pause(ad, true);
			} else if (!g_strcmp0(value, MP_LB_EVENT_PAUSE_CLICKED)) {
				ad->music_pos = mp_player_mgr_get_position() / 1000.0;
				mp_play_control_play_pause(ad, false);
			} else if (!g_strcmp0(value, MP_LB_EVENT_SHUFFLE_ON_CLICKED)) {
				mp_play_control_shuffle_set(ad, FALSE);
			} else if (!g_strcmp0(value, MP_LB_EVENT_SHUFFLE_OFF_CLICKED)) {
				mp_play_control_shuffle_set(ad, TRUE);
			} else if (!g_strcmp0(value, MP_LB_EVENT_REPEAT_ALL_CLICKED)) {
				mp_setting_set_repeat_state(MP_SETTING_REP_1);
				mp_playlist_mgr_set_repeat(ad->playlist_mgr, MP_PLST_REPEAT_ONE);
			} else if (!g_strcmp0(value, MP_LB_EVENT_REPEAT_1_CLICKED)) {
				mp_setting_set_repeat_state(MP_SETTING_REP_NON);
				mp_playlist_mgr_set_repeat(ad->playlist_mgr, MP_PLST_REPEAT_NONE);
			} else if (!g_strcmp0(value, MP_LB_EVENT_REPEAT_A_CLICKED)) {
				mp_setting_set_repeat_state(MP_SETTING_REP_ALL);
				mp_playlist_mgr_set_repeat(ad->playlist_mgr, MP_PLST_REPEAT_ALL);
			}
			IF_FREE(value);

			return TRUE;
		} else {
			_mp_main_create_default_playing_list(ad, 0);
			if (g_strcmp0(value, MP_LB_EVENT_NEXT_PRESSED) &&
			        g_strcmp0(value, MP_LB_EVENT_NEXT_RELEASED) &&
			        g_strcmp0(value, MP_LB_EVENT_PREV_PRESSED) &&
			        g_strcmp0(value, MP_LB_EVENT_PREV_RELEASED) &&
			        g_strcmp0(value, MP_LB_EVENT_PLAY_CLICKED) &&
			        g_strcmp0(value, MP_LB_EVENT_PAUSE_CLICKED)) {
				*start_playback = false;
			} else {
				*start_playback = true;
				if (!g_strcmp0(value, MP_LB_EVENT_NEXT_PRESSED)) {
					mp_play_control_ff(1, 0, 1);
				} else if (!g_strcmp0(value, MP_LB_EVENT_NEXT_RELEASED)) {
					mp_play_control_ff(0, 0, 1);
				} else if (!g_strcmp0(value, MP_LB_EVENT_PREV_PRESSED)) {
					mp_play_control_rew(1, 0, 1);
				} else if (!g_strcmp0(value, MP_LB_EVENT_PREV_RELEASED)) {
					mp_play_control_rew(0, 0, 1);
				}
			}
		}

	} else if (!app_control_get_extra_data(app_control, MP_NOWPLAYING_LIST_INDEX, &value)) {
		*activate_window = false;
		int index = 0;
		if (value) {
			index = atoi(value);
		}
		IF_FREE(value);

		app_control_get_extra_data(app_control, MP_REFRESH_PLAYLIST, &value);

		mp_play_destory(ad);

		mp_common_create_playlist_mgr();
		mp_playlist_mgr_clear(ad->playlist_mgr);

		mp_playlist_mgr_lazy_append_with_file(ad->playlist_mgr, MP_GROUP_LIST_DATA, NULL, index);
		*start_playback = true;
		return true;
	}
	int ret = 0;
	bundle *b = NULL;
	DEBUG_TRACE("ready to call _parse_widget_event");
	int err = app_control_to_bundle(app_control, &b);
	if (err) {
		ERROR_TRACE("app_control_to_bundle() .. [0x%x]", err);
	}

	if (b) {
		DEBUG_TRACE("calling _parse_widget_event");
		ret = _parse_widget_event(b, activate_window);
	}

	return ret;

	IF_FREE(value);
#endif
	return 0;
}

static bool _mp_main_check_servic_type(struct appdata *ad, app_control_h app_control)
{
	startfunc;
	MP_CHECK_FALSE(app_control);
	MP_CHECK_FALSE(ad);

	bool ret = false;
	char *val = NULL;
	app_control_get_extra_data(app_control, "signal", &val);
	if (!g_strcmp0(val, "serviceType")) {
		const char *type = (CHECK_STORE) ? "store" : "native";
		DEBUG_TRACE("Samsung hub request service type [%s]", type);
		app_control_h reply = NULL;
		app_control_create(&reply);
		app_control_add_extra_data(reply, "resultCode", "RESULT_OK");
		app_control_add_extra_data(reply, "serviceType", type);
		app_control_reply_to_launch_request(reply, app_control, APP_CONTROL_RESULT_SUCCEEDED);
		app_control_destroy(reply);

		ret = true;
	}
	IF_FREE(val);

	return ret;
}

static int
_mp_main_parse_service(struct appdata *ad, app_control_h app_control, bool *activate_window, bool *start_playback)
{
	startfunc;
	int ret = 0;
	MP_CHECK_VAL(app_control, -1);

	app_control_foreach_extra_data(app_control, __mp_main_app_control_extra_data_cb, NULL);

	char *value = NULL;
#ifdef MP_SOUND_PLAYER
	/* create playlist mgr before parse service */
	mp_common_create_playlist_mgr();

	if (!app_control_get_extra_data(app_control, MP_REQ_TYPE, &value)) {
		DEBUG_TRACE("request_type: %s", value);
		if (_mp_main_parse_request_type(ad, app_control, value, start_playback)) {
			WARN_TRACE("Error: _mp_main_parse_request_type");
			ret = -1;
		}
		IF_FREE(value);
	} else if (mp_common_parse_view_operation(app_control)) {
		*start_playback = TRUE;
	} else {
		ERROR_TRACE("No uri...");
	}

	IF_FREE(ad->cookie);
	IF_FREE(ad->proxy);

	char *cookie = NULL;
	if (!app_control_get_extra_data(app_control, "cookie", &cookie)) {
		ad->cookie = cookie;
		cookie = NULL;
	}
	char *proxy = NULL;
	if (!app_control_get_extra_data(app_control, "proxy", &proxy)) {
		ad->proxy = proxy;
		proxy = NULL;
	}
	_mp_main_set_transient(ad, app_control);

#else /* MP_SOUND_PLAYER */
	if (!app_control_get_extra_data(app_control, "Launch By", &value)) {
		if (!g_strcmp0(value, "By Sfinder")) {
			DEBUG_TRACE("Launch by Sfinder");
			mp_media_list_h media = NULL;
			int count;
			char *uri = NULL;
			app_control_get_uri(app_control, &uri);
			mp_common_create_playlist_mgr();

			mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
			mp_media_info_list_create(&media, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, count);
			mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, media, count, 0, uri);
			mp_media_info_list_destroy(media);
			IF_FREE(uri);

			*start_playback = true;

			goto END;
		}

		IF_FREE(value);
	}

	if (!app_control_get_extra_data(app_control, MP_REQ_TYPE, &value)) {
		DEBUG_TRACE("request_type: %s", value);
		if (_mp_main_parse_request_type(ad, app_control, value, start_playback)) {
			WARN_TRACE("Error: _mp_main_parse_request_type");
			ret = -1;
		} else if (!g_strcmp0(value, MP_LIVEBOX)) {
			int count;
			mp_media_list_h media = NULL;
			char *uri = NULL;

			*activate_window = false;

			mp_common_create_playlist_mgr();
			app_control_get_extra_data(app_control, MP_LIVEBOX_URI, &uri);

			mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
			mp_media_info_list_create(&media, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, count);
			mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, media, count, 0, uri);
			mp_media_info_list_destroy(media);

			*start_playback = true;

			_mp_main_parse_playback_control(ad, app_control);

			IF_FREE(uri);
		}
		IF_FREE(value);
	} else {
		if (!app_control_get_extra_data(app_control, MP_MM_KEY, &value)) {
			DEBUG_TRACE("mm key event, ad->player_state : %d", ad->player_state);
			*activate_window = false;

			/* Do not start playback on pause cd. */
			if (g_strcmp0(value, "XF86AudioPause")) {
				if (ad->player_state == PLAY_STATE_PAUSED) {
					mp_play_control_play_pause(ad, true);
				} else {

					_mp_main_create_default_playing_list(ad, 0);
					*start_playback = true;
				}
			}

			IF_FREE(value);
		}

		_mp_main_parse_playback_control(ad, app_control);
	}
END:

#endif
	IF_FREE(value);
	return ret;
}

#ifdef MP_FEATURE_DESKTOP_MODE
static void
_mp_main_win_profile_changed_cb(void *data, Evas_Object *obj, void *event)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	const char *profile = elm_config_profile_get();
	if (!g_strcmp0(profile, ELM_PROFILE_DESKTOP)) {
		/* desktop mode */
		DEBUG_TRACE("profile = [%s]", profile);
		ad->desktop_mode = true;

		elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_HIDE);

		/* set win icon */
		Evas_Object *o = evas_object_image_add(evas_object_evas_get(ad->win_main));
		evas_object_image_file_set(o, DESKTOP_ICON, NULL);
		elm_win_icon_object_set(ad->win_main, o);
	} else {
		/* mobile mode */
		ad->desktop_mode = false;

		elm_win_indicator_mode_set(ad->win_main, ELM_WIN_INDICATOR_SHOW);
	}

#ifdef MP_SOUND_PLAYER

#else
	/*MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ALL);
	TODO: unset back button if desktop mode supported */
#endif
}
#endif

#ifdef MP_FEATURE_LANDSCAPE
static void
_win_rotation_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	static int current_angle = 0;
	int changed_angle = elm_win_rotation_get(obj);

	WARN_TRACE("window rotated [%d] => [%d]", current_angle, changed_angle);
	if (current_angle != changed_angle) {
		current_angle = changed_angle;
		mp_device_orientation_cb(changed_angle, data);
	}
}
#endif

static void
_mp_atexit_cb(void)
{
	ERROR_TRACE("#exit() invoked. music-player is exiting");
}

#ifdef MP_DEBUG_MODE
static void
_mp_main_window_flush_pre(void *data, Evas * e, void *event_info)
{
	DEBUG_TRACE("");

	static int first;
	if (!first) {
		TA_E_L(0, "RENDER_FLUSH_POST(service to render)");
		TA_E_L(0, "RENDER_FLUSH_POST(main to render)");
		first = true;
	}
	evas_event_callback_del(e, EVAS_CALLBACK_RENDER_FLUSH_POST, _mp_main_window_flush_pre);
}
#endif

static void
_mp_csc_feature_init(struct appdata *ad)
{
	MP_CHECK(ad);
}

#ifdef MP_FEATURE_SPLIT_WINDOW
static int
_mp_main_multi_window(struct appdata *ad, app_control_h app_control)
{
	char *val_startup = NULL;
	char *val_layout = NULL;
	int id = -1;

	int id_startup_by = (int)evas_object_data_get(ad->win_main, "id_startup_by");
	int id_layout_pos = (int)evas_object_data_get(ad->win_main, "id_layout_pos");

	if (app_control_get_extra_data(app_control, "window_startup_type", &val_startup) != APP_CONTROL_ERROR_NONE) {
		val_startup = strdup("0");
	}
	if (app_control_get_extra_data(app_control, "window_layout_id", &val_layout) != APP_CONTROL_ERROR_NONE) {
		val_layout = strdup("-1");
	}
	EVENT_TRACE("id_startup_by is [%d] id_layout_pos is [%d]", id_startup_by, id_layout_pos);

	if (id_startup_by == -1) {
		id = elm_win_aux_hint_add(ad->win_main, "wm.policy.win.startup.by", val_startup);
		evas_object_data_set(ad->win_main, "id_startup_by", (void *)id);
		EVENT_TRACE("It is launched by split launcher but full window mode");
	} else {
		elm_win_aux_hint_val_set(ad->win_main, id_startup_by, val_startup);
		EVENT_TRACE("Split window mode");
	}

	if (id_layout_pos == -1) {
		id = elm_win_aux_hint_add(ad->win_main, "wm.policy.win.zone.desk.layout.pos", val_layout);
		evas_object_data_set(ad->win_main, "id_layout_pos", (void *)id);
	} else {
		elm_win_aux_hint_val_set(ad->win_main, id_layout_pos, val_layout);
	}

	IF_FREE(val_startup);
	IF_FREE(val_layout);

	int relaunch = false;
	char *operation = NULL;
	if (app_control_get_operation(app_control, &operation) == APP_CONTROL_ERROR_NONE && strcmp(operation, "http://tizen.org/appcontrol/operation/main") == 0) {
		if (__is_relaunch == 1) {
			EVENT_TRACE("relaunch event");
			relaunch = true;
		}
	}
	IF_FREE(operation);

	if (relaunch) {
		elm_win_activate(ad->win_main);
	}

	__is_relaunch = 1;

	return relaunch;
}

#endif

/**< Called before main loop */
static bool
mp_create(void *data)
{
	eventfunc;
	PROFILE_IN("mp_create");
	struct appdata *ad = data;
	g_ad = (struct appdata *)data;
	atexit(_mp_atexit_cb);

#ifdef MP_FEATURE_SPLIT_WINDOW
	__is_relaunch = 0;
#endif

	PROFILE_IN("_mp_csc_feature_get");
	_mp_csc_feature_init(ad);
	PROFILE_OUT("_mp_csc_feature_get");

	MP_CHECK_VAL(ad, EINA_FALSE);

	PROFILE_IN("elm_theme_extension_add");
	/* do extension add before add elm object.*/
	char edje_path[1024] ={0};
	char * path = app_get_resource_path();
	MP_CHECK_VAL(path, EINA_FALSE);
	snprintf(edje_path, 1024, "%s%s/%s", path, "edje", THEME_NAME);

	/*Elm_Theme *th = elm_theme_new();*/
	elm_theme_extension_add(NULL, edje_path);
	free(path);
	PROFILE_OUT("elm_theme_extension_add");

	PROFILE_IN("bindtextdomain");
	bindtextdomain(DOMAIN_NAME, LOCALE_DIR);

	PROFILE_OUT("bindtextdomain");

	PROFILE_IN("mp_create_win");
	ad->win_main = mp_create_win("music-player");
	mp_retv_if(ad->win_main == NULL, EINA_FALSE);

#ifdef MP_FEATURE_SPLIT_WINDOW
	elm_win_wm_desktop_layout_support_set(ad->win_main, EINA_TRUE);
	evas_object_data_set(ad->win_main, "id_startup_by", (void *) - 1);
	evas_object_data_set(ad->win_main, "id_layout_pos", (void *) - 1);
#endif

#ifdef MP_FEATURE_LANDSCAPE
	if (elm_win_wm_rotation_supported_get(ad->win_main)) {
		const int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win_main, rots, 4);
		evas_object_smart_callback_add(ad->win_main, "wm,rotation,changed", _win_rotation_changed_cb, ad);
	}
#else
	elm_win_wm_rotation_preferred_rotation_set(ad->win_main, 0);
#endif
	PROFILE_OUT("mp_create_win");

#ifdef MP_FEATURE_DESKTOP_MODE
	evas_object_smart_callback_add(ad->win_main, "profile,changed", _mp_main_win_profile_changed_cb, ad);

#endif

	ad->evas = evas_object_evas_get(ad->win_main);

	if (!_mp_main_init(ad)) {
		ERROR_TRACE("Fail when init music");
		return EINA_FALSE;
	}

	mp_setting_init(ad);
	mp_language_mgr_create();
#ifdef MP_FEATURE_GL
	Evas_Object *bg = mp_widget_create_bg(ad->win_main);
	elm_win_resize_object_add(ad->win_main, bg);
#endif
	PROFILE_IN("elm_conformant_add");
	Evas_Object *conformant = NULL;
	conformant = elm_conformant_add(ad->win_main);
	MP_CHECK_FALSE(conformant);

	elm_object_signal_emit(conformant, "elm,state,indicator,overlap", "");
	evas_object_data_set(conformant, "overlap", (void *)EINA_TRUE);

	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(conformant);
	elm_win_resize_object_add(ad->win_main, conformant);

	ad->conformant = conformant;
	PROFILE_OUT("elm_conformant_add");

	ad->view_manager = mp_view_mgr_create(ad->conformant);
	MP_CHECK_FALSE(ad->view_manager);

	elm_object_content_set(ad->conformant, ad->view_manager->navi);

	/* bluetooth  control when launch app */
	mp_app_grab_mm_keys(ad);

	PROFILE_OUT("mp_create");
	return EINA_TRUE;
}

/**< Called after main loop */
static void
mp_terminate(void *data)
{
	eventfunc;
	struct appdata *ad = data;
	DEBUG_TRACE_FUNC();
	mp_retm_if(ad == NULL, "ad is null");

#ifdef MP_WATCH_DOG
	mp_watch_dog_finalize();
#endif

	mp_volume_finalize();

	if(ad->stream_info) {
		int error = sound_manager_destroy_stream_information(ad->stream_info);
		if (error != SOUND_MANAGER_ERROR_NONE) {
			ERROR_TRACE("unable to destroy stream. error code [%x]", error);
		}
	}

#ifdef MP_SOUND_PLAYER
	mp_setting_set_nowplaying_id(0);
#endif

	mp_language_mgr_destroy();
	mp_ecore_idler_del(ad->app_init_idler);
	mp_ecore_timer_del(ad->longpress_timer);
	mp_ecore_timer_del(ad->live_pos_timer);

#ifdef MP_FEATURE_AUTO_OFF
	mp_ecore_timer_del(ad->auto_off_timer);
	mp_ecore_timer_del(ad->pause_off_timer);
#endif

	if (ad->sleep_unlock_timer) {
		mp_util_sleep_lock_set(FALSE, TRUE);
	}

#ifdef MP_FEATURE_CLOUD
	mp_cloud_destroy();
#endif

	mp_app_ungrab_mm_keys(ad);

	if (ad->key_down) {
		ecore_event_handler_del(ad->key_down);
	}
	if (ad->key_up) {
		ecore_event_handler_del(ad->key_up);
	}
	if (ad->mouse_button_down) {
		ecore_event_handler_del(ad->mouse_button_down);
	}
	if (ad->visibility_change) {
		ecore_event_handler_del(ad->visibility_change);
		ad->visibility_change = NULL;
	}
	if (ad->focus_in) {
		ecore_event_handler_del(ad->focus_in);
		ad->focus_in = NULL;
	}
	if (ad->focus_out) {
		ecore_event_handler_del(ad->focus_out);
		ad->focus_out = NULL;
	}
#ifdef MP_FEATURE_PALM_TOUCH
	if (ad->hold) {
		ecore_event_handler_del(ad->hold);
		ad->hold = NULL;
	}
#endif
	if (ad->player_state != PLAY_STATE_NONE) {
		mp_player_mgr_stop(ad);
		mp_player_mgr_destroy(ad);
	}

	if (!mp_util_is_other_player_playing()) {
		int ret_set = 0;
		ret_set = preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_OFF);
		if (ret_set) {
			ERROR_TRACE("set preference failed");
		}
	}
	ad->freeze_indicator_icon = false;
	mp_setting_set_player_state(MP_PLAY_STATE_NONE);
	mp_minicontroller_destroy(ad);
#ifdef MP_FEATURE_LOCKSCREEN
	mp_lockscreenmini_destroy(ad);
#endif

	mp_player_mgr_vol_type_unset();
	mp_player_mgr_safety_volume_set(0);

	if (!mp_player_mgr_session_finish()) {
		ERROR_TRACE("Error when set session");
	}
#ifdef MP_ENABLE_INOTIFY
	mp_app_inotify_finalize(ad);
#endif
	mp_http_mgr_destory(ad);

	mp_playlist_mgr_destroy(ad->playlist_mgr);
	ad->playlist_mgr = NULL;

	mp_util_free_track_info(ad->current_track_info);
	ad->current_track_info = NULL;

	mp_media_info_disconnect();
	mp_setting_deinit(ad);
	if (!mp_app_noti_ignore(ad)) {
		ERROR_TRACE("Error when ignore noti");
	}

	mp_view_mgr_destory(GET_VIEW_MGR);

#ifdef MP_3D_FEATURE
	mp_coverflow_view_destroy(ad);
#endif

	MP_TA_ACUM_ITEM_SHOW_RESULT_TO(MP_TA_SHOW_FILE);
	MP_TA_RELEASE();

#ifdef MP_FEATURE_CONTEXT_ENGINE
	mp_context_log_disconnect();
#endif

#ifdef MP_FEATURE_MUSIC_VIEW
	mp_music_view_mgr_release();
#endif

	return;
}

/**< Called when every window goes back */
static void
mp_pause(void *data)
{
	eventfunc;
	int ret = 0;
	ret = mp_player_mgr_vol_type_unset();
	DEBUG_TRACE("sound_manager_unset_current_sound_type = %d", ret);
	return;
}

/**< Called when any window comes on top */
static void
mp_resume(void *data)
{
	eventfunc;

	return;
}

static Eina_Bool _check_app_control_timer_cb(void *data)
{
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	ad->app_control_check_timer = NULL;

	if (g_normal_launched == false) {
		DEBUG_TRACE("It's launched to check store available. terminate App");
		elm_exit();
	}
	return false;
}

/**< Called at the first idler and relaunched by AUL*/
static void
app_control(app_control_h app_control, void *data)
{
	eventfunc;
	PROFILE_IN("mp_service");

	char *operation = NULL;
	struct appdata *ad = data;
	mp_ret_if(ad == NULL);

	bool activate_window = true;
	bool start_playback = false;

	if (ad->exit_job) {
		ecore_job_del(ad->exit_job);
		ad->exit_job = NULL;
	}

	if (!app_control_get_operation(app_control, &operation)) {
		DEBUG_TRACE("operation: %s", operation);
	}
	IF_FREE(operation);

	/* @@ initialize session type @@ */
	/* @@ important session init should be set before play @@ */
	if (!mp_player_mgr_session_init()) {
		ERROR_TRACE("Error when set session");
	}

	if (_mp_main_check_servic_type(ad, app_control)) {
		/* check service type by samsung hub */
		if (g_normal_launched == false) {
			evas_object_show(ad->win_main);
			elm_win_iconified_set(ad->win_main, EINA_TRUE);
			elm_win_lower(ad->win_main);
			ad->app_control_check_timer = ecore_timer_add(0.1, _check_app_control_timer_cb, ad);
			WARN_TRACE("Samsung hub checked service type before launched");
		}
		return;
	}

#ifdef MP_FEATURE_SPLIT_WINDOW
	if (_mp_main_multi_window(ad, app_control)) {
		return;
	}
#endif

	PROFILE_IN("_mp_main_parse_service");
	if (_mp_main_parse_service(ad, app_control, &activate_window, &start_playback)) {
		ERROR_TRACE("Error: _mp_main_parse_service");
		elm_exit();
		return;
	}
	PROFILE_OUT("_mp_main_parse_service");

	if (_mp_main_parse_livebox_event(app_control, &activate_window, &start_playback)) {
		DEBUG_TRACE("Livebox event: activate_window[%d], start_playback[%d]", activate_window, start_playback);
	}

	if (test_uri) {
		ERROR_TRACE("Test url [%s]", test_uri);
		start_playback = true;
		activate_window = true;
		mp_common_create_playlist_mgr();
		mp_playlist_mgr_item_append(ad->playlist_mgr, test_uri, NULL, NULL, NULL, MP_TRACK_URI);
	}

	int launch_by_shortcut = false;
#ifdef MP_FEATURE_OPTIMIZATION_LAUNCH_TIME
	int early_show_main_win = true;
#ifdef MP_SOUND_PLAYER
	if (!start_playback) {
		early_show_main_win = false;
	}
#else
	if (activate_window) {
		MpTab_e tab = MP_TAB_SONGS;
		char *shortcut_main_info = NULL;
		_mp_common_parse_open_shortcut(app_control, &tab, &shortcut_main_info);
		if (shortcut_main_info) {
			launch_by_shortcut = true;
		}
	}
#endif
	if (activate_window && !launch_by_shortcut && early_show_main_win) {
		evas_object_show(ad->win_main);
	}
#endif
	if (start_playback) {
		mp_play_destory(ad);
		ad->paused_by_user = FALSE;
#ifdef MP_SOUND_PLAYER
		ad->app_is_foreground = true;		/* for error popup */
#endif
		int ret = mp_play_new_file(ad, TRUE);
		if (ret) {
			ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
			if (ret == MP_PLAY_ERROR_NETWORK) {
				mp_play_next_file(ad, true);
			}
#endif
		}
#ifndef MP_SOUND_PLAYER
		ad->app_is_foreground = false;
		ad->is_focus_out = true;
		_show_minicontroller(ad);
#endif
	}

#ifndef MP_SOUND_PLAYER
	if (start_playback) {
		DEBUG_TRACE("Start playback");
		if (activate_window) {
			mp_common_create_initial_view(ad, NULL, NULL);
			mp_common_show_player_view(0, true, false, true);
		} else {
			DEBUG_TRACE("View will be created on player_start");
			goto END;	/* view should be created after player started... */
		}
	}
#else
	if (!start_playback) {
		ERROR_TRACE("check service key, start_playback is false");
		mp_app_exit(ad);
		return;
	}
#endif

	if (activate_window) {
		mp_common_create_initial_view(ad, app_control, &launch_by_shortcut);
	} else {
		DEBUG_TRACE("unactivate window");
		goto END;
	}
	if (launch_by_shortcut) {
		/* window activate should be done in idler.. */
		goto END;
	}


#ifdef MP_FEATURE_LANDSCAPE
#ifndef defined (MP_3D_FEATURE)
	if (ad->screen_mode == MP_SCREEN_MODE_LANDSCAPE) {
		evas_object_hide(ad->conformant);
	}
#endif
#endif

	evas_object_show(ad->win_main);

	if (activate_window) {
		DEBUG_TRACE("activate window");
		elm_win_activate(ad->win_main);
		ad->app_is_foreground = true;
	} else {
		DEBUG_TRACE("lower window");
		elm_win_iconified_set(ad->win_main, EINA_TRUE);
		elm_win_lower(ad->win_main);
		ad->app_is_foreground = false;
		ad->is_focus_out = true;
		_show_minicontroller(ad);
	}

	/* TEMP_BLOCK */
	/*
	if (power_get_state() == POWER_STATE_SCREEN_OFF)
		ad->is_lcd_off = true;
	*/
	display_state_e state;
	if (device_display_get_state(&state) == DISPLAY_STATE_SCREEN_OFF) {
		ad->is_lcd_off = true;
	}
	bool reply_requested = false;
	app_control_is_reply_requested(app_control, &reply_requested);
	if (reply_requested) {
		DEBUG_TRACE("send reply to caller");
		app_control_h reply = NULL;
		app_control_create(&reply);
		app_control_reply_to_launch_request(reply, app_control, APP_CONTROL_RESULT_SUCCEEDED);
		app_control_destroy(reply);
	}
	PROFILE_OUT("mp_service");


#ifdef MP_DEBUG_MODE
	TA_S_L(0, "RENDER_FLUSH_POST(service to render)");
	evas_event_callback_add(evas_object_evas_get(ad->win_main), EVAS_CALLBACK_RENDER_FLUSH_POST,  _mp_main_window_flush_pre, NULL);
#endif

END:
	if (!g_normal_launched) {
		if (!_mp_main_is_launching_available(ad)) {
			return;
		}
		ad->app_init_idler = ecore_idler_add(_mp_main_app_init_idler_cb, ad);
		g_normal_launched = true;
	}

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
	app_control_h db_service = NULL;

	int ret = 0;
	app_control_create(&db_service);
	app_control_set_app_id(db_service, "org.tizen.music-player.service");
	ret = app_control_send_launch_request(db_service, NULL, NULL);
	if (ret == APP_CONTROL_ERROR_NONE) {
		DEBUG_TRACE("Succeeded to launch app.");
	} else {
		DEBUG_TRACE("Failed to launch a app. ret %d", ret);
	}

	app_control_destroy(db_service);
	db_service = NULL;
#endif
	endfunc;
	return;
}

static void
mp_low_battery(app_event_info_h event_info, void *data)
{
	eventfunc;
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK(ad);

	app_event_low_battery_status_e status = -1;

	int ret = app_event_get_low_battery_status(event_info, &status);
	if (ret == APP_ERROR_NONE) {
		ad->low_battery_status = status;
		if (status <= APP_EVENT_LOW_BATTERY_POWER_OFF) {
			mp_app_exit(ad);
		}
	} else {
		DEBUG_TRACE("Failed to get battery status. ret %d", ret);
	}
	return;
}

/*
**	this is register in app_event_callback_s for catch event of device rotate
**	in the case, we want to get when rotation start.
**	MP_VIEW_ROTATE_START event to indicate rotation start
**	also see MP_VIEW_ROTATE event which indicates rotation done
*/

static void
mp_device_orientation(app_event_info_h event_info, void *data)
{
	eventfunc;
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK(ad);

#ifdef MP_FEATURE_LANDSCAPE
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_VIEW_ROTATE_START);
#endif

	return;
}

void
mp_device_orientation_cb(app_device_orientation_e orientation, void *user_data)
{
	eventfunc;
	struct appdata *ad = user_data;
	MP_CHECK(ad);


#ifdef MP_FEATURE_DESKTOP_MODE
	if (ad->desktop_mode) {
		return;
	}
#endif

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode) {
		mp_mini_player_rotation_cb(orientation, ad);
		return;
	}
#endif

#ifdef MP_FEATURE_LANDSCAPE
	mp_app_rotate(orientation, user_data);
#else
	ERROR_TRACE("Temporary unsupported");
	return;
#endif
}

static void
__mp_language_changed_cb(app_event_info_h event_info, void *user_data)
{
	eventfunc;
	struct appdata *ad = user_data;
	mp_popup_destroy(ad);

	char *locale = NULL;
	int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ERROR_TRACE("Unable to fetch the current language setting with return value %d", retcode);
	}
	if (locale) {
		mp_error("locale is [%s]", locale);
		elm_language_set(locale);
		free(locale);
		locale = NULL;
	}

	mp_language_mgr_update();

#ifndef MP_SOUND_PLAYER
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_LANG_CHANGED);
#endif
}

EXPORT_API int
main(int argc, char *argv[])
{
	startfunc;
	struct appdata ad;

	ui_app_lifecycle_callback_s event_callbacks;

	int nRet = APP_ERROR_NONE;
	app_event_handler_h  hLowMemoryHandle;
	app_event_handler_h  hLowBatteryHandle;
	app_event_handler_h  hLanguageChangedHandle;
	app_event_handler_h  hDeviceOrientationChangedHandle;
	app_event_handler_h  hRegionFormatChangedHandle;

	ad.low_battery_status = 0;

	event_callbacks.create = mp_create;
	event_callbacks.terminate = mp_terminate;
	event_callbacks.pause = mp_pause;
	event_callbacks.resume = mp_resume;
	event_callbacks.app_control = app_control;

	nRet = ui_app_add_event_handler(&hLowMemoryHandle, APP_EVENT_LOW_MEMORY, NULL, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		WARN_TRACE("APP_EVENT_LOW_MEMORY ui_app_add_event_handler failed : [%d]!!!", nRet);
	}

	nRet = ui_app_add_event_handler(&hLowBatteryHandle, APP_EVENT_LOW_BATTERY, mp_low_battery, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		ERROR_TRACE("APP_EVENT_LOW_BATTERY ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}

	nRet = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, __mp_language_changed_cb, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		ERROR_TRACE("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}

	nRet = ui_app_add_event_handler(&hDeviceOrientationChangedHandle, APP_EVENT_DEVICE_ORIENTATION_CHANGED, mp_device_orientation, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		ERROR_TRACE("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
		return -1;
	}

	nRet = ui_app_add_event_handler(&hRegionFormatChangedHandle, APP_EVENT_REGION_FORMAT_CHANGED, NULL, (void *)&ad);
	if (nRet != APP_ERROR_NONE) {
		WARN_TRACE("APP_EVENT_REGION_FORMAT_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
	}

	MP_TA_INIT();
	TA_S_L(0, "RENDER_FLUSH_POST(main to render)");
	/* Enable OpenGL */
#ifdef MP_FEATURE_GL
	setenv("ELM_ENGINE", "gl", 1);
#endif
	memset(&ad, 0x0, sizeof(struct appdata));

	if (argc == 1) {
		DEBUG_TRACE("%s", argv[0]);
		if (mp_util_is_streaming(argv[0]) || g_file_test(argv[0], G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
			test_uri = g_strdup(argv[0]);
		}
	}

	return ui_app_main(argc, argv, &event_callbacks, &ad);
}

