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

#include "mp-list.h"
#include "mp-util.h"
#include "mp-index.h"
#include "mp-widget.h"

#ifdef MP_FEATURE_SWEEP
static void
_mp_list_gl_flick_left_cb(void *data, Evas_Object * obj, void *event_info)
{
	//struct appdata *ad = (struct appdata *)data;
	//MP_CHECK(ad);
	//if (ad->vertical_scroll)
	//	return;
	elm_object_scroll_freeze_push(obj);
}

static void
_mp_list_gl_flick_right_cb(void *data, Evas_Object * obj, void *event_info)
{
	//struct appdata *ad = (struct appdata *)data;
	//MP_CHECK(ad);
	//if (ad->vertical_scroll)
	//	return;
	elm_object_scroll_freeze_push(obj);
}

static void
_mp_list_gl_flick_stop_cb(void *data, Evas_Object * obj, void *event_info)
{
	//struct appdata *ad = (struct appdata *)data;
	//MP_CHECK(ad);
	//ad->vertical_scroll = false;
	elm_object_scroll_freeze_pop(obj);
}

static void
_mp_list_gl_mode_left(void *data, Evas_Object * obj, void *event_info)
{
	MP_CHECK(obj);
	MP_CHECK(event_info);
	MP_CHECK(elm_genlist_item_select_mode_get(event_info) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	//MP_CHECK(elm_genlist_item_select_mode_get(event_info) != ELM_OBJECT_SELECT_MODE_NONE);
	// disable sweep if edit mode.
	MP_CHECK(elm_genlist_item_flip_get(event_info) == EINA_FALSE);
	// Finish genlist sweep
	elm_genlist_item_decorate_mode_set(event_info, "slide", EINA_FALSE);
	elm_genlist_item_select_mode_set(event_info, ELM_OBJECT_SELECT_MODE_DEFAULT);
	mp_list_item_data_t *item_data = elm_object_item_data_get(event_info);
	item_data->checked = false;
}

static void
_mp_list_gl_mode_right(void *data, Evas_Object * obj, void *event_info)
{
	MP_CHECK(obj);
	MP_CHECK(event_info);
	MP_CHECK(elm_genlist_item_select_mode_get(event_info) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	/* reset old sweep item mode */
	Elm_Object_Item *it = (Elm_Object_Item *)elm_genlist_decorated_item_get(obj);
	if (it && (it != event_info)) {
		elm_genlist_item_decorate_mode_set(it, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DEFAULT);
		mp_list_item_data_t *item_data = elm_object_item_data_get(it);
		item_data->checked = false;
	}

	// disable sweep if edit mode.
	MP_CHECK(elm_genlist_item_flip_get(event_info) == EINA_FALSE);
	// Start genlist sweep
	elm_genlist_item_decorate_mode_set(event_info, "slide", EINA_TRUE);
	elm_genlist_item_select_mode_set(event_info, ELM_OBJECT_SELECT_MODE_NONE);

	mp_list_item_data_t *item_data = elm_object_item_data_get(event_info);
	item_data->checked = true;
}


static void
_mp_list_gl_mode_cancel(void *data, Evas_Object * obj, void *event_info)
{
	//struct appdata *ad = (struct appdata *)data;
	//MP_CHECK(ad);
	MP_CHECK(obj);

	mp_util_reset_genlist_mode_item(obj);

	//ad->vertical_scroll = true;
}
#endif

static void
_mp_list_drag_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpList_t *list = (MpList_t*)data;
	MP_CHECK(list);

        list->scroll_drag_status = true;
}

static void
_mp_list_drag_stop_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpList_t *list = (MpList_t*)data;
	MP_CHECK(list);

        list->scroll_drag_status = false;
}


unsigned int
_mp_list_get_count(void *thiz, MpListEditType_e type)
{
	MpList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);

	unsigned int count = MP_LIST_OBJ_IS_GENGRID(list->genlist) ? elm_gengrid_items_count(list->genlist) : elm_genlist_items_count(list->genlist);

	if (list->bottom_counter_item)
		--count;

	return count;
}

static unsigned int
_mp_list_get_select_count(void *thiz)
{
	MpList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);
	unsigned int count = 0;
	Elm_Object_Item *item;
	mp_list_item_data_t *data = NULL;

	item = mp_list_first_item_get(list->genlist);
	while (item)
	{
		data = elm_object_item_data_get(item);
		item = mp_list_item_next_get(item);
		if (data && data->item_type == MP_LIST_ITEM_TYPE_NORMAL && data->checked)
		{
			count++;
		}
	}
	return count;
}

static void
_mp_list_layout_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	ad->del_cb_invoked = 0;

	MpList_t *list = data;
	MP_CHECK(list);

	MP_CHECK(list->destory_cb);
	list->destory_cb(list);

	endfunc;
}

static Evas_Object *
_mp_list_view_create_box(MpList_t *list)
{
	startfunc;
	Evas_Object *box = elm_box_add(list->layout);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(box);

	return box;
}

int _mp_list_set_fastscroll(void *thiz)
{
	startfunc;
	MpList_t *list = thiz;
	MP_CHECK_VAL(list,-1);

	if (mp_list_get_editable_count(list, MP_LIST_EDIT_TYPE_NORMAL))
	{
		if (!list->fast_scroll)
		{
			list->fast_scroll = mp_index_create(list->layout, 0, list);
			if (list->fast_scroll == NULL)
				ERROR_TRACE("list->fast_scroll create failed");
			elm_object_part_content_set(list->layout, "elm.swallow.content.index", list->fast_scroll);
			mp_index_append_item(list->fast_scroll, list);
		}
		elm_object_signal_emit(list->layout, "show.fastscroll", "*");
		if (list->genlist)
			elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	}
	else
		elm_object_signal_emit(list->layout, "hide.fastscroll", "*");

	return 0;
}

static int _mp_list_show_fastscroll(void *thiz)
{
	startfunc;
	MpList_t *list = thiz;
	MP_CHECK_VAL(list,-1);

	_mp_list_set_fastscroll(list);
	return 0;
}

static int _mp_list_hide_fastscroll(void *thiz)
{
	startfunc;
	MpList_t *list = thiz;
	MP_CHECK_VAL(list,-1);

	elm_object_signal_emit(list->layout, "hide.fastscroll", "*");
	return 0;
}

static void _mp_list_set_reorder(void *thiz, bool reorder)
{
	DEBUG_TRACE("");
	MpList_t *list = thiz;
	MP_CHECK(list);

	if (reorder)
	{
		mp_list_reorder_mode_set(list->genlist, EINA_TRUE);

		mp_list_select_mode_set(list->genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	}
	else
	{

		mp_list_select_mode_set(list->genlist, ELM_OBJECT_SELECT_MODE_DEFAULT);
		mp_list_reorder_mode_set(list->genlist, EINA_FALSE);

	}

}

static void _mp_list_realized_item_update(void *thiz, const char *part, int field)
{
	DEBUG_TRACE("");
	MpList_t *list = thiz;
	MP_CHECK(list);
	MP_CHECK(list->genlist);
	Elm_Object_Item *it = NULL;

	if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		it = elm_gengrid_first_item_get(list->genlist);
				while (it) {
					elm_gengrid_item_update(it);
					it = elm_gengrid_item_next_get(it);
				}
	} else {
		it = elm_genlist_first_item_get(list->genlist);
		while (it) {
			elm_genlist_item_update(it);
			it = elm_genlist_item_next_get(it);
		}

	}
}

static void _mp_list_set_edit(void *thiz, bool edit)
{
	DEBUG_TRACE("");
	MpList_t *list = thiz;
	MP_CHECK(list);

	if (edit)
	{
		if (!MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
			Elm_Object_Item *sweeped_item = (Elm_Object_Item *)elm_genlist_decorated_item_get(list->genlist);
			if (sweeped_item)
			{
				mp_list_item_data_t *data = elm_object_item_data_get(sweeped_item);
				if (data) data->checked = false;
				elm_genlist_item_decorate_mode_set(sweeped_item, "slide", EINA_FALSE);
				elm_genlist_item_select_mode_set(sweeped_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
				elm_genlist_item_update(sweeped_item);
			}

		}

		if (list->reorderable)
			mp_list_reorder_mode_set(list->genlist, EINA_TRUE);

		mp_list_select_mode_set(list->genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	}
	else
	{
		if (!MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		}
		mp_list_select_mode_set(list->genlist, ELM_OBJECT_SELECT_MODE_DEFAULT);
		mp_list_reorder_mode_set(list->genlist, EINA_FALSE);

		if (mp_list_get_checked_count(list))
		{
			Elm_Object_Item *item = mp_list_first_item_get(list->genlist);
			while (item)
			{
				if (mp_list_item_select_mode_get(item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
				{
					mp_list_item_data_t *item_data =
						(mp_list_item_data_t *) elm_object_item_data_get(item);
					MP_CHECK(item_data);
					item_data->checked = EINA_FALSE;
				}
				item = mp_list_item_next_get(item);
			}
		}
	}

	if (MP_LIST_OBJ_IS_GENGRID(list->genlist))
		elm_gengrid_realized_items_update(list->genlist);

	//char *title = NULL;
	//title = _mp_edit_view_get_view_title(list);
	//mp_common_set_toolbar_button_sensitivity(layout_data, 0);
}


static void
_mp_list_edit_mode_sel(void *thiz, void *data)
{
	startfunc;
	MpList_t *list = (MpList_t *)thiz;
	mp_list_item_data_t *it_data = (mp_list_item_data_t *)data;
	Elm_Object_Item *gli = (Elm_Object_Item *)it_data->it;
	mp_list_item_selected_set(gli, EINA_FALSE);

	if (!MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		if (elm_genlist_item_flip_get(gli) || elm_genlist_item_select_mode_get(gli) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
			return;
	}

	mp_list_item_check_set(gli, !it_data->checked);

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
        MP_CHECK(view);

        mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count(list));

	//view->selection_info = mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count(list));
}

static void
_mp_list_change_display_mode(void *thiz, MpListDisplayMode_e mode)
{
	MpList_t *list = (MpList_t *)thiz;
	MP_CHECK(list);

	list->display_mode = mode;

	if (mode == MP_LIST_DISPLAY_MODE_THUMBNAIL)
		elm_object_signal_emit(list->layout, "hide.fastscroll", "*");
	else
	{
		if (list->fast_scroll)
			elm_object_signal_emit(list->layout, "show.fastscroll", "*");
	}
	if (list->update)
		list->update(list);
}

static void
_mp_list_selected_item_data_get(void *thiz, GList **selected)
{
	startfunc;
	MpList_t *list = (MpList_t *)thiz;
	GList *sel_list = NULL;

	if (!list->genlist)
		goto END;

	Elm_Object_Item *item = mp_list_first_item_get(list->genlist);
	mp_list_item_data_t *gl_item = NULL;

	if (!item)
		goto END;

	while (item)
	{
		if (mp_list_item_select_mode_get(item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY)
		{
			gl_item = elm_object_item_data_get(item);
			if (gl_item && gl_item->checked)
				sel_list = g_list_append(sel_list, gl_item);
		}
		item = mp_list_item_next_get(item);
	}
	END:
	if (selected)
		*selected = sel_list;
}

static void
_mp_list_all_item_data_get(void *thiz, GList **selected)
{
	startfunc;
	MpList_t *list = (MpList_t *)thiz;
	GList *sel_list = NULL;

	if (!list->genlist)
		goto END;

	Elm_Object_Item *item = mp_list_first_item_get(list->genlist);
	mp_list_item_data_t *gl_item = NULL;

	if (!item)
		goto END;

	while (item)
	{
                gl_item = elm_object_item_data_get(item);
                if (gl_item)
                        sel_list = g_list_append(sel_list, gl_item);
		item = mp_list_item_next_get(item);
	}
	END:
	if (selected)
		*selected = sel_list;
}

void mp_list_init(MpList_t *list, Evas_Object *parent, MpListType_e list_type)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(parent);

	//initialize attribute
	list->list_type = list_type;

	list->layout = mp_common_load_edj(parent, MP_EDJ_NAME, "list_layout");
	MP_CHECK(list->layout);

	list->box = _mp_list_view_create_box(list);
	MP_CHECK(list->box);
	evas_object_size_hint_weight_set(list->box, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(list->box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(list->layout, "list_content", list->box);
	evas_object_show(list->box);

	//initialize method
	list->get_count = _mp_list_get_count;
	list->get_select_count = _mp_list_get_select_count;
#ifdef MP_FEATURE_SWEEP
	list->flick_left_cb = _mp_list_gl_flick_left_cb;
	list->flick_right_cb = _mp_list_gl_flick_right_cb;
	list->flick_stop_cb = _mp_list_gl_flick_stop_cb;
	list->mode_left_cb = _mp_list_gl_mode_left;
	list->mode_right_cb = _mp_list_gl_mode_right;
	list->mode_cancel_cb = _mp_list_gl_mode_cancel;
#endif
	list->set_edit = _mp_list_set_edit;
	list->set_reorder = _mp_list_set_reorder;
	list->show_fastscroll = _mp_list_show_fastscroll;
	list->hide_fastscroll = _mp_list_hide_fastscroll;
	list->edit_mode_sel = _mp_list_edit_mode_sel;
	list->change_display_mode = _mp_list_change_display_mode;
	list->selected_item_data_get = _mp_list_selected_item_data_get;
	list->all_item_data_get = _mp_list_all_item_data_get;
	list->drag_start_cb = _mp_list_drag_start_cb;
	list->drag_stop_cb = _mp_list_drag_stop_cb;
	list->realized_item_update = _mp_list_realized_item_update;
	//add free callback
	evas_object_event_callback_add(list->layout, EVAS_CALLBACK_FREE, _mp_list_layout_del_cb,
				       list);
}

Evas_Object *mp_list_get_layout(MpList_t *list)
{
	startfunc;
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->layout);
	return list->layout;
}
int mp_list_hide_fast_scroll(MpList_t *list)
{
	startfunc;
	MP_CHECK_VAL(list, -1);
	MP_CHECK_VAL(list->hide_fastscroll, -1);
	return list->hide_fastscroll(list);
}

int mp_list_show_fast_scroll(MpList_t *list)
{
	startfunc;
	MP_CHECK_VAL(list, -1);
	MP_CHECK_VAL(list->show_fastscroll, -1);
	return list->show_fastscroll(list);
}

void mp_list_update(MpList_t *list)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->update);
	list->update(list);
}

void mp_list_realized_item_part_update(MpList_t *list, const char *part, int field)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->realized_item_update);
	list->realized_item_update(list, part, field);
}

void mp_list_set_reorder(MpList_t *list, bool reorder)
{
	startfunc;
	MP_CHECK(list);
	list->reorderable = reorder;
	MP_CHECK(list->set_reorder);
	list->set_reorder(list, reorder);
}

bool mp_list_get_reorder(MpList_t *list)
{
	startfunc;
	MP_CHECK_FALSE(list);
	return list->reorderable;
}

void mp_list_set_edit(MpList_t *list, bool edit)
{
	startfunc;
	MP_CHECK(list);
	list->edit_mode = edit;
	MP_CHECK(list->set_edit);
	list->set_edit(list, edit);
	if (!edit)
	{
		MpView_t *view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
		MP_CHECK(view);
		view->selection_info = mp_util_create_selectioninfo_with_count(view, 0);
	}
}

bool mp_list_get_edit(MpList_t *list)
{
	MP_CHECK_FALSE(list);
	return list->edit_mode;
}

void mp_list_set_edit_type(MpList_t *list, MpListEditType_e type)
{
	MP_CHECK(list);
	list->edit_type = type;
	mp_debug("list edit type set as [%d]", list->edit_type);
}

MpListEditType_e mp_list_get_edit_type(MpList_t *list)
{
	MP_CHECK_VAL(list, 0);
	return list->edit_type;
}

void mp_list_edit_mode_sel(MpList_t *list, void *data)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->edit_mode_sel);
	list->edit_mode_sel(list, data);
}

mp_group_type_e mp_list_get_group_type(MpList_t *list)
{
	startfunc;
	MP_CHECK_VAL(list, -1);
	MP_CHECK_VAL(list->get_group_type, -1);
	return list->get_group_type(list);
}

mp_track_type_e mp_list_get_track_type(MpList_t *list)
{
	startfunc;
	MP_CHECK_VAL(list, -1);
	MP_CHECK_VAL(list->get_track_type, -1);
	return list->get_track_type(list);
}

void *mp_list_get_playlist_handle(MpList_t *list)
{
	startfunc;
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->get_playlist_handle);
	return list->get_playlist_handle(list);
}

unsigned int mp_list_get_editable_count(MpList_t *list, MpListEditType_e type)
{
	startfunc;
	MP_CHECK_VAL(list, 0);
	MP_CHECK_VAL(list->get_count, 0);
	return list->get_count(list, type);
}

unsigned int mp_list_get_checked_count(MpList_t *list)
{
	startfunc;
	MP_CHECK_VAL(list, 0);
	MP_CHECK_VAL(list->get_select_count, 0);
	return list->get_select_count(list);
}

bool
mp_list_is_display_mode_changable(MpList_t *list)
{
	MP_CHECK_FALSE(list);
	return list->display_mode_changable;
}

MpListDisplayMode_e
mp_list_get_display_mode(MpList_t *list)
{
	MP_CHECK_VAL(list, 0);
	return list->display_mode;
}

void
mp_list_change_display_mode(MpList_t *list, MpListDisplayMode_e mode)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->display_mode_changable);
	MP_CHECK(list->change_display_mode);

	list->change_display_mode(list, mode);
}

Elm_Object_Item *
mp_list_first_item_get(Evas_Object *obj)
{
	MP_CHECK_NULL(obj);
	return MP_LIST_OBJ_IS_GENGRID(obj) ? elm_gengrid_first_item_get(obj) : elm_genlist_first_item_get(obj);
}

Elm_Object_Item *
mp_list_item_next_get(Elm_Object_Item *item)
{
	MP_CHECK_NULL(item);
	Evas_Object *obj = elm_object_item_widget_get(item);
	MP_CHECK_NULL(obj);
	return MP_LIST_OBJ_IS_GENGRID(obj) ? elm_gengrid_item_next_get(item) : elm_genlist_item_next_get(item);
}

void
mp_list_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode select_mode)
{
	MP_CHECK(obj);

	if (MP_LIST_OBJ_IS_GENGRID(obj))
		elm_gengrid_select_mode_set(obj, select_mode);
	else
		elm_genlist_select_mode_set(obj, select_mode);
}

Elm_Object_Select_Mode
mp_list_select_mode_get(Evas_Object *obj)
{
	MP_CHECK_VAL(obj, ELM_OBJECT_SELECT_MODE_DEFAULT);
	return MP_LIST_OBJ_IS_GENGRID(obj) ? elm_gengrid_select_mode_get(obj) : elm_genlist_select_mode_get(obj);
}

void
mp_list_item_select_mode_set(Elm_Object_Item *item, Elm_Object_Select_Mode select_mode)
{
	MP_CHECK(item);
	Evas_Object *obj = elm_object_item_widget_get(item);
	MP_CHECK(obj);

	if (MP_LIST_OBJ_IS_GENGRID(obj))
		elm_gengrid_item_select_mode_set(item, select_mode);
	else
		elm_genlist_item_select_mode_set(item, select_mode);
}

Elm_Object_Select_Mode
mp_list_item_select_mode_get(Elm_Object_Item *item)
{
	MP_CHECK_VAL(item, ELM_OBJECT_SELECT_MODE_DEFAULT);
	Evas_Object *obj = elm_object_item_widget_get(item);
	MP_CHECK_VAL(obj, ELM_OBJECT_SELECT_MODE_DEFAULT);
	return MP_LIST_OBJ_IS_GENGRID(obj) ? elm_gengrid_item_select_mode_get(item) : elm_genlist_item_select_mode_get(item);
}

void
mp_list_reorder_mode_set(Evas_Object *obj, Eina_Bool reorder_mode)
{
	MP_CHECK(obj);
	if (MP_LIST_OBJ_IS_GENGRID(obj))
		elm_gengrid_reorder_mode_set(obj, reorder_mode);
	else
		elm_genlist_reorder_mode_set(obj, reorder_mode);
}

void
mp_list_item_selected_set(Elm_Object_Item *item, Eina_Bool selected)
{
	MP_CHECK(item);
	Evas_Object *obj = elm_object_item_widget_get(item);
	MP_CHECK(obj);

	if (MP_LIST_OBJ_IS_GENGRID(obj))
		elm_gengrid_item_selected_set(item, selected);
	else
		elm_genlist_item_selected_set(item, selected);
}

Eina_Bool
mp_list_item_selected_get(Elm_Object_Item *item)
{
	MP_CHECK_FALSE(item);
	Evas_Object *obj = elm_object_item_widget_get(item);
	MP_CHECK_FALSE(obj);
	return MP_LIST_OBJ_IS_GENGRID(obj) ? elm_gengrid_item_selected_get(item) : elm_genlist_item_selected_get(item);
}

void mp_list_selected_item_data_get(MpList_t *list, GList **selected)
{
	startfunc;
	MP_CHECK(list);

	if (list->selected_item_data_get)
		list->selected_item_data_get(list, selected);

	return;
}

void mp_list_all_item_data_get(MpList_t *list, GList **selected)
{
	startfunc;
	MP_CHECK(list);

	if (list->all_item_data_get)
		list->all_item_data_get(list, selected);

	return;
}


//this is for fastscroll index
const char * mp_list_get_list_item_label(MpList_t *list, Elm_Object_Item *item)
{
	MP_CHECK_NULL(list);
	if (list->get_label)
		return list->get_label(list, item);

	return NULL;
}

void mp_list_double_tap(MpList_t *list)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->genlist);
	MP_LIST_OBJ_IS_GENGRID(list->genlist) ?
		elm_gengrid_item_bring_in(elm_gengrid_first_item_get(list->genlist), ELM_GENGRID_ITEM_SCROLLTO_TOP):
		elm_genlist_item_bring_in(elm_genlist_first_item_get(list->genlist), ELM_GENLIST_ITEM_SCROLLTO_TOP);
}

void mp_list_rotate(MpList_t *list)
{
	startfunc;
	MP_CHECK(list);
	if (list->rotate)
		list->rotate(list);
}

mp_list_item_data_t *mp_list_item_data_create(MpListItemType_e item_type)
{
	mp_list_item_data_t *item_data = calloc(1, sizeof(mp_list_item_data_t));
	if (item_data)
	{
		item_data->item_type = item_type;
	}

	return item_data;
}

void mp_list_item_check_set(Elm_Object_Item *item, Eina_Bool checked)
{
	MP_CHECK(item);
	mp_list_item_data_t *item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);

	item_data->checked = checked;
        Evas_Object *check_box_layout = elm_object_item_part_content_get(item, "elm.icon.2");
        Evas_Object *chk = elm_object_part_content_get(check_box_layout, "elm.swallow.content");
	if (chk)
		elm_check_state_set(chk, checked);
	else
	{
		chk = elm_object_item_part_content_get(item, "elm.swallow.end");	// gengrid
		if (chk)
			elm_check_state_set(chk, checked);
	}
}

static char *_mp_list_bottom_counter_item_text_get_cb(void *data, Evas_Object *obj, const char *part)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	MpList_t *list = (MpList_t *)evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->bottom_counter_text_get_cb);
	if (!strcmp(part,"elm.text")) {
		return list->bottom_counter_text_get_cb(list);
	}
	else {
		return NULL;
	}
}

static void _mp_list_bottom_counter_item_del_cb(void *data, Evas_Object *obj)
{
	mp_list_item_data_t *item_data = data;
	IF_FREE(item_data);

	MpList_t *list = (MpList_t *)evas_object_data_get(obj, "list_data");
	MP_CHECK(list);
	list->bottom_counter_item = NULL;
}

Elm_Object_Item *mp_list_bottom_counter_item_append(MpList_t *list)
{
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->genlist);
	MP_CHECK_NULL(list->bottom_counter_text_get_cb);

	if (list->bottom_counter_item) {
		elm_object_item_del(list->bottom_counter_item);
		list->bottom_counter_item = NULL;
	}

	evas_object_data_set(list->genlist, "list_data", list);

	static Elm_Genlist_Item_Class itc = { 0, };
	itc.item_style = "music/1text/bottom_counter";
	itc.func.text_get = _mp_list_bottom_counter_item_text_get_cb;
	itc.func.del = _mp_list_bottom_counter_item_del_cb;

	mp_list_item_data_t *item_data = mp_list_item_data_create(MP_LIST_ITEM_TYPE_BOTTOM_COUNTER);
	MP_CHECK_NULL(item_data);
	Elm_Object_Item *item = elm_genlist_item_append(list->genlist, &itc, item_data, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	list->bottom_counter_item = item;
	return item;
}

GList * mp_list_get_checked_path_list(MpList_t *list)
{
        MP_CHECK_NULL(list);
        GList *sel_list = NULL;
        GList *path_list = NULL;
        GList *node = NULL;

        mp_list_selected_item_data_get(list, &sel_list);
        MP_CHECK_NULL(sel_list);

        node = g_list_first(sel_list);
        while (node)
        {
                mp_list_item_data_t *item = node->data;
                if (item && item->handle)
                {
                        char *file_path = NULL;
                        if (list->list_type == MP_LIST_TYPE_GROUP) {
                                mp_media_info_group_get_main_info(item->handle,&file_path);
                        } else {
                                mp_media_info_get_file_path(item->handle,&file_path);
                        }
                        char *path = g_strdup(file_path);
                        path_list = g_list_append(path_list, path);
                }

                node = g_list_next(node);
        }
        g_list_free(sel_list);
        return path_list;
}

bool mp_list_is_in_checked_path_list(GList *path_list, char *file_path)
{
        MP_CHECK_FALSE(path_list);
        MP_CHECK_FALSE(file_path);
        GList *node = NULL;

        node = g_list_first(path_list);
        while (node)
        {
                char *path = node->data;
                if (!g_strcmp0(path, file_path))
                {
                        return true;
                }
                node = g_list_next(node);
        }

        return false;
}

void mp_list_free_checked_path_list(GList *path_list)
{
        MP_CHECK(path_list);
        GList *node = NULL;

        node = g_list_first(path_list);
        while (node)
        {
                char *path = node->data;
                SAFE_FREE(path);
                node = g_list_next(node);
        }
        g_list_free(path_list);
}

void mp_list_item_reorder_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

        MpList_t *list = data;
        MP_CHECK(list);
        MP_CHECK(list->genlist);

        int cur_sequence = 0;
        Elm_Object_Item *temp = elm_genlist_first_item_get(list->genlist);
        while (temp) {
                mp_list_item_data_t *item_data = (mp_list_item_data_t *) elm_object_item_data_get(temp);
                MP_CHECK(item_data);
                if (cur_sequence != item_data->index) {
                        mp_view_mgr_post_event(GET_VIEW_MGR, MP_REORDER_ENABLE);
                        return;
                }
                temp = elm_genlist_item_next_get(temp);
                cur_sequence++;
        }
        mp_view_mgr_post_event(GET_VIEW_MGR, MP_REORDER_DISABLE);
}

