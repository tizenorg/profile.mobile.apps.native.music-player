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
#include "mp-lockscreenmini.h"
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

#ifdef MP_FEATURE_LOCKSCREEN

int LOCKSCREEN_MINI_CONTROLLER_WIDTH;
#define LOCKSCREEN_MINI_CONTROLLER_HEIGHT (93)

//#define LOCKSCREEN_MSG_DOMAIN_CONTROL_ACCESS (int)ECORE_X_ATOM_E_ILLUME_ACCESS_CONTROL


enum {
	LOCKSCREEN_FF_PRESSED = 1,
	LOCKSCREEN_REW_PRESSED = 2,
};

static time_t press_time;
static time_t release_time;
static Evas_Object *_load_edj(Evas_Object * parent, const char *file, const char *group);
static void _load_lockscreenmini(struct appdata *ad);
#ifdef LOCKSCREEN_ENABLE_PROGRESS
static void _mp_lockscreenmini_progress_val_set(struct appdata *ad, double position);
#endif
static void _mp_lockscreenmini_update_layout(struct appdata *ad, bool landscape);
#ifdef MP_FEATURE_LOCKSCREEN
static void _mp_lockscreenmini_set_repeate_image(void *data, int repeate_state);
static void _mp_lockscreenmini_set_shuffle_image(void *data, int shuffle_state);
#endif

static bool _mp_lockscreenmini_is_long_press()
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

#ifdef LOCKSCREEN_ENABLE_PROGRESS
static inline void
_mp_lockscreenmini_update_elapsed_time(struct appdata *ad, bool get_pos)
{
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_layout);

	double played_ratio = 0.;
	double music_pos = 0.;
	if (get_pos) {
		music_pos = mp_player_mgr_get_position() / 1000;
	} else {
		music_pos = ad->music_pos;
	}

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
		if (ad->current_track_info) {
			snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
			         PLAY_TIME_ARGS(ad->current_track_info->duration / 1000.));
		}
		snprintf(play_time, sizeof(play_time), "%" PLAY_TIME_FORMAT, PLAY_TIME_ARGS(music_pos));
	}

	edje_object_part_text_set(_EDJ(ad->lockmini_layout), "np_progress_text_total", total_time);
	edje_object_part_text_set(_EDJ(ad->lockmini_layout), "np_progress_text_playing", play_time);

	if (ad->music_length > 0. && music_pos > 0.) {
		played_ratio = music_pos / ad->music_length;
	}
	_mp_lockscreenmini_progress_val_set(ad, played_ratio);
}

static Eina_Bool
_lockscreenmini_update_progresstime_cb(void *data)
{
	TIMER_TRACE();
	struct appdata *ad = data;
	mp_retvm_if(ad == NULL, ECORE_CALLBACK_CANCEL, "appdata is NULL");

	if (ad->is_lcd_off) {
		mp_ecore_timer_del(ad->lockmini_progress_timer);
		return ECORE_CALLBACK_CANCEL;
	}
	if (ad->progress_dragging) {
		return ECORE_CALLBACK_RENEW;
	}

	if (ad->player_state == PLAY_STATE_PLAYING) {
		_mp_lockscreenmini_update_elapsed_time(ad, true);
	}

	return ECORE_CALLBACK_RENEW;
}

static void
_lockscreenmini_progress_timer_add(void *data)
{
	struct appdata *ad = data;
	mp_retm_if(ad == NULL, "appdata is NULL");
	DEBUG_TRACE();

	mp_ecore_timer_del(ad->lockmini_progress_timer);

	_mp_lockscreenmini_update_elapsed_time(ad, true);
	if (ad->player_state == PLAY_STATE_PLAYING) {
		ad->lockmini_progress_timer = ecore_timer_add(1.0, _lockscreenmini_update_progresstime_cb, ad);
	}
}
#endif

#ifdef LOCKSCREEN_ENABLE_SHUFFLE_REPEAT
static char *_mp_lockscreenmini_shuffle_access_info_cb(void *data, Evas_Object *obj)
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

#ifdef LOCKSCREEN_ENABLE_SHUFFLE_REPEAT
static char *_mp_lockscreenmini_repeat_access_info_cb(void *data, Evas_Object *obj)
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
		/*repeat all*/
		operation_txt = GET_SYS_STR(MP_TTS_REPEAT_ALL_BUTTON);
	}
	return g_strdup(operation_txt);
}
#endif

#ifdef MP_FEATURE_LOCKSCREEN
static void _mp_lockscreenmini_set_shuffle_image(void *data, int shuffle_state)
{
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_layout);
	ERROR_TRACE("");

	if (shuffle_state) {
		elm_object_signal_emit(ad->lockmini_layout, "set_shuffle_on", "*");
	} else {
		elm_object_signal_emit(ad->lockmini_layout, "set_shuffle_off", "*");
	}
}
static void _mp_lockscreenmini_set_repeate_image(void *data, int repeate_state)
{
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_layout);
	ERROR_TRACE("");

	if (MP_PLST_REPEAT_ONE == repeate_state) {
		elm_object_signal_emit(ad->lockmini_layout, "set_repeat_btn_1", "*");
	} else if (MP_PLST_REPEAT_NONE == repeate_state) {
		elm_object_signal_emit(ad->lockmini_layout, "set_repeat_btn_a", "*");
	} else {
		elm_object_signal_emit(ad->lockmini_layout, "set_repeat_btn_all", "*");
	}

}
#endif
static Evas_Object *
_load_edj(Evas_Object *parent, const char *file, const char *group)
{
	Evas_Object *eo;
	int r;

	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
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

static void _lockscreen_cb(minicontrol_viewer_event_e event_type, bundle *event_arg)
{
	/*Need to handle events*/
}

static void
_load_lockscreenmini(struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(ad);
	Evas_Object *win = NULL;

#ifndef MP_SOUND_PLAYER
	win = minicontrol_create_window("music-minicontrol.LOCKSCREEN", MINICONTROL_TARGET_VIEWER_STOCK_LOCK_SCREEN, _lockscreen_cb);
#else
	win = minicontrol_create_window("sound-minicontrol.LOCKSCREEN", MINICONTROL_TARGET_VIEWER_STOCK_LOCK_SCREEN, _lockscreen_cb);
#endif


	if (!win) {
		return;
	}
	elm_win_alpha_set(win, EINA_TRUE);

	ad->win_lockmini = win;

	/* load edje */
	_mp_lockscreenmini_update_layout(ad, false);

	/*evas_object_show(eo);*/

	return;
}

static void
_mp_lockscreenmini_register_reader(void *data)
{
	struct appdata *ad = data;
	MP_CHECK(ad);
}

#ifdef LOCKSCREEN_ENABLE_PROGRESS
static void
_mp_lockscreenmini_progess_box_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK(ad->lockmini_progress_box);

	ad->lockmini_progress_box = NULL;
}
static void
_mp_lockscreenmini_progressarea_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_progress_bar);

	Evas_Event_Mouse_Down *ev = event_info;
	evas_object_data_set(obj, "pressed_x", (void *)ev->canvas.x);
	evas_object_data_set(obj, "timestamp", (void *)ev->timestamp);

	ad->progress_dragging = true;

#if 0
	elm_object_signal_emit(ad->lockmini_progress_bar, "signal.lockscreenmini.progress.click", "*");

	Evas_Object *progressbar = obj;
	int w = 0, current = 0, x = 0;
	double ratio = 0.0;

	evas_object_geometry_get(progressbar, &x, NULL, &w, NULL);
	current = ev->canvas.x - x;
	if (current < 0) {
		current = 0;
	} else if (current > w) {
		current = w;
	}

	ratio = (double)current / w;
	DEBUG_TRACE("canvas.x:%d  x:%d  w:%d", ev->canvas.x, x, w);

	int duration = mp_player_mgr_get_duration();
	if (duration <= 0) {
		mp_track_info_t *track_info = ad->current_track_info;
		if (track_info) {
			duration = track_info->duration;
		}
	}

	ad->music_length = duration / 1000.;

	ad->music_pos = ratio * ad->music_length;

	_mp_lockscreenmini_progress_val_set(ad, ratio);
	_mp_lockscreenmini_update_elapsed_time(ad, false);
#endif
	endfunc;
}
#endif
#if 0
static void
_mp_lockscreenmini_progressarea_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_progress_bar);

	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Object *progressbar = obj;
	int w = 0, current = 0, x = 0;
	double ratio = 0.0;
	int new_pos;

	evas_object_geometry_get(progressbar, &x, NULL, &w, NULL);
	current = ev->canvas.x - x;
	if (current < 0) {
		current = 0;
	} else if (current > w) {
		current = w;
	}

	ratio = (double)current / w;

	new_pos = ratio * ad->music_length;
	ad->music_pos = new_pos;

	_mp_lockscreenmini_progress_val_set(ad, ratio);
	_mp_lockscreenmini_update_elapsed_time(ad, false);
	endfunc;
}
#endif
#ifdef LOCKSCREEN_ENABLE_PROGRESS
static void
_mp_lockscreenmini_progressarea_up_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_progress_bar);

	elm_object_signal_emit(ad->lockmini_progress_bar, "signal.lockscreenmini.progress.unclick", "*");

	if (ad->progress_dragging == false) {
		return;
	}

	int pressed_x = (int)evas_object_data_get(obj, "pressed_x");
	unsigned int timestamp = (unsigned int)evas_object_data_get(obj, "timestamp");
	Evas_Event_Mouse_Up *ev = event_info;
	if (abs(ev->canvas.x - pressed_x) > 10 || (ev->timestamp - timestamp) > 500) {
		return;
	}

	Evas_Object *progressbar = obj;
	int w = 0, current = 0, x = 0;
	double ratio = 0.0;

	evas_object_geometry_get(progressbar, &x, NULL, &w, NULL);
	current = ev->canvas.x - x;

	DEBUG_TRACE("canvas.x:%d  x:%d  w:%d", ev->canvas.x, x, w);
	if (current < 0) {
		current = 0;
	} else if (current > w) {
		current = w;
	}

	ratio = (double)current / w;

	ad->music_pos = ratio * ad->music_length;

	DEBUG_TRACE("ad->music_pos=%lf", ad->music_pos);
	mp_player_mgr_set_position(ad->music_pos * 1000, NULL, NULL);

	ad->progress_dragging = false;

	_mp_lockscreenmini_progress_val_set(ad, ratio);
	_mp_lockscreenmini_update_elapsed_time(ad, false);
	endfunc;
}

static void
_mp_lockscreenmini_progress_val_set(struct appdata *ad, double position)
{
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_layout);
	MP_CHECK(ad->lockmini_progress_bar);

	edje_object_part_drag_value_set(_EDJ(ad->lockmini_progress_bar), "progressbar_playing", position, 0.0);
	return;
}

static void _mp_lockscreenmini_create_progress_layout(struct appdata *ad)
{
	startfunc;

	ad->lockmini_progress_box = mp_common_load_edj(ad->lockmini_layout, LOCKSCREENMINI_EDJ_NAME, "lockscreenmini_progress_box");
	MP_CHECK(ad->lockmini_progress_box);
	evas_object_event_callback_add(ad->lockmini_progress_box, EVAS_CALLBACK_DEL, _mp_lockscreenmini_progess_box_del_cb, ad);
	elm_object_part_content_set(ad->lockmini_layout, "progress_box", ad->lockmini_progress_box);

	ad->lockmini_progress_bar = mp_common_load_edj(ad->lockmini_progress_box, LOCKSCREENMINI_EDJ_NAME, "lockscreenmini_player_progressbar");
	MP_CHECK(ad->lockmini_progress_bar);
	elm_object_part_content_set(ad->lockmini_progress_box, "progress_bar", ad->lockmini_progress_bar);
	_mp_lockscreenmini_progress_val_set(ad, 0.0);

	evas_object_event_callback_add(ad->lockmini_progress_bar, EVAS_CALLBACK_MOUSE_DOWN,
	                               _mp_lockscreenmini_progressarea_down_cb, ad);
	evas_object_event_callback_add(ad->lockmini_progress_bar, EVAS_CALLBACK_MOUSE_UP,
	                               _mp_lockscreenmini_progressarea_up_cb, ad);
#if 0
	evas_object_event_callback_add(ad->lockmini_progress_bar, EVAS_CALLBACK_MOUSE_MOVE,
	                               _mp_lockscreenmini_progressarea_move_cb, ad);
#endif
}
#endif
void
mp_lockscreenmini_update_winmini_size(struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(ad);

	double scale = elm_config_scale_get();
	DEBUG_TRACE("scale: %f", scale);

	if ((scale - 1.7) < 0.0001) {
		LOCKSCREEN_MINI_CONTROLLER_WIDTH = 318;
	} else if ((scale - 1.8) < 0.0001) {
		LOCKSCREEN_MINI_CONTROLLER_WIDTH = 267;
	} else if ((scale - 2.4) < 0.0001) {
		LOCKSCREEN_MINI_CONTROLLER_WIDTH = 300;
	} else if ((scale - 2.6) < 0.0001) {
		LOCKSCREEN_MINI_CONTROLLER_WIDTH = 300;
	} else if ((scale - 2.8) < 0.0001) {
		LOCKSCREEN_MINI_CONTROLLER_WIDTH = 300;
	}

	evas_object_resize(ad->win_lockmini, LOCKSCREEN_MINI_CONTROLLER_WIDTH * scale, LOCKSCREEN_MINI_CONTROLLER_HEIGHT * scale);

	return;
}

int
mp_lockscreenmini_create(struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");

	if (!(ad->lockmini_layout && ad->win_lockmini)) {

		_load_lockscreenmini(ad);
		if (ad->lockmini_layout == NULL) {
			DEBUG_TRACE("ERROR");
			return -1;
		}
		_mp_lockscreenmini_register_reader(ad);
	}

	mp_lockscreenmini_update_winmini_size(ad);

	mp_lockscreenmini_show(ad);
	return 0;
}


int
mp_lockscreenmini_show(struct appdata *ad)
{
	DEBUG_TRACE("minicontroller view show!!");
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");
	MP_CHECK_VAL(ad->win_lockmini, -1);
	MP_CHECK_VAL(!ad->is_lcd_off, -1);
	/* Not show minicontrol when current track not exsit */
	MP_CHECK_VAL(ad->current_track_info, -1);

	ad->b_lockmini_show = TRUE;
	mp_lockscreenmini_update(ad);

	FILE *fp = fopen(MP_LSCR_CONTROL, "w");
	if (fp) {
		fclose(fp);
	}

	evas_object_show(ad->win_lockmini);
	return 0;
}

#ifdef LOCKSCREEN_ENABLE_PROGRESS
static void _mp_lockscreenmini_update_progressbar_state(struct appdata *ad)
{
	startfunc;
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->lockmini_progress_bar);
}
#endif

static void _mp_lockscreenmini_update_btn(struct appdata *ad)
{
	startfunc;
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_lockmini);
	MP_CHECK(!ad->is_lcd_off);

	if (ad->player_state == PLAY_STATE_PLAYING) {
		edje_object_signal_emit(elm_layout_edje_get(ad->lockmini_layout), "play_btn_visible", "play_btn");
		edje_object_signal_emit(elm_layout_edje_get(ad->lockmini_layout), "pause_btn_invisible", "pause_btn");
	} else {
		edje_object_signal_emit(elm_layout_edje_get(ad->lockmini_layout), "play_btn_invisible", "play_btn");
		edje_object_signal_emit(elm_layout_edje_get(ad->lockmini_layout), "pause_btn_visible", "pause_btn");
	}

#ifdef LOCKSCREEN_ENABLE_PROGRESS
	_mp_lockscreenmini_update_progressbar_state(ad);
#endif
}

static Eina_Bool
_mp_lockscreenmini_btn_update_timer(void *data)
{
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	_mp_lockscreenmini_update_btn(data);

	ad->lockmini_button_timer = NULL;
	return EINA_FALSE;
}

static void _mp_lockscreenmini_update_playpause_btn(struct appdata *ad)
{
	mp_ecore_timer_del(ad->lockmini_button_timer);

	if (ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED) {
		_mp_lockscreenmini_update_btn(ad);
	} else {
		ad->lockmini_button_timer = ecore_timer_add(1.0, _mp_lockscreenmini_btn_update_timer, ad);
	}
}

void
mp_lockscreenmini_update_control(struct appdata *ad)
{
	startfunc;
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_lockmini);
	MP_CHECK(!ad->is_lcd_off);

	_mp_lockscreenmini_update_playpause_btn(ad);
}

#ifdef MP_FEATURE_LOCKSCREEN
void
mp_lockscreenmini_update_shuffle_and_repeat_btn(struct appdata *ad)
{
	startfunc;
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_lockmini);
	MP_CHECK(!ad->is_lcd_off);

	int shuffle_state = 0;
	int repeat_state = 0;
	mp_setting_get_shuffle_state(&shuffle_state);
	mp_setting_get_repeat_state(&repeat_state);

	_mp_lockscreenmini_set_shuffle_image((void *)ad, shuffle_state);
	_mp_lockscreenmini_set_repeate_image((void *)ad, repeat_state);

}

#endif
static void _mp_lockscreenmini_play_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
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
#ifdef LOCKSCREEN_ENABLE_SHUFFLE_REPEAT
static void _mp_lockscreenmini_repeat_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("repeat button clicked");

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	int repeat_state = 0;
	mp_setting_get_repeat_state(&repeat_state);
	repeat_state++;
	repeat_state %= 3;
	mp_setting_set_repeat_state(repeat_state);
	_mp_lockscreenmini_set_repeate_image(ad, repeat_state);
	mp_playlist_mgr_set_repeat(ad->playlist_mgr, repeat_state);
	mp_player_view_update_state(GET_PLAYER_VIEW);
}
#endif
static void _mp_lockscreenmini_ff_rew_btn_pressed_cb(void *data, Evas_Object *obj, void *event_info)
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

static void _mp_lockscreenmini_ff_rew_btn_unpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button unpressed");
	time(&release_time);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->lockmini_layout);

	char *source = (char *)data;
	if (!g_strcmp0(source, CONTROLLER_FF_SOURCE)) {
		elm_object_signal_emit(ad->lockmini_layout, "signal.button.unpressed", "ff_btn");
		mp_play_control_ff(false, false, false);
	} else {
		elm_object_signal_emit(ad->lockmini_layout, "signal.button.unpressed", "rew_btn");
		mp_play_control_rew(false, false, false);
	}
}

static void _mp_lockscreenmini_ff_rew_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button clicked");
	if (_mp_lockscreenmini_is_long_press()) {
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
_mp_lockscreenmini_update_layout(struct appdata *ad, bool landscape)
{
	MP_CHECK(ad);

	if (ad->lockmini_progress_timer) {
		ecore_timer_del(ad->lockmini_progress_timer);
		ad->lockmini_progress_timer = NULL;
	}
	mp_ecore_timer_del(ad->lockmini_button_timer);

	mp_evas_object_del(ad->lockmini_layout);
	ad->lockmini_layout = _load_edj(ad->win_lockmini, LOCKSCREENMINI_EDJ_NAME, "music-lockscreenmini");

	if (!ad->lockmini_layout) {
		return ;
	}

#ifdef LOCKSCREEN_ENABLE_PROGRESS
	_mp_lockscreenmini_create_progress_layout(ad);
#endif
	elm_win_resize_object_add(ad->win_lockmini, ad->lockmini_layout);

	/*add focused UI related*/
#ifdef LOCKSCREEN_ENABLE_SHUFFLE_REPEAT
	/*-------> shuffle button ------->*/
	Evas_Object *shuffle_focus_btn = elm_button_add(ad->lockmini_layout);
	elm_object_style_set(shuffle_focus_btn, "focus");
	elm_object_part_content_set(ad->lockmini_layout, "shuffle_focus", shuffle_focus_btn);
	elm_object_focus_custom_chain_append(ad->lockmini_layout, shuffle_focus_btn, NULL);
#endif
	/*-------> REW button ------->*/
	Evas_Object *rew_focus_btn = elm_button_add(ad->lockmini_layout);
	elm_object_style_set(rew_focus_btn, "focus");
	elm_object_part_content_set(ad->lockmini_layout, "rew_focus", rew_focus_btn);
	elm_object_focus_custom_chain_append(ad->lockmini_layout, rew_focus_btn, NULL);
	evas_object_smart_callback_add(rew_focus_btn, "clicked", _mp_lockscreenmini_ff_rew_btn_clicked_cb, CONTROLLER_REW_SOURCE);
	evas_object_smart_callback_add(rew_focus_btn, "pressed", _mp_lockscreenmini_ff_rew_btn_pressed_cb, CONTROLLER_REW_SOURCE);
	evas_object_smart_callback_add(rew_focus_btn, "unpressed", _mp_lockscreenmini_ff_rew_btn_unpressed_cb, CONTROLLER_REW_SOURCE);

	/*-------> play/pause button ------->*/
	Evas_Object *play_pause_focus_btn = elm_button_add(ad->lockmini_layout);
	elm_object_style_set(play_pause_focus_btn, "focus");
	elm_object_part_content_set(ad->lockmini_layout, "play_pause_focus", play_pause_focus_btn);
	elm_object_focus_custom_chain_append(ad->lockmini_layout, play_pause_focus_btn, NULL);
	evas_object_smart_callback_add(play_pause_focus_btn, "clicked", _mp_lockscreenmini_play_pause_btn_clicked_cb, NULL);

	/*------->FF button ------->*/
	Evas_Object *ff_focus_btn = elm_button_add(ad->lockmini_layout);
	elm_object_style_set(ff_focus_btn, "focus");
	elm_object_part_content_set(ad->lockmini_layout, "ff_focus", ff_focus_btn);
	elm_object_focus_custom_chain_append(ad->lockmini_layout, ff_focus_btn, NULL);
	evas_object_smart_callback_add(ff_focus_btn, "clicked", _mp_lockscreenmini_ff_rew_btn_clicked_cb, CONTROLLER_FF_SOURCE);
	evas_object_smart_callback_add(ff_focus_btn, "pressed", _mp_lockscreenmini_ff_rew_btn_pressed_cb, CONTROLLER_FF_SOURCE);
	evas_object_smart_callback_add(ff_focus_btn, "unpressed", _mp_lockscreenmini_ff_rew_btn_unpressed_cb, CONTROLLER_FF_SOURCE);

#ifdef LOCKSCREEN_ENABLE_SHUFFLE_REPEAT
	/*-------> repeat button ------->*/
	Evas_Object *repeat_focus_btn = elm_button_add(ad->lockmini_layout);
	elm_object_style_set(repeat_focus_btn, "focus");
	elm_object_part_content_set(ad->lockmini_layout, "repeat_focus", repeat_focus_btn);
	elm_object_focus_custom_chain_append(ad->lockmini_layout, repeat_focus_btn, NULL);
	evas_object_smart_callback_add(repeat_focus_btn, "clicked", _mp_lockscreenmini_repeat_btn_clicked_cb, NULL);
#endif
	_mp_lockscreenmini_update_btn(ad);
}

static void
_mp_lockscreenmini_title_set(struct appdata *ad)
{
	DEBUG_TRACE("title set");

	MP_CHECK(ad);
	Evas_Object *label = elm_object_part_content_get(ad->lockmini_layout, "elm.text");

	mp_track_info_t *current_item = ad->current_track_info;
	MP_CHECK(current_item);

	char *markup_title = elm_entry_utf8_to_markup(current_item->title);
	char *markup_artist = elm_entry_utf8_to_markup(current_item->artist);

	char *title_shadow = g_strdup_printf("far_shadow,bottom shadow_color=#00000080");
	char *title_format = "<align=center><style=%s><font_size=%d><color=#%s><color_class=%s>%s - %s</font></color_class></font_size></style></align>";
	char *title = NULL;
	char *title_format_left = "<align=left><style=%s><font_size=%d><color=#%s><color_class=%s>%s - %s</font></color_class></font_size></style></align>";

	if (markup_title && markup_artist) {
		if (strlen(markup_artist) + strlen(markup_title) + 3 <= 50) {
			title = g_strdup_printf(title_format, title_shadow, 30, "FFFFFFFF", "ATO003", markup_title, markup_artist);
		} else {
			title = g_strdup_printf(title_format_left, title_shadow, 30, "FFFFFFFF", "ATO003", markup_title, markup_artist);
		}
	}

	/*edje_object_part_text_set(_EDJ(ad->lockmini_layout), "artist.text", markup_artist);*/
	if (!label) {
		label = mp_widget_slide_title_create(ad->lockmini_layout, "slide_roll", title);
		elm_object_part_content_set(ad->lockmini_layout, "elm.text", label);
	} else {
		elm_object_text_set(label, title);
	}

	if (ad->lockmini_visible) {
		elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_ALWAYS);
		elm_label_slide_go(label);
	} else {
		elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_NONE);
		elm_label_slide_go(label);
	}

	SAFE_FREE(title);
	SAFE_FREE(markup_title);
	SAFE_FREE(markup_artist);
	evas_object_show(label);
}

void
mp_lockscreenmini_update(struct appdata *ad)
{

	DEBUG_TRACE();
	mp_retm_if(ad == NULL, "appdata is NULL");
	MP_CHECK(ad->win_lockmini);
	MP_CHECK(!ad->is_lcd_off);

	mp_lockscreenmini_update_control(ad);
	if (ad->player_state == PLAY_STATE_PLAYING) {
#ifdef LOCKSCREEN_ENABLE_PROGRESS
		_lockscreenmini_progress_timer_add(ad);
#endif
	}

	mp_track_info_t *current_item = ad->current_track_info;
	if (current_item) {
#ifdef LOCKSCREEN_ENABLE_PROGRESS
		_mp_lockscreenmini_update_elapsed_time(ad, true);
#endif
#ifdef LOCKSCREEN_ENABLE_SHUFFLE_REPEAT
		mp_lockscreenmini_update_shuffle_and_repeat_btn(ad);
#endif

		_mp_lockscreenmini_title_set(ad);

		evas_object_show(ad->lockmini_layout);
	}
}

int
mp_lockscreenmini_hide(struct appdata *ad)
{
	DEBUG_TRACE("lockscreenmini view hide!!\n");
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");
	MP_CHECK_VAL(ad->win_lockmini, -1);

	evas_object_hide(ad->win_lockmini);
	ad->b_lockmini_show = FALSE;

	mp_ecore_timer_del(ad->lockmini_progress_timer);
	mp_ecore_timer_del(ad->lockmini_button_timer);

	return 0;

}

int
mp_lockscreenmini_destroy(struct appdata *ad)
{
	DEBUG_TRACE("lockscreenmini view destroy!!");
	mp_retvm_if(ad == NULL, -1, "appdata is NULL");
	MP_CHECK_VAL(ad->win_lockmini, -1);

	if (ad->lockmini_layout != NULL) {
		ad->b_lockmini_show = FALSE;
	}

	evas_object_hide(ad->win_lockmini);
	mp_ecore_timer_del(ad->lockmini_progress_timer);
	mp_ecore_timer_del(ad->lockmini_button_timer);
	ad->lockmini_visible = false;

	ecore_file_remove(MP_LSCR_CONTROL);
	return 0;
}

void
mp_lockscreenmini_visible_set(struct appdata *ad, bool visible)
{
	DEBUG_TRACE("visible: %d", visible);
	MP_CHECK(ad);
	MP_CHECK(ad->win_lockmini);

	ad->lockmini_visible = visible;
	_mp_lockscreenmini_title_set(ad);
	mp_lockscreenmini_update_control(ad);

	if (visible) {
#ifdef LOCKSCREEN_ENABLE_PROGRESS
		_lockscreenmini_progress_timer_add(ad);
#endif
	} else {
		display_state_e lock_state;
		int ret = device_display_get_state(&lock_state);
		if (ret == DEVICE_ERROR_NONE) {
			ERROR_TRACE("[SUCCESSFULL] Return value is %d", ret);
		} else {
			ERROR_TRACE("[ERROR] Return value is %d", ret);
		}
		if (lock_state == DISPLAY_STATE_SCREEN_OFF || lock_state == DISPLAY_STATE_SCREEN_DIM) {
			ERROR_TRACE("timer locks deleted");
			mp_ecore_timer_del(ad->lockmini_progress_timer);
		}
	}
}

bool
mp_lockscreenmini_visible_get(struct appdata *ad)
{
	MP_CHECK_FALSE(ad);
	MP_CHECK_FALSE(ad->win_lockmini);

	return ad->lockmini_visible;
}

void mp_lockscreenmini_on_lcd_event(struct appdata *ad, bool lcd_on)
{
	DEBUG_TRACE("lcd_on [%d]", lcd_on);
	MP_CHECK(ad);

	if (lcd_on && mp_util_is_now_active_player()) {
		if (ad->player_state == PLAY_STATE_PLAYING) {
			mp_lockscreenmini_show(ad);
		}
	}

	mp_lockscreenmini_visible_set(ad, lcd_on);
}

void mp_lockscreenmini_update_progressbar(struct appdata *ad)
{
#ifdef LOCKSCREEN_ENABLE_PROGRESS
	_mp_lockscreenmini_update_elapsed_time(ad, true);
#endif
}

#endif
