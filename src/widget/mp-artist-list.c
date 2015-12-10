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

#include "mp-artist-list.h"
#include "mp-artist-detail-view.h"
#include "mp-create-playlist-view.h"
#include "mp-select-track-view.h"
#include "mp-popup.h"
/*#include "shortcut.h"*/
#include "mp-menu.h"
#include "mp-util.h"
#include "mp-widget.h"
#include "mp-common.h"


#define ARTIST_GRID_W 175
#define ARTIST_GRID_H 222
#define ARTIST_GRID_LAND_W 179
#define ARTIST_GRID_LAND_H 240

static char *
_mp_artist_list_group_index_text_get(void *data, Evas_Object *obj, const char *part)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	char *text = NULL;
	if (!strcmp(part, "elm.text.sub")) {
		MpArtistList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);

		unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));

		if (count == 1) {
			text = g_strdup(GET_STR(STR_MP_1_ARTIST));
		} else {
			text = g_strdup_printf(GET_STR(STR_MP_PD_ARTISTS), count);
		}
	}

	return text;
}

static void
_mp_artist_list_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	mp_edit_create_add_to_playlist_popup(data);
	return;
}

static char *
_mp_artist_list_label_get(void *data, Evas_Object *obj, const char *part)
{
	char *name = NULL;
	int ret = 0;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	mp_retv_if(svc_item == NULL, NULL);

	if (!strcmp(part, "elm.text")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}
		return elm_entry_utf8_to_markup(name);
	} else if (!strcmp(part, "elm.text.sub")) {
		char **album_thumbs = NULL;
		int album_count = 0;
		int song_count = 0;

		mp_media_info_group_get_album_thumnail_paths(item->handle, &album_thumbs, &album_count);
		mp_media_info_group_get_track_count(item->handle, &song_count);

		char *sub_text = NULL;
		if (album_count == 1 && song_count == 1) {
			sub_text = g_strdup(GET_STR(STR_MP_1_ALBUM_1_SONG));
		} else if (album_count == 1 && song_count > 1) {
			sub_text = g_strdup_printf(GET_STR(STR_MP_1_ALBUM_PD_SONGS), song_count);
		} else {
			sub_text = g_strdup_printf(GET_STR(STR_MP_PD_ALBUMS_PD_SONGS), album_count, song_count);
		}
		return  sub_text;
	}

	return NULL;
}

/*
static void
_mp_artist_list_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	mp_list_item_data_t *item = data;
	MP_CHECK(item);
	MP_CHECK(item->it);
	item->artist_album_page++;
	mp_debug("next artist album page = %d", item->artist_album_page);
	elm_genlist_item_fields_update(item->it, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
}
*/

static Evas_Object *
_mp_artist_list_album_icon_get(Evas_Object *obj, mp_list_item_data_t *item)
{
	MP_CHECK_NULL(item);
	MP_CHECK_NULL(item->handle);

	char *artist_name = NULL;
	mp_media_info_group_get_main_info(item->handle, &artist_name);

	char **album_thumbs = NULL;
	int album_count = 0;
	int song_count = 0;
	int i = 0;
	int icon_area_w = 78;
	int image_size = 68;
	int thumnail_max = 8;
	char *path   = NULL;

	mp_media_info_group_get_album_thumnail_paths(item->handle, &album_thumbs, &album_count);
	mp_media_info_group_get_track_count(item->handle, &song_count);

	for (i = 0; i < album_count; i++) {
		icon_area_w += 24;
		if (i >= (thumnail_max - 1)) {
			break;
		}
	}

	for (i = 0; i < album_count ; i++) {
		path = album_thumbs[i];
	}
	Evas_Object *icon = mp_util_create_lazy_update_thumb_icon(obj, path, image_size, image_size);
	return icon;
}


Evas_Object *
_mp_artist_list_icon_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *icon = NULL;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	mp_retv_if(svc_item == NULL, NULL);


	Evas_Object *content = NULL;

	if (item->display_mode == MP_LIST_DISPLAY_MODE_NORMAL) {
		if (!strcmp(part, "elm.icon.1") || !strcmp(part, "elm.swallow.icon")) {
			content = elm_layout_add(obj);
			Evas_Object *icon = _mp_artist_list_album_icon_get(obj, item);
			elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
			elm_layout_content_set(content, "elm.swallow.content", icon);

			return content;
		}
	}
	MpArtistList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);

	Evas_Object *button = NULL;
	if (!strcmp(part, "elm.slide.swallow.1")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		/*elm_object_text_set(button, GET_STR(STR_MP_ADD_TO));
		mp_language_mgr_register_object(button, OBJ_TYPE_ELM_OBJECT, NULL, STR_MP_ADD_TO);*/
		mp_util_domain_translatable_text_set(button, STR_MP_ADD_TO);
		evas_object_smart_callback_add(button, "clicked", _mp_artist_list_add_to_playlist_cb, list);
		return button;
	}

	Evas_Object *check = NULL;
	if (list->edit_mode) {
		if (!strcmp(part, "elm.icon.2")) {
			content = elm_layout_add(obj);
			check = elm_check_add(obj);
			elm_object_style_set(check, "default");
			evas_object_propagate_events_set(check, EINA_FALSE);
			evas_object_smart_callback_add(check, "changed", mp_common_view_check_changed_cb, NULL);
			elm_check_state_pointer_set(check, &item->checked);
			elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
			elm_layout_content_set(content, "elm.swallow.content", check);

			return content;
		}
	}
	return icon;
}

static void
_mp_artist_list_item_del_cb(void *data, Evas_Object *obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);

	free(item_data);
}


static void
_mp_artist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	MpArtistList_t *list = (MpArtistList_t *)data;
	eventfunc;
	int ret = 0;
	int index = 0;/*(int)data;*/
	char *name = NULL;
	char *thumbnail = NULL;

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);
	MP_CHECK(list);
	if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		elm_gengrid_item_selected_set(gli, EINA_FALSE);
		/*temp update item to avoid blue check in checkbox*/
		elm_gengrid_item_update(gli);
	} else {
		elm_genlist_item_selected_set(gli, EINA_FALSE);
	}

	mp_list_item_data_t *gli_data = elm_object_item_data_get(gli);
	MP_CHECK(gli_data);

	index = gli_data->index;
	if (index >= 0) {
		/*item_handle = mp_media_info_group_list_nth_item(gli_data->handle, index);*/
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		mp_media_info_group_get_thumbnail_path(gli_data->handle, &thumbnail);
		mp_retm_if(ret != 0, "Fail to get value");
		mp_retm_if(name == NULL, "Fail to get value");
	}

	if (list->function_type == MP_LIST_FUNC_ADD_TRACK) {
		MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
		MP_CHECK(view_manager);
		MpSelectTrackView_t *view_select_track = mp_select_track_view_create(view_manager->navi);
		MP_CHECK(view_select_track);
		mp_view_mgr_push_view(view_manager, (MpView_t *)view_select_track, NULL);

		mp_view_set_title((MpView_t *)view_select_track, STR_MP_TILTE_SELECT_ITEM);
		mp_track_list_set_data((MpTrackList_t *)view_select_track->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_BY_ARTIST, MP_TRACK_LIST_TYPE_STR, name, -1);
		mp_list_update(view_select_track->content_to_show);
		mp_view_update_options((MpView_t *)view_select_track);
		mp_list_set_edit(view_select_track->content_to_show, TRUE);
		mp_list_view_set_cancel_btn((MpListView_t *)view_select_track, true);
		mp_list_view_set_done_btn((MpListView_t *)view_select_track, true, MP_DONE_SELECT_ADD_TRACK_TYPE);
		mp_list_view_set_select_all((MpListView_t *)view_select_track, TRUE);
		return;
	}

	if (list->edit_mode) {
		mp_list_edit_mode_sel((MpList_t *)list, gli_data);

		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		ERROR_TRACE("update options of edit view");
		mp_view_update_options_edit((MpView_t *)view);
		return;
	}

	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MP_CHECK(view_manager);
	MpArtistDetailView_t *view_artist_detail = mp_artist_detail_view_create(view_manager->navi, name, thumbnail);
	MP_CHECK(view_artist_detail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_artist_detail, NULL);
	mp_view_update_options((MpView_t *)view_artist_detail);
	mp_view_set_title((MpView_t *)view_artist_detail, name);

}

static void
_mp_artist_list_append_group_index_item(void *thiz)
{
	MpArtistList_t *list = (MpArtistList_t *)thiz;
	MP_CHECK(list);
	MP_CHECK(list->genlist);

	if (!list->itc_group_index) {
		list->itc_group_index = elm_genlist_item_class_new();
		MP_CHECK(list->itc_group_index);
		list->itc_group_index->item_style = "music/groupindex.sub";
		list->itc_group_index->func.text_get = _mp_artist_list_group_index_text_get;
		list->itc_group_index->func.del = _mp_artist_list_item_del_cb;
	}

	mp_list_item_data_t *item_data = mp_list_item_data_create(MP_LIST_ITEM_TYPE_GROUP_TITLE);
	MP_CHECK(item_data);
	item_data->it = list->group_it =  elm_genlist_item_append(list->genlist, list->itc_group_index, item_data, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
	elm_genlist_item_select_mode_set(list->group_it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

void mp_artist_list_show_group_index(void *thiz, bool show)
{
	startfunc;
	MP_CHECK(thiz);
	MpArtistList_t *list = thiz;
	MP_CHECK(list->genlist);

	DEBUG_TRACE("show group index: %d   list->group_it: %0x", show, list->group_it);
	if (show) {
		_mp_artist_list_append_group_index_item(list);
	} else if (list->group_it) {
		elm_object_item_del(list->group_it);
		list->group_it = NULL;
	}
}

static void _mp_artist_list_load_list(void *thiz, int count)
{
	MpArtistList_t *list = thiz;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle;
	gint index = 0;
	int ret = 0;

	DEBUG_TRACE("count: %d", count);

	if (count < 0) {
		goto END;
	}

	ret = mp_media_info_group_list_create(&svc_handle, MP_GROUP_BY_ARTIST, list->type_str, list->filter_str, 0, count);

	if (ret != 0) {
		DEBUG_TRACE("Fail to get items");
		goto END;
	}

	if (list->artist_list) {
		mp_media_info_group_list_destroy(list->artist_list);
	}
	list->artist_list = svc_handle;

	mp_artist_list_show_group_index(list, false);

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
		item_data->group_type = list->group_type;
		item_data->index = index;
		item_data->display_mode = list->display_mode;
		item_data->checked = mp_list_is_in_checked_path_list(list->checked_path_list, title);

		Elm_Object_Item *parent_group = NULL;
		if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
			list_item = elm_gengrid_item_append(list->genlist, list->gengrid_itc, item_data,
			                                    _mp_artist_select_cb, (void *)list);
		} else {
			list_item = elm_genlist_item_append(list->genlist, list->itc, item_data, parent_group,
			                                    ELM_GENLIST_ITEM_NONE, _mp_artist_select_cb, list);
		}

		item_data->it = list_item;
	}

END:
	endfunc;
}

void _mp_artist_list_destory_cb(void *thiz)
{
	eventfunc;
	MpArtistList_t *list = thiz;
	MP_CHECK(list);

	if (list->artist_list) {
		mp_media_info_group_list_destroy(list->artist_list);
	}

	IF_FREE(list->type_str);
	IF_FREE(list->filter_str);

	if (list->itc) {
		elm_genlist_item_class_free(list->itc);
		list->itc = NULL;
	}

	if (list->itc_group_index) {
		elm_genlist_item_class_free(list->itc_group_index);
		list->itc_group_index = NULL;
	}

	if (list->gengrid_itc) {
		elm_gengrid_item_class_free(list->gengrid_itc);
		list->gengrid_itc = NULL;
	}
	mp_list_free_checked_path_list(list->checked_path_list);

	free(list);
}

static mp_group_type_e _mp_artist_list_get_group_type(void *thiz)
{
	MpArtistList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);
	return MP_GROUP_BY_ARTIST;
}

static void
_mp_artist_list_item_highlighted_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpArtistList_t *list = data;
	MP_CHECK(list);
	MP_CHECK(!MP_LIST_OBJ_IS_GENGRID(obj));

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	Evas_Object *layout = elm_object_item_part_content_get(item, "elm.icon");
	if (layout) {
		elm_object_signal_emit(layout, "elm,state,selected", "elm");
	}
}

static void
_mp_artist_list_item_unhighlighted_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpArtistList_t *list = data;
	MP_CHECK(list);
	MP_CHECK(!MP_LIST_OBJ_IS_GENGRID(obj));

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	Evas_Object *layout = elm_object_item_part_content_get(item, "elm.icon");
	if (layout) {
		elm_object_signal_emit(layout, "elm,state,unselected", "elm");
	}
}

/*static void
_mp_artist_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpArtistList_t *list = (MpArtistList_t *)data;
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

	if (list->scroll_drag_status)
		return;

	Elm_Object_Item *temp = NULL;
	temp = elm_genlist_first_item_get(list->genlist);
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

	pop_item_count = 3;
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

}*/

static void
_mp_artist_list_genlist_create(MpArtistList_t *list)
{
	startfunc;
	MP_CHECK(list);

	/*create new genlist*/
	mp_evas_object_del(list->genlist);

	/*create new genlist*/
	list->genlist = mp_widget_genlist_create(list->box);
	MP_CHECK(list->genlist);
	elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_homogeneous_set(list->genlist, EINA_TRUE);
	elm_genlist_mode_set(list->genlist, ELM_LIST_COMPRESS);
	evas_object_show(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);
	/*add long press callback*/
	/*evas_object_smart_callback_add(list->genlist, "longpressed", _mp_artist_list_item_longpressed_cb, list);*/
	evas_object_data_set(list->genlist, "list_data", list);

	if (!list->itc) {
		list->itc = elm_genlist_item_class_new();
		if (list->itc == NULL) {
			ERROR_TRACE("Unable to create artist list genlist");
			return;
		}
		list->itc->item_style = "type1";
		list->itc->func.text_get = _mp_artist_list_label_get;
		list->itc->func.content_get = _mp_artist_list_icon_get;
		list->itc->func.del = _mp_artist_list_item_del_cb;
	}

	evas_object_smart_callback_add(list->genlist, "drag,start,left", list->flick_left_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,right", list->flick_right_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,stop", list->flick_stop_cb, NULL);

	evas_object_smart_callback_add(list->genlist, "drag,start,right", list->mode_right_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,left", list->mode_left_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,up", list->mode_cancel_cb, NULL);
	evas_object_smart_callback_add(list->genlist, "drag,start,down", list->mode_cancel_cb, NULL);

	evas_object_smart_callback_add(list->genlist, "highlighted", _mp_artist_list_item_highlighted_cb, list);
	evas_object_smart_callback_add(list->genlist, "unhighlighted", _mp_artist_list_item_unhighlighted_cb, list);

	endfunc;
}

static void
_mp_artist_list_set_grid_style(MpArtistList_t *list)
{
	bool landscape = mp_util_is_landscape();

	MP_CHECK(list->gengrid_itc);

	list->gengrid_itc->item_style = "music/artist_grid";

	double scale = elm_config_scale_get();
	int w;
	int h;
	if (landscape) {
		w = (int)(ARTIST_GRID_LAND_W * scale);
		h = (int)(ARTIST_GRID_LAND_H * scale);
	} else {
		w = (int)(ARTIST_GRID_W * scale);
		h = (int)(ARTIST_GRID_H * scale);
	}
	elm_gengrid_item_size_set(list->genlist, w, h);
}


static void
_mp_artist_list_gengrid_create(MpArtistList_t *list)
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
		if (list->gengrid_itc == NULL) {
			ERROR_TRACE("Unable to create artist list gengrid");
			return;
		}
		list->gengrid_itc->func.text_get = _mp_artist_list_label_get;
		list->gengrid_itc->func.content_get = _mp_artist_list_icon_get;
		list->gengrid_itc->func.del = _mp_artist_list_item_del_cb;
	}

	_mp_artist_list_set_grid_style(list);
	elm_gengrid_align_set(list->genlist, 0.5, 0.0);

	evas_object_smart_callback_add(list->genlist, "highlighted", _mp_artist_list_item_highlighted_cb, list);
	evas_object_smart_callback_add(list->genlist, "unhighlighted", _mp_artist_list_item_unhighlighted_cb, list);
	/*evas_object_smart_callback_add(list->genlist, "longpressed", _mp_artist_list_item_longpressed_cb, list);*/

	endfunc;
}


void _mp_artist_list_update(void *thiz)
{
	startfunc;
	int count = 0, res = 0;
	MpArtistList_t *list = thiz;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	res = mp_media_info_group_list_count(MP_GROUP_BY_ARTIST, NULL, list->filter_str, &count);
	MP_CHECK(res == 0);

	mp_list_free_checked_path_list(list->checked_path_list);
	list->checked_path_list = mp_list_get_checked_path_list((MpList_t *)list);

	mp_evas_object_del(list->genlist);
	mp_evas_object_del(list->no_content);

	if (count) {
		if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			_mp_artist_list_gengrid_create(list);
		} else {
			_mp_artist_list_genlist_create(list);
		}

		evas_object_data_set(list->genlist, "list_handle", list);

		if (ad->del_cb_invoked == 0) {
			mp_list_bottom_counter_item_append((MpList_t *)list);
		}

		/* load list */
		_mp_artist_list_load_list(thiz, count);
		list->show_fastscroll(list);
	} else {
		list->no_content = mp_widget_create_no_contents(list->box , MP_NOCONTENT_ARTISTS, NULL, list);
		list->hide_fastscroll(list);

		elm_box_pack_end(list->box, list->no_content);
	}

}

void _mp_artist_list_rotate(void *thiz)
{
	MpArtistList_t *list = thiz;
	if (mp_list_get_display_mode((MpList_t *)list) == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		_mp_artist_list_set_grid_style(list);
	}
	if (list->genlist) {
		elm_genlist_realized_items_update(list->genlist);
	}
}

static const char *_get_label(void *thiz, void *event_info)
{
	MpArtistList_t *list = thiz;
	MP_CHECK_NULL(list);
	char *title = NULL;

	mp_list_item_data_t *artist =  elm_object_item_data_get(event_info);
	MP_CHECK_NULL(artist);

	mp_media_info_group_get_main_info(artist->handle, &title);
	return title;
}

static char *_mp_artist_list_bottom_counter_text_cb(void *thiz)
{
	MpArtistList_t *list = thiz;
	MP_CHECK_NULL(list);

	unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));

	char *text = NULL;
	if (count == 1) {
		text = g_strdup(GET_STR(STR_MP_1_ARTIST));
	} else {
		text = g_strdup_printf(GET_STR(STR_MP_PD_ARTISTS), count);
	}

	return text;
}

static void _mp_artist_list_set_edit(void *thiz, bool edit)
{
	startfunc;
	MpArtistList_t *list = thiz;
	MP_CHECK(list);

	mp_artist_list_show_group_index(list, false);

	if (list->set_edit_default) {
		list->set_edit_default(list, edit);
	}
}


static unsigned int
_mp_artist_list_get_count(void *thiz, MpListEditType_e type)
{
	MpArtistList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);

	int count = MP_LIST_OBJ_IS_GENGRID(list->genlist) ? elm_gengrid_items_count(list->genlist) : elm_genlist_items_count(list->genlist);

	if (list->group_it) { /*group index*/
		--count;
	}

	if (list->bottom_counter_item) {
		--count;
	}

	return count;
}

MpArtistList_t *mp_artist_list_create(Evas_Object *parent)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpArtistList_t *list = calloc(1, sizeof(MpArtistList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_GROUP);

	list->update = _mp_artist_list_update;
	list->destory_cb = _mp_artist_list_destory_cb;
	list->get_group_type = _mp_artist_list_get_group_type;
	list->rotate = _mp_artist_list_rotate;
	list->get_label = _get_label;

	list->display_mode_changable = true;

	list->bottom_counter_text_get_cb = _mp_artist_list_bottom_counter_text_cb;

	list->set_edit_default = list->set_edit;
	list->set_edit = _mp_artist_list_set_edit;
	list->get_count = _mp_artist_list_get_count;

	endfunc;
	return list;
}

void mp_artist_list_set_data(MpArtistList_t *list, ...)
{
	startfunc;
	MP_CHECK(list);

	va_list var_args;
	int field;

	va_start(var_args, list);
	do {
		field = va_arg(var_args, int);

		switch (field) {
		case MP_ARTIST_LIST_TYPE: {
			int val = va_arg((var_args), int);

			list->group_type = val;
			DEBUG_TRACE("list->group_type = %d", list->group_type);
			break;
		}

		case MP_ARTIST_LIST_FUNC: {
			int val = va_arg((var_args), int);

			list->function_type = val;
			DEBUG_TRACE("list->function_type = %d", list->function_type);
			break;
		}

		case MP_ARTIST_LIST_TYPE_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->type_str);
			list->type_str = g_strdup(val);
			DEBUG_TRACE("list->type_str = %s", list->type_str);

			break;
		}
		case MP_ARTIST_LIST_FILTER_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->filter_str);
			list->filter_str = g_strdup(val);
			DEBUG_TRACE("list->filter_str = %s", list->filter_str);

			break;
		}

		case MP_ARTIST_LIST_DISPLAY_MODE: {
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

void mp_artist_list_copy_data(MpArtistList_t *src, MpArtistList_t *dest)
{
	MP_CHECK(src);
	MP_CHECK(dest);

	dest->group_type = src->group_type;
	dest->function_type = src->function_type;
	SAFE_FREE(dest->type_str);
	dest->type_str = g_strdup(src->type_str);
	SAFE_FREE(dest->filter_str);
	dest->filter_str = g_strdup(src->filter_str);
	dest->display_mode = src->display_mode;
}
