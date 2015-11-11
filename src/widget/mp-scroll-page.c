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


/*this fill should be modified by removing the smart object*/
#include <Ecore.h>
#include <stdbool.h>
#include "mp-scroll-page.h"
#include "mp-define.h"

#define SCROLL_PAGE_WIDTH	720
#define SCROLL_PAGE_FLICK_DISTANCE	20

typedef struct {
	Evas_Object *obj;
	Evas_Object *scroller;
	Evas_Object *box;
	Ecore_Timer *bring_timer;
	Ecore_Animator *animator;

	int page_width;
	int page_height;

	int page_count;
	bool drag_started;
	int prev_x;
	int current_page;

	void (*page_change_callback)(int page, void *user_data);
	void *page_change_user_data;

	bool reloacation_flag;
	MpScrollPageType_e location_page_type;
} scroll_page_s;

static Evas_Object *_mp_scroll_page_scroller_get_page_at(void *data, unsigned int idx)
{
	scroll_page_s *sd = (scroll_page_s *)data;
	MP_CHECK_NULL(sd);

	Eina_List *page_list;
	Evas_Object *box;
	Evas_Object *page;

	box = sd->box;
	page_list = elm_box_children_get(box);
	if (NULL == page_list) {
		return NULL;
	}

	page = eina_list_nth(page_list, idx);
	eina_list_free(page_list);

	return page;
}

static void _mp_scroll_page_scroller_focus(void *data)
{
	scroll_page_s *sd = (scroll_page_s *)data;
	MP_CHECK(sd);

	Evas_Object *page = NULL;
	page = _mp_scroll_page_scroller_get_page_at(sd, sd->current_page);
	MP_CHECK(page);

	elm_object_focus_set(page, EINA_TRUE);
}

static void
_mp_scroll_page_scroller_drag_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	scroll_page_s *sd = data;
	MP_CHECK(sd);

	sd->drag_started = true;
}

static void
_mp_scroll_page_scroller_drag_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	scroll_page_s *sd = data;
	MP_CHECK(sd);

	if (sd->drag_started == false) {
		return;
	}

	int pagenum;
	int prev_page = sd->current_page;
	elm_scroller_current_page_get(sd->scroller, &pagenum, NULL);
	sd->current_page = pagenum;
	sd->drag_started = false;

	_mp_scroll_page_scroller_focus(sd);
	/*if page ckange callback set, call it*/
	if (sd->page_change_callback && prev_page != sd->current_page) {
		sd->page_change_callback(sd->current_page, sd->page_change_user_data);
	}

}

static void
_mp_scroll_page_scroller_anim_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	scroll_page_s *sd = data;
	MP_CHECK(sd);

	int pagenum;
	int prev_page = sd->current_page;
	elm_scroller_current_page_get(sd->scroller, &pagenum, NULL);
	sd->current_page = pagenum;

	_mp_scroll_page_scroller_focus(sd);
	/*if page ckange callback set, call it*/
	if (sd->page_change_callback && prev_page != sd->current_page) {
		sd->page_change_callback(sd->current_page, sd->page_change_user_data);
	}
}


static Eina_Bool
_animator_cb(void *data)
{
	scroll_page_s *sd = (scroll_page_s *)data;
	MP_CHECK_FALSE(sd);

	elm_scroller_page_show(sd->scroller, sd->current_page, 0);
	sd->animator = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static void
_mp_scroll_page_smart_add(Evas_Object *obj)
{
	startfunc;
	MP_CHECK(obj);

	scroll_page_s *sd = calloc(1, sizeof(scroll_page_s));
	mp_assert(sd);

	sd->obj = obj;
	evas_object_smart_data_set(obj, sd);

	endfunc;
}

static void
_mp_scroll_page_smart_del(Evas_Object *obj)
{
	startfunc;
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);
	mp_ecore_timer_del(sd->bring_timer);
	mp_ecore_animator_del(sd->animator);
	free(sd);
	endfunc;
}

static void
_mp_scroll_page_smart_move(Evas_Object *obj, Evas_Coord x, Evas_Coord y)
{
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	evas_object_move(sd->scroller, x, y);
	evas_object_smart_changed(obj);
}

static void
_mp_scroll_page_smart_resize(Evas_Object *obj, Evas_Coord w, Evas_Coord h)
{
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	evas_object_resize(sd->scroller, w, h);
	elm_scroller_page_size_set(sd->scroller, w, 0);

	sd->page_width = w;
	sd->page_height = h;
	evas_object_smart_changed(obj);

	mp_ecore_animator_del(sd->animator);
	/*FIXME: we used animator here because directly using page_show() doesn't change the page properly.
	  Using animator shows a flickering when page switching.
	  Need to fix it in elm_scroller. */
	sd->animator = ecore_animator_add(_animator_cb, sd);
}

static void
_mp_scroll_page_smart_show(Evas_Object *obj)
{
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	if (sd->scroller) {
		evas_object_show(sd->scroller);
	}
}

static void
_mp_scroll_page_smart_hide(Evas_Object *obj)
{
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	if (sd->scroller) {
		evas_object_hide(sd->scroller);
	}
}

static void
_mp_scroll_page_smart_clip_set(Evas_Object *obj, Evas_Object *clip)
{
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	if (sd->scroller) {
		evas_object_clip_set(sd->scroller, clip);
	}
}

static void
_mp_scroll_page_smart_clip_unset(Evas_Object *obj)
{
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	if (sd->scroller) {
		evas_object_clip_unset(sd->scroller);
	}
};


static void
_mp_scroll_page_smart_calculate(Evas_Object *obj)
{
	MP_CHECK(obj);
	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);
	MP_CHECK(sd->box);

	Eina_List *layout_list = elm_box_children_get(sd->box);
	MP_CHECK(layout_list);

	Eina_List *l;
	Evas_Object *layout;
	EINA_LIST_FOREACH(layout_list, l, layout) {
		Evas_Object *rect = elm_object_part_content_get(layout, "elm.swallow.bg");
		if (rect) {
			evas_object_size_hint_min_set(rect, sd->page_width, 0);
		}
		Evas_Object *content = elm_object_part_content_get(layout, "elm.swallow.content");
		if (content) {
			evas_object_size_hint_min_set(content, sd->page_width, 0);
		}

	}
	eina_list_free(layout_list);
	layout_list = NULL;

	if (sd->reloacation_flag == TRUE && (sd->location_page_type > SCROLL_PAGE_MIN && sd->location_page_type < SCROLL_PAGE_MAX)) {
		Evas_Coord	x, y, w, h;
		elm_scroller_region_get(sd->scroller, &x, &y, &w, &h);
		x = sd->page_width * sd->location_page_type;
		elm_scroller_region_show(sd->scroller, x, y, w, h);
		elm_scroller_current_page_get(sd->scroller, &(sd->current_page), NULL);
		DEBUG_TRACE("current page is %d", sd->current_page);
		sd->prev_x = x;
		sd->reloacation_flag = FALSE;
		sd->location_page_type = SCROLL_PAGE_MIN;
	}
}


Evas_Object *
mp_scroll_page_add(Evas_Object *parent)
{
	Evas_Object *obj;
	static Evas_Smart_Class sc;
	static Evas_Smart *smart = NULL;

	if (!smart)	{
		memset(&sc, 0x0, sizeof(Evas_Smart_Class));
		sc.name = "page_control";
		sc.version = EVAS_SMART_CLASS_VERSION;
		sc.add = _mp_scroll_page_smart_add;
		sc.del = _mp_scroll_page_smart_del;
		sc.move = _mp_scroll_page_smart_move;
		sc.resize = _mp_scroll_page_smart_resize;
		sc.show = _mp_scroll_page_smart_show;
		sc.hide = _mp_scroll_page_smart_hide;
		sc.clip_set = _mp_scroll_page_smart_clip_set;
		sc.clip_unset = _mp_scroll_page_smart_clip_unset;
		sc.calculate = _mp_scroll_page_smart_calculate;
		if (!(smart = evas_smart_class_new(&sc))) {
			return NULL;
		}
	}

	obj = evas_object_smart_add(evas_object_evas_get(parent), smart);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	if (sd) {

		Evas_Object *scroller = elm_scroller_add(parent);
		elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_FALSE);
		elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
		elm_object_scroll_lock_y_set(scroller, EINA_TRUE);
		elm_scroller_single_direction_set(scroller, ELM_SCROLLER_SINGLE_DIRECTION_HARD);
		sd->scroller = scroller;

		Evas_Object *box = elm_box_add(scroller);
		elm_box_horizontal_set(box, EINA_TRUE);
		elm_box_homogeneous_set(box, EINA_TRUE);
		elm_object_content_set(scroller, box);
		sd->box = box;

		evas_object_smart_callback_add(scroller, "scroll,drag,start", _mp_scroll_page_scroller_drag_start_cb, sd);
		evas_object_smart_callback_add(scroller, "scroll,drag,stop", _mp_scroll_page_scroller_drag_stop_cb, sd);
		evas_object_smart_callback_add(scroller, "scroll,anim,stop", _mp_scroll_page_scroller_anim_stop_cb, sd);

		evas_object_smart_member_add(scroller, obj);
	}

	return obj;
}

static Evas_Object *
_mp_scroll_page_min_size_layout_add(Evas_Object *parent, Evas_Object *content, scroll_page_s *sd)
{
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(content);
	MP_CHECK_NULL(sd);

	Evas_Object *layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *rect = evas_object_rectangle_add(evas_object_evas_get(layout));
	evas_object_color_set(rect, 248, 246, 239, 255);
	evas_object_size_hint_min_set(rect, sd->page_width, sd->page_height);
	evas_object_size_hint_weight_set(rect, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(rect, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(layout, "elm.swallow.bg", rect);

	elm_object_part_content_set(layout, "elm.swallow.content", content);
	evas_object_show(layout);

	return layout;
}

void
mp_scroll_page_content_append(Evas_Object *obj, Evas_Object *content)
{
	MP_CHECK(obj);
	MP_CHECK(content);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);
	MP_CHECK(sd->box);

	Evas_Object *min_layout = _mp_scroll_page_min_size_layout_add(obj, content, sd);
	elm_box_pack_end(sd->box, min_layout);
	sd->page_count++;
}

void
mp_scroll_page_content_append_typed(Evas_Object *obj, Evas_Object *content, MpScrollPageType_e page_type)
{
	MP_CHECK(obj);
	MP_CHECK(content);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);
	MP_CHECK(sd->box);

	Evas_Object *min_layout = _mp_scroll_page_min_size_layout_add(obj, content, sd);
	evas_object_data_set(min_layout, "page_type", (void *)page_type);
	elm_box_pack_end(sd->box, min_layout);
	sd->page_count++;
}

void
mp_scroll_page_content_pre_append(Evas_Object *obj, Evas_Object *content)
{
	MP_CHECK(obj);
	MP_CHECK(content);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);
	MP_CHECK(sd->box);

	Evas_Object *min_layout = _mp_scroll_page_min_size_layout_add(obj, content, sd);
	elm_box_pack_start(sd->box, min_layout);
	sd->page_count++;
}

void
mp_scroll_page_remove(Evas_Object *obj, MpScrollPageType_e page_type)
{
	MP_CHECK(obj);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);
	MP_CHECK(sd->box);

	Eina_List *children_list = NULL;
	Eina_List *l = NULL;
	Evas_Object *sub_obj = NULL;

	children_list = elm_box_children_get(sd->box);
	EINA_LIST_FOREACH(children_list, l, sub_obj) {
		MpScrollPageType_e obj_type = (MpScrollPageType_e)evas_object_data_get(sub_obj, "page_type");
		if (obj_type == page_type) {
			elm_box_unpack(sd->box, sub_obj);
			sd->page_count--;
			break;
		}
	}

	if (children_list) {
		eina_list_free(children_list);
		children_list = NULL;
	}
}

Evas_Object *
mp_scroll_page_index_icon_add(Evas_Object *parent, unsigned int total, int index)
{
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(index < total);

	Evas_Object *box = elm_box_add(parent);
	elm_box_horizontal_set(box, EINA_TRUE);
	elm_box_align_set(box, 0.0, 0.5);
	elm_box_homogeneous_set(box, EINA_TRUE);
	elm_box_padding_set(box, 5, 0);

	int i;
	for (i = 0; i < total ; i++) {
		const char *group = (i == index) ? MP_ICON_PAGE_INDEX_ON : MP_ICON_PAGE_INDEX_OFF;
		Evas_Object *icon = elm_icon_add(box);
		elm_image_file_set(icon, IMAGE_EDJ_NAME, group);
		evas_object_size_hint_min_set(icon, SCALED_SIZE(29), SCALED_SIZE(7));
		evas_object_show(icon);
		elm_box_pack_end(box, icon);
	}

	evas_object_show(box);
	return box;
}

void mp_scroll_page_set_page_change_callback(Evas_Object *obj, page_change_callback callback, void *userdata)
{
	MP_CHECK(obj);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	sd->page_change_callback = callback;
	sd->page_change_user_data = userdata;
}

void
mp_scroll_page_set_page_location(Evas_Object *obj, MpScrollPageType_e page_type)
{
	MP_CHECK(obj);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);
	MP_CHECK(sd->box);

	switch (page_type) {
	case SCROLL_PAGE_STORE:
		sd->current_page = 0;
		break;
	case SCROLL_PAGE_RADIO:
		sd->current_page = (sd->page_count - 2);
		break;
	case SCROLL_PAGE_PLAYER:
		sd->current_page = (sd->page_count - 1);
		break;
	default:
		mp_error("unhandled page type %d", page_type);
		return;
	}
	elm_scroller_page_show(sd->scroller, sd->current_page, 0);
	_mp_scroll_page_scroller_focus(sd);
}


MpScrollPageType_e mp_scroll_page_get_current_page_type(Evas_Object *obj)
{
	MP_CHECK_VAL(obj, SCROLL_PAGE_PLAYER);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK_VAL(sd, SCROLL_PAGE_PLAYER);
	MP_CHECK_VAL(sd->box, SCROLL_PAGE_PLAYER);

	if (sd->current_page == (sd->page_count - 1)) {
		return SCROLL_PAGE_PLAYER;
	} else if (sd->current_page == 0) {
		return SCROLL_PAGE_STORE;
	} else {
		return SCROLL_PAGE_RADIO;
	}
}

void mp_scroll_page_hide_scroll_bar(Evas_Object *obj)
{
	MP_CHECK(obj);

	scroll_page_s *sd = evas_object_smart_data_get(obj);
	MP_CHECK(sd);

	elm_object_style_set(sd->scroller, NULL);

}
