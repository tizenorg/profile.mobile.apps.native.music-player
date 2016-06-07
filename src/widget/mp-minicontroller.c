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

#include <sys/time.h>
#include <minicontrol-provider.h>
#include <minicontrol-type.h>
#include "mp-minicontroller.h"
#include "mp-player-control.h"
#include "mp-play.h"
#include "Ecore.h"
#include "mp-player-mgr.h"
#include "mp-util.h"
#include "mp-widget.h"
#include "mp-setting-ctrl.h"
#include "mp-player-view.h"
#include <device/display.h>
#include <device/callback.h>

#define MINI_CONTROLLER_HEIGHT (93)
#define WL_INDI_H 27		//Window Layout Indicator Height
#define PAUSE_TIME_OUT 120.

#define CTR_EDJ_SIG_SRC "ctrl_edj"
#define CTR_PROG_SIG_SRC "ctrl_prog"

#define BUFFER_MAX		256


Evas_Coord xD, yD, xU, yU, xDMove, yDMove;

static time_t press_time;
static time_t release_time;
int MINI_CONTROLLER_WIDTH;
int MINI_CONTROLLER_WIDTH_LANDSCAPE;

static void _minicontroller_action_cb(void *data, Evas_Object * obj, const char *emission, const char *source);
static Evas_Object *_load_edj(Evas_Object * parent, const char *file, const char *group);
static void _load_minicontroller(struct appdata *ad);
#ifdef MINICONTROLLER_ENABLE_PROGRESS
static void _mp_minicontroller_progress_val_set(struct appdata *ad, double position);
#endif
static void _mp_minicontroller_update_layout(struct appdata *ad, bool landscape);
#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
static void _mp_minicontroller_set_repeate_image(void *data, int repeate_state);
static void _mp_minicontroller_set_shuffle_image(void *data, int shuffle_state);
#endif
static void _mp_minicontroller_title_set(struct appdata *ad);

static bool _mp_minicontroller_is_long_press()
{
	bool result = false;
	DEBUG_TRACE("press time is %s", ctime(&press_time));
	DEBUG_TRACE("release time is %s", ctime(&release_time));
	if (difftime(release_time, press_time) > 1.0) {
		result = true;
	}

	memset(&release_time, 0, sizeof(time_t));
	memset(&press_time, 0, sizeof(time_t));

	DEBUG_TRACE("is %s long press", result ? "" : "not");
	return result;
}

#ifdef MINICONTROLLER_ENABLE_PROGRESS
static inline void
_mp_minicontroller_update_elapsed_time(struct appdata *ad, bool get_position)
{
	MP_CHECK(ad);
	MP_CHECK(ad->minicontroller_layout);

	double played_ratio = 0.;
	double music_pos = 0.;
	if (get_position) {
		music_pos = mp_player_mgr_get_position() / 1000;
	} else {
		music_pos = ad->music_pos;
	}
	/*
		int sec = mp_player_mgr_get_position() / 1000;
		int min = sec / 60;
		sec = sec % 60;


		char *time_text = g_strdup_printf("%02d:%02d", min, sec);
		if (time_text) {
			edje_object_part_text_set(_EDJ(ad->minicontroller_layout), "elm.elapsed_time", time_text);
			free(time_text);
			time_text = NULL;
		}
	*/
	char play_time[16] = { 0, };
	char total_time[16] = { 0, };

	double duration = ad->music_length;

	if (duration > 0.) {
		if (duration > 3600.) {
			snprintf(total_time, sizeof(total_time), "%" MUSIC_TIME_FORMAT,
			         MUSIC_TIME_ARGS(duration));
			snprintf(play_time, sizeof(play_time), "%" MUSIC_TIME_FORMAT, MUSIC_TIME_ARGS(music_pos));
		} else {
			snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
			         PLAY_TIME_ARGS(duration));
			snprintf(play_time, sizeof(play_time), "%" PLAY_TIME_FORMAT, PLAY_TIME_ARGS(music_pos));
		}
	} else {
		if (ad->current_track_info)
			snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
			         PLAY_TIME_ARGS(ad->current_track_info->duration / 1000.));
		snprintf(play_time, sizeof(play_time), "%" PLAY_TIME_FORMAT, PLAY_TIME_ARGS(music_pos));
	}

	edje_object_part_text_set(_EDJ(ad->minicontroller_layout), "np_progress_text_total", total_time);
	edje_object_part_text_set(_EDJ(ad->minicontroller_layout), "np_progress_text_playing", play_time);

	if (ad->music_length > 0. && music_pos > 0.) {
		played_ratio = music_pos / ad->music_length;
	}
	_mp_minicontroller_progress_val_set(ad, played_ratio);
}

static Eina_Bool
_minicontroller_update_progresstime_cb(void *data)
{
	TIMER_TRACE();
	struct appdata *ad = data;
	mp_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	if (ad->is_lcd_off) {
		mp_ecore_timer_del(ad->minicon_progress_timer);
		return ECORE_CALLBACK_CANCEL;
	}

	if (ad->player_state == PLAY_STATE_PLAYING) {
		_mp_minicontroller_update_elapsed_time(ad, true);
	}

	return ECORE_CALLBACK_RENEW;
}

static void
_minicontroller_progress_timer_add(void *data)
{
	struct appdata *ad = data;
	mp_retm_if(ad == NULL, "appdata is NULL");
	DEBUG_TRACE();

	mp_ecore_timer_del(ad->minicon_progress_timer);

	_mp_minicontroller_update_elapsed_time(ad, true);

	if (ad->player_state == PLAY_STATE_PLAYING) {
		ad->minicon_progress_timer = ecore_timer_add(1.0, _minicontroller_update_progresstime_cb, ad);
	}

}
#endif
static bool
_mp_minicontroller_landscape_is(struct appdata *ad, int angle)
{
	startfunc;
	MP_CHECK_FALSE(ad);
	MP_CHECK_FALSE(ad->win_minicon);

	bool landscape  = false;
	if (angle == 90 || angle == 270) {
		landscape = true;
	} else {
		landscape = false;
	}
	return landscape;
}

#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
static char * _mp_minicontroller_shuffle_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("shuffle button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;
	int shuffle_state = 0;
	mp_setting_get_shuffle_state(&shuffle_state);

	if (shuffle_state == 1) {
		operation_txt = GET_SYS_STR(MP_TTS_SHUFFLE_OFF_BUTTON);
	} else {
		operation_txt = GET_SYS_STR(MP_TTS_SHUFFLE_ON_BUTTON);
	}

	return g_strdup(operation_txt);
}
#endif

#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
static char * _mp_minicontroller_repeat_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("repeat button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;
	int repeat_state = 0;
	mp_setting_get_repeat_state(&repeat_state);
	if (MP_PLST_REPEAT_ONE == repeat_state) {
		operation_txt = GET_SYS_STR(MP_TTS_REPEAT_ONE_BUTTON);
	} else if (MP_PLST_REPEAT_NONE == repeat_state) {
		operation_txt = GET_SYS_STR(MP_TTS_REPEAT_OFF_BUTTON);
	} else {
		//repeat all
		operation_txt = GET_SYS_STR(MP_TTS_REPEAT_ALL_BUTTON);
	}
	return g_strdup(operation_txt);
}
#endif

static void
_mp_minicontroller_action_show_player_view(struct appdata *ad)
{
	startfunc;
	MP_CHECK(ad);
#ifndef MP_SOUND_PLAYER
	if (GET_PLAYER_VIEW != mp_view_mgr_get_top_view(GET_VIEW_MGR)) {
		if (!ad->is_focus_out) {
			minicontrol_send_event(ad->win_minicon, MINICONTROL_EVENT_REQUEST_HIDE, NULL);
		} else {
			mp_util_app_resume();
		}
		mp_common_show_player_view(MP_PLAYER_NORMAL, true, false, true);
	} else
#endif
	{
		if (!ad->is_focus_out) {
			minicontrol_send_event(ad->win_minicon, MINICONTROL_EVENT_REQUEST_HIDE, NULL);
		} else {
			mp_util_app_resume();
		}
	}
}

#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
static void _mp_minicontroller_set_shuffle_image(void *data, int shuffle_state)
{
	struct appdata *ad = (struct appdata*)data;
	MP_CHECK(ad);
	MP_CHECK(ad->minicontroller_layout);
	ERROR_TRACE("");
	bool landscape = _mp_minicontroller_landscape_is(ad, ad->quickpanel_angle);
	if (landscape) {
		if (shuffle_state) {
			elm_object_signal_emit(ad->minicontroller_layout, "set_shuffle_on_ld", "*");
		} else {
			elm_object_signal_emit(ad->minicontroller_layout, "set_shuffle_off_ld", "*");
		}
	} else {
		if (shuffle_state) {
			elm_object_signal_emit(ad->minicontroller_layout, "set_shuffle_on", "*");
		} else {
			elm_object_signal_emit(ad->minicontroller_layout, "set_shuffle_off", "*");
		}
	}
}
static void _mp_minicontroller_set_repeate_image(void *data, int repeate_state)
{
	struct appdata *ad = (struct appdata*)data;
	MP_CHECK(ad);
	MP_CHECK(ad->minicontroller_layout);
	ERROR_TRACE("");
	bool landscape = _mp_minicontroller_landscape_is(ad, ad->quickpanel_angle);
	if (landscape) {
		if (MP_PLST_REPEAT_ONE == repeate_state) {
			elm_object_signal_emit(ad->minicontroller_layout, "set_repeat_btn_1_ld", "*");
		} else if (MP_PLST_REPEAT_NONE == repeate_state) {
			elm_object_signal_emit(ad->minicontroller_layout, "set_repeat_btn_a_ld", "*");
		} else {
			elm_object_signal_emit(ad->minicontroller_layout, "set_repeat_btn_all_ld", "*");
		}
	} else {
		if (MP_PLST_REPEAT_ONE == repeate_state) {
			elm_object_signal_emit(ad->minicontroller_layout, "set_repeat_btn_1", "*");
		} else if (MP_PLST_REPEAT_NONE == repeate_state) {
			elm_object_signal_emit(ad->minicontroller_layout, "set_repeat_btn_a", "*");
		} else {
			elm_object_signal_emit(ad->minicontroller_layout, "set_repeat_btn_all", "*");
		}
	}

}
#endif

static void
_minicontroller_action_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	struct appdata *ad = (struct appdata *)data;
	mp_retm_if(ad == NULL, "appdata is NULL");
	//EVENT_TRACE("emission: %s", emission);
	if (emission) {
		if (!g_strcmp0(emission, "close_btn_clicked")) {
			EVENT_TRACE("CLOSE");
			if (!mp_util_is_other_player_playing()) {
				int ret_set = 0;
				ret_set = preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_OFF);
				if (ret_set) {
					ERROR_TRACE("set preference failed");
				}
			}
			elm_exit();
		} else if (!g_strcmp0(emission, "albumart_clicked")) {
			EVENT_TRACE("albumart");
			_mp_minicontroller_action_show_player_view(ad);
			return;
		}
	}

}

static Evas_Object *
_load_edj(Evas_Object * parent, const char *file, const char *group)
{
	Evas_Object *eo;
	int r;

	eo = elm_layout_add(parent);
	if (eo) {
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", file);

		r = elm_layout_file_set(eo, edje_path, group);
		free(path);
		if (!r) {
			evas_object_del(eo);
			return NULL;
		}
		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_win_resize_object_add(parent, eo);
		evas_object_show(eo);
	}

	return eo;
}

static void _quick_panel_cb(minicontrol_viewer_event_e event_type, bundle *event_arg)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	minicontrol_viewer_event_e event_hide = (minicontrol_viewer_event_e)MINICONTROL_EVENT_REQUEST_HIDE;
	if (event_type == event_hide) {
		DEBUG_TRACE("CLOSE");
		if (!mp_util_is_other_player_playing()) {
			int ret_set = 0;
			ret_set = preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_OFF);
			if (ret_set) {
				ERROR_TRACE("set preference failed");
			}
		}
		mp_play_control_reset_ff_rew();
		xD = 0;
		yD = 0;
		xDMove = 0;
		yDMove = 0;
		xU = 0;
		yU = 0;
		if (!ad->is_sdcard_removed) {
			elm_exit();
		}
		ad->is_sdcard_removed = false;
	}
	if (event_type == (minicontrol_viewer_event_e)MINICONTROL_VIEWER_EVENT_REPORT_ANGLE) {
		mp_minicontroller_rotate(ad, ad->quickpanel_angle);
	}
}

static void
_load_minicontroller(struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(ad);
	Evas_Object *win = NULL;

#ifndef MP_SOUND_PLAYER
	win = minicontrol_create_window("musicplayer-mini", MINICONTROL_TARGET_VIEWER_QUICK_PANEL, _quick_panel_cb);

#else
	win = minicontrol_create_window("soundplayer-mini", MINICONTROL_TARGET_VIEWER_QUICK_PANEL, _quick_panel_cb);
#endif

	if (!win) {
		return;
	}

	elm_win_alpha_set(win, EINA_TRUE);

	ad->win_minicon = win;

	/* load edje */
	bool landscape = _mp_minicontroller_landscape_is(ad, ad->quickpanel_angle);
	_mp_minicontroller_update_layout(ad, landscape);

	//evas_object_show(eo);

	return;
}

static void
_mp_minicontroller_register_reader(void *data)
{
	struct appdata *ad = data;
	MP_CHECK(ad);
}

#ifdef MINICONTROLLER_ENABLE_PROGRESS

static void
_mp_minicontroller_progess_box_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK(ad->progress_box);

	ad->progress_box = NULL;
}

static void
_mp_minicontroller_progress_val_set(struct appdata *ad, double position)
{
	MP_CHECK(ad);
	MP_CHECK(ad->minicontroller_layout);

	if (ad->progress_bar) {
		edje_object_part_drag_value_set(_EDJ(ad->progress_bar), "progressbar_playing", position, 0.0);
	}
	return;
}

static void _mp_minicontroller_create_progress_layout(struct appdata *ad)
{
	startfunc;

	ad->progress_box = mp_common_load_edj(ad->minicontroller_layout, MINICON_EDJ_NAME, "minicontroller_progress_box");
	MP_CHECK(ad->progress_box);
	evas_object_event_callback_add(ad->progress_box, EVAS_CALLBACK_DEL, _mp_minicontroller_progess_box_del_cb, ad);
	elm_object_part_content_set(ad->minicontroller_layout, "progress_box", ad->progress_box);

	ad->progress_bar = mp_common_load_edj(ad->progress_box, MINICON_EDJ_NAME, "mc_player_progressbar");
	MP_CHECK(ad->progress_bar);
	elm_object_part_content_set(ad->progress_box, "progress_bar", ad->progress_bar);
	_mp_minicontroller_progress_val_set(ad, 0.0);
}
#endif

void
mp_minicontroller_update_winmini_size(struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(ad);

	int angle = ad->quickpanel_angle;
	int w = 0;
	if ((elm_config_scale_get() - 1.7) < 0.0001) {
		MINI_CONTROLLER_WIDTH = 318;
		MINI_CONTROLLER_WIDTH_LANDSCAPE = 564;
	} else if ((elm_config_scale_get() - 1.8) < 0.0001) {
		MINI_CONTROLLER_WIDTH = 267;
		MINI_CONTROLLER_WIDTH_LANDSCAPE = 444;
	} else if ((elm_config_scale_get() - 2.4) < 0.0001) {
		MINI_CONTROLLER_WIDTH = 300;
		MINI_CONTROLLER_WIDTH_LANDSCAPE = 533;
	} else if ((elm_config_scale_get() - 2.6) < 0.0001) {
		MINI_CONTROLLER_WIDTH = 277;
		MINI_CONTROLLER_WIDTH_LANDSCAPE = 492;
	} else if ((elm_config_scale_get() - 2.8) < 0.0001) {
		MINI_CONTROLLER_WIDTH = 257;
		MINI_CONTROLLER_WIDTH_LANDSCAPE = 457;
	}

	if (angle == 90 || angle == 270) {
		w = MINI_CONTROLLER_WIDTH_LANDSCAPE;
	} else {
		w = MINI_CONTROLLER_WIDTH;
	}

	double scale = elm_config_scale_get();
	DEBUG_TRACE("scale: %f and width: %d and height: %d", scale, w, MINI_CONTROLLER_HEIGHT);
	evas_object_resize(ad->win_minicon, w * scale, MINI_CONTROLLER_HEIGHT * scale);

	return;
}

int
mp_minicontroller_create(struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");
	//MP_CHECK_VAL(!ad->is_lcd_off, -1);

	if (!(ad->minicontroller_layout && ad->win_minicon)) {

		_load_minicontroller(ad);
		if (ad->minicontroller_layout == NULL) {
			DEBUG_TRACE("ERROR");
			return -1;
		}

		//mp_language_mgr_register_object(ad->minicontroller_layout, OBJ_TYPE_EDJE_OBJECT, "elm.text.app_name", "IDS_COM_BODY_MUSIC");
		//elm_object_part_text_set(ad->minicontroller_layout, "elm.text.app_name", GET_SYS_STR("IDS_COM_BODY_MUSIC"));
	}

	mp_minicontroller_update_winmini_size(ad);

	mp_minicontroller_show(ad);
	_mp_minicontroller_title_set(ad);

	return 0;
}


int
mp_minicontroller_show(struct appdata *ad)
{
	DEBUG_TRACE("minicontroller view show!!");
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");
	MP_CHECK_VAL(ad->win_minicon, -1);
	MP_CHECK_VAL(!ad->is_lcd_off, -1);
	/* Not show minicontrol when current track not exsit */
	MP_CHECK_VAL(ad->current_track_info, -1);

	ad->b_minicontroller_show = TRUE;
	mp_minicontroller_update(ad, false);
	evas_object_show(ad->win_minicon);
	return 0;

}

static void _mp_minicontroller_update_btn(struct appdata *ad)
{
	startfunc;
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_minicon);
	MP_CHECK(!ad->is_lcd_off);

	if (ad->player_state == PLAY_STATE_PLAYING) {
		elm_object_signal_emit(ad->minicontroller_layout, "set_pause", "c_source");
	} else {
		elm_object_signal_emit(ad->minicontroller_layout, "set_play", "c_source");
	}

}

static Eina_Bool
_mp_minicontroller_btn_update_timer(void *data)
{
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	_mp_minicontroller_update_btn(data);

	ad->minicon_button_timer = NULL;
	return EINA_FALSE;
}

static void _mp_minicontroller_update_playpause_btn(struct appdata *ad)
{
	mp_ecore_timer_del(ad->minicon_button_timer);

	if (ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED) {
		_mp_minicontroller_update_btn(ad);
	} else {
		ad->minicon_button_timer = ecore_timer_add(1.0, _mp_minicontroller_btn_update_timer, ad);
	}
}

void
mp_minicontroller_update_control(struct appdata *ad)
{
	startfunc;
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_minicon);
	MP_CHECK(!ad->is_lcd_off);

	_mp_minicontroller_update_playpause_btn(ad);
}

void
mp_minicontroller_update_shuffle_and_repeat_btn(struct appdata *ad)
{
#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
	startfunc;
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_minicon);
	MP_CHECK(!ad->is_lcd_off);

	int shuffle_state = 0;
	int repeat_state = 0;
	mp_setting_get_shuffle_state(&shuffle_state);
	mp_setting_get_repeat_state(&repeat_state);

	_mp_minicontroller_set_shuffle_image((void *)ad, shuffle_state);
	_mp_minicontroller_set_repeate_image((void *)ad, repeat_state);
#endif
}

#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
/*focused UI callbacks*/
static void _mp_minicontroller_shuffle_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("shuffle button clicked");

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	int shuffle_state = 0;
	mp_setting_get_shuffle_state(&shuffle_state);
	shuffle_state = !shuffle_state;
	mp_play_control_shuffle_set(ad, shuffle_state);
	_mp_minicontroller_set_shuffle_image(ad, shuffle_state);
}
#endif
static void _mp_minicontroller_play_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("play/pause button clicked");

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (ad->player_state == PLAY_STATE_PLAYING) {
		mp_play_control_play_pause(ad, false);
	} else {
		mp_play_control_play_pause(ad, true);
	}
}

#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
static void _mp_minicontroller_repeat_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("repeat button clicked");

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	int repeat_state = 0;
	mp_setting_get_repeat_state(&repeat_state);
	repeat_state++;
	repeat_state %= 3;
	mp_setting_set_repeat_state(repeat_state);
	_mp_minicontroller_set_repeate_image(ad, repeat_state);
	mp_playlist_mgr_set_repeat(ad->playlist_mgr, repeat_state);
	mp_player_view_update_state(GET_PLAYER_VIEW);
}
#endif

static void _mp_minicontroller_ff_rew_btn_pressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button pressed");
	time(&press_time);
	char *source = (char *)data;
	if (!g_strcmp0(source, CONTROLLER_FF_SOURCE)) {
		mp_play_control_ff(true, false, false);
	} else {
		mp_play_control_rew(true, false, false);
	}
}

static void _mp_minicontroller_ff_rew_btn_unpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button unpressed");
	time(&release_time);
	char *source = (char *)data;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->minicontroller_layout);
	if (!g_strcmp0(source, CONTROLLER_FF_SOURCE)) {
		mp_play_control_ff(false, false, false);
		elm_object_signal_emit(ad->minicontroller_layout, "ff_btn_unpressed", "c_source");
	} else {
		mp_play_control_rew(false, false, false);
		elm_object_signal_emit(ad->minicontroller_layout, "rew_btn_unpressed", "c_source");
	}
}

static void _mp_minicontroller_ff_rew_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button clicked");
	if (_mp_minicontroller_is_long_press()) {
		return;
	}

	char *source = (char *)data;
	if (!g_strcmp0(source, CONTROLLER_FF_SOURCE)) {
		mp_play_control_ff(false, false, true);
	} else {
		mp_play_control_rew(false, false, true);
	}
}

/*end of focused UI callbacks*/

static void
_mp_minicontroller_ff_rew_btn_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;
	mp_play_control_reset_ff_rew();
}

static void
_mp_minicontroller_update_layout(struct appdata *ad, bool landscape)
{
	MP_CHECK(ad);

	if (ad->minicon_progress_timer) {
		ecore_timer_del(ad->minicon_progress_timer);
		ad->minicon_progress_timer = NULL;
	}
	mp_ecore_timer_del(ad->minicon_button_timer);

	mp_evas_object_del(ad->minicontroller_layout);

	if (landscape) {
		DEBUG_TRACE("angle: 90 or 270");
		ad->minicontroller_layout = _load_edj(ad->win_minicon, MINICON_EDJ_NAME, "music-minicontroller-ld");
	} else {
		DEBUG_TRACE("angel: 0");
		ad->minicontroller_layout = _load_edj(ad->win_minicon, MINICON_EDJ_NAME, "music-minicontroller");
	}

	if (!ad->minicontroller_layout) {
		return ;
	}

	elm_win_resize_object_add(ad->win_minicon, ad->minicontroller_layout);

	/*add focused UI related*/
	ad->minicon_icon = elm_image_add(ad->minicontroller_layout);
	evas_object_size_hint_align_set(ad->minicon_icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(ad->minicon_icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_image_fill_outside_set(ad->minicon_icon, true);
	elm_object_part_content_set(ad->minicontroller_layout, "albumart_image", ad->minicon_icon);
	elm_object_focus_custom_chain_append(ad->minicontroller_layout, ad->minicon_icon, NULL);
	elm_object_focus_allow_set(ad->minicon_icon, EINA_TRUE);

#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
	/*-------> shuffle button ------->*/
	Evas_Object *shuffle_focus_btn = elm_button_add(ad->minicontroller_layout);
	elm_object_style_set(shuffle_focus_btn, "focus");
	elm_object_part_content_set(ad->minicontroller_layout, "shuffle_focus", shuffle_focus_btn);
	elm_object_focus_custom_chain_append(ad->minicontroller_layout, shuffle_focus_btn, NULL);
	evas_object_smart_callback_add(shuffle_focus_btn, "clicked", _mp_minicontroller_shuffle_btn_clicked_cb, NULL);
#endif

	/*-------> REW button ------->*/
	Evas_Object *rew_focus_btn = elm_button_add(ad->minicontroller_layout);
	elm_object_style_set(rew_focus_btn, "focus");
	elm_object_part_content_set(ad->minicontroller_layout, "rew_btn_focus", rew_focus_btn);

	elm_object_focus_custom_chain_append(ad->minicontroller_layout, rew_focus_btn, NULL);
	evas_object_smart_callback_add(rew_focus_btn, "clicked", _mp_minicontroller_ff_rew_btn_clicked_cb, CONTROLLER_REW_SOURCE);
	evas_object_smart_callback_add(rew_focus_btn, "pressed", _mp_minicontroller_ff_rew_btn_pressed_cb, CONTROLLER_REW_SOURCE);
	evas_object_smart_callback_add(rew_focus_btn, "unpressed", _mp_minicontroller_ff_rew_btn_unpressed_cb, CONTROLLER_REW_SOURCE);
	evas_object_event_callback_add(rew_focus_btn, EVAS_CALLBACK_DEL, _mp_minicontroller_ff_rew_btn_del_cb, NULL);

	/*-------> play/pause button ------->*/
	Evas_Object *play_pause_focus_btn = elm_button_add(ad->minicontroller_layout);
	elm_object_style_set(play_pause_focus_btn, "focus");
	elm_object_part_content_set(ad->minicontroller_layout, "play_pause_focus", play_pause_focus_btn);
	elm_object_focus_custom_chain_append(ad->minicontroller_layout, play_pause_focus_btn, NULL);
	evas_object_smart_callback_add(play_pause_focus_btn, "clicked", _mp_minicontroller_play_pause_btn_clicked_cb, NULL);

	/*------->FF button ------->*/
	Evas_Object *ff_focus_btn = elm_button_add(ad->minicontroller_layout);
	elm_object_style_set(ff_focus_btn, "focus");
	elm_object_part_content_set(ad->minicontroller_layout, "ff_btn_focus", ff_focus_btn);

	elm_object_focus_custom_chain_append(ad->minicontroller_layout, ff_focus_btn, NULL);
	evas_object_smart_callback_add(ff_focus_btn, "clicked", _mp_minicontroller_ff_rew_btn_clicked_cb, CONTROLLER_FF_SOURCE);
	evas_object_smart_callback_add(ff_focus_btn, "pressed", _mp_minicontroller_ff_rew_btn_pressed_cb, CONTROLLER_FF_SOURCE);
	evas_object_smart_callback_add(ff_focus_btn, "unpressed", _mp_minicontroller_ff_rew_btn_unpressed_cb, CONTROLLER_FF_SOURCE);
	evas_object_event_callback_add(ff_focus_btn, EVAS_CALLBACK_DEL, _mp_minicontroller_ff_rew_btn_del_cb, NULL);

#ifdef MINICONTROLLER_ENABLE_SHUFFLLE_REPEAT
	/*-------> repeat button ------->*/
	Evas_Object *repeat_focus_btn = elm_button_add(ad->minicontroller_layout);
	elm_object_style_set(repeat_focus_btn, "focus");
	elm_object_part_content_set(ad->minicontroller_layout, "repeat_focus", repeat_focus_btn);
	elm_object_focus_custom_chain_append(ad->minicontroller_layout, repeat_focus_btn, NULL);
	evas_object_smart_callback_add(repeat_focus_btn, "clicked", _mp_minicontroller_repeat_btn_clicked_cb, NULL);
#endif

	/*-------> close button ------->*/
	edje_object_signal_callback_add(_EDJ(ad->minicontroller_layout), "*", "*", _minicontroller_action_cb, ad);
	_mp_minicontroller_update_btn(ad);
	_mp_minicontroller_register_reader(ad);
}

static void
_mp_minicontroller_title_set(struct appdata *ad)
{
	MP_CHECK(ad);
	MP_CHECK(ad->minicontroller_layout);

	Evas_Object *label = elm_object_part_content_get(ad->minicontroller_layout, "elm.text");

	mp_track_info_t *current_item = ad->current_track_info;
	MP_CHECK(current_item);

	int r = 0;
	int g = 0;
	int b = 0;
	int a = 255;

	char *markup_title = elm_entry_utf8_to_markup(current_item->title);
	char *markup_artist = elm_entry_utf8_to_markup(current_item->artist);

	char *title_format = "<align=left><font_size=%d><color=#%02x%02x%02x%02x>%s - </color></font_size><font_size=%d><color=#%02x%02x%02x%02x>%s</color></font_size></align>";
	char *title = NULL;
	if ((markup_title == NULL || strlen(markup_title) == 0)
	        && (markup_artist == NULL || strlen(markup_artist) == 0)) {
		title = NULL;
	} else {
		title = g_strdup_printf(title_format, 24, r, g, b, a, markup_title, 24, r, g, b, a, markup_artist);
	}

	if (!label) {
		label = mp_widget_slide_title_create(ad->minicontroller_layout, "slide_roll", title);
		elm_object_part_content_set(ad->minicontroller_layout, "elm.text", label);
	} else {
		elm_object_text_set(label, title);
	}

	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_slide_go(label);

	SAFE_FREE(title);
	SAFE_FREE(markup_title);
	SAFE_FREE(markup_artist);
	evas_object_show(label);

}

void
mp_minicontroller_update(struct appdata *ad, bool with_title)
{

	DEBUG_TRACE();
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_minicon);
	MP_CHECK(!ad->is_lcd_off);

	_mp_minicontroller_update_playpause_btn(ad);
	if (ad->player_state == PLAY_STATE_PLAYING) {
#ifdef MINICONTROLLER_ENABLE_PROGRESS
		_minicontroller_progress_timer_add(ad);
#endif
	}

	mp_track_info_t *current_item = ad->current_track_info;
	if (current_item) {
		SECURE_DEBUG("album art is %s", current_item->thumbnail_path);
		if (mp_util_is_image_valid(ad->evas, current_item->thumbnail_path)
		        && strcmp(BROKEN_ALBUMART_IMAGE_PATH, current_item->thumbnail_path)) {
			elm_image_file_set(ad->minicon_icon, current_item->thumbnail_path, NULL);
		} else {
			char default_thumbnail[1024] = {0};
			char *shared_path = app_get_shared_resource_path();
			snprintf(default_thumbnail, 1024, "%s%s/%s", shared_path, "shared_images", DEFAULT_THUMBNAIL);
			free(shared_path);
			elm_image_file_set(ad->minicon_icon, default_thumbnail, NULL);
		}
#ifdef MINICONTROLLER_ENABLE_PROGRESS
		_mp_minicontroller_update_elapsed_time(ad, true);
#endif
		mp_minicontroller_update_shuffle_and_repeat_btn(ad);

		if (with_title) {
			_mp_minicontroller_title_set(ad);
		}

		evas_object_show(ad->minicontroller_layout);
	}
}

int
mp_minicontroller_hide(struct appdata *ad)
{
	DEBUG_TRACE("minicontroller view hide!!\n");
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");
	MP_CHECK_VAL(ad->win_minicon, -1);

	evas_object_hide(ad->win_minicon);
	ad->b_minicontroller_show = FALSE;

	mp_ecore_timer_del(ad->minicon_progress_timer);
	mp_ecore_timer_del(ad->minicon_button_timer);

	return 0;

}

int
mp_minicontroller_destroy(struct appdata *ad)
{
	DEBUG_TRACE("minicontroller view destroy!!");
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");
	MP_CHECK_VAL(ad->win_minicon, -1);

	if (ad->minicontroller_layout != NULL) {
		//evas_object_hide(ad->minicontroller_layout);
		//evas_object_del(ad->minicontroller_layout);
		//ad->minicontroller_layout = NULL;
		ad->b_minicontroller_show = FALSE;
	}
	/*
		if (ad->win_minicon)
		{
			evas_object_del(ad->win_minicon);
			ad->win_minicon = NULL;
		}
	*/
	evas_object_hide(ad->win_minicon);
	mp_ecore_timer_del(ad->minicon_progress_timer);
	mp_ecore_timer_del(ad->minicon_button_timer);

	ad->minicon_visible = false;

	return 0;
}

void
mp_minicontroller_rotate(struct appdata *ad, int angle)
{
	startfunc;
	MP_CHECK(ad);
	MP_CHECK(ad->win_minicon);
	MP_CHECK(ad->minicontroller_layout);

	int w = 0;
	const char *signal = NULL;
	bool landscape  = _mp_minicontroller_landscape_is(ad, angle);
	if (landscape) {
		signal = "sig_set_landscape_mode";
		w = MINI_CONTROLLER_WIDTH_LANDSCAPE;
		landscape = true;
	} else {
		signal = "sig_set_portrait_mode";
		w = MINI_CONTROLLER_WIDTH;
		landscape = false;
	}

	elm_object_signal_emit(ad->minicontroller_layout, signal, "c_source");

	double scale = elm_config_scale_get();
	evas_object_resize(ad->win_minicon, w * scale, MINI_CONTROLLER_HEIGHT * scale);
	_mp_minicontroller_update_layout(ad, landscape);
	mp_minicontroller_update(ad, true);
}

void
mp_minicontroller_visible_set(struct appdata *ad, bool visible)
{
	DEBUG_TRACE("visible: %d", visible);
	MP_CHECK(ad);
	MP_CHECK(ad->win_minicon);
	MP_CHECK(ad->minicontroller_layout);

	ad->minicon_visible = visible;
	_mp_minicontroller_title_set(ad);
	mp_minicontroller_update_control(ad);

	if (visible) {
#ifdef MINICONTROLLER_ENABLE_PROGRESS
		_minicontroller_progress_timer_add(ad);
#endif
	} else {
		display_state_e lock_state;
		int ret = device_display_get_state(&lock_state);
		if (ret == DEVICE_ERROR_NONE) {
			ERROR_TRACE("[SUCCESSFULL] return value %d", ret);
		} else {
			ERROR_TRACE("[ERROR] Return value is %d", ret);
		}

		DEBUG_TRACE("lock_state: %d", lock_state);
		if (lock_state == DISPLAY_STATE_SCREEN_OFF || lock_state == DISPLAY_STATE_SCREEN_DIM) {
			ERROR_TRACE("Timer deleted");
			mp_ecore_timer_del(ad->minicon_progress_timer);
		}
	}
}

bool
mp_minicontroller_visible_get(struct appdata *ad)
{
	MP_CHECK_FALSE(ad);
	MP_CHECK_FALSE(ad->win_minicon);

	return ad->minicon_visible;
}

void mp_minicontroller_on_lcd_event(struct appdata *ad, bool lcd_on)
{
	DEBUG_TRACE("lcd_on [%d]", lcd_on);
	MP_CHECK(ad);
	MP_CHECK(ad->win_minicon);

	if (lcd_on && mp_util_is_now_active_player()) {
		mp_minicontroller_show(ad);
	}

	mp_minicontroller_visible_set(ad, lcd_on);
}

void mp_minicontroller_update_progressbar(struct appdata *ad)
{
#ifdef MINICONTROLLER_ENABLE_PROGRESS
	_mp_minicontroller_update_elapsed_time(ad, false);
#endif
}
