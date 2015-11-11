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

#include "mp-player-volume-widget.h"
#include "mp-player-debug.h"
#include "mp-define.h"
#include "mp-volume.h"
#include "mp-widget.h"
#include "mp-player-mgr.h"
#include "mp-util.h"
#include "mp-common.h"

#define VOLUME_WIDGET_GROUP_NAME		"mp-volume-widget"
#define VOLUME_WIDGET_GROUP_NAME_LD		"mp-volume-widget-ld"
#define VOLUME_WIDGET_SLIDER_HANDLE		"volume_widget_slider_control_area"
#define VOLUME_WIDGET_INDICATOR			"volume_widget_indicator"


#define VOLUME_WIDGET_SLIDER_LD_SIZE		(233 * elm_config_scale_get())
#define VOLUME_WIDGET_SLIDER_LD_START_POINT	(94 * elm_config_scale_get())

#define VOLUME_WIDGET_SLIDER_LEVEL_MAX		15
#define VOLUME_WIDGET_SLIDER_LEVEL_MIN		0

#define MP_VOLUME_WIDGET_W 	64
#define MP_VOLUME_WIDGET_H	348

#define MP_VOLUME_WIDGET_LD_W 	116
#define MP_VOLUME_WIDGET_LD_H	438

#define VOLUME_SLIDER_MUL_START_POINT	96

#ifndef _EDJ
#define _EDJ(obj) elm_layout_edje_get(obj) /**< get evas object from elm layout */
#endif

typedef struct {
	Evas_Object *obj;
	bool dragging;

	Volume_Widget_Cb event_cb;
	void *user_data;

	int max;
	int current;
} Volume_Widget_Data;

int VOLUME_WIDGET_SLIDER_SIZE;
int VOLUME_WIDGET_SLIDER_START_POINT;

static inline void
_mp_player_volume_widget_set_indicator(Volume_Widget_Data *wd, int val)
{
	MP_CHECK(wd);

	char *text = g_strdup_printf("%d", val);
	elm_object_part_text_set(wd->obj, VOLUME_WIDGET_INDICATOR, text);
	SAFE_FREE(text);

	wd->current = val;
}

static inline int
_mp_player_volume_widget_get_val(Volume_Widget_Data *wd)
{
	MP_CHECK_VAL(wd, 0);

	double val = 0.0;
	edje_object_part_drag_value_get(_EDJ(wd->obj), VOLUME_WIDGET_SLIDER_HANDLE, NULL, &val);
	return (int)(val * wd->max);
}

static void
_mp_player_volume_widget_drag_start_cb(void *data, Evas_Object *obj, const char *emission,	const char *source)
{
	startfunc;
	Volume_Widget_Data *wd = data;
	MP_CHECK(wd);

	if (wd->event_cb) {
		wd->event_cb(wd->user_data, obj, VOLUME_WIDGET_EVENT_DRAG_START);
	}
}

static void
_mp_player_volume_widget_drag_stop_cb(void *data, Evas_Object *obj, const char *emission,	const char *source)
{
	startfunc;
	Volume_Widget_Data *wd = data;
	MP_CHECK(wd);

	if (wd->event_cb) {
		wd->event_cb(wd->user_data, obj, VOLUME_WIDGET_EVENT_DRAG_STOP);
	}
}

static void
_mp_player_volume_widget_mousedown_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	if (data == NULL && obj == NULL && event_info == NULL) {
		return;
	}

	if ((elm_config_scale_get() - 1.7) < 0.0001) {
		VOLUME_WIDGET_SLIDER_SIZE = 274;
		VOLUME_WIDGET_SLIDER_START_POINT = 506;
	} else if ((elm_config_scale_get() - 1.8) < 0.0001) {
		VOLUME_WIDGET_SLIDER_SIZE = 216;
		VOLUME_WIDGET_SLIDER_START_POINT = 420;
	} else if ((elm_config_scale_get() - 2.4) < 0.0001) {
		VOLUME_WIDGET_SLIDER_SIZE = 376;
		VOLUME_WIDGET_SLIDER_START_POINT = 689;
	} else if ((elm_config_scale_get() - 2.6) < 0.0001) {
		VOLUME_WIDGET_SLIDER_SIZE = 376;
		VOLUME_WIDGET_SLIDER_START_POINT = 689;
	} else if ((elm_config_scale_get() - 2.8) < 0.0001) {
		VOLUME_WIDGET_SLIDER_SIZE = 376;
		VOLUME_WIDGET_SLIDER_START_POINT = 689;
	}
	Volume_Widget_Data *wd = data;
	MP_CHECK(wd);

	Evas_Event_Mouse_Down *ev = event_info;
	int current = 0;

	int val = mp_player_mgr_volume_get_current();

	int max_vol = VOLUME_WIDGET_SLIDER_LEVEL_MAX;

#ifdef MP_FEATURE_LANDSCAPE
	if (mp_util_is_landscape()) {
		current = ev->canvas.y - VOLUME_WIDGET_SLIDER_LD_START_POINT;
		double dval = ((VOLUME_WIDGET_SLIDER_LD_SIZE - current) * max_vol) / VOLUME_WIDGET_SLIDER_LD_SIZE + 1;
		val = ((VOLUME_WIDGET_SLIDER_LD_SIZE - current) * max_vol) / VOLUME_WIDGET_SLIDER_LD_SIZE + 1;
		if ((dval - val) > 0.5) {
			val =  val + 1;
		}
	} else
#endif
	{
		current = ev->canvas.y;
		double dval = (((VOLUME_WIDGET_SLIDER_START_POINT - current) * max_vol) * 1.0) / VOLUME_WIDGET_SLIDER_SIZE ;
		val = dval;
		DEBUG_TRACE("dval = %f, val = %d, dval-val=%f ", dval, val, (dval - val));
		if ((dval - val) > 0.5) {
			val =  val + 1;
		}
	}

	if (val < VOLUME_WIDGET_SLIDER_LEVEL_MIN) {
		val = VOLUME_WIDGET_SLIDER_LEVEL_MIN;
	} else if (val > max_vol) {
		val = max_vol;
	}

	//int val = _mp_player_volume_widget_get_val(wd);
	DEBUG_TRACE("val = %d", val);
	DEBUG_TRACE("wd->current = %d", wd->current);

	if (val != wd->current) {
		if (!mp_player_mgr_volume_set(val)) {
			return ;
		}
		_mp_player_volume_widget_set_indicator(wd, val);
	}
	if (wd->event_cb) {
		wd->event_cb(wd->user_data, obj, VOLUME_WIDGET_EVENT_DRAG_STOP);
	}

	endfunc;
}

static void
_mp_player_volume_widget_mouseup_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	if (data == NULL && obj == NULL && event_info == NULL) {
		return;
	}

	Volume_Widget_Data *wd = data;
	MP_CHECK(wd);

	mp_player_volume_widget_set_val(wd->obj, mp_player_mgr_volume_get_current());

	if (wd->event_cb) {
		wd->event_cb(wd->user_data, obj, VOLUME_WIDGET_EVENT_DRAG_STOP);
	}
	//when up ,the slider position is wrong cause by this code
	endfunc;
}


static void
_mp_player_volume_widget_changed_cb(void *data, Evas_Object *obj, const char *emission,	const char *source)
{
	startfunc;
	Volume_Widget_Data *wd = data;
	MP_CHECK(wd);

	int val = _mp_player_volume_widget_get_val(wd);
	DEBUG_TRACE("val = %d", val);
	DEBUG_TRACE("wd->current = %d", wd->current);
	if (val != wd->current) {
		if (!mp_player_mgr_volume_set(val)) {
			mp_player_volume_widget_set_val(wd->obj, 9);
		} else {
			_mp_player_volume_widget_set_indicator(wd, val);
		}
	}

	if (wd->event_cb) {
		wd->event_cb(wd->user_data, obj, VOLUME_WIDGET_EVENT_DRAG_MAX);
	}
}

static void
_mp_player_volume_widget_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Volume_Widget_Data *wd = data;
	SAFE_FREE(wd);
}

Evas_Object *
mp_player_volume_widget_add(Evas_Object *parent)
{
	startfunc;
	MP_CHECK_NULL(parent);

	Evas_Object *widget = NULL;
	if (mp_util_is_landscape()) {
		widget = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, VOLUME_WIDGET_GROUP_NAME_LD);
#ifdef MP_FEATURE_SPLIT_WINDOW
		evas_object_resize(widget, MP_VOLUME_WIDGET_LD_W, MP_VOLUME_WIDGET_LD_H);
#endif
	} else {
		widget = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, VOLUME_WIDGET_GROUP_NAME);
#ifdef MP_FEATURE_SPLIT_WINDOW
		evas_object_resize(widget, MP_VOLUME_WIDGET_W, MP_VOLUME_WIDGET_H);
#endif
	}
	evas_object_size_hint_weight_set(widget, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(widget, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Volume_Widget_Data *wd = calloc(1, sizeof(Volume_Widget_Data));
	MP_CHECK_NULL(wd);
	wd->obj = widget;
	wd->max = mp_player_mgr_volume_get_max();
	evas_object_data_set(widget, "widget_data", wd);

	mp_player_volume_widget_set_val(widget, mp_player_mgr_volume_get_current());

	Evas_Object *edj_obj = _EDJ(widget);
	edje_object_signal_callback_add(edj_obj, "drag,start", VOLUME_WIDGET_SLIDER_HANDLE, _mp_player_volume_widget_drag_start_cb, wd);
	edje_object_signal_callback_add(edj_obj, "drag", VOLUME_WIDGET_SLIDER_HANDLE, _mp_player_volume_widget_changed_cb, wd);
	edje_object_signal_callback_add(edj_obj, "drag,stop", VOLUME_WIDGET_SLIDER_HANDLE, _mp_player_volume_widget_drag_stop_cb, wd);

	evas_object_event_callback_add(widget, EVAS_CALLBACK_MOUSE_DOWN, _mp_player_volume_widget_mousedown_cb, wd);
	evas_object_event_callback_add(widget, EVAS_CALLBACK_MOUSE_UP, _mp_player_volume_widget_mouseup_cb, wd);
	evas_object_event_callback_add(widget, EVAS_CALLBACK_FREE, _mp_player_volume_widget_del_cb, wd);

#ifdef MP_SOUND_PLAYER
	edje_object_signal_emit(edj_obj, "hide,sound,alive", "*");
#endif

	return widget;
}

void
mp_player_volume_widget_event_callback_add(Evas_Object *obj, Volume_Widget_Cb event_cb, void *user_data)
{
	MP_CHECK(obj);
	Volume_Widget_Data *wd = evas_object_data_get(obj, "widget_data");
	MP_CHECK(wd);

	wd->event_cb = event_cb;
	wd->user_data = user_data;
}

int
mp_player_volume_widget_set_val(Evas_Object *obj, int val)
{
	MP_CHECK_VAL(obj, 0);
	Volume_Widget_Data *wd = evas_object_data_get(obj, "widget_data");
	MP_CHECK_VAL(wd, 0);

	if (val < 0) {
		val = 0;
	}
	if (val > wd->max) {
		val = wd->max;
	}
	double ratio = (double)val / (double)wd->max;
	edje_object_part_drag_value_set(_EDJ(wd->obj), VOLUME_WIDGET_SLIDER_HANDLE, 1.0, ratio);
	_mp_player_volume_widget_set_indicator(wd, val);

	return val;
}

