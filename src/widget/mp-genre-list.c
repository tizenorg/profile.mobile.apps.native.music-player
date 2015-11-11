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

#include <stdio.h>

#include "mp-genre-list.h"
#include "mp-track-list.h"
#include "mp-genre-detail-view.h"
#include "mp-create-playlist-view.h"
#include "mp-select-track-view.h"
#include "mp-popup.h"
/*#include "shortcut.h"*/
#include "mp-menu.h"
#include "mp-util.h"
#include "mp-widget.h"
#include "mp-common.h"

#define GENRE_GRID_W 175
#define GENRE_GRID_H 222
#define GENRE_GRID_LAND_W 179
#define GENRE_GRID_LAND_H 240

static void
_mp_genre_select_cb(void *data, Evas_Object *obj, void *event_info);

static char *
_mp_genre_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *name = NULL;
	int ret = 0;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);

	mp_retv_if(svc_item == NULL, NULL);

	if (!strcmp(part, "elm.text.main.left") || !strcmp(part, "elm.slide.text.1") || !strcmp(part, "elm.text")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}

		if (!strcmp(part, "elm.text.1")) {
			return elm_entry_utf8_to_markup(name);
		} else {
			return g_strdup(name);
		}

	}
	/*
	else if (!strcmp(part, "elm.text.2"))
	{
		int count = 0;
		char *sub_text = NULL;
		mp_media_info_group_get_track_count(svc_item, &count);

	if (count == 1)
		sub_text = g_strdup(GET_STR(STR_MP_1_SONG));
	else
		sub_text = g_strdup_printf(GET_STR(STR_MP_PD_SONGS), count);

		return sub_text;
	}*/

	/*DEBUG_TRACE("Unusing part: %s", part);*/
	return NULL;
}



Evas_Object *
_mp_genre_list_icon_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = NULL;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	mp_retv_if(svc_item == NULL, NULL);

	bool landscape = mp_util_is_landscape();

	Evas_Object *content = NULL;
	content = elm_layout_add(obj);

	if (!strcmp(part, "elm.icon.1") || !strcmp(part, "elm.swallow.icon")) {
		char *thumb_name = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
		int w, h;
		if (item->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			if (landscape) {
				w = MP_LANDSCAPE_ALBUM_THUMB_ICON_SIZE * elm_config_scale_get();
			} else {
				w = MP_ALBUM_THUMB_ICON_SIZE * elm_config_scale_get();
			}
		} else {
			w = MP_LIST_ICON_SIZE;
		}
		h = w;
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumb_name, w, h);

		elm_layout_theme_set(content, "layout", "list/B/type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);
	}

	MpGenreList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);

	Evas_Object *check = NULL;
	DEBUG_TRACE("list->edit_mode = %d", list->edit_mode);

	if (list->edit_mode) {			/* if edit mode*/
		if (!strcmp(part, "elm.edit.icon.1") || !strcmp(part, "elm.swallow.end")) {		/* swallow checkbox or radio button*/
			check = elm_check_add(obj);
			if (MP_LIST_OBJ_IS_GENGRID(obj)) {
				elm_object_style_set(check, "grid");
			} else {
				elm_object_style_set(check, "default/genlist");
			}
			elm_check_state_pointer_set(check, &item->checked);
			return check;
		}
	}
	return content;
}

static void
_mp_genre_list_item_del_cb(void *data, Evas_Object *obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);

	free(item_data);
}

static void
_mp_genre_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpGenreList_t *list = (MpGenreList_t *)data;
	MP_CHECK(list);
	int ret = 0;
	int index = 0;/*(int)data;*/
	char *name = NULL;
	char *artist = NULL;
	char *title = NULL;
	char *thumbnail = NULL;

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);
	if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		elm_gengrid_item_selected_set(gli, EINA_FALSE);
	} else {
		elm_genlist_item_selected_set(gli, EINA_FALSE);
	}

	mp_list_item_data_t *gli_data = elm_object_item_data_get(gli);
	MP_CHECK(gli_data);

	index = gli_data->index;

	DEBUG_TRACE("index is %d", index);
	if (index >= 0) {
		/*item_handle = mp_media_info_group_list_nth_item(gli_data->handle, index);*/
		DEBUG_TRACE("");
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		DEBUG_TRACE("");
		ret = mp_media_info_group_get_sub_info(gli_data->handle, &artist);
		DEBUG_TRACE("");
		mp_media_info_group_get_thumbnail_path(gli_data->handle, &thumbnail);
		DEBUG_TRACE("thumbnail=%s", thumbnail);
		mp_retm_if(ret != 0, "Fail to get value");
		mp_retm_if(name == NULL, "Fail to get value");

		title = name;
	}


	if (list->function_type == MP_LIST_FUNC_ADD_TRACK) {
		MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
		MP_CHECK(view_manager);
		MpSelectTrackView_t *view_select_track = mp_select_track_view_create(view_manager->navi);
		MP_CHECK(view_select_track);
		mp_view_mgr_push_view(view_manager, (MpView_t *)view_select_track, NULL);

		mp_view_set_title((MpView_t *)view_select_track, STR_MP_TILTE_SELECT_ITEM);
		mp_track_list_set_data((MpTrackList_t *)view_select_track->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_BY_GENRE, MP_TRACK_LIST_TYPE_STR, name, -1);
		mp_list_update(view_select_track->content_to_show);
		mp_view_update_options((MpView_t *)view_select_track);
		mp_list_set_edit(view_select_track->content_to_show, TRUE);
		mp_list_view_set_select_all((MpListView_t *)view_select_track, TRUE);
		return;
	}

	if (list->edit_mode) {
		/*mp_edit_view_genlist_sel_cb(data, obj, event_info);*/
		mp_list_edit_mode_sel((MpList_t *)list, gli_data);
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		mp_view_update_options_edit(view);
		return;
	}

	/* create the view of genre detail */

	MpGenreDetailView_t *view_genre_detail = mp_genre_detail_view_create(GET_NAVIFRAME, name, artist, thumbnail);
	mp_view_mgr_push_view(GET_VIEW_MGR, (MpView_t *)view_genre_detail, NULL);

	mp_view_update_options((MpView_t *)view_genre_detail);
	mp_view_set_title((MpView_t *)view_genre_detail, title);

}

void _mp_genre_list_load_list(void *thiz, int count)
{
	MpGenreList_t *list = thiz;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle;
	gint index = 0;
	int ret = 0;

	DEBUG_TRACE("count: %d", count);

	if (count < 0) {
		goto END;
	}

	if (list->genre_list) {
		mp_media_info_group_list_destroy(list->genre_list);
		list->genre_list = NULL;
	}

	ret = mp_media_info_group_list_create(&list->genre_list, MP_GROUP_BY_GENRE, NULL, NULL, 0, count);

	if (ret != 0) {
		DEBUG_TRACE("Fail to get items");
		goto END;
	}

	svc_handle = list->genre_list ;

	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		Elm_Object_Item *list_item = NULL;
		char *title = NULL;

		item = mp_media_info_group_list_nth_item(svc_handle, index);
		if (!item) {
			DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
			goto END;
		}
		mp_media_info_group_get_main_info(item, &title);

		mp_list_item_data_t *item_data;
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->index = index;
		item_data->checked = mp_list_is_in_checked_path_list(list->checked_path_list, title);

		if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
			list_item = elm_gengrid_item_append(list->genlist, list->gengrid_itc, item_data,
			                                    _mp_genre_select_cb, (void *)list);
		} else {
			Elm_Object_Item *parent_group = NULL;
			list_item = elm_genlist_item_append(list->genlist, list->itc, item_data, parent_group,
			                                    ELM_GENLIST_ITEM_NONE, _mp_genre_select_cb, (void *)list);
		}
		item_data->it = list_item;
		elm_object_item_data_set(item_data->it, item_data);
	}

END:
	endfunc;
}

void _mp_genre_list_destory_cb(void *thiz)
{
	eventfunc;
	MpGenreList_t *list = thiz;
	MP_CHECK(list);

	if (list->genre_list) {
		mp_media_info_group_list_destroy(list->genre_list);
	}


	if (list->itc) {
		elm_genlist_item_class_free(list->itc);
		list->itc = NULL;
	}

	if (list->gengrid_itc) {
		elm_gengrid_item_class_free(list->gengrid_itc);
		list->gengrid_itc = NULL;
	}
	mp_list_free_checked_path_list(list->checked_path_list);

	free(list);
}

static mp_group_type_e _mp_genre_list_get_group_type(void *thiz)
{
	MpGenreList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);
	return MP_GROUP_BY_GENRE;
}

static void
_mp_genre_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpGenreList_t *list = (MpGenreList_t *)data;
	MP_CHECK(list);

	if (list->edit_mode) {
		return ;
	}

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	int pop_item_count = 5;
	char *title = NULL;
	Evas_Object *popup = NULL;
	mp_list_item_data_t *item_data = NULL;

	if (list->scroll_drag_status) {
		return;
	}

	Elm_Object_Item *temp = NULL;
	if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		temp = elm_gengrid_first_item_get(list->genlist);
		while (temp) {
			item_data = elm_object_item_data_get(temp);
			item_data->checked = false;
			temp = elm_gengrid_item_next_get(temp);
		}
	} else {
		temp = elm_genlist_first_item_get(list->genlist);
		while (temp) {
			item_data = elm_object_item_data_get(temp);
			item_data->checked = false;
			temp = elm_genlist_item_next_get(temp);
		}
	}

	item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);
	item_data->checked = true;

	pop_item_count = 4;
	mp_media_info_group_get_main_info(item_data->handle, &title);

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

	char *up_title = g_strdup(title);

	elm_object_part_text_set(popup, "title,text", up_title);
	IF_FREE(up_title);

	mp_genlist_popup_item_append(popup, STR_MP_PLAY_ALL, NULL, NULL, NULL,
	                             mp_common_playall_cb, list);

	mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PLAYLIST, NULL, NULL, NULL,
	                             mp_common_list_add_to_playlist_cb, list);

	mp_genlist_popup_item_append(popup, STR_MP_DELETE, NULL, NULL, NULL,
	                             mp_common_list_delete_cb, list);

	if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		MP_GENGRID_ITEM_LONG_PRESSED(obj, popup, event_info);
	} else {
		MP_GENLIST_ITEM_LONG_PRESSED(obj, popup, event_info);
	}
}

static void
_mp_genre_list_genlist_create(MpGenreList_t *list)
{
	MP_CHECK(list);

	/*create new genlist*/
	list->genlist = mp_widget_genlist_create(list->box);
	MP_CHECK(list->genlist);
	elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_homogeneous_set(list->genlist, EINA_TRUE);
	evas_object_show(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);
	/*add long press callback*/
	evas_object_smart_callback_add(list->genlist, "longpressed", _mp_genre_list_item_longpressed_cb, list);

	const char *group_slide_style = "mode/slide";
#ifdef MP_FEATURE_ADD_TO_HOME
	group_slide_style = "mode/slide2";
#endif
	if (!list->itc) {
		list->itc = elm_genlist_item_class_new();
		MP_CHECK(list->itc);
		list->itc->item_style = "1line";
		list->itc->decorate_all_item_style = "musiclist/edit_default";
		list->itc->decorate_item_style = group_slide_style;
		list->itc->func.text_get = _mp_genre_list_label_get;
		list->itc->func.content_get = _mp_genre_list_icon_get;
		list->itc->func.del = _mp_genre_list_item_del_cb;
	}

	/*
	evas_object_smart_callback_add(list->genlist, "drag,start,left", list->flick_left_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,right", list->flick_right_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,stop", list->flick_stop_cb, NULL);

	evas_object_smart_callback_add(list->genlist, "drag,start,right", list->mode_right_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,left", list->mode_left_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,up", list->mode_cancel_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,down", list->mode_cancel_cb, NULL);
	*/
	endfunc;
}

static void
_mp_genre_list_set_grid_style(MpGenreList_t *list)
{
	bool landscape = mp_util_is_landscape();

	MP_CHECK(list->gengrid_itc);

	/*if (landscape)
		list->gengrid_itc->item_style = "music/artist_grid";
	else*/
	list->gengrid_itc->item_style = "music/artist_grid";

	double scale = elm_config_scale_get();
	int w;
	int h;
	if (landscape) {
		w = (int)(GENRE_GRID_LAND_W * scale);
		h = (int)(GENRE_GRID_LAND_H * scale);
	} else {
		w = (int)(GENRE_GRID_W * scale);
		h = (int)(GENRE_GRID_H * scale);
	}
	elm_gengrid_item_size_set(list->genlist, w, h);
}

static void
_mp_genre_list_gengrid_create(MpGenreList_t *list)
{
	startfunc;
	MP_CHECK(list);

	/*create new genlist*/
	mp_evas_object_del(list->genlist);

	list->genlist = elm_gengrid_add(list->box);
	MP_CHECK(list->genlist);
	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(list->genlist);
	MP_LIST_OBJ_SET_AS_GENGRID(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);

	if (!list->gengrid_itc) {
		list->gengrid_itc = elm_gengrid_item_class_new();
		MP_CHECK(list->gengrid_itc);
		list->gengrid_itc->func.text_get = _mp_genre_list_label_get;
		list->gengrid_itc->func.content_get = _mp_genre_list_icon_get;
		list->gengrid_itc->func.del = _mp_genre_list_item_del_cb;
	}
	_mp_genre_list_set_grid_style(list);
	/*evas_object_smart_callback_add(list->genlist, "longpressed", _mp_genre_list_item_longpressed_cb, list);*/

	elm_gengrid_align_set(list->genlist, 0.5, 0.0);
	endfunc;
}

static void _mp_genre_list_update(void *thiz)
{
	startfunc;
	int count = 0, res = 0;
	MpGenreList_t *list = thiz;
	MP_CHECK(list);

	res = mp_media_info_group_list_count(MP_GROUP_BY_GENRE, NULL, NULL, &count);
	MP_CHECK(res == 0);

	mp_list_free_checked_path_list(list->checked_path_list);
	list->checked_path_list = mp_list_get_checked_path_list((MpList_t *)list);

	mp_evas_object_del(list->genlist);
	mp_evas_object_del(list->no_content);

	if (count) {
		if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			_mp_genre_list_gengrid_create(list);
		} else {
			_mp_genre_list_genlist_create(list);
		}

		evas_object_data_set(list->genlist, "list_handle", list);

		/* load list */
		_mp_genre_list_load_list(thiz, count);

		/*mp_list_bottom_counter_item_append((MpList_t *)list);*/
	} else {
		DEBUG_TRACE("count is 0");

		list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_GENRES, NULL, NULL);
		elm_box_pack_end(list->box, list->no_content);
	}

}

void _mp_genre_list_rotate(void *thiz)
{
	MpGenreList_t *list = thiz;
	if (mp_list_get_display_mode((MpList_t *)list) == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		_mp_genre_list_set_grid_style(list);
	}
}

static const char *_get_label(void *thiz, void *event_info)
{
	MpGenreList_t *list = thiz;
	MP_CHECK_NULL(list);
	char *title = NULL;

	mp_list_item_data_t *genre =  elm_object_item_data_get(event_info);
	MP_CHECK_NULL(genre);

	mp_media_info_group_get_main_info(genre->handle, &title);
	return title;
}

static char *_mp_genre_list_bottom_counter_text_cb(void *thiz)
{
	MpGenreList_t *list = thiz;
	MP_CHECK_NULL(list);

	unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));

	char *text = NULL;
	if (count == 1) {
		text = g_strdup(GET_STR(STR_MP_1_GENRE));
	} else {
		text = g_strdup_printf(GET_STR(STR_MP_PD_GENRE), count);
	}

	return text;
}

MpGenreList_t *mp_genre_list_create(Evas_Object *parent)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpGenreList_t *list = calloc(1, sizeof(MpGenreList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_GROUP);

	list->update = _mp_genre_list_update;
	list->destory_cb = _mp_genre_list_destory_cb;
	list->get_group_type = _mp_genre_list_get_group_type;
	list->rotate = _mp_genre_list_rotate;
	list->get_label = _get_label;

	list->display_mode_changable = true;
	list->bottom_counter_text_get_cb = _mp_genre_list_bottom_counter_text_cb;

	return list;
}

void mp_genre_list_set_data(MpGenreList_t *list, ...)
{
	startfunc;
	MP_CHECK(list);

	va_list var_args;
	int field;

	va_start(var_args, list);
	do {
		field = va_arg(var_args, int);
		switch (field) {

		case MP_GENRE_LIST_FUNC: {
			int val = va_arg((var_args), int);

			list->function_type = val;
			DEBUG_TRACE("list->function_type = %d", list->function_type);
			break;
		}
		case MP_GENRE_LIST_DISPLAY_MODE: {
			int val = va_arg((var_args), int);
			list->display_mode = val;
			DEBUG_TRACE("list->display_mode = %d", list->display_mode);

			break;
		}

		default:
			DEBUG_TRACE("Invalid arguments");
		}

	} while (field >= 0);

	va_end(var_args);
}

void mp_genre_list_copy_data(MpGenreList_t *src, MpGenreList_t *dest)
{
	MP_CHECK(src);
	MP_CHECK(dest);

	dest->function_type = src->function_type;
	dest->display_mode = src->display_mode;
}


