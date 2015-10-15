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

#include "music.h"
#include "mp-ta.h"
#include "mp-player-control.h"
#include "mp-player-mgr.h"
//#include "mp-player-drm.h"
#include "mp-player-view.h"
#include "mp-item.h"
#include "mp-playlist-mgr.h"
#include "mp-widget.h"
#include "mp-app.h"
#include "mp-streaming-mgr.h"
#include "mp-util.h"
#include "mp-player-debug.h"
#include "mp-minicontroller.h"
#include "mp-lockscreenmini.h"
#include "mp-play.h"
#include "mp-setting-ctrl.h"

#ifdef MP_FEATURE_AVRCP_13
#include "mp-avrcp.h"
#endif

#ifdef MP_FEATURE_CLOUD
#include "mp-cloud.h"
#endif

#define PAUSE_OFF_TIMEOUT			(2 * 60)	// sec

#ifndef MP_SOUND_PLAYER
#include "mp-common.h"
#endif

#ifdef MP_FEATURE_APP_IN_APP
#include "mp-mini-player.h"
#endif

#ifdef MP_FEATURE_CONTEXT_ENGINE
#include "mp-context.h"
#endif

extern bool track_deleted;

static Eina_Bool
_mp_play_set_pos(void *data)
{
	TIMER_TRACE();
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK_FALSE(ad);

	if (ad->is_lcd_off)
	{
		MP_TIMER_FREEZE(ad->live_pos_timer);
		return EINA_TRUE;
	}
	mp_setting_save_playing_info(ad);

	return EINA_TRUE;
}

static Eina_Bool _mp_item_update_db_idler(void *data)
{
	if (!mp_item_update_db(data))
	{
		WARN_TRACE("Error when update db");
	}
	return ECORE_CALLBACK_CANCEL;
}

//this function should be called in player ready state.
bool
mp_play_start_in_ready_state(void *data)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);
	MP_CHECK_FALSE(ad->player_state == PLAY_STATE_READY);

	mp_plst_item * current_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_FALSE(current_item);

	if (!ad->paused_by_user)
	{
		int error = mp_player_mgr_play(ad);
		if (error)
		{
			mp_play_control_on_error(ad, error, true);
			//Don't destory player if play return error because of seek
			if (error != PLAYER_ERROR_INVALID_OPERATION)
				mp_play_destory(ad);
			return FALSE;
		}
	}
	else
	{
		DEBUG_TRACE("stay in pause state..");
		if (track_deleted) {
		        mp_play_start(ad);
		}
		ad->freeze_indicator_icon = true;
		mp_play_stop(ad);
		ad->freeze_indicator_icon = false;

		return false;
	}

	//if (ad->is_focus_out)
	//{
		if (!ad->win_minicon)
			mp_minicontroller_create(ad);
		else if (!ad->b_minicontroller_show)
			mp_minicontroller_show(ad);
#ifdef MP_FEATURE_LOCKSCREEN
		if (!ad->win_lockmini)
		{
			mp_lockscreenmini_create(ad);
		}
		else if (!ad->b_lockmini_show)
		{
			mp_lockscreenmini_show(ad);
		}
#endif
	//}

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode)
		mp_mini_player_refresh(ad);
#endif

	if (current_item->uid) {

		if (current_item->track_type ==  MP_TRACK_URI)
		{
			ecore_idler_add(_mp_item_update_db_idler, current_item->uid);
		}
	}

#ifndef MP_SOUND_PLAYER
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAY_TIME_COUNT_UPDATED);
#endif
	mp_setting_set_nowplaying_id(getpid());

#ifdef MP_3D_FEATURE
	/* Get latest playing info */
	mp_coverflow_view_update(ad, MP_CF_VIEW_UPDATE_NOWPLAYING);
	/* Get latest album */
	mp_coverflow_view_update(ad, MP_CF_VIEW_UPDATE_SELECTED_ALBUM);
#endif
	endfunc;
	return TRUE;
}

int
mp_play_new_file(void *data, bool check_drm)
{
	startfunc;
	struct appdata *ad = data;
	mp_retvm_if (ad == NULL, -1, "appdata is NULL");
	PROFILE_IN("mp_playlist_mgr_get_current");
	mp_plst_item * current_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	PROFILE_OUT("mp_playlist_mgr_get_current");
	MP_CHECK_VAL(current_item, -1);
	int res = 0;

#ifdef MP_FEATURE_CLOUD
	if (current_item->track_type == MP_TRACK_CLOUD)
	{
		char *uri = NULL;
		int ret = mp_cloud_play_available(current_item->uid, current_item);
		if (ret == MP_CLOUD_PLAY_UNAVAILABLE)
		{
			return MP_PLAY_ERROR_NETWORK;
		}
		else if (ret == MP_CLOUD_PLAY_STREAMING)
		{
			return MP_PLAY_ERROR_STREAMING;
		}
		else
		{
			//play offline
			IF_FREE(current_item->uri);
			current_item->uri = uri;
		}
	}
#endif

	PROFILE_IN("mp_player_control_ready_new_file");
	if (mp_util_is_streaming(current_item->uri))
		res = mp_streaming_mgr_play_new_streaming(ad);
	else
		res = mp_player_control_ready_new_file(ad, check_drm);
	PROFILE_OUT("mp_player_control_ready_new_file");

	return res;
}

bool
mp_play_item_play_current_item(void *data)
{
	startfunc;

	MP_CHECK_FALSE(data);

	struct appdata *ad = (struct appdata *)data;
	MP_CHECK_FALSE(ad);

	mp_plst_item * current_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_FALSE(current_item);

	mp_play_destory(ad);
	int ret = mp_play_new_file(ad, TRUE);
	if (ret)
	{
		ERROR_TRACE("Fail to play new file");
		ad->freeze_indicator_icon = false;
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK)
			mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
#endif
		return FALSE;
	}
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYING_TRACK_CHANGED);
	endfunc;

	return TRUE;
}


void
mp_play_prev_file(void *data)
{
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	mp_plst_item *item = mp_playlist_mgr_get_prev(ad->playlist_mgr);
	if (item)
	{
		ad->freeze_indicator_icon = TRUE;

		mp_playlist_mgr_set_current(ad->playlist_mgr, item);
		mp_play_item_play_current_item(ad);
	}
	else
	{
		mp_error("mp_play_list_get_prev_item return false");
#ifdef MP_SOUND_PLAYER
		if (ad->is_focus_out)
			//mp_app_exit(ad);
			DEBUG_TRACE("No playlist and windows focus out");
		else
#endif
		{
			mp_widget_text_popup(data, GET_SYS_STR("IDS_COM_POP_FILE_NOT_FOUND"));
			mp_play_stop_and_updateview(data, FALSE);
		}
	}
}

void
mp_play_next_file(void *data, bool forced)
{
	struct appdata *ad = data;
	mp_plst_item *item = NULL;

	mp_retm_if (ad == NULL, "appdata is NULL");
	MP_CHECK(ad->playlist_mgr);

	int repeat = mp_playlist_mgr_get_repeat(ad->playlist_mgr);

	if (!forced)
	{
		ad->auto_next = true;
		if (repeat == MP_PLST_REPEAT_ONE
			||(repeat == MP_PLST_REPEAT_ALL && mp_playlist_mgr_count(ad->playlist_mgr) == 1))
		{
			DEBUG_TRACE("play same track");
			if (ad->camcoder_start)
			{
				WARN_TRACE("Camera is camcording. unable to play next");
				mp_player_mgr_stop(ad);
				return;
			}
			ad->freeze_indicator_icon = true;
			mp_play_item_play_current_item(ad);
			return;
		}
	}
	else
		ad->auto_next = false;
#ifdef MP_FEATURE_CLOUD
	int i;
	for (i=0; i< mp_playlist_mgr_count(ad->playlist_mgr); i++)
	{
		item = mp_playlist_mgr_get_next(ad->playlist_mgr, forced, true);
		if (item && item->track_type == MP_TRACK_CLOUD)
		{
			if (mp_cloud_play_available(item->uid, NULL) != MP_CLOUD_PLAY_UNAVAILABLE)
				break;
		}
		else
			break;
	}
#else
	item = mp_playlist_mgr_get_next(ad->playlist_mgr, forced, true);
#endif
#ifdef MP_SOUND_PLAYER
	if (ad->is_focus_out) {
		DEBUG_TRACE("No playlist and windows focus out");
	}
#endif
	MpPlayerView_t *player_view = (MpPlayerView_t *)GET_PLAYER_VIEW;

	if (item)
	{
		mp_playlist_mgr_set_current(ad->playlist_mgr, item);

		ad->freeze_indicator_icon = TRUE;

		mp_play_item_play_current_item(ad);
	}
	else
	{
		WARN_TRACE("mp_play_list_get_next_item return false");
		ad->auto_next = false;
		ad->auto_resume = false;
		ad->paused_by_user = true;
		ad->freeze_indicator_icon = true;
		mp_player_mgr_stop(ad);
		mp_player_mgr_destroy(ad);
		ad->freeze_indicator_icon = false;
		mp_setting_set_player_state(MP_PLAY_STATE_PAUSED);
		ad->music_pos = 0;
		mp_player_view_update_progressbar(player_view);
		mp_play_item_play_current_item(ad);

#ifdef MP_SOUND_PLAYER
		if (ad->is_focus_out) {
			//if (!mp_minicontroller_visible_get(ad))
			//	mp_app_exit(ad);
			DEBUG_TRACE("End of playlist and windows focus out");
		}
		else
#endif
		{
			DEBUG_TRACE("End of playlist");
#ifdef MP_FEATURE_AUTO_OFF
			if (mp_playlist_mgr_get_repeat(ad->playlist_mgr)== MP_PLST_REPEAT_NONE
				&& ad->auto_off_timer)
			{
				mp_ecore_timer_del(ad->auto_off_timer);
				mp_setting_reset_auto_off_time();
				mp_app_exit(ad);
			}
#endif
		}
	}

}

void
mp_play_prepare(void  *data)
{
	startfunc;
	struct appdata *ad = data;

	//get duration here for streaming
	ad->music_length = mp_player_mgr_get_duration() / 1000.0;

	//if (ad->is_focus_out)
	//{
        if (!ad->prepare_by_init)
        {
		if (!ad->win_minicon)
			mp_minicontroller_create(ad);
		else if (!ad->b_minicontroller_show)
			mp_minicontroller_show(ad);
#ifdef MP_FEATURE_LOCKSCREEN
		if (!ad->win_lockmini)
		{
			mp_lockscreenmini_create(ad);
		}
		else if (!ad->b_lockmini_show)
		{
			mp_lockscreenmini_show(ad);
		}
#endif
        }

        ad->prepare_by_init = false;

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode)
		mp_mini_player_refresh(ad);
#endif

#ifndef MP_SOUND_PLAYER
        mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_PLAYING_LIST);
	mp_setting_save_now_playing(ad);
	_mp_play_set_pos(ad);
#else
	mp_setting_save_playing_info(ad);
#endif
	/*if (ad->current_track_info)
		mp_setting_write_playing_status(ad->current_track_info->uri, "paused");*/

	if (ad->create_view_on_play)
	{
		mp_view_mgr_push_view_with_effect(GET_VIEW_MGR, (MpView_t *)ad->preload_player_view, NULL, false);
		mp_view_update((MpView_t *)ad->preload_player_view);
		ad->create_view_on_play = false;
		ad->preload_player_view = NULL;
	}
}

static void
_mp_play_start_lazy(void *data)
{
	eventfunc;
	struct appdata *ad = data;
	MP_CHECK(ad);

#ifdef MP_FEATURE_PLAY_SPEED
	mp_player_mgr_set_play_speed(mp_setting_get_play_speed());
#endif

#ifndef MP_SOUND_PLAYER
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_PLAYING_LIST);
	mp_setting_save_now_playing(ad);
	_mp_play_set_pos(ad);
#endif

	mp_plst_item * item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);

	if (!item->uid && !mp_check_file_exist(item->uri))
	{
		char *title = NULL, *album = NULL, * artist = NULL, * genre = NULL;

		mp_player_mgr_get_content_info(&title, &album, &artist, NULL, &genre, NULL);

		if (ad->current_track_info)
		{
			if (ad->current_track_info->title == 0 || strlen(ad->current_track_info->title) == 0)
			{
				if (title == NULL || strlen(title) == 0)
				{
					title = mp_util_get_title_from_path(item->uri);
					DEBUG_TRACE("title from path: %s", title);
				}
				IF_FREE(ad->current_track_info->title);
				ad->current_track_info->title = title;
			}
			else
				IF_FREE(title);

			if (ad->current_track_info->artist == NULL || strlen(ad->current_track_info->artist) == 0)
			{
				IF_FREE(ad->current_track_info->artist);
				ad->current_track_info->artist = artist;
			}
			else
				IF_FREE(artist);

			IF_FREE(ad->current_track_info->album);
			ad->current_track_info->album = album;
			IF_FREE(ad->current_track_info->genre);
			ad->current_track_info->genre = genre;
		}

		mp_player_view_set_title(GET_PLAYER_VIEW);
                //update minicontrol title if title info gotten for streaming play
                mp_minicontroller_update(ad, true);
	}

	if (!ad->live_pos_timer)
		ad->live_pos_timer = ecore_timer_add(1, _mp_play_set_pos, ad);
	mp_util_set_livebox_update_timer();

#ifdef MP_FEATURE_DRM_CONSUMPTION
	mp_drm_start_consumption(item->uri);
#endif
	mp_util_sleep_lock_set(TRUE, FALSE);

	//if (ad->is_focus_out)
	//{
		if (!ad->win_minicon)
			mp_minicontroller_create(ad);
		else
			mp_minicontroller_show(ad);

#ifdef MP_FEATURE_LOCKSCREEN
		if (!ad->win_lockmini)
		{
			mp_lockscreenmini_create(ad);
		}
		else
		{
			mp_lockscreenmini_show(ad);
		}
                mp_setting_save_playing_info(ad);
#endif
	//}

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode)
		mp_mini_player_refresh(ad);
#endif

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_START_PLAYBACK);

	if (ad->current_track_info) {
		if (mp_setting_read_playing_status(ad->current_track_info->uri, "playing") != 1)
				mp_setting_write_playing_status(ad->current_track_info->uri, "playing");
	}
	mp_setting_set_player_state(MP_PLAY_STATE_PLAYING);
	mp_app_grab_mm_keys(ad);

#ifdef MP_FEATURE_AVRCP_13
	mp_avrcp_noti_player_state(MP_AVRCP_STATE_PLAYING);
#endif

#ifdef MP_FEATURE_CONTEXT_ENGINE
	mp_context_log_insert(ad, true);
	mp_context_notify_playback(true, item->uri);
#endif

#ifdef MP_FEATURE_AUTO_OFF
	mp_ecore_timer_del(ad->pause_off_timer);
#endif
	endfunc;
}

/*static Eina_Bool
_mp_play_start_lazy_idler(void *data)
{
	eventfunc;
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	MpView_t *player_view = GET_PLAYER_VIEW;
	if (player_view)
	{
		bool is_transit = false;
		mp_view_is_now_push_transit(player_view, &is_transit);
		if (is_transit)
		{
			WARN_TRACE("now transition.. wait next idle");
			return ECORE_CALLBACK_RENEW;
		}
	}

	ad->create_on_play_lay_idler = NULL;
	_mp_play_start_lazy(ad);

	return ECORE_CALLBACK_DONE;
}*/


void
mp_play_start(void *data)
{
	startfunc;
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	ad->music_length = mp_player_mgr_get_duration() / 1000.0;
	ad->player_state = PLAY_STATE_PLAYING;


#ifndef MP_SOUND_PLAYER
	if (mp_view_mgr_count_view(GET_VIEW_MGR) == 0)
	{
		mp_common_create_initial_view(ad, NULL, NULL);
		evas_object_show(ad->win_main);
		elm_win_iconified_set(ad->win_main, EINA_TRUE);
		elm_win_lower(ad->win_main);
		ad->app_is_foreground = false;
		ad->is_focus_out = true;
	}

	if (ad->create_view_on_play)
	{
		mp_view_mgr_push_view_with_effect(GET_VIEW_MGR, (MpView_t *)ad->preload_player_view, NULL, false);
		mp_view_update((MpView_t *)ad->preload_player_view);
		ad->create_view_on_play = false;
		ad->preload_player_view = NULL;
	}
#endif
	mp_ecore_idler_del(ad->create_on_play_lay_idler);

		_mp_play_start_lazy(ad);


	endfunc;
}

#ifdef MP_FEATURE_AUTO_OFF
static Eina_Bool
_mp_play_control_paused_off_timer_cb(void *data)
{
	TIMER_TRACE();
	struct appdata *ad = data;
	MP_CHECK_VAL(ad, ECORE_CALLBACK_CANCEL);

	if (ad->player_state == PLAY_STATE_PLAYING)
	{
		ad->pause_off_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->is_focus_out && !ad->app_is_foreground)
	{
		ad->pause_off_timer = NULL;
		WARN_TRACE("exit by pause timer");
		if (!mp_util_is_other_player_playing())
		{
			if (preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_OFF) != 0)
			{
				ERROR_TRACE("set Preference failed");
			}
		}
		elm_exit();
	}
	else
	{
		WARN_TRACE("pause off timer but foreground");
		return ECORE_CALLBACK_RENEW;
	}

	return ECORE_CALLBACK_DONE;
}
#endif

void
mp_play_pause(void *data)
{
	startfunc;
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	MP_TIMER_FREEZE(ad->live_pos_timer);

	ad->player_state = PLAY_STATE_PAUSED;

	if (ad->b_minicontroller_show)
	{
		mp_minicontroller_update_control(ad);
	}
#ifdef MP_FEATURE_LOCKSCREEN
	if (ad->b_lockmini_show)
	{
		mp_lockscreenmini_update_control(ad);
	}
#endif


	if (!ad->paused_by_other_player)
		preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_PAUSE);

#ifdef MP_FEATURE_DRM_CONSUMPTION
	mp_drm_pause_consumption();
#endif
	mp_util_sleep_lock_set(FALSE, FALSE);

	if (ad->win_minicon)
		mp_minicontroller_update_control(ad);

#ifdef MP_FEATURE_LOCKSCREEN
	if (ad->win_lockmini)
	{
		mp_lockscreenmini_update_control(ad);
	}
        mp_setting_save_playing_info(ad);
#endif

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode)
		mp_mini_player_refresh(ad);
#endif

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PAUSE_PLAYBACK);

	if (ad->current_track_info) {
		if (mp_setting_read_playing_status(ad->current_track_info->uri, "paused") != 1)
			mp_setting_write_playing_status(ad->current_track_info->uri, "paused");
	}
	mp_setting_set_player_state(MP_PLAY_STATE_PAUSED);

	ad->player_state = PLAY_STATE_PAUSED;
	ad->paused_by_other_player = FALSE;

#ifdef MP_FEATURE_AUTO_OFF
	mp_ecore_timer_del(ad->pause_off_timer);
	mp_debug("pause off timer set");
	ad->pause_off_timer = ecore_timer_add(PAUSE_OFF_TIMEOUT, _mp_play_control_paused_off_timer_cb, ad);
#endif

#ifdef MP_FEATURE_AVRCP_13
	mp_avrcp_noti_player_state(MP_AVRCP_STATE_PAUSED);
#endif

	endfunc;
}

void
mp_play_stop(void *data)
{
	startfunc;
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	MP_TIMER_FREEZE(ad->live_pos_timer);

	ad->player_state = PLAY_STATE_READY;
	ad->music_pos = 0;

	if (!ad->freeze_indicator_icon)
	{
		if (!mp_util_is_other_player_playing())
			preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_STOP);

		mp_setting_set_player_state(MP_PLAY_STATE_STOP);
	}
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_STOP_PLAYBACK);

	if (mp_minicontroller_visible_get(ad))
	{
		mp_minicontroller_update_control(ad);
	}
#ifdef MP_FEATURE_LOCKSCREEN
	if (mp_lockscreenmini_visible_get(ad))
	{
		mp_lockscreenmini_update_control(ad);
	}
        mp_setting_save_playing_info(ad);
#endif
#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode)
		mp_mini_player_refresh(ad);
#endif

#ifdef MP_FEATURE_DRM_CONSUMPTION
	mp_drm_stop_consumption();
	mp_drm_set_consumption(FALSE);
#endif
	mp_util_sleep_lock_set(FALSE, FALSE);

	if (ad->current_track_info) {
		if (mp_setting_read_playing_status(ad->current_track_info->uri, "stop") != 1)
			mp_setting_write_playing_status(ad->current_track_info->uri, "stop");
	}
	mp_setting_set_player_state(MP_PLAY_STATE_STOP);
	mp_player_view_update_buffering_progress(GET_PLAYER_VIEW, -1);

#ifdef MP_FEATURE_AVRCP_13
	mp_avrcp_noti_player_state(MP_AVRCP_STATE_STOPPED);
#endif

#ifdef MP_FEATURE_CONTEXT_ENGINE
	mp_context_log_insert(ad, false);
	mp_context_notify_playback(false, NULL);
#endif

	endfunc;
}

void
mp_play_resume(void *data)
{
	startfunc;
	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	if (!ad->live_pos_timer)
		ad->live_pos_timer = ecore_timer_add(1, _mp_play_set_pos, ad);
	mp_util_set_livebox_update_timer();

	ad->player_state = PLAY_STATE_PLAYING;

	preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_PLAY);

#ifdef MP_FEATURE_DRM_CONSUMPTION
	mp_drm_resume_consumption();
#endif
	mp_util_sleep_lock_set(TRUE, FALSE);

        if (!ad->win_minicon)
                mp_minicontroller_create(ad);
        else
                mp_minicontroller_show(ad);

        if (ad->b_minicontroller_show)
                mp_minicontroller_update_control(ad);

#ifdef MP_FEATURE_LOCKSCREEN
		if (!ad->win_lockmini)
		{
                mp_lockscreenmini_create(ad);
		}
        else
        {
                mp_lockscreenmini_show(ad);
        }

		if (ad->b_lockmini_show)
		{
                mp_lockscreenmini_update_control(ad);
		}
                mp_setting_save_playing_info(ad);
#endif

#ifdef MP_FEATURE_APP_IN_APP
	if (ad->mini_player_mode)
		mp_mini_player_refresh(ad);
#endif

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_RESUME_PLAYBACK);
	mp_app_grab_mm_keys(ad);

	if (ad->current_track_info)
	{
		if (mp_setting_read_playing_status(ad->current_track_info->uri, "playing") != 1)
			mp_setting_write_playing_status(ad->current_track_info->uri, "playing");
	}
	ad->freeze_indicator_icon = false;
	mp_setting_set_player_state(MP_PLAY_STATE_PLAYING);

#ifdef MP_FEATURE_AVRCP_13
	mp_avrcp_noti_player_state(MP_AVRCP_STATE_PLAYING);
#endif

	ad->auto_resume = false;

#ifdef MP_FEATURE_AUTO_OFF
	mp_ecore_timer_del(ad->pause_off_timer);
#endif
	endfunc;
}

bool
mp_play_destory(void *data)
{
	startfunc;
	struct appdata *ad = data;
	mp_retvm_if (ad == NULL, FALSE, "appdata is NULL");

	if (ad->current_track_info) {
		if (mp_setting_read_playing_status(ad->current_track_info->uri, "stop") != 1)
			mp_setting_write_playing_status(ad->current_track_info->uri, "stop");
	}

	mp_player_mgr_stop(ad);
	mp_player_mgr_unrealize(ad);
	mp_player_mgr_destroy(ad);

	ad->music_pos = 0.;
	ad->music_length = 0.;
	endfunc;
	return TRUE;
}

bool
mp_play_fast_destory(void *data)
{
	startfunc;
	struct appdata *ad = data;
	mp_retvm_if (ad == NULL, FALSE, "appdata is NULL");

	mp_player_mgr_destroy(ad);

	ad->music_pos = 0.;
	ad->music_length = 0.;
	endfunc;
	return TRUE;
}

void
mp_play_stop_and_updateview(void *data, bool mmc_removed)
{
	startfunc;

	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	if (ad->player_state != PLAY_STATE_NONE)
	{
		DEBUG_TRACE("mp_play_stop_and_updateview");
		ad->freeze_indicator_icon = false;
		mp_play_destory(ad);
	}

#ifndef MP_SOUND_PLAYER
	mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_UNSET_NOW_PLAYING);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PAUSE_PLAYBACK);
	mp_view_update(mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ALL));
#ifdef MP_3D_FEATURE
	if (ad->coverflow_view && ad->coverflow_view->back_button)
		mp_coverflow_view_back_2d_view(ad);
#endif

#endif

        if (ad->b_minicontroller_show)
                mp_minicontroller_hide(ad);

#ifdef MP_FEATURE_LOCKSCREEN
		if (ad->b_lockmini_show)
		{
                mp_lockscreenmini_hide(ad);
		}
#endif

	return;
}

void
mp_play_next_and_updateview(void *data)
{
	startfunc;

	struct appdata *ad = data;
	mp_retm_if (ad == NULL, "appdata is NULL");

	mp_play_next_file(ad, true);
#ifdef MP_SOUND_PLAYER
	mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL);
#endif
        if (ad->b_minicontroller_show)
                mp_minicontroller_hide(ad);

#ifdef MP_FEATURE_LOCKSCREEN
	if (ad->b_lockmini_show)
	{
		mp_lockscreenmini_hide(ad);
	}
#endif

	return;
}

