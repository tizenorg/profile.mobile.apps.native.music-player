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

#include <Elementary.h>
#include "mp-smart-event-box.h"
#include "mp-player-debug.h"
#include "mp-define.h"

#define SCROLL_THREASHOLD 20
#define FLICK_THREASHOLD 5

#ifndef ABS
#define ABS(x) ((x) < 0 ? -(x) : (x))
#endif

typedef struct _EventCallbackData_t {
	void (*func)(void *);
	void *data;
} EventCallbackData_t;

typedef struct _MpSmartEventBoxData_t {
	bool down;
	bool move;

	Evas_Coord x, y, w, h;
	Evas_Coord down_x;
	Evas_Coord down_y;

	EventCallbackData_t cbs[MP_EVENT_MAX];

} MpSmartEventBoxData_t;


static void
__mouse_down_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	Evas_Event_Mouse_Down *ev = (Evas_Event_Mouse_Down *) event_info;
	MpSmartEventBoxData_t *box_d = data;
	MP_CHECK(box_d);

	box_d->down_x = ev->canvas.x;
	box_d->down_y = ev->canvas.y;

	box_d->down = true;

	return;
}

static void
__mouse_up_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	MpSmartEventBoxData_t *box_d = data;
	MP_CHECK(box_d);

	box_d->down = false;
	//elm_coords_finger_size_adjust(1, &minw, 1, &minh);

	if (!box_d->move) {
		box_d->cbs[MP_EVENT_CLICK].func(box_d->cbs[MP_EVENT_CLICK].data);
	}

	box_d->down_x = -1;

	goto END;

END:
	box_d->move = false;
}

static void
__mouse_move_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	Evas_Coord diff_x = 0, diff_y = 0;
	Evas_Event_Mouse_Up *mu = (Evas_Event_Mouse_Up *) event_info;
	MpSmartEventBoxData_t *box_d = data;
	MP_CHECK(box_d);

	if (box_d->move) {
		return;
	}
	if (box_d->down_x < 0) {
		return;
	}

	//elm_coords_finger_size_adjust(1, &minw, 1, &minh);

	diff_x = box_d->down_x - mu->canvas.x;
	diff_y = box_d->down_y - mu->canvas.y;

	if ((ABS(diff_x) > SCROLL_THREASHOLD) || (ABS(diff_y) > SCROLL_THREASHOLD)) {
		// dragging
		box_d->move = true;

		//if (ABS(diff_y)<FLICK_THREASHOLD && ABS(diff_x)<FLICK_THREASHOLD)
		//goto END;

		if (ABS(diff_y) > SCROLL_THREASHOLD) {
			if (diff_y < 0) {	//down
				goto flick_down;
			} else {	//up
				goto flick_up;
			}
		} else {
			if (diff_x < 0) {
				//right
				goto flick_right;
			} else {
				//left
				goto flick_left;
			}
		}
	}

flick_up:
	goto END;

flick_down:
	goto END;

flick_left:
	box_d->cbs[MP_EVENT_LEFT].func(box_d->cbs[MP_EVENT_LEFT].data);
	goto END;

flick_right:
	box_d->cbs[MP_EVENT_RIGHT].func(box_d->cbs[MP_EVENT_RIGHT].data);
	goto END;

END:

	return;
}


static void
_mp_smart_event_box_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	MpSmartEventBoxData_t *b_data = data;
	IF_FREE(b_data);
}

Evas_Object *
mp_smart_event_box_add(Evas_Object * parent)
{
	//Evas *e = NULL;
	//Evas_Object *obj = NULL;
	MpSmartEventBoxData_t *data = NULL;

	Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(parent));
	MP_CHECK_NULL(rect);

	data = calloc(1, sizeof(MpSmartEventBoxData_t));
	MP_CHECK_NULL(data);

	evas_object_data_set(rect, "obj_data", data);

	//evas_object_size_hint_min_set(rect, 0, 15);
	evas_object_color_set(rect, 0, 0, 0, 0);
	evas_object_size_hint_fill_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, 0.0);


	evas_object_event_callback_add(rect, EVAS_CALLBACK_DEL, _mp_smart_event_box_del_cb, data);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_DOWN, __mouse_down_cb, data);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_MOVE, __mouse_move_cb, data);
	evas_object_event_callback_add(rect, EVAS_CALLBACK_MOUSE_UP, __mouse_up_cb, data);

	data->down_x = -1;

	return rect;
}

void mp_smart_event_box_callback_add(Evas_Object *event_box, MpEventCallback_e event, void (*event_cb)(void *), void *user_data)
{
	MpSmartEventBoxData_t *data = evas_object_data_get(event_box, "obj_data");
	MP_CHECK(data);

	data->cbs[event].func = event_cb;
	data->cbs[event].data = user_data;
}


