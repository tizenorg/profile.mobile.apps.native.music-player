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

#include "mp-now-playing.h"
#include "mp-player-mgr.h"
#include "mp-player-debug.h"
#include "mp-player-control.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-play.h"
#include "mp-setting-ctrl.h"
#include "mp-minicontroller.h"

typedef struct {
	Evas_Object *now_playing;
	Evas_Object *thumbnail;
	Evas_Object *progress;
	Evas_Object *label;
	Evas_Object *label_artist;
	Evas_Object *spectrum;
	//mini eq ani
	Evas_Object *mini_eq;
	Ecore_Timer *timer;
	Ecore_Timer *stop_update_timer;
	bool landscape;
	bool dragging;
	MpNowplayingCb play_bt_clicked;
	MpNowplayingCb clicked;
	void *userdata;
	char *play_time;
	Evas_Object* shuffle_focus_btn;
	Evas_Object* repeate_focus_btn;
} MpNpData_t;

#define GET_WIDGET_DATA(o) evas_object_data_get(o, "widget_d");
#define NOW_PLAYING_LABEL_LEN 524
#define NOW_PLAYING_TITLE_SIZE 23
#define NOW_PLAYING_TITLE_COLOR "FAFAFAFF"
#define NOW_PLAYING_ARTIST_SIZE 16
#define NOW_PLAYING_ARTIST_COLOR "FAFAFAFF"

#define NOW_PLAYING_REW_SOURCE "control_previous"
#define NOW_PLAYING_FF_SOURCE "control_next"
#define NOW_PLAYING_LONG_PRESS_INTERVAL 1.0
#define NOW_PLAYING_FF_REW_INTERVAL 0.3
#define NOW_PLAYING_LONG_PRESS_TIME_INCREASE	1.0	//sec

#ifdef MP_NOW_PLAYING_MINI_EQ
#undef MP_NOW_PLAYING_MINI_EQ
#endif

static void _mp_now_playing_update_playpause_btn(Evas_Object *obj);
static void _mp_now_playing_update_time(void *data);

char *g_tts_located_part = NULL;

#ifdef MP_NOW_PLAYING_MINI_EQ
static Evas_Object *
_mp_now_playing_create_thumb_icon(Evas_Object * obj, const char *path, int w, int h)
{
	Evas_Object *thumbnail = elm_image_add(obj);
	elm_image_file_set(thumbnail, IMAGE_EDJ_NAME, path);
	evas_object_size_hint_weight_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_fill_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	return thumbnail;
}
#endif

void _progressbar_value_update_to_zero(void *data)
{
	startfunc;
	MpNpData_t *wd =  data;
	MP_CHECK(wd);
	MP_CHECK(wd->progress);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	double value = 0.0;

	elm_progressbar_value_set(wd->progress, value);
	endfunc;
}

static void _progressbar_value_set(void *data)
{
	MpNpData_t *wd =  data;
	MP_CHECK(wd);
	MP_CHECK(wd->progress);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	double pos = ad->music_pos;
	double duration = ad->music_length;
	double value = 0.0;
	if (duration > 0.0) {
		value = pos / duration;
	}

	ERROR_TRACE("wishjox pos :%f, duration: %f, val: %f", pos, duration, value);
	elm_progressbar_value_set(wd->progress, value);
}

#ifdef MP_NOW_PLAYING_MINI_EQ
static void _mini_eq_get_image(char **path1, char **path2, char **path3)
{
	int index1 = rand() % 8 + 1;
	int index2 = rand() % 8 + 1;
	int index3 = rand() % 8 + 1;

	switch (index1) {
	case 1:
		*path1 = MP_MINI_EQ_ANI_01;
		break;
	case 2:
		*path1 = MP_MINI_EQ_ANI_02;
		break;
	case 3:
		*path1 = MP_MINI_EQ_ANI_03;
		break;
	case 4:
		*path1 = MP_MINI_EQ_ANI_04;
		break;
	case 5:
		*path1 = MP_MINI_EQ_ANI_05;
		break;
	case 6:
		*path1 = MP_MINI_EQ_ANI_06;
		break;
	case 7:
		*path1 = MP_MINI_EQ_ANI_07;
		break;
	case 8:
		*path1 = MP_MINI_EQ_ANI_08;
		break;
	default:
		*path1 = MP_MINI_EQ_ANI_01;

	}
	switch (index2) {
	case 1:
		*path2 = MP_MINI_EQ_ANI_01;
		break;
	case 2:
		*path2 = MP_MINI_EQ_ANI_02;
		break;
	case 3:
		*path2 = MP_MINI_EQ_ANI_03;
		break;
	case 4:
		*path2 = MP_MINI_EQ_ANI_04;
		break;
	case 5:
		*path2 = MP_MINI_EQ_ANI_05;
		break;
	case 6:
		*path2 = MP_MINI_EQ_ANI_06;
		break;
	case 7:
		*path2 = MP_MINI_EQ_ANI_07;
		break;
	case 8:
		*path2 = MP_MINI_EQ_ANI_08;
		break;
	default:
		*path2 = MP_MINI_EQ_ANI_02;

	}
	switch (index3) {
	case 1:
		*path3 = MP_MINI_EQ_ANI_01;
		break;
	case 2:
		*path3 = MP_MINI_EQ_ANI_02;
		break;
	case 3:
		*path3 = MP_MINI_EQ_ANI_03;
		break;
	case 4:
		*path3 = MP_MINI_EQ_ANI_04;
		break;
	case 5:
		*path3 = MP_MINI_EQ_ANI_05;
		break;
	case 6:
		*path3 = MP_MINI_EQ_ANI_06;
		break;
	case 7:
		*path3 = MP_MINI_EQ_ANI_07;
		break;
	case 8:
		*path3 = MP_MINI_EQ_ANI_08;
		break;
	default:
		*path3 = MP_MINI_EQ_ANI_03;

	}
}
#endif

static Eina_Bool
_mp_nowplaying_timer_cb(void *data)
{
	TIMER_TRACE();
	MpNpData_t *wd =  data;
	MP_CHECK_FALSE(wd);
	MP_CHECK_FALSE(wd->now_playing);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	if (!ad->app_is_foreground) {
		MP_TIMER_FREEZE(wd->timer);
	}

	if (wd->dragging) {
		return EINA_TRUE;
	}

	ad->music_pos = mp_player_mgr_get_position() / 1000.0;
	_mp_now_playing_update_time(wd);

#ifdef MP_NOW_PLAYING_MINI_EQ
	char *path1 = NULL;
	char *path2 = NULL;
	char *path3 = NULL;

	_mini_eq_get_image(&path1, &path2, &path3);

	Evas_Object *unused_image = NULL;
	unused_image = elm_object_part_content_unset(wd->mini_eq, "image1");
	evas_object_del(unused_image);
	unused_image = elm_object_part_content_unset(wd->mini_eq, "image2");
	evas_object_del(unused_image);
	unused_image = elm_object_part_content_unset(wd->mini_eq, "image3");
	evas_object_del(unused_image);

	Evas_Object *image1 = _mp_now_playing_create_thumb_icon(wd->mini_eq, path1, 11, 38);
	Evas_Object *image2 = _mp_now_playing_create_thumb_icon(wd->mini_eq, path2, 11, 38);
	Evas_Object *image3 = _mp_now_playing_create_thumb_icon(wd->mini_eq, path3, 11, 38);
	elm_object_part_content_set(wd->mini_eq, "image1", image1);
	elm_object_part_content_set(wd->mini_eq, "image2", image2);
	elm_object_part_content_set(wd->mini_eq, "image3", image3);
#endif
	//int show = ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PLAYING);
	//_mp_now_playing_update_playpause_btn(wd->now_playing);

	return ECORE_CALLBACK_RENEW;
}

static void
_widget_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpNpData_t *wd = data;
	MP_CHECK(wd);

	mp_ecore_timer_del(wd->timer);
	mp_ecore_timer_del(wd->stop_update_timer);
	mp_play_control_reset_ff_rew();
	IF_FREE(wd->play_time);

	IF_FREE(wd);
}

static void _mp_now_playing_update_btn(Evas_Object *obj)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (ad->player_state == PLAY_STATE_PLAYING) {
		edje_object_signal_emit(elm_layout_edje_get(obj), "control_play_visible", "control_play");
		edje_object_signal_emit(elm_layout_edje_get(obj), "control_pause_invisible", "control_pause");
	} else {
		edje_object_signal_emit(elm_layout_edje_get(obj), "control_play_invisible", "control_play");
		edje_object_signal_emit(elm_layout_edje_get(obj), "control_pause_visible", "control_pause");
	}
}

static Eina_Bool
_btn_update_timer(void *data)
{
	TIMER_TRACE("");
	MpNpData_t *wd = GET_WIDGET_DATA(data);
	MP_CHECK_FALSE(wd);

	_mp_now_playing_update_btn(data);

	wd->stop_update_timer = NULL;
	return EINA_FALSE;
}

static void _mp_now_playing_update_playpause_btn(Evas_Object *obj)
{
	MP_CHECK(obj);
	MpNpData_t *wd = GET_WIDGET_DATA(obj);
	MP_CHECK(wd);

	mp_ecore_timer_del(wd->stop_update_timer);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED) {
		_mp_now_playing_update_btn(obj);
	} else {
		wd->stop_update_timer = ecore_timer_add(1.0, _btn_update_timer, obj);
	}
}

static void
_mp_now_playing_update_time(void *data)
{

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(!ad->is_lcd_off);

	MpNpData_t *wd =  data;
	MP_CHECK(wd);
	MP_CHECK(wd->progress);

	char play_time[16] = { 0, };
	char total_time[16] = { 0, };

	double duration = ad->music_length;

	if (duration > 0.) {
		if (duration > 3600.) {
			snprintf(total_time, sizeof(total_time), "%" MUSIC_TIME_FORMAT,
			         MUSIC_TIME_ARGS(duration));
			snprintf(play_time, sizeof(play_time), "%" MUSIC_TIME_FORMAT, MUSIC_TIME_ARGS(ad->music_pos));
		} else {
			snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
			         PLAY_TIME_ARGS(duration));
			snprintf(play_time, sizeof(play_time), "%" PLAY_TIME_FORMAT, PLAY_TIME_ARGS(ad->music_pos));
		}
	} else {
		if (ad->current_track_info)
			snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
			         PLAY_TIME_ARGS(ad->current_track_info->duration / 1000.));
		snprintf(play_time, sizeof(play_time), "%" PLAY_TIME_FORMAT, PLAY_TIME_ARGS(ad->music_pos));
	}

	if (g_strcmp0(play_time, wd->play_time)) {
		IF_FREE(wd->play_time);
		wd->play_time = g_strdup(play_time);

		edje_object_part_text_set(_EDJ(wd->now_playing), "np_progress_text_total", total_time);
		edje_object_part_text_set(_EDJ(wd->now_playing), "np_progress_text_playing", play_time);

		_progressbar_value_set(wd);
	}

}

static void
_mp_now_playing_progressarea_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;

	MpNpData_t *wd =  data;
	MP_CHECK(wd);
	MP_CHECK(wd->progress);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Object *progressbar = obj;
	int w = 0, current = 0, x = 0;
	double ratio = 0.0;

	MP_TIMER_FREEZE(wd->timer);

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

	wd->dragging = true;

	ERROR_TRACE("wishjox pos :%f, duration: %f, val: %f", ad->music_pos , ad->music_length, ratio);

	elm_progressbar_value_set(wd->progress, ratio);
	_mp_now_playing_update_time(wd);
	endfunc;
}

static void
_mp_now_playing_progressarea_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpNpData_t *wd =  data;
	MP_CHECK(wd);
	MP_CHECK(wd->progress);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

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

	elm_progressbar_value_set(wd->progress, ratio);
	_mp_now_playing_update_time(wd);
	endfunc;
}

static void
_mp_now_playing_progressarea_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpNpData_t *wd =  data;
	MP_CHECK(wd);
	MP_CHECK(wd->progress);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Event_Mouse_Down *ev = event_info;
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
	if (mp_player_mgr_set_position(ad->music_pos * 1000, NULL, NULL)) {
		mp_now_playing_thaw_timer(wd->now_playing);
	}

	wd->dragging = false;

	elm_progressbar_value_set(wd->progress, ratio);
	_mp_now_playing_update_time(wd);
	endfunc;
}

/*static char* _mp_now_playing_progress_playing_text_tts_info_cb(void *data, Evas_Object *obj)
{
	MpNpData_t *wd =  data;
	MP_CHECK_NULL(wd);

	const char *playing_text = edje_object_part_text_get(_EDJ(wd->now_playing), "np_progress_text_playing");
	return g_strdup(playing_text);
}

static char* _mp_now_playing_progress_total_text_tts_info_cb(void *data, Evas_Object *obj)
{
	MpNpData_t *wd =  data;
	MP_CHECK_NULL(wd);

	const char *playing_text = edje_object_part_text_get(_EDJ(wd->now_playing), "np_progress_text_total");
	return g_strdup(playing_text);
}*/

#define CONTROL_W 450
#define ALBUMART_W 195
#define CENTER_MIN_W 160

static void _set_layout(Evas_Object *obj)
{
	int w, h;
	struct appdata *ad = mp_util_get_appdata();
	evas_object_geometry_get(ad->win_main, NULL, NULL, &w, &h);
	DEBUG_TRACE("Nowplaying bar w: %d", w);
	evas_object_size_hint_min_set(obj, w, 0);
	evas_object_size_hint_max_set(obj, w, -1);

	if (w < CONTROL_W * elm_config_scale_get()) {
		DEBUG_TRACE("control_only mode");
		elm_object_signal_emit(obj, "control_only", "*");
	} else if (w < (CONTROL_W + ALBUMART_W)*elm_config_scale_get()) {
		DEBUG_TRACE("hide_center");
		elm_object_signal_emit(obj, "hide_center", "*");
	} else if (w < (CONTROL_W + ALBUMART_W + CENTER_MIN_W)*elm_config_scale_get()) {
		DEBUG_TRACE("center min mode");
		elm_object_signal_emit(obj, "center_min", "*");
	} else {
		DEBUG_TRACE("default mode");
		elm_object_signal_emit(obj, "set_default", "*");
	}
}

static void _mp_now_left_area_clicked_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	startfunc;
	DEBUG_TRACE("album clicked");

	MpNpData_t *wd = data;
	MP_CHECK(wd);
	if (wd->clicked) {
		wd->clicked(wd->userdata);
	}
}

static void _mp_now_playing_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button clicked");

	MpNpData_t *wd = data;
	MP_CHECK(wd);
	wd->play_bt_clicked(wd->userdata);
	_mp_now_playing_update_playpause_btn(wd->now_playing);

}

static void _mp_now_playing_btn_pressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button pressed");
	char *source = (char *)data;
	if (!g_strcmp0(source, NOW_PLAYING_FF_SOURCE)) {
		mp_play_control_ff(true, false, true);
	} else {
		mp_play_control_rew(true, false, true);
	}
}

static void _mp_now_playing_btn_unpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("button unpressed");
	char *source = (char *)data;
	if (!g_strcmp0(source, NOW_PLAYING_FF_SOURCE)) {
		mp_play_control_ff(false, false, true);
	} else {
		mp_play_control_rew(false, false, true);
	}
}

Evas_Object *mp_now_playing_create(Evas_Object *parent, MpNowplayingCb play_bt_clicked, MpNowplayingCb clicked, void *data)
{
	startfunc;
	Evas_Object *playing_pannel = NULL;
	MpNpData_t *wd = NULL;

	int r = -1;

#ifdef MP_FEATURE_LANDSCAPE
	bool landscape = false;
	landscape  = mp_util_is_landscape();
#endif
	playing_pannel = elm_layout_add(parent);
	if (playing_pannel) {
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", PLAY_VIEW_EDJ_NAME);

		r = elm_layout_file_set(playing_pannel, edje_path, "mp_now_playing");
		free(path);

		if (!r) {
			ERROR_TRACE("Error: elm_layout_file_set");
			evas_object_del(playing_pannel);
			return NULL;
		}
		evas_object_size_hint_weight_set(playing_pannel, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	}

	wd = calloc(1, sizeof(MpNpData_t));
	if (!wd) {
		ERROR_TRACE("Error: memory alloc failed");
		evas_object_del(playing_pannel);
		return NULL;
	}

	evas_object_data_set(playing_pannel, "widget_d", wd);

	/* wd->landscape = landscape;*/

	Evas_Object *thumbnail = elm_image_add(playing_pannel);
	evas_object_size_hint_align_set(thumbnail, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	//elm_image_aspect_fixed_set(thumbnail, true);
	elm_image_fill_outside_set(thumbnail, true);
	elm_image_prescale_set(thumbnail, 100);
	//elm_object_part_content_set(playing_pannel, "thumb_image", thumbnail);
	wd->thumbnail = thumbnail;

	Evas_Object *progress = elm_progressbar_add(playing_pannel);
	elm_object_style_set(progress, "music/list_progress");
	elm_progressbar_horizontal_set(progress, EINA_TRUE);
	evas_object_size_hint_align_set(progress, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progress, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progress, 0.0);
	evas_object_show(progress);

	Evas_Object *progress_focus_btn = elm_button_add(playing_pannel);
	elm_object_style_set(progress_focus_btn, "focus");
	elm_object_part_content_set(progress_focus_btn, "elm.swallow.content", progress);
	wd->progress = progress;

	evas_object_event_callback_add(progress, EVAS_CALLBACK_MOUSE_DOWN,
	                               _mp_now_playing_progressarea_down_cb, wd);
	evas_object_event_callback_add(progress, EVAS_CALLBACK_MOUSE_UP,
	                               _mp_now_playing_progressarea_up_cb, wd);
	evas_object_event_callback_add(progress, EVAS_CALLBACK_MOUSE_MOVE,
	                               _mp_now_playing_progressarea_move_cb, wd);

#ifdef MP_NOW_PLAYING_MINI_EQ
	Evas_Object *mini_eq_ani = NULL;
	mini_eq_ani = mp_common_load_edj(playing_pannel, PLAY_VIEW_EDJ_NAME, "mini_eq_layout");
	if (mini_eq_ani == NULL) {
		ERROR_TRACE("create mini_eq_ani failed");
	}
	elm_object_part_content_set(playing_pannel, "mini_icon", mini_eq_ani);
	char *path1 = NULL;
	char *path2 = NULL;
	char *path3 = NULL;
	_mini_eq_get_image(&path1, &path2, &path3);
	Evas_Object *image1 = _mp_now_playing_create_thumb_icon(mini_eq_ani, path1, 11, 38);
	Evas_Object *image2 = _mp_now_playing_create_thumb_icon(mini_eq_ani, path2, 11, 38);
	Evas_Object *image3 = _mp_now_playing_create_thumb_icon(mini_eq_ani, path3, 11, 38);
	elm_object_part_content_set(mini_eq_ani, "image1", image1);
	elm_object_part_content_set(mini_eq_ani, "image2", image2);
	elm_object_part_content_set(mini_eq_ani, "image3", image3);
	wd->mini_eq = mini_eq_ani;
#endif
	mp_retvm_if(playing_pannel == NULL, NULL, "now playing view is NULL");

	wd->clicked = clicked;
	wd->play_bt_clicked = play_bt_clicked;
	wd->userdata = data;
	wd->now_playing = playing_pannel;

	evas_object_show(playing_pannel);

	wd->timer = ecore_timer_add(0.25, _mp_nowplaying_timer_cb, wd);
	MP_TIMER_FREEZE(wd->timer);

	evas_object_event_callback_add(playing_pannel, EVAS_CALLBACK_FREE, _widget_del_cb, wd);

	edje_object_signal_callback_add(_EDJ(playing_pannel), "now_playing_clicked", "*", _mp_now_left_area_clicked_cb, wd);

	//elm_object_signal_callback_add(playing_pannel, SIGNAL_NOW_PLAYING_CLICKED, "*", _mp_now_playing_control_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_PREVIOUS, "*", _mp_now_playing_control_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_NP_PLAY, "*", _mp_now_playing_control_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_NP_PAUSE, "*", _mp_now_playing_control_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_NEXT, "*", _mp_now_playing_control_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_MOUSE_DOWN, "control_previous",_mp_now_playing_seek_btn_down_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_MOUSE_UP, "control_previous",_mp_now_playing_seek_btn_up_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_MOUSE_DOWN, "control_next",_mp_now_playing_seek_btn_down_cb, wd);
	//elm_object_signal_callback_add(playing_pannel, SIGNAL_MOUSE_UP, "control_next",_mp_now_playing_seek_btn_up_cb, wd);

	elm_object_part_content_set(wd->now_playing, "now_playing_label", wd->label);

	/*set focused UI*/
	elm_object_focus_allow_set(playing_pannel, EINA_TRUE);
	elm_object_focus_allow_set(progress, EINA_TRUE);
	//elm_object_focus_custom_chain_append(playing_pannel, progress_focus_btn, NULL);

	Evas_Object *prev_focus_btn = elm_button_add(playing_pannel);
	elm_object_style_set(prev_focus_btn, "focus");
	elm_object_part_content_set(playing_pannel, "previous_focus", prev_focus_btn);
	elm_object_focus_custom_chain_append(playing_pannel, prev_focus_btn, NULL);
	evas_object_smart_callback_add(prev_focus_btn, "pressed", _mp_now_playing_btn_pressed_cb, "control_previous");
	evas_object_smart_callback_add(prev_focus_btn, "unpressed", _mp_now_playing_btn_unpressed_cb, "control_previous");

	Evas_Object *play_pause_focus_btn = elm_button_add(playing_pannel);
	elm_object_style_set(play_pause_focus_btn, "focus");
	elm_object_part_content_set(playing_pannel, "play_pause_focus", play_pause_focus_btn);
	elm_object_focus_custom_chain_append(playing_pannel, play_pause_focus_btn, NULL);
	evas_object_smart_callback_add(play_pause_focus_btn, "clicked", _mp_now_playing_btn_clicked_cb, wd);
	Evas_Object *next_focus_btn = elm_button_add(playing_pannel);
	elm_object_style_set(next_focus_btn, "focus");
	elm_object_part_content_set(playing_pannel, "next_focus", next_focus_btn);
	elm_object_focus_custom_chain_append(playing_pannel, next_focus_btn, NULL);
	evas_object_smart_callback_add(next_focus_btn, "pressed", _mp_now_playing_btn_pressed_cb, "control_next");
	evas_object_smart_callback_add(next_focus_btn, "unpressed", _mp_now_playing_btn_unpressed_cb, "control_next");

	_mp_now_playing_update_btn(wd->now_playing);

	return playing_pannel;
}

void mp_now_playing_thaw_timer(Evas_Object *now_playing)
{
	startfunc;
	MpNpData_t *wd = GET_WIDGET_DATA(now_playing);
	MP_CHECK(wd);

	elm_label_slide_mode_set(wd->label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_slide_go(wd->label);

	if (mp_player_mgr_get_state() == PLAYER_STATE_PLAYING) {
		MP_TIMER_THAW(wd->timer);
	}
}

void mp_now_playing_freeze_timer(Evas_Object *now_playing)
{
	startfunc;
	MpNpData_t *wd = GET_WIDGET_DATA(now_playing);
	MP_CHECK(wd);

#ifdef MP_NOW_PLAYING_MINI_EQ
	elm_object_part_content_set(wd->mini_eq, "image1", NULL);
	elm_object_part_content_set(wd->mini_eq, "image2", NULL);
	elm_object_part_content_set(wd->mini_eq, "image3", NULL);
#endif

	elm_label_slide_mode_set(wd->label, ELM_LABEL_SLIDE_MODE_NONE);
	elm_label_slide_go(wd->label);

	MP_TIMER_FREEZE(wd->timer);
}

static void _mp_now_playing_set_title(Evas_Object *now_playing, const char *title, const char *artist)
{
	startfunc;
	MpNpData_t *wd = GET_WIDGET_DATA(now_playing);
	MP_CHECK(wd);

	char *markup_title = elm_entry_utf8_to_markup(title);
	char *markup_artist = elm_entry_utf8_to_markup(artist);
	char *mtitle = NULL;
	char *martist = NULL;

#ifdef MP_FEATURE_LANDSCAPE
	bool landscape = mp_util_is_landscape();
	if (wd->landscape_layout_mode) {
		mtitle = g_strdup_printf("<align=left><font_size=%d><color=#%s>%s</color></font_size> <font_size=%d><color=#%s>- %s</color></font_size></align>",
		                         NOW_PLAYING_TITLE_SIZE, NOW_PLAYING_TITLE_COLOR,
		                         markup_title,
		                         NOW_PLAYING_ARTIST_SIZE, NOW_PLAYING_ARTIST_COLOR,
		                         markup_artist);
	} else
#endif
	{
		char *title_format = "<align=left><font_size=%d>%s</font_size></align>";
		mtitle = g_strdup_printf(title_format, 28, markup_title);

		martist = g_strdup_printf(title_format, 25, markup_artist);
	}
	Evas_Object *label_title = elm_label_add(wd->now_playing);
	elm_object_style_set(label_title, "slide_roll");
	elm_label_slide_mode_set(label_title, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label_title, 1);
	elm_object_text_set(label_title, mtitle);
	elm_label_slide_go(label_title);
	elm_object_part_content_set(wd->now_playing, "now_playing_label", label_title);

	Evas_Object *label_artist = elm_label_add(wd->now_playing);

	elm_object_style_set(label_artist, "slide_roll");
	elm_label_slide_mode_set(label_artist, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label_artist, 1);
	elm_object_text_set(label_artist, martist);
	elm_label_slide_go(label_artist);
	elm_object_part_content_set(wd->now_playing, "now_playing_artist", label_artist);

	IF_FREE(mtitle);
	IF_FREE(martist);
	IF_FREE(markup_title);
	IF_FREE(markup_artist);

}
void mp_now_playing_update(Evas_Object *now_playing, const char *title, const char *artist, const char *thumbnail, bool with_title)
{
	startfunc;
	MpNpData_t *wd = GET_WIDGET_DATA(now_playing);
	MP_CHECK(wd);
	MP_CHECK(wd->thumbnail);
	//MP_CHECK(wd->label_artist);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	elm_image_file_set(wd->thumbnail, thumbnail, NULL);
	if (with_title) {
		_mp_now_playing_set_title(now_playing, title, artist);
	}

	ad->music_pos = mp_player_mgr_get_position() / 1000.0;

	if (ad->player_state == PLAY_STATE_PAUSED && ad->is_Longpress) {
		_progressbar_value_set(wd);
	}
	/*
	else if (ad->player_state == PLAY_STATE_PAUSED && !((MpView_t *)(wd->userdata))->rotate_flag)
	{
		_progressbar_value_update_to_zero(wd);
	}
	*/
	else if (((MpView_t *)(wd->userdata))->rotate_flag) {
		_progressbar_value_set(wd);
#ifdef MP_NOW_PLAYING_MINI_EQ
		elm_object_part_content_set(wd->mini_eq, "image1", NULL);
		elm_object_part_content_set(wd->mini_eq, "image2", NULL);
		elm_object_part_content_set(wd->mini_eq, "image3", NULL);
#endif
	}

#ifndef MP_SOUND_PLAYER
	mp_setting_save_now_playing(ad);
#endif

	_mp_now_playing_update_time(wd);
	_mp_now_playing_update_playpause_btn(wd->now_playing);
}

bool mp_now_playing_is_landscape(Evas_Object *now_playing)
{
	MP_CHECK_FALSE(now_playing);
	MpNpData_t *wd = GET_WIDGET_DATA(now_playing);
	MP_CHECK_FALSE(wd);

	return wd->landscape;
}

void mp_now_playing_set_layout(Evas_Object *now_playing)
{
	_set_layout(now_playing);
}

