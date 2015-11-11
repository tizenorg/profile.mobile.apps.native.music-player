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

#include "mp-album-detail-list.h"
#include "mp-player-view.h"
#include "mp-create-playlist-view.h"
#include "mp-ctxpopup.h"
#include "mp-popup.h"
#include "mp-util.h"
#include "mp-common.h"
#include "mp-widget.h"
#include "mp-play.h"
#include "mp-edit-callback.h"
#include "mp-album-detail-view.h"

#include <media_content.h>


static bool _mp_ablum_detail_list_check_artist_name(mp_media_list_h thiz, int count)
{
	startfunc;
	MP_CHECK_FALSE(thiz);
	mp_media_list_h svc_handle = thiz;
	MP_CHECK_FALSE(svc_handle);
	int i = 0;
	int j = 0;
	if (count > 1) {
		for (i = 0; i < count - 1; i++) {
			mp_media_info_h item = NULL;
			char* artist = NULL;
			item = mp_media_info_list_nth_item(svc_handle, i);
			MP_CHECK_FALSE(item);
			mp_media_info_get_artist(item, &artist);
			MP_CHECK_FALSE(artist);

			for (j = i + 1; j < count; j++) {
				mp_media_info_h item_temp = NULL;
				char* artist_temp = NULL;
				item_temp = mp_media_info_list_nth_item(svc_handle, j);
				MP_CHECK_FALSE(item_temp);
				mp_media_info_get_artist(item_temp, &artist_temp);
				MP_CHECK_FALSE(artist_temp);
				if (!g_strcmp0(artist, artist_temp)) {
					continue ;
				} else {
					return TRUE;
				}
			}
		}
	} else if (count == 1) {
		return FALSE;
	}
	return FALSE;
}

static char *
_mp_album_detail_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = (mp_media_info_h)(item->handle);
	mp_retvm_if(!track, NULL, "data is null");

	MpAlbumDetailList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	static char result[DEF_STR_LEN + 1] = { 0, };

	if (!strcmp(part, "elm.text.main.left")) {
		char *title = NULL;
		bool match = mp_common_track_is_current(track, (MpList_t *)list);


		if (list->track_type == MP_TRACK_BY_FOLDER) {
			mp_media_info_get_display_name(track, &title);
		} else {
			mp_media_info_get_title(track,  &title);
		}

		mp_retv_if(!title, NULL);

		char *markup = NULL;
		if (match) {
			char *info = elm_entry_utf8_to_markup(title);

			int r = 21;
			int g = 108;
			int b = 148;
			int a = 255 ;

			memset(result, 0x00, DEF_STR_LEN + 1);
			snprintf(result, DEF_STR_LEN,
			         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, info);
			IF_FREE(info);

			return g_strdup(result);

		} else {
			markup = elm_entry_utf8_to_markup(title);
		}
		return markup;
	}
	if (!list->edit_mode) {
		if (!strcmp(part, "elm.text.sub.right")) {
			int duration;
			char time[16] = "";
			bool match = mp_common_track_is_current(track, (MpList_t *)list);

			mp_media_info_get_duration(track, &duration);

			mp_util_song_format_duration(time, duration);
			time[15] = '\0';
			if (match) {
				int r = 21;
				int g = 108;
				int b = 148;
				int a = 255 ;

				memset(result, 0x00, DEF_STR_LEN + 1);
				snprintf(result, DEF_STR_LEN,
				         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, time);

				return g_strdup(result);
			}
			return g_strdup(time);
		}
	}
	return NULL;
}


Evas_Object *
_mp_album_detail_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = item->handle;
	mp_retvm_if(!track, NULL, "data is null");

	if (!strcmp(part, "elm.icon")) {
		return NULL;
	}

	MpAlbumDetailList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	if (list->edit_mode) {
		// if edit mode
		if (!strcmp(part, "elm.icon.2")) {
			// swallow checkbox or radio button
			Evas_Object *content = NULL;
			Evas_Object *icon = NULL;
			content = elm_layout_add(obj);

			icon = elm_check_add(obj);
			elm_object_style_set(icon, "default");
			evas_object_propagate_events_set(icon, EINA_FALSE);
			evas_object_smart_callback_add(icon, "changed", mp_common_view_check_changed_cb, NULL);
			elm_check_state_pointer_set(icon, &item->checked);

			elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
			elm_layout_content_set(content, "elm.swallow.content", icon);

			return content;
		}
	}

	return NULL;
}

static void
_mp_album_detail_list_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);
	SAFE_FREE(item_data);
}

static void
_mp_album_detail_genlist_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	if (list->edit_mode) {
		mp_list_edit_mode_sel(list, item);
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		mp_view_update_options_edit(view);
		return;
	}

	mp_common_play_track_list(item, obj);

	return;
}

static void _mp_album_detail_list_append_item(MpAlbumDetailList_t *list, mp_media_list_h svc_handle, int count)
{

	int index = 0;
	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		item = mp_media_info_list_nth_item(svc_handle, index);
		mp_list_item_data_t *item_data;

		/* check DRM FL */
#if 0
		if (mp_list_get_edit((MpList_t *)list) && mp_list_get_edit_type((MpList_t *)list) == MP_LIST_EDIT_TYPE_SHARE) {
			char *file_name = NULL;
			mp_media_info_get_file_path(item,  &file_name);

			if (mp_drm_check_foward_lock(file_name)) {
				continue;
			}
		}
#endif
#ifdef MP_FEATURE_PERSONAL_PAGE
		char *path = NULL;
		mp_media_info_get_file_path(item, &path);
		if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_NONE) {
			goto append_album_items;
		}

		if (mp_util_is_in_personal_page((const char *)path)) {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_ADD) {
				continue;
			}
		} else {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_REMOVE) {
				continue;
			}
		}
append_album_items:
#endif

		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->index = index;
		item_data->group_type = MP_GROUP_NONE;

		item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
		                                        ELM_GENLIST_ITEM_NONE, _mp_album_detail_genlist_sel_cb, list);
		elm_object_item_data_set(item_data->it, item_data);
	}

	endfunc;

}

static char *
_mp_album_detail_list_album_text_get(void *data, Evas_Object *obj, const char *part)
{
	startfunc;

	MpAlbumDetailList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	if (!strcmp(part, "elm.text.main.left.top")) {
		return g_strdup(elm_entry_utf8_to_markup(list->artist));
	} else if (!strcmp(part, "elm.text.sub.left.bottom")) {
		char *text = NULL;
		char *tmp = NULL;
		char time[16] = "";
		char make_up[DEF_STR_LEN + 1] = {0, };

		mp_util_format_duration(time, list->total_duration);
		time[15] = '\0';
		tmp = (list->track_count == 1) ? g_strdup(GET_STR(STR_MP_1_SONG)) : g_strdup_printf(GET_STR(STR_MP_PD_SONGS), list->track_count);
		text = g_strdup_printf("%s / %s", tmp, time);

		memset(make_up, 0x00, DEF_STR_LEN + 1);
		snprintf(make_up, DEF_STR_LEN, "%s", text);
		IF_FREE(text);
		IF_FREE(tmp);

		return g_strdup(make_up);
	}
	return NULL;

}

static char *
_mp_album_detail_list_shuffle_text_get(void *data, Evas_Object *obj, const char *part)
{
	startfunc;
	int res = -1;
	int count = 0;
	char *markup = NULL;
	static char result[DEF_STR_LEN + 1] = { 0, };

	if (!strcmp(part, "elm.text.main.left")) {
		MpAlbumDetailList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);

		int r = 21;
		int g = 108;
		int b = 148;
		int a = 255 ;

		res = mp_media_info_list_count(MP_TRACK_BY_ALBUM, list->type_str, NULL,  NULL, 0, &count);
		MP_CHECK_NULL(res == 0);

		markup = (count == 1) ? g_strdup(GET_STR(STR_MP_SHUFFLE_1_TRACK)) : g_strdup_printf(GET_STR(STR_MP_SHUFFLE_PD_TRACKS), count);

		memset(result, 0x00, DEF_STR_LEN + 1);
		snprintf(result, DEF_STR_LEN, "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, markup);
		IF_FREE(markup);

		return g_strdup(result);
	}
	return NULL;

}

Evas_Object *
_mp_album_detail_list_album_icon_get(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.icon.1")) {
		MpAlbumDetailList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);

		Evas_Object *icon = NULL;
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);

		icon = mp_util_create_thumb_icon(obj, list->thumbnail, MP_LIST_ALBUM_ICON_SIZE, MP_LIST_ALBUM_ICON_SIZE);

		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		return content;
	}
	return NULL;
}

Evas_Object *
_mp_album_detail_list_shuffle_icon_get(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.icon.1")) {
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);

		Evas_Object *icon;

		icon = mp_util_create_image(obj, IMAGE_EDJ_NAME, MP_LITE_SHUFFLE_ICON, MP_LIST_SHUFFLE_ICON_SIZE, MP_LIST_SHUFFLE_ICON_SIZE);
		evas_object_color_set(icon, 21, 108, 148, 255);

		elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		return content;
	}
	return NULL;

}


static void
_mp_album_detail_list_shuffle_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);
	mp_play_control_shuffle_set(NULL, true);
	mp_common_play_track_list_with_playlist_id(item, obj, 0);

	return;
}

static void
_mp_album_detail_list_shuffle_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static int _mp_album_detail_list_get_total_duration(MpAlbumDetailList_t *list, mp_media_list_h svc_handle, int count)
{
	int total_duration = 0;
	int duration = 0;
	int index = 0;
	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		item = mp_media_info_list_nth_item(svc_handle, index);
#ifdef MP_FEATURE_PERSONAL_PAGE
		char *path = NULL;
		mp_media_info_get_file_path(item, &path);
		if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_NONE) {
			goto calc_total_duration;
		}

		if (mp_util_is_in_personal_page((const char *)path)) {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_ADD) {
				continue;
			}
		} else {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_REMOVE) {
				continue;
			}
		}
calc_total_duration:
#endif
		mp_media_info_get_duration(item, &duration);
		duration /= 1000;
		duration *= 1000;
		total_duration += duration;
	}
	DEBUG_TRACE("total_duration %d", total_duration);
	return total_duration;
}

static void _mp_album_detail_list_append_album_item(MpAlbumDetailList_t *list)
{
	startfunc;
	MP_CHECK(list);
	Elm_Object_Item *list_item = NULL;

	list->itc_album = elm_genlist_item_class_new();
	MP_CHECK(list->itc_album);
	list->itc_album->item_style = "2line.top";//"music/1text.2icon.3";//"music/3text.1icon.2"
	list->itc_album->func.text_get = _mp_album_detail_list_album_text_get;
	list->itc_album->decorate_all_item_style = NULL;
	list->itc_album->func.content_get = _mp_album_detail_list_album_icon_get;
	list->itc_album->func.del = _mp_album_detail_list_shuffle_item_del_cb;

	mp_list_item_data_t *item_data;
	item_data = calloc(1, sizeof(mp_list_item_data_t));
	MP_CHECK(item_data);
	item_data->item_type = MP_LIST_ITEM_TYPE_SELECTABLE_GROUP_TITLE;

	item_data->it = list_item = elm_genlist_item_append(list->genlist, list->itc_album, item_data, NULL,
	                            ELM_GENLIST_ITEM_NONE, _mp_album_detail_list_shuffle_cb, list);

	if (!list->edit_mode) {
		elm_genlist_item_select_mode_set(list_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
	endfunc;
}

void mp_album_detail_list_show_shuffle(void *thiz, bool show)
{
	startfunc;
	MP_CHECK(thiz);
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK(list->genlist);

	DEBUG_TRACE("show shuffle: %d   list->shuffle_it: %0x", show, list->shuffle_it);
	if (show) {
		_mp_album_detail_list_append_album_item(list);

		list->itc_shuffle = elm_genlist_item_class_new();
		MP_CHECK(list->itc_shuffle);
		//list->itc_shuffle->item_style = "music/1line";
		list->itc_shuffle->item_style = "1line";
		list->itc_shuffle->func.text_get = _mp_album_detail_list_shuffle_text_get;
		list->itc_shuffle->decorate_all_item_style = NULL;
		list->itc_shuffle->func.content_get = _mp_album_detail_list_shuffle_icon_get;
		list->itc_shuffle->func.del = _mp_album_detail_list_shuffle_item_del_cb;

		mp_list_item_data_t *item_data;
		item_data = mp_list_item_data_create(MP_LIST_ITEM_TYPE_SHUFFLE);
		MP_CHECK(item_data);

		item_data->it = list->shuffle_it = elm_genlist_item_append(list->genlist, list->itc_shuffle, item_data, NULL,
		                                   ELM_GENLIST_ITEM_NONE, _mp_album_detail_list_shuffle_cb, list);
		elm_object_item_data_set(item_data->it, item_data);
	} else if (list->shuffle_it) {
		elm_object_item_del(list->shuffle_it);
		list->shuffle_it = NULL;
	}
}

void mp_album_detail_list_popup_delete_genlist_item(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK(list->genlist);

	if (list->track_count > 0) {
		list->track_count--;
	}
}

void mp_album_detail_list_update_genlist(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK(list->genlist);

	if (list->track_count <= 0) {
		mp_list_update(thiz);
	} else {
		elm_genlist_realized_items_update(list->genlist);
	}
}

static void _mp_album_detail_list_load_list(void *thiz, int count)
{
	startfunc;
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle = NULL;

	/*clear genlist*/
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	if (item) {
		elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
		elm_genlist_clear(list->genlist);
	}

	mp_ecore_timer_del(list->load_timer);

	/*get data from DB*/
	PROFILE_IN("mp_media_info_list_create");
	mp_media_info_list_create(&svc_handle, list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, 0, count);
	PROFILE_OUT("mp_media_info_list_create");

	mp_album_detail_list_show_shuffle(list, true);

	list->total_duration = _mp_album_detail_list_get_total_duration(list, svc_handle, list->track_count);

	list->various_name = _mp_ablum_detail_list_check_artist_name(svc_handle, count);

	PROFILE_IN("_mp_album_detail_list_append_item");
	_mp_album_detail_list_append_item(list, svc_handle, count);
	PROFILE_OUT("_mp_album_detail_list_append_item");

	if (list->track_list[0]) {
		mp_media_info_list_destroy(list->track_list[0]);
	}
	list->track_list[0] = svc_handle;

}
static void _mp_album_detail_list_destory_cb(void *thiz)
{
	eventfunc;
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK(list);

	mp_ecore_timer_del(list->load_timer);

	if (list->track_list[0]) {
		mp_media_info_list_destroy(list->track_list[0]);
		list->track_list[0] = NULL;
	}
	if (list->track_list[1]) {
		mp_media_info_list_destroy(list->track_list[1]);
		list->track_list[1] = NULL;
	}

	if (list->itc) {
		elm_genlist_item_class_free(list->itc);
		list->itc = NULL;
	}
	if (list->itc_album) {
		elm_genlist_item_class_free(list->itc_album);
		list->itc_album = NULL;
	}
	if (list->itc_shuffle) {
		elm_genlist_item_class_free(list->itc_shuffle);
		list->itc_shuffle = NULL;
	}

	IF_FREE(list->type_str);
	IF_FREE(list->type_str2);
	IF_FREE(list->filter_str);
	IF_FREE(list->artist);
	IF_FREE(list->thumbnail);
	free(list);
}

/*static void
_mp_album_detail_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpAlbumDetailList_t *list = (MpAlbumDetailList_t*)data;
	MP_CHECK(list);

        struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

        char *title = NULL;
	 char *file_path = NULL;
        int pop_item_count = 5;
        Elm_Object_Item *temp = NULL;
        Evas_Object *popup = NULL;
        mp_list_item_data_t *item_data = NULL;

        MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
        MP_CHECK(top_view);

        if (list->scroll_drag_status || list->edit_mode == 1 || list->reorderable ||
                MP_VIEW_SEARCH == top_view->view_type || list->shuffle_it == item)
                return;

        temp = elm_genlist_item_next_get(list->shuffle_it);
	while (temp) {
                item_data = elm_object_item_data_get(temp);
                item_data->checked = false;
		temp = elm_genlist_item_next_get(temp);
	}

	item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);

        item_data->checked = true;

        mp_media_info_get_title(item_data->handle, &title);
	mp_media_info_get_file_path(item_data->handle, &file_path);

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

        char *up_title = g_strdup(title);

        elm_object_part_text_set(popup, "title,text", up_title);
        IF_FREE(up_title);

        mp_genlist_popup_item_append(popup, STR_MP_SET_AS, NULL, NULL, NULL,
                                     mp_common_list_set_as_cb, list);

        mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PLAYLIST, NULL, NULL, NULL,
                                     mp_common_list_add_to_playlist_cb, list);
        if (list->track_type != MP_TRACK_BY_FAVORITE) {
        	 bool favourite = false;
        	char *str = NULL;
        	Evas_Smart_Cb cb = NULL;

        	mp_media_info_get_favorite(item_data->handle, &favourite);
        	if (favourite) {
        		str = STR_MP_POPUP_REMOVE_FROM_FAVORITE;
        		cb = mp_common_list_unfavorite_cb;
        	}
        	else {
        		str = STR_MP_ADD_TO_FAVOURITES;
        		cb = mp_common_list_add_to_favorite_cb;
        	}

        	mp_genlist_popup_item_append(popup, str, NULL, NULL, NULL, cb, list);
        }

        mp_genlist_popup_item_append(popup, STR_MP_DELETE, NULL, NULL, NULL,
                                             mp_common_list_delete_cb, list);

#ifdef MP_FEATURE_PERSONAL_PAGE
	if (mp_util_is_personal_page_on())
	{
		if (mp_util_is_in_personal_page((const char *)file_path))
		{
			DEBUG_TRACE("remove from personal page");
			list->personal_page_storage = MP_LIST_PERSONAL_PAGE_PRIVATE;
			mp_genlist_popup_item_append(popup, STR_MP_REMOVE_FROM_PERSONAL_PAGE, NULL, NULL, NULL,
												 mp_common_longpress_private_move_cb, list);
		}
		else
		{
			DEBUG_TRACE("add to personal page");
			list->personal_page_storage = MP_LIST_PERSONAL_PAGE_NORMAL;
			mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PERSONAL_PAGE, NULL, NULL, NULL,
												 mp_common_longpress_private_move_cb, list);
		}
	}
#endif
        mp_genlist_popup_item_append(popup, STR_MP_POPUP_MORE_INFO, NULL, NULL, NULL,
                                     mp_common_list_more_info_cb, list);
#ifdef MP_FEATURE_ALBUMART_UPDATE
	mp_genlist_popup_item_append(popup, STR_MP_UPDATE_ALBUM_ART, NULL, NULL, NULL,
                                     mp_common_list_update_albumart_cb, list);
#endif

	MP_GENLIST_ITEM_LONG_PRESSED(obj, popup, event_info);
}*/

static void _mp_album_detail_list_item_highlighted(void *data, Evas_Object *obj, void *event_info)
{
	MpAlbumDetailList_t *list = data;
	MP_CHECK(list);

	Elm_Object_Item *it = event_info;
	MP_CHECK(it);

	if (list->shuffle_it == it) {
		Evas_Object *icon = elm_object_item_part_content_get(it, "elm.icon.2");
		if (icon) {
			elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SHUFFLE_PRESS);
		}
	}
}

static void _mp_album_detail_list_item_unhighlighted(void *data, Evas_Object *obj, void *event_info)
{
	MpAlbumDetailList_t *list = data;
	MP_CHECK(list);

	Elm_Object_Item *it = event_info;
	MP_CHECK(it);

	if (list->shuffle_it == it) {
		Evas_Object *icon = elm_object_item_part_content_get(it, "elm.icon.2");
		if (icon) {
			elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SHUFFLE);
		}
	}
}

static void _mp_album_detail_list_update(void *thiz)
{
	startfunc;
	int count = 0, res = 0;
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK(list);

	PROFILE_IN("mp_media_info_list_count");
	res = mp_media_info_list_count(list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, &count);
	PROFILE_OUT("mp_media_info_list_count");
	MP_CHECK(res == 0);
	list->track_count = count;

	mp_evas_object_del(list->no_content);
	mp_evas_object_del(list->genlist);

	if (count) {
		/*create new genlist*/
		PROFILE_IN("elm_genlist_add");
		list->genlist = mp_widget_genlist_create(list->box);
		elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
		evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_genlist_homogeneous_set(list->genlist, EINA_TRUE);
		elm_genlist_mode_set(list->genlist, ELM_LIST_COMPRESS);
		evas_object_show(list->genlist);
		/*packet genlist to box*/
		elm_box_pack_end(list->box, list->genlist);
		evas_object_data_set(list->genlist, "list_data", list);

		if (!list->itc) {
			list->itc = elm_genlist_item_class_new();
			if (list->itc) {
				list->itc->item_style = "1line.2";//"music/2text";
				//list->itc->decorate_all_item_style = "decorate/edit_default";//edit_default
				list->itc->func.text_get = _mp_album_detail_list_label_get;
				list->itc->func.content_get = _mp_album_detail_list_icon_get;
				list->itc->func.del = _mp_album_detail_list_item_del_cb;
			}
		}

		//evas_object_smart_callback_add(list->genlist, "longpressed", _mp_album_detail_list_item_longpressed_cb, list);
		evas_object_smart_callback_add(list->genlist, "scroll,drag,start", list->drag_start_cb, list);
		evas_object_smart_callback_add(list->genlist, "scroll,drag,stop", list->drag_stop_cb, list);
		evas_object_smart_callback_add(list->genlist, "highlighted", _mp_album_detail_list_item_highlighted, list);
		evas_object_smart_callback_add(list->genlist, "unhighlighted", _mp_album_detail_list_item_unhighlighted, list);

//		mp_list_bottom_counter_item_append((MpList_t *)list);

		PROFILE_OUT("elm_genlist_add");
		/* load list */
		PROFILE_IN("_mp_album_detail_list_load_list");
		_mp_album_detail_list_load_list(thiz, count);
		PROFILE_OUT("_mp_album_detail_list_load_list");

		if (!mp_list_get_editable_count(thiz, mp_list_get_edit_type(thiz))) {
			goto NoContents;
		}

		return;
	}

NoContents:
	mp_evas_object_del(list->genlist);
	if (!list->no_content) {
		list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_TRACKS, NULL, list);
		elm_box_pack_end(list->box, list->no_content);
	}
}

static mp_track_type_e _mp_album_detail_list_get_track_type(void *thiz)
{
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK_VAL(list, MP_TRACK_ALL);
	return list->track_type;
}

static const char *_get_label(void *thiz, void *event_info)
{
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK_NULL(list);
	char *title = NULL;

	mp_list_item_data_t *track =  elm_object_item_data_get(event_info);
	MP_CHECK_NULL(track);

	mp_media_info_get_title(track->handle, &title);
	return title;
}

static void _mp_album_detail_list_set_edit(void *thiz, bool edit)
{
	startfunc;
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK(list);

	mp_album_detail_list_show_shuffle(list, false);

	/* check DRM FL */
	if (mp_list_get_edit_type((MpList_t*)list) == MP_LIST_EDIT_TYPE_SHARE) {
		_mp_album_detail_list_update(list);
	}

	if (list->set_edit_default) {
		list->set_edit_default(list, edit);
	}
}

static unsigned int
_mp_album_detail_list_get_editable_count(void *thiz, MpListEditType_e type)
{
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);

	int count = elm_genlist_items_count(list->genlist);

	if (type == MP_LIST_EDIT_TYPE_SHARE) {
		Elm_Object_Item *gl_item = elm_genlist_first_item_get(list->genlist);
		while (gl_item) {
			mp_list_item_data_t *item_data = elm_object_item_data_get(gl_item);
			if (item_data && item_data->handle) {
				char *path = NULL;
				mp_media_info_get_file_path(item_data->handle, &path);
			}

			gl_item = elm_genlist_item_next_get(gl_item);
		}
	}

	if (list->shuffle_it) {
		--count;
	}

	if (list->bottom_counter_item) {
		--count;
	}

	return count - 1;
}

static char *
_mp_album_detail_list_bottom_counter_text_cb(void *thiz)
{
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK_NULL(list);

	unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));

	char *text = NULL;
	if (count == 1) {
		text = g_strdup(GET_STR(STR_MP_1_SONG));
	} else {
		text = g_strdup_printf(GET_STR(STR_MP_PD_SONGS), count);
	}

	return text;
}

static mp_group_type_e _mp_album_detail_list_get_group_type(void *thiz)
{
	MpAlbumDetailList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);
	return MP_GROUP_NONE;
}

MpAlbumDetailList_t * mp_album_detail_list_create(Evas_Object *parent)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpAlbumDetailList_t *list = calloc(1, sizeof(MpAlbumDetailList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_ALBUM_DETAIL);

	list->update = _mp_album_detail_list_update;
	list->destory_cb = _mp_album_detail_list_destory_cb;
	list->get_track_type = _mp_album_detail_list_get_track_type;
	list->get_group_type = _mp_album_detail_list_get_group_type;
	//list->get_playlist_handle = _mp_album_detail_list_get_handle;
	list->get_label = _get_label;

	list->set_edit_default = list->set_edit;
	list->set_edit = _mp_album_detail_list_set_edit;
	list->get_count = _mp_album_detail_list_get_editable_count;
	list->bottom_counter_text_get_cb = _mp_album_detail_list_bottom_counter_text_cb;

	return list;
}

void mp_album_detail_list_set_data(MpAlbumDetailList_t *list, ...)
{
	startfunc;
	MP_CHECK(list);

	va_list var_args;
	int field;

	va_start(var_args, list);
	do {
		field = va_arg(var_args, int);
		DEBUG_TRACE("field is %d", field);

		switch (field) {
		case MP_ALBUM_DETAIL_LIST_TYPE: {
			int val = va_arg((var_args), int);

			list->track_type = val;
			DEBUG_TRACE("list->track_type = %d", list->track_type);
			break;
		}

		case MP_ALBUM_DETAIL_TYPE_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->type_str);
			list->type_str = g_strdup(val);
			DEBUG_TRACE("list->type_str = %s", list->type_str);

			break;
		}

		case MP_ALBUM_DETAIL_ARTIST: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->artist);
			list->artist = g_strdup(val);
			DEBUG_TRACE("list->artist = %s", list->artist);

			break;
		}

		case MP_ALBUM_DETAIL_THUMBNAIL: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->thumbnail);
			list->thumbnail = g_strdup(val);
			DEBUG_TRACE("list->thumbnail = %s", list->thumbnail);

			break;
		}
		default:
			DEBUG_TRACE("Invalid arguments");
		}

	} while (field >= 0);

	va_end(var_args);
}

void mp_album_detail_list_copy_data(MpAlbumDetailList_t *src, MpAlbumDetailList_t *dest)
{
	MP_CHECK(src);
	MP_CHECK(dest);

	dest->track_type = src->track_type;

	SAFE_FREE(dest->type_str);
	dest->type_str = g_strdup(src->type_str);
	SAFE_FREE(dest->artist);
	dest->artist = g_strdup(src->artist);
	SAFE_FREE(dest->thumbnail);
	dest->thumbnail = g_strdup(src->thumbnail);
}

