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

#include "mp-list-view.h"
#include "music.h"
#include "mp-util.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-edit-view.h"
#include "mp-add-track-view.h"
#include "mp-playlist-detail-view.h"
#include "mp-select-track-view.h"

#define CHECK_LIST_VIEW(view, val) \
	do {\
		MP_CHECK_VAL(view, val);\
		mp_retvm_if (view->list_view_magic != LIST_VIEW_MAGIC, val,\
		             "Error: param is not view object!!!", view->list_view_magic);\
	} while (0);


static int
_mp_list_view_set_edit_mode(void *thiz,  bool edit)
{
	startfunc;

	MpListView_t *view = (MpListView_t *)thiz;
	MP_CHECK_VAL(view, -1);
	mp_evas_object_del(view->selection_info);

	MpList_t *list = view->content_to_show;
	MP_CHECK_VAL(list, -1);
	mp_list_set_edit(list, edit);
	mp_list_view_set_select_all(view, edit);

	return 0;
}

static void _mp_list_select_all_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpListView_t *view = (MpListView_t *)data;
	mp_retm_if(view == NULL, "view is NULL");

	mp_list_item_data_t *it_data;
	Elm_Object_Item *it;

	MP_CHECK(view->content_to_show);
	Evas_Object *genlist = view->content_to_show->genlist;
	MP_CHECK(genlist);

	it = mp_list_first_item_get(genlist);
	/* check if all the item selected */
	bool all_selected_flag = TRUE;
	while (it) {
		if (mp_list_item_select_mode_get(it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY || elm_object_item_disabled_get(it) == TRUE) {
			it = mp_list_item_next_get(it);
			continue;
		}

		it_data = elm_object_item_data_get(it);
		if (it_data) {
			if (it_data->checked == FALSE) {
				all_selected_flag = FALSE;
				break;
			}
		}
		it = mp_list_item_next_get(it);
	}
	/* set items */
	Eina_Bool value = EINA_FALSE;
	value = all_selected_flag ? EINA_FALSE : EINA_TRUE;

	DEBUG_TRACE("all_selected_flag is %d\tvalue is %d", all_selected_flag, value);
	it = mp_list_first_item_get(genlist);
	MpList_t *list = view->content_to_show;
	while (it) {
		if (mp_list_item_select_mode_get(it) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY || elm_object_item_disabled_get(it) == TRUE) {
			it = mp_list_item_next_get(it);
			continue;
		}
		mp_list_item_check_set(it, value);
		it = mp_list_item_next_get(it);
	}
	/* view->selection_info = mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count(list)); */

	mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count(list));
	mp_view_update_options_edit((MpView_t *)view);

	endfunc;
}

static void _mp_list_select_all_layout_down_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	MP_CHECK(obj);
	Evas_Object *check = elm_object_part_content_get(obj, "elm.icon");
	Eina_Bool state = elm_check_state_get(check);
	elm_check_state_set(check, !state);
	_mp_list_select_all_cb(data, check, NULL);
}

static Evas_Object *_mp_list_view_set_select_all(void *thiz,  bool edit)
{

	MpListView_t *view = (MpListView_t *)thiz;

	if (view->select_all_btn) {
		evas_object_del(view->select_all_btn);
		view->select_all_btn = NULL;
	}

	if (edit == TRUE) {
		mp_widget_create_select_all_layout(view->layout, _mp_list_select_all_cb, _mp_list_select_all_layout_down_cb, view, &view->select_all_btn, &view->select_all_layout);
		elm_object_part_content_set(view->layout, "select_all_bg", view->select_all_layout);
		elm_object_signal_emit(view->layout, "SHOW_INFO_TEXT_BAR", "");
	} else {
		elm_object_signal_emit(view->layout, "HIDE_INFO_TEXT_BAR", "");
	}

	return view->select_all_btn;
}

static int
_mp_list_view_double_tap(void *thiz)
{
	startfunc;
	MpListView_t *view = (MpListView_t *)thiz;
	mp_list_double_tap(view->content_to_show);
	return 0;
}

static Evas_Object *_mp_list_view_set_done_button(void *thiz,  bool edit, mp_done_operator_type_t type)
{

	MpListView_t *view = (MpListView_t *)thiz;
	Evas_Object *done_button = NULL;

	if (view->done_btn) {
		evas_object_del(view->done_btn);
		view->done_btn = NULL;
	}

	if (edit == TRUE) {
		if (type == MP_DONE_DELETE_TYPE) {
			done_button =  mp_widget_create_navi_right_btn(view->layout, view->navi_it, mp_edit_view_delete_cb, view);
		} else if (type == MP_DONE_ADD_TO_TYPE) {
			done_button =  mp_widget_create_navi_right_btn(view->layout, view->navi_it, mp_edit_view_add_to_playlist_cb, view);
		} else if (type == MP_DONE_ADD_TRACK_TYPE) {
			done_button =  mp_widget_create_navi_right_btn(view->layout, view->navi_it, mp_add_track_view_add_to_playlist_cb, view);
		} else if (type == MP_DONE_REMOVED_TYPE) {
			done_button =  mp_widget_create_navi_right_btn(view->layout, view->navi_it, mp_edit_view_remove_cb, view);
		} else if (type == MP_DONE_REORDER_TYPE) {
			done_button =  mp_widget_create_navi_right_btn(view->layout, view->navi_it, mp_edit_view_list_item_reorder_update_cb, view);
		} else if (type == MP_DONE_SELECT_ADD_TRACK_TYPE) {
			done_button =  mp_widget_create_navi_right_btn(view->layout, view->navi_it, mp_select_track_view_add_to_playlist_cb, view->content_to_show);
		}
		MP_CHECK_NULL(done_button);
		view->done_btn = done_button;
		elm_object_disabled_set(view->done_btn, EINA_TRUE);
	}

	return done_button;
}

static Evas_Object *_mp_list_view_set_cancel_button(void *thiz,  bool edit)
{

	MpListView_t *view = (MpListView_t *)thiz;
	Evas_Object *cancel_button = NULL;

	if (view->cancel_btn) {
		evas_object_del(view->cancel_btn);
		view->cancel_btn = NULL;
	}

	if (edit == TRUE) {
		cancel_button =  mp_widget_create_navi_left_btn(view->layout, view->navi_it, mp_common_view_cancel_cb, view);
		MP_CHECK_NULL(cancel_button);
		view->cancel_btn = cancel_button;
	}

	return cancel_button;
}


int mp_list_view_init(Evas_Object *parent, MpListView_t *view, MpViewType_e view_type, ...)
{
	startfunc;
	int ret = 0;
	MP_CHECK_VAL(view, -1);

	/*initalize parent class */
	ret = mp_view_init(parent, (MpView_t *)view, view_type);
	MP_CHECK_VAL(ret == 0, -1);

	/*initialize data*/
	view->list_view_magic = LIST_VIEW_MAGIC;

	/*parent's function overriding.
	view->update = _mp_list_view_update;

	initialize functions*/
	view->set_edit_mode = _mp_list_view_set_edit_mode;
	view->set_select_all = _mp_list_view_set_select_all;
	view->double_tap = _mp_list_view_double_tap;
	view->set_done_button = _mp_list_view_set_done_button;
	view->set_cancel_button = _mp_list_view_set_cancel_button;

	return 0;
}

int mp_list_view_fini(MpListView_t *view)
{
	startfunc;
	int ret = 0;
	CHECK_LIST_VIEW(view, -1);

	view->list_view_magic = 0;

	ret = mp_view_fini((MpView_t *)view);
	MP_CHECK_VAL(ret == 0, -1);

	return 0;
}

int mp_list_view_is_list_view(MpListView_t *view, bool *val)
{
	MP_CHECK_VAL(val, -1);
	*val = false;
	CHECK_LIST_VIEW(view, -1);
	*val = true;
	return 0;
}

int mp_list_view_set_edit_mode(MpListView_t *view,  bool edit)
{
	CHECK_LIST_VIEW(view, -1);
	MP_CHECK_VAL(view->set_edit_mode, -1);
	view->set_edit_mode(view, edit);

	return 0;
}

Evas_Object *mp_list_view_set_select_all(MpListView_t *view,  bool flag)
{
	CHECK_LIST_VIEW(view, NULL);
	MP_CHECK_VAL(view->set_select_all, NULL);
	return view->set_select_all(view, flag);
}

int mp_list_view_double_tap(MpListView_t *view)
{
	CHECK_LIST_VIEW(view, -1);
	MP_CHECK_VAL(view->double_tap, -1);
	view->double_tap(view);
	return 0;
}
Evas_Object *mp_list_view_set_done_btn(MpListView_t *view,  bool flag, mp_done_operator_type_t type)
{
	CHECK_LIST_VIEW(view, NULL);
	MP_CHECK_VAL(view->set_done_button, NULL);
	return view->set_done_button(view, flag, type);
}

Evas_Object *mp_list_view_set_cancel_btn(MpListView_t *view,  bool flag)
{
	CHECK_LIST_VIEW(view, NULL);
	MP_CHECK_VAL(view->set_cancel_button, NULL);
	return view->set_cancel_button(view, flag);
}
