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

#include <sound_manager.h>
#include <player.h>

#include "music.h"
#include "mp-ta.h"
#include "mp-player-control.h"
#include "mp-player-mgr.h"
/* #include "mp-player-drm.h" */
#include "mp-item.h"
#include "mp-playlist-mgr.h"
#include "mp-play.h"
#include "mp-util.h"
#include "mp-setting-ctrl.h"
#include "mp-player-mgr.h"
#include "mp-app.h"
#include "mp-player-debug.h"
#include "mp-minicontroller.h"
#include "mp-lockscreenmini.h"
#include "mp-widget.h"
#include "mp-streaming-mgr.h"
#include "mp-ug-launch.h"

#ifdef MP_SOUND_PLAYER

#else
#include "mp-common.h"
#endif
#include "mp-view-mgr.h"

#ifdef MP_3D_FEATURE
#endif
#ifdef MP_FEATURE_AVRCP_13
#include "mp-avrcp.h"
#endif
#include "mp-player-view.h"

#include "mp-file-tag-info.h"

#define CTR_EDJ_SIG_SRC "ctrl_edj"
#define CTR_PROG_SIG_SRC "ctrl_prog"

#define LONG_PRESS_INTERVAL             1.0	/* sec */
#define MEDIA_KEY_LONG_PRESS_INTERVAL             1.0	/* sec */
#define FF_REW_INTERVAL             0.5		/* sec */
#define LONG_PRESS_TIME_INCREASE	3.0	/* sec */
#define SEEK_DIFF 10

/* static Eina_Bool _mp_play_mute_popup_cb(void *data); */

#define HW_ISSUE_TEMP		/* temporary for DV */

#ifndef MP_SOUND_PLAYER
static void
_mp_play_control_play_next_on_error(void *data)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK(ad);
	mp_plst_item *current = mp_playlist_mgr_get_current(ad->playlist_mgr);
	mp_plst_item *next = mp_playlist_mgr_get_next(ad->playlist_mgr, false, false);

	if (next == current) {
		WARN_TRACE("There is no playable track.. ");
		return;
	}

	mp_playlist_mgr_item_remove_item(ad->playlist_mgr, current);
	if (next) {
		mp_playlist_mgr_set_current(ad->playlist_mgr, next);
		mp_play_new_file(ad, true);
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK)
			mp_play_next_file(ad, true);
#endif
	} else {
#ifdef MP_SOUND_PLAYER
		mp_app_exit(ad);
#endif
		next = mp_playlist_mgr_get_nth(ad->playlist_mgr, 0);
		if (next) {
			mp_playlist_mgr_set_current(ad->playlist_mgr, next);
		} else {
			mp_player_control_stop(ad);
			mp_view_mgr_pop_a_view(GET_VIEW_MGR, GET_PLAYER_VIEW);
		}
	}

	if (GET_PLAYER_VIEW)
		mp_player_view_refresh(GET_PLAYER_VIEW);

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MGR_ITEM_CHANGED);

}
#endif

static void
_mp_play_error_handler(struct appdata *ad, const char *msg)
{
	startfunc;
	mp_play_destory(ad);

	mp_util_post_status_message(ad, msg);
#ifndef MP_SOUND_PLAYER
	_mp_play_control_play_next_on_error(ad);
#endif
}

static void
_mp_play_control_long_press_seek_done_cb(void *data)
{
	MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYER);
	if (view) {
		mp_player_view_progress_timer_thaw(view);
	}

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_minicontroller_update_progressbar(ad);
#ifdef MP_FEATURE_LOCKSCREEN
		mp_lockscreenmini_update_progressbar(ad);
#endif
}

static void
_mp_player_control_move_position(int diff)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	double pos = 0, duration = 0, new_pos = 0, music_pos = 0;;

	duration = mp_player_mgr_get_duration();
	pos = mp_player_mgr_get_position();

	ad->music_length = duration / 1000;
	music_pos = pos / 1000;

	new_pos = music_pos + diff;

	int req_seek_pos = 0;
	if (new_pos < 0.) {
		music_pos = 0;
		req_seek_pos = 0;
	} else if (new_pos > ad->music_length) {
		music_pos = ad->music_length;
		req_seek_pos = duration;
	} else {
		music_pos = new_pos;
		req_seek_pos = new_pos * 1000;
	}

	if (mp_player_mgr_set_position(req_seek_pos, _mp_play_control_long_press_seek_done_cb, ad)) {
		double get_pos = mp_player_mgr_get_position() / 1000.0;
		MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYER);
		if (view) {
			mp_player_view_progress_timer_freeze(view);

			if (get_pos != music_pos) {
				music_pos = get_pos;
			}
			ad->music_pos = music_pos;
			mp_player_view_update_progressbar(view);
		}
	}

	#ifndef MP_SOUND_PLAYER
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_NOW_PLAYING);
	/* mp_setting_save_now_playing(ad); */
	#endif


}

static Eina_Bool
_mp_play_control_long_press_timer_cb(void *data)
{
	startfunc;
	struct appdata *ad = data;

	int error = 0;

	mp_retvm_if (ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");
	int res = true;
	if (ad->player_state != PLAY_STATE_PAUSED && ad->player_state != PLAY_STATE_PLAYING && ad->player_state != PLAY_STATE_READY) {
		return ECORE_CALLBACK_RENEW;
	}

	if (ad->is_ff) {
		ad->ff_rew_distance += LONG_PRESS_TIME_INCREASE;
#ifdef MP_FEATURE_AVRCP_13
		if (!ad->is_Longpress)
			mp_avrcp_noti_player_state(MP_AVRCP_STATE_FF);
#endif
	} else {
		ad->ff_rew_distance -= LONG_PRESS_TIME_INCREASE;
#ifdef MP_FEATURE_AVRCP_13
		if (!ad->is_Longpress)
			mp_avrcp_noti_player_state(MP_AVRCP_STATE_FF);
#endif
	}

	ad->is_Longpress = true;
	if (!mp_player_mgr_is_active()) {
		error = mp_player_mgr_prepare(ad);
		if (error) {
			DEBUG_TRACE("failed to prepare _player");
		}
	}
	_mp_player_control_move_position(ad->ff_rew_distance);

	if (ad->longpress_timer)
		ecore_timer_interval_set(ad->longpress_timer, FF_REW_INTERVAL);

	endfunc;

	return ECORE_CALLBACK_RENEW;

}

static void
_mp_play_control_add_longpressed_timer(void *data, double interval)
{
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");
	MP_CHECK(!ad->longpress_timer);

	ad->longpress_timer =
		ecore_timer_add(interval, _mp_play_control_long_press_timer_cb, ad);
}

void
_mp_play_control_del_longpressed_timer(void *data)
{
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	ad->ff_rew_distance = 0;
	mp_ecore_timer_del(ad->longpress_timer);
}

static void
_mp_play_control_completed_cb(void *userdata)
{
	eventfunc;
	struct appdata *ad = userdata;
	MP_CHECK(ad);

	mp_play_control_end_of_stream(ad);
}

static void
_mp_play_control_interrupted_cb(player_interrupted_code_e code, void *userdata)
{
	eventfunc;
	struct appdata *ad = userdata;
	MP_CHECK(ad);

	switch (code) {
	case PLAYER_INTERRUPTED_BY_MEDIA:
		WARN_TRACE("receive MM_MSG_CODE_INTERRUPTED_BY_OTHER_APP");
		break;
	case PLAYER_INTERRUPTED_BY_CALL:
		WARN_TRACE("receive PLAYER_INTERRUPTED_BY_CALL");
		break;
	case PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG:
		WARN_TRACE("receive MM_MSG_CODE_INTERRUPTED_BY_EARJACK_UNPLUG");
		break;
	case PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT:
		WARN_TRACE("receive MM_MSG_CODE_INTERRUPTED_BY_RESOURCE_CONFLICT");
		break;
	case PLAYER_INTERRUPTED_BY_ALARM:
		WARN_TRACE("receive MM_MSG_CODE_INTERRUPTED_BY_ALARM_START");
		break;
	case PLAYER_INTERRUPTED_COMPLETED:
		WARN_TRACE("PLAYER_INTERRUPTED_COMPLETED");
		/* ready to resume */
		if (ad->player_state == PLAY_STATE_PAUSED) {
			ad->paused_by_user = false;
			int error = mp_player_mgr_resume(ad);
			if (!error) {
				mp_setting_set_nowplaying_id(getpid());
				if (ad->player_state == PLAY_STATE_PAUSED)
					mp_play_resume(ad);
				ad->player_state = PLAY_STATE_PLAYING;
			} else {
				ad->auto_resume = true;
				mp_play_control_on_error(ad, error, true);
			}
		}
		return;
		break;
	default:
		ERROR_TRACE("Unhandled code: %d", code);
		break;
	}

	/* if playback interrupted while seeking, seek complete callback will not be ivoked. */
	mp_player_mgr_seek_done(ad);

	ad->paused_by_user = true;
	ad->is_sdcard_removed = true;
	mp_play_pause(ad);
	if (ad->mirror_to_local) {
		ad->mirror_to_local = false;
		mp_play_control_play_pause(ad, true);
	}

}

static void
_mp_play_control_error_cb(int error_code, void *userdata)
{
	eventfunc;
	struct appdata *ad = userdata;
	MP_CHECK(ad);

	ERROR_TRACE("Error from player");

	char *error_msg = NULL;

	switch (error_code) {
	case PLAYER_ERROR_NOT_SUPPORTED_FILE:	/* can receive error msg while playing. */
		ERROR_TRACE("receive PLAYER_ERROR_NOT_SUPPORTED_FILE");
		_mp_play_error_handler(ad, GET_STR("IDS_MUSIC_POP_UNABLE_TO_PLAY_UNSUPPORTED_FILETYPE"));
		break;
	case PLAYER_ERROR_CONNECTION_FAILED:
		ERROR_TRACE("MM_ERROR_PLAYER_STREAMING_CONNECTION_FAIL");
		_mp_play_error_handler(ad, GET_SYS_STR("IDS_COM_POP_CONNECTION_FAILED"));
		break;
	default:
		ERROR_TRACE("error_code: 0x%x", error_code);
		error_msg = GET_STR("IDS_MUSIC_POP_UNABLE_TO_PLAY_ERROR_OCCURRED");
		_mp_play_error_handler(ad, error_msg);
		break;
	}

	mp_player_view_update_buffering_progress(GET_PLAYER_VIEW, -1);

}

static void
_mp_play_control_buffering_cb(int percent, void *userdata)
{
	startfunc;
	struct appdata *ad = userdata;
	MP_CHECK(ad);

	mp_debug("Buffering : %d%%", percent);
	mp_player_view_update_buffering_progress(GET_PLAYER_VIEW, percent);
}

static void
_mp_play_control_prepare_cb(void *userdata)
{
	eventfunc;
	struct appdata *ad = userdata;
	MP_CHECK(ad);

	if ((mp_player_mgr_get_player_type() == MP_PLAYER_TYPE_MMFW) && (mp_player_mgr_get_state() != PLAYER_STATE_READY)) {
		WARN_TRACE("player state is not PLAYER_STATE_READY");
		return;
	}
	/* reset if track changed */
	ad->ff_rew_distance = 0;
	mp_play_prepare(ad);
	ad->player_state = PLAY_STATE_READY;
	if (!ad->camcoder_start || !ad->auto_next || !ad->paused_by_user) {
		mp_play_start_in_ready_state(ad);
	} else {
		WARN_TRACE("Stay in ready state because of camcorder.ad->camcoder_start [%d], ad->auto_next [%d], ad->paused_by_user [%d]", ad->camcoder_start , ad->auto_next, ad->paused_by_user);
		ad->freeze_indicator_icon = false;
		ad->resume_on_cam_end = true;
		mp_util_sleep_lock_set(FALSE, FALSE);
	}

}

#ifdef MP_SOUND_PLAYER
static void
_mp_play_text_popup_exit_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	mp_evas_object_del(obj);
	elm_exit();
}
#endif

/*error handler in case of failure on player create, realize, start, resume */
void
mp_play_control_on_error(struct appdata *ad, int ret, bool add_watch)
{
	const char *message = NULL;

	if (ret == PLAYER_ERROR_SOUND_POLICY) {
		WARN_TRACE("PLAYER_ERROR_SOUND_POLICY");

		if (mp_util_is_call_connected()) {
			message = STR_MP_UNABLE_TO_PLAY_DURING_CALL;
#ifndef MP_SOUND_PLAYER
			MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ALL);
			MP_CHECK(view);
			if (mp_player_mgr_get_state() == PLAYER_STATE_READY) {
				mp_play_destory(ad);
			}
#endif
		} else if (!ad->auto_resume)
			message = STR_MP_UNABLE_TO_PLAY_ERROR_OCCURED;

		if (add_watch && (ad->auto_next || ad->auto_resume)) {
			WARN_TRACE("Enter add watch callback to resume after call or alarm");
	WARN_TRACE("Leave add watch callback to resume after call or alarm");
		}
	} else if (ret == PLAYER_ERROR_INVALID_OPERATION && mp_player_mgr_is_seeking()) {
		WARN_TRACE("Trying to resume while seeking. Do not show error msg");
	} else {
		message = STR_MP_UNABLE_TO_PLAY_ERROR_OCCURED;
	}

	if (message) {
		if (ad->is_focus_out)
			mp_util_post_status_message(ad, GET_STR(message));
		else
#ifndef MP_SOUND_PLAYER
	mp_widget_text_popup(ad, GET_STR(message));
#else
	mp_widget_text_cb_popup(ad, GET_STR(message), _mp_play_text_popup_exit_timeout_cb);
#endif
	}
}

static void
_mp_play_control_duration_changed_cb(void *data)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	ad->music_length = mp_player_mgr_get_duration() / 1000.0;
	if (ad->music_length <= 0) {
		mp_track_info_t *track_info = ad->current_track_info;
		if (track_info)
			ad->music_length = track_info->duration / 1000.0;
	}
}

int
mp_player_control_ready_new_file(void *data, bool check_drm)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_FALSE(item);

	int error = 0;
	char *uri = NULL;
	uri = item->uri;

	SECURE_DEBUG("current item pathname : [%s]", uri);
	if (mp_util_is_streaming(uri)) {

	} else if (!mp_check_file_exist(uri)) {
		ERROR_TRACE("There is no such file\n");
		_mp_play_error_handler(ad, GET_SYS_STR("IDS_COM_POP_FILE_NOT_EXIST"));
		return -1;
	}

	ad->player_state = PLAY_STATE_NONE;

	mp_player_type_e player_type = MP_PLAYER_TYPE_MMFW;
	void *extra_data = NULL;

	/* lock wake up */
	mp_util_sleep_lock_set(TRUE, FALSE);
	PROFILE_IN("mp_player_mgr_create");
	{
		error = mp_player_mgr_create(ad, uri, player_type, extra_data);
	}
	PROFILE_OUT("mp_player_mgr_create");

	if (ad->b_minicontroller_show)
		mp_minicontroller_update(ad, true);

#ifdef MP_FEATURE_LOCKSCREEN
	if (ad->b_lockmini_show) {
		mp_lockscreenmini_update(ad);
	}
#endif
	if (error) {
		_mp_play_error_handler(ad, GET_STR("IDS_MUSIC_POP_UNABLE_TO_PLAY_ERROR_OCCURRED"));
		return error;
	}

	mp_player_mgr_set_started_db(mp_play_start, ad);
	mp_player_mgr_set_completed_cb(_mp_play_control_completed_cb, ad);
	mp_player_mgr_set_interrupted_cb(_mp_play_control_interrupted_cb, ad);
	mp_player_mgr_set_error_cb(_mp_play_control_error_cb, ad);
	mp_player_mgr_set_buffering_cb(_mp_play_control_buffering_cb, ad);
	mp_player_mgr_set_prepare_cb(_mp_play_control_prepare_cb, ad);
	mp_player_mgr_set_paused_cb(mp_play_pause, ad);
	mp_player_mgr_set_duration_changed_cb(_mp_play_control_duration_changed_cb, ad);

	ad->prepare_by_init = false;
	PROFILE_IN("mp_player_mgr_realize");
	error = mp_player_mgr_realize(ad);
	PROFILE_OUT("mp_player_mgr_realize");
	if (error) {
		_mp_play_error_handler(ad, GET_STR("IDS_MUSIC_POP_UNABLE_TO_PLAY_ERROR_OCCURRED"));
		return error;
	}

	return error;
}


void
mp_play_control_play_pause(struct appdata *ad, bool play)
{
	mp_retm_if (ad == NULL, "appdata is NULL");

	SECURE_DEBUG("play [%d], ad->player_state: %d", play, ad->player_state);

/* int ret = 0; */

	_mp_play_control_del_longpressed_timer(ad);
	int status = 0;

	if (play) {
		ad->paused_by_user = FALSE;

		if (ad->player_state == PLAY_STATE_PAUSED) {
			int error = mp_player_mgr_resume(ad);
			status = error;
			if (!error) {
				mp_setting_set_nowplaying_id(getpid());
				if (ad->player_state == PLAY_STATE_PAUSED)
					mp_play_resume(ad);
				ad->player_state = PLAY_STATE_PLAYING;
			} else {
				mp_play_control_on_error(ad, error, FALSE);
				if (mp_setting_read_playing_status(ad->current_track_info->uri, "paused") != 1) {
					mp_setting_write_playing_status(ad->current_track_info->uri, "paused");
				}
				return;
			}
			if (ad->win_minicon)
				mp_minicontroller_update_control(ad);

#ifdef MP_FEATURE_LOCKSCREEN
			if (ad->win_lockmini) {
				mp_lockscreenmini_update_control(ad);
			}
#endif
		} else if (ad->player_state == PLAY_STATE_READY) {
			mp_play_start_in_ready_state(ad);
		} else if (ad->player_state == PLAY_STATE_PLAYING) {
			WARN_TRACE("player_state is already playing. Update view state");
			if (ad->win_minicon)
				mp_minicontroller_update(ad, false);

#ifdef MP_FEATURE_LOCKSCREEN
			if (ad->win_lockmini) {
				mp_lockscreenmini_update(ad);
			}
#endif
			mp_player_view_update_state(GET_PLAYER_VIEW);

		} else if (ad->player_state == PLAY_STATE_PREPARING) {
			WARN_TRACE("player_state is preparing. Skip event");
		} else {
			/* silentmode -> go to listview -> click one track -> silent mode play no -> go to playing view -> click play icon */
			mp_play_new_file(ad, TRUE);
		}

#ifdef MP_FEATURE_AUTO_OFF
		mp_ecore_timer_del(ad->pause_off_timer);
#endif
	} else {
		/* invoke player_pause() to prevent auto resume if user pause playback in pause state */
		if (ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED) {
			if (mp_player_mgr_pause(ad)) {
				ad->paused_by_user = TRUE;
			}
		} else if (ad->player_state == PLAY_STATE_PREPARING) {
			WARN_TRACE("player_state is prepareing. set paused_by_user!!!");
			ad->paused_by_user = TRUE;
			ad->freeze_indicator_icon = false;
			if (ad->win_minicon)
				mp_minicontroller_update_control(ad);

#ifdef MP_FEATURE_LOCKSCREEN
			if (ad->win_lockmini) {
				mp_lockscreenmini_update_control(ad);
			}
#endif
			mp_player_view_update_state(GET_PLAYER_VIEW);
		}
	}
	if (play) {
		if (ad->current_track_info) {
			if (mp_setting_read_playing_status(ad->current_track_info->uri, "playing") != 1)
				mp_setting_write_playing_status(ad->current_track_info->uri, "playing");
		}
	} else {
		if (ad->current_track_info) {
			if (mp_setting_read_playing_status(ad->current_track_info->uri, "paused") != 1)
				mp_setting_write_playing_status(ad->current_track_info->uri, "paused");
		}
	}
}

void
mp_play_control_resume_via_media_key(struct appdata *ad)
{
	mp_retm_if (ad == NULL, "appdata is NULL");

	SECURE_DEBUG("ad->player_state: %d", ad->player_state);

	MP_CHECK(ad->player_state == PLAY_STATE_PAUSED);

	_mp_play_control_del_longpressed_timer(ad);

	ad->paused_by_user = FALSE;

	int error = 0;
	error = mp_player_mgr_resume(ad);
	if (error) {
		if (error == PLAYER_ERROR_SOUND_POLICY) {
			WARN_TRACE("resume error by sound policy error. retry 1 more only");
			sleep(1);
			mp_play_control_play_pause(ad, true);
			return;
		} else {
			mp_play_control_on_error(ad, error, FALSE);
		}
	} else {
		if (ad->player_state == PLAY_STATE_PAUSED)
			mp_play_resume(ad);
		ad->player_state = PLAY_STATE_PLAYING;

		if (ad->win_minicon)
			mp_minicontroller_update_control(ad);

#ifdef MP_FEATURE_LOCKSCREEN
		if (ad->win_lockmini) {
			mp_lockscreenmini_update_control(ad);
		}
#endif
	}

#ifdef MP_FEATURE_AUTO_OFF
	mp_ecore_timer_del(ad->pause_off_timer);
#endif
}

void mp_player_control_stop(struct appdata *ad)
{
	startfunc;
	mp_player_mgr_stop(ad);
	mp_player_mgr_destroy(ad);

	mp_playlist_mgr_clear(ad->playlist_mgr);
	if (ad->current_track_info) {
		mp_util_free_track_info(ad->current_track_info);
		ad->current_track_info = NULL;
	}

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_UNSET_NOW_PLAYING);
	if (ad->b_minicontroller_show)
		mp_minicontroller_hide(ad);
#ifdef MP_FEATURE_LOCKSCREEN
	if (ad->b_lockmini_show) {
		mp_lockscreenmini_hide(ad);
	}
#endif
}

void
mp_play_control_next(void)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	mp_retm_if (ad == NULL, "appdata is NULL");
	mp_play_next_file(ad, TRUE);
}

static void
mp_play_control_set_position_cb(void *data)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	mp_retm_if (ad == NULL, "appdata is NULL");

	MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYER);
	mp_player_view_update_progressbar(view);

	mp_minicontroller_update(ad, false);
}

void
mp_play_control_prev(void)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	mp_retm_if (ad == NULL, "appdata is NULL");
	int pos = mp_player_mgr_get_position();

	if (pos > 3000)/* mp_playlist_mgr_count(ad->playlist_mgr) == 1 */ {
		if (mp_player_mgr_set_position(0, mp_play_control_set_position_cb, NULL)) {
			double get_pos = mp_player_mgr_get_position() / 1000.0;

			ad->music_pos = 0;

			if (get_pos != ad->music_pos) {
				ad->music_pos = get_pos;
			}
		}

		return;
	}
	mp_play_prev_file(ad);
}

void
mp_play_control_ff(int press, bool event_by_mediakey, bool clicked)
{
	struct appdata *ad = mp_util_get_appdata();
	mp_retm_if (ad == NULL, "appdata is NULL");
	EVENT_TRACE("Next button press[%d]\n", press);

	ad->is_ff = TRUE;

	double interval = LONG_PRESS_INTERVAL;
	if (event_by_mediakey)
		interval = MEDIA_KEY_LONG_PRESS_INTERVAL;

	if (press) {
		_mp_play_control_add_longpressed_timer(ad, interval);
	} else {
		_mp_play_control_del_longpressed_timer(ad);

		if (ad->is_Longpress) {
			mp_play_control_reset_ff_rew();
		} else if (clicked) {
			EVENT_TRACE("Click operation");
			if (!event_by_mediakey) {
				EVENT_TRACE("play next");
				mp_play_control_next();
			} else {
				/* if media key event is MEDIA_KEY_FASTFORWARD, move position even if short press */
				_mp_player_control_move_position(SEEK_DIFF);
			}
		}
	}


}

void
mp_play_control_rew(int press, bool event_by_mediakey, bool clicked)
{
	struct appdata *ad = mp_util_get_appdata();
	mp_retm_if (ad == NULL, "appdata is NULL");
	EVENT_TRACE("Previous button press[%d]\n", press);

	ad->is_ff = FALSE;

	double interval = LONG_PRESS_INTERVAL;
	if (event_by_mediakey)
		interval = MEDIA_KEY_LONG_PRESS_INTERVAL;

	if (press) {
		_mp_play_control_add_longpressed_timer(ad, interval);
	} else {
		_mp_play_control_del_longpressed_timer(ad);

		if (ad->is_Longpress) {
			mp_play_control_reset_ff_rew();
		} else if (clicked) {
			if (!event_by_mediakey)
				mp_play_control_prev();
			else {
				/* if media key event is MEDIA_KEY_REWIND, move position even if short press */
				_mp_player_control_move_position(-SEEK_DIFF);
			}
		}
	}

}

void mp_play_control_reset_ff_rew(void)
{
	struct appdata *ad = mp_util_get_appdata();
	mp_retm_if (ad == NULL, "appdata is NULL");

	_mp_play_control_del_longpressed_timer(ad);

	if (ad->is_Longpress) {
		ad->is_Longpress = false;

#ifdef MP_FEATURE_AVRCP_13
		if (ad->player_state == PLAY_STATE_PLAYING)
			mp_avrcp_noti_player_state(MP_AVRCP_STATE_PLAYING);
		else if (ad->player_state == PLAY_STATE_PAUSED)
			mp_avrcp_noti_player_state(MP_AVRCP_STATE_PAUSED);
		else
			mp_avrcp_noti_player_state(MP_AVRCP_STATE_STOPPED);
#endif
	}
}

void
mp_play_control_menu_cb(void *data, Evas_Object * o, const char *emission, const char *source)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	EVENT_TRACE("mp_play_control_menu_cb with[%s]\n", emission);

	if (!strcmp(emission, SIGNAL_SHUFFLE_ON)) {			/* TURN OFF SHUFFLE */
		mp_play_control_shuffle_set(ad, FALSE);
	} else if (!strcmp(emission, SIGNAL_SHUFFLE_OFF)) {			/* TURN ON SHUFFE */
		mp_play_control_shuffle_set(ad, TRUE);
	} else if (!strcmp(emission, SIGNAL_REP_ALL))	/* off -1 - all - off  //off - all - 1 */ {
		/* repeat 1 */
		mp_setting_set_repeat_state(MP_SETTING_REP_1);
		mp_playlist_mgr_set_repeat(ad->playlist_mgr, MP_PLST_REPEAT_ONE);
#ifdef MP_FEATURE_AVRCP_13
		mp_avrcp_noti_repeat_mode(MP_AVRCP_REPEAT_ONE);
#endif
	} else if (!strcmp(emission, SIGNAL_REP_OFF)) {
		mp_setting_set_repeat_state(MP_SETTING_REP_ALL);
		mp_playlist_mgr_set_repeat(ad->playlist_mgr, MP_PLST_REPEAT_ALL);
#ifdef MP_FEATURE_AVRCP_13
		mp_avrcp_noti_repeat_mode(MP_AVRCP_REPEAT_ALL);
#endif
	} else if (!strcmp(emission, SIGNAL_REP_1)) {
		mp_setting_set_repeat_state(MP_SETTING_REP_NON);
		mp_playlist_mgr_set_repeat(ad->playlist_mgr, MP_PLST_REPEAT_NONE);
#ifdef MP_FEATURE_AVRCP_13
		mp_avrcp_noti_repeat_mode(MP_AVRCP_REPEAT_OFF);
#endif
	}
	if (ad->win_minicon) {
		mp_minicontroller_update_shuffle_and_repeat_btn(ad);
	}

#ifdef MP_FEATURE_LOCKSCREEN
		if (ad->win_lockmini) {
			mp_lockscreenmini_update_shuffle_and_repeat_btn(ad);
		}
#endif
}


void
mp_play_control_end_of_stream(void *data)
{
	eventfunc;
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	ad->music_pos = ad->music_length;
	mp_player_view_update_progressbar(GET_PLAYER_VIEW);
	mp_minicontroller_update_progressbar(ad);
#ifdef MP_SOUND_PLAYER
	mp_plst_item *plst_item = NULL;

	plst_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	if (plst_item) {
		mp_playlist_mgr_set_current(ad->playlist_mgr, plst_item);
		ad->music_pos = 0;
		mp_play_item_play_current_item(ad);
		mp_play_control_play_pause(ad, false);
	}
#else
	mp_play_next_file(ad, FALSE);
#endif
}

void
mp_play_control_shuffle_set(void *data, bool shuffle_enable)
{
	struct appdata *ad = mp_util_get_appdata();
	mp_retm_if (ad == NULL, "appdata is NULL");

#ifdef MP_FEATURE_AVRCP_13
	if (shuffle_enable)
		mp_avrcp_noti_shuffle_mode(MP_AVRCP_SHUFFLE_ON);
	else
		mp_avrcp_noti_shuffle_mode(MP_AVRCP_SHUFFLE_OFF);
#endif

	mp_playlist_mgr_set_shuffle(ad->playlist_mgr, shuffle_enable);
	mp_player_view_update_state(GET_PLAYER_VIEW);
	mp_setting_set_shuffle_state(shuffle_enable);
}

