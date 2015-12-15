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

#include "mp-track-list.h"
#include "mp-player-view.h"
#include "mp-search-list.h"
#include "mp-ctxpopup.h"
#include "mp-popup.h"
#include "mp-util.h"
#include "mp-widget.h"
#include "mp-artist-detail-view.h"
#include "mp-album-detail-view.h"
#include "mp-play.h"
#include "mp-common.h"

static Evas_Object *
_mp_search_list_no_content_add(void *data)
{
	DEBUG_TRACE_FUNC();
	MpSearchList_t *list = (MpSearchList_t *)data;
	MP_CHECK_VAL(list, NULL);

	Evas_Object *no_contents = NULL;

	no_contents = elm_layout_add(list->box);
	evas_object_size_hint_weight_set(no_contents, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(no_contents, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//elm_layout_file_set(no_contents, EDJ_NAME, "no_result");
	elm_layout_theme_set(no_contents, "layout", "nocontents", "default");
	//edje_object_part_text_set(_EDJ(no_contents), "elm.text", GET_STR("IDS_MUSIC_BODY_NO_RESULTS_FOUND"));
	//mp_language_mgr_register_object(no_contents, OBJ_TYPE_EDJE_OBJECT, "elm.text", "IDS_MUSIC_BODY_NO_RESULTS_FOUND");
	mp_util_domain_translatable_part_text_set(no_contents, "elm.text", "IDS_MUSIC_BODY_NO_RESULTS_FOUND");

	if (list->genlist) {
		elm_box_unpack(list->box, list->genlist);
		evas_object_hide(list->genlist);
	}

	elm_box_pack_end(list->box, no_contents);
	evas_object_show(no_contents);

	return no_contents;
}

static int
_mp_search_list_set_sentinel(void *thiz, int count)
{
	MpSearchList_t *list = (MpSearchList_t *)thiz;
	MP_CHECK_VAL(list, -1);
	if (0 >= count && (list->filter_str && strlen(list->filter_str))) {
		ERROR_TRACE("no tracks");
		if (!list->no_content) {
			list->no_content = _mp_search_list_no_content_add(list);
		}
		return -1;
	}

	if (list->no_content) {
		elm_box_unpack(list->box, list->no_content);
		evas_object_del(list->no_content);
		list->no_content = NULL;
		elm_box_pack_end(list->box, list->genlist);
		evas_object_show(list->genlist);
	}

	return 0;
}

static char *
_mp_search_track_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	startfunc;
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h track = item_data->handle;
	mp_retvm_if(!track, NULL, "data is null");

	MpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(part);

	if (!strcmp(part, "elm.text")) {
		char *title = NULL;

		mp_media_info_get_title(track,  &title);

		mp_retv_if(!title, NULL);
		if (!strcmp(part, "elm.text")) {
			bool res = false;
			char *markup_name = (char *)mp_util_search_markup_keyword(title, list->filter_str, &res);
			return (res) ? g_strdup(markup_name) : elm_entry_utf8_to_markup(title);
		} else {
			return g_strdup(title);
		}
	} else if (!strcmp(part, "elm.text.sub")) {
		char *artist = NULL;

		mp_media_info_get_artist(track, &artist);
		mp_retv_if(!artist, NULL);
		return g_strdup(artist);
	}
	return NULL;
}

Evas_Object *
_mp_search_track_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	startfunc;
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h track = item_data->handle;
	mp_retvm_if(!track, NULL, "data is null");

	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);
		char *thumbpath = NULL;
		Evas_Object *icon;

		mp_media_info_get_thumbnail_path(track, &thumbpath);
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);
		return content;
	}
	return NULL;
}


static void
_mp_search_track_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	Elm_Object_Item *gli2 = NULL;
	elm_genlist_item_selected_set(gli, FALSE);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpSearchList_t *list = (MpSearchList_t *)data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	DEBUG_TRACE("item selected");

	mp_plst_item *plst_item = NULL;
	mp_plst_item *to_play = NULL;
	char *prev_item_uid = NULL;

	plst_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	if (plst_item) {
		prev_item_uid = g_strdup(plst_item->uid);
	}

	if (!ad->playlist_mgr) {
		mp_common_create_playlist_mgr();
	}

	mp_playlist_mgr_clear(ad->playlist_mgr);
	gli2 = elm_genlist_first_item_get(obj);
	while (gli2) {
		if (elm_genlist_item_select_mode_get(gli2)  != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			mp_list_item_data_t *item_data = elm_object_item_data_get(gli2);
			if (item_data && item_data->item_type == MP_LIST_ITEM_TYPE_NORMAL && item_data->handle) {
				if (item_data->group_type == MP_GROUP_NONE || item_data->group_type == MP_GROUP_BY_ALLSHARE) {
					char *uri = NULL;
					char *uid = NULL;
					char *title = NULL;
					char *artist = NULL;

					mp_track_type track_type = MP_TRACK_URI;

					mp_media_info_get_media_id(item_data->handle, &uid);
					mp_media_info_get_file_path(item_data->handle, &uri);
					mp_media_info_get_title(item_data->handle, &title);
					mp_media_info_get_artist(item_data->handle, &artist);

#ifdef MP_FEATURE_CLOUD
					mp_storage_type_e storage;
					mp_media_info_get_storage_type(item->handle, &storage);
					if (storage == MP_STORAGE_CLOUD) {
						track_type = MP_TRACK_CLOUD;
					}
#endif
					plst_item = mp_playlist_mgr_item_append(ad->playlist_mgr, uri, uid, title, artist, track_type);
					if (gli2 == gli && plst_item) {
						to_play = plst_item;
					}
				}
			}
		}
		gli2 = elm_genlist_item_next_get(gli2);
	}

	MP_CHECK(to_play);
	if (!ad->current_track_info || g_strcmp0(ad->current_track_info->uri , to_play->uri)) {
		mp_playlist_mgr_set_current(ad->playlist_mgr, to_play);
		mp_play_destory(ad);
		ad->paused_by_user = FALSE;
	}

	//disable conformant resizing in player view when the keypad is enabled
	elm_object_signal_emit(ad->conformant, "elm,state,virtualkeypad,disable", "");
	mp_common_show_player_view(MP_PLAYER_NORMAL, false, true, true);

	IF_FREE(prev_item_uid);

	return;
}

static char *
_mp_search_album_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	startfunc;
	char *name = NULL;
	int ret = 0;

	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h svc_item = item_data->handle;
	mp_retv_if(svc_item == NULL, NULL);

	MpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(part);

	if (!strcmp(part, "elm.text")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}

		if (!strcmp(part, "elm.text")) {
			bool res = false;
			char *markup_name = (char *)mp_util_search_markup_keyword(name, list->filter_str, &res);
			return (res) ? g_strdup(markup_name) : elm_entry_utf8_to_markup(name);
		} else {
			return g_strdup(name);
		}

	} else if (!strcmp(part, "elm.text.sub")) {
		ret = mp_media_info_group_get_sub_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}
		return g_strdup(name);
	}

	DEBUG_TRACE("Unusing part: %s", part);
	return NULL;
}

Evas_Object *
_mp_search_album_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	startfunc;

	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h svc_item = item_data->handle;
	mp_retv_if(svc_item == NULL, NULL);
	MP_CHECK_NULL(part);

	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);
		Evas_Object *icon = NULL;
		char *thumb_name = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumb_name, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);
		return content;
	}

	return NULL;
}

static void
_mp_search_album_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	//MpSearchList_t *list = (MpSearchList_t *)data;
	DEBUG_TRACE("");
	int ret = 0;
	char *name = NULL;
	char *artist = NULL;
	char *title = NULL;
	char *thumbnail = NULL;

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);
	elm_genlist_item_selected_set(gli, FALSE);

	DEBUG_TRACE("");
	mp_list_item_data_t *gli_data = elm_object_item_data_get(gli);
	MP_CHECK(gli_data);

	if (gli_data->handle) {
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		ret = mp_media_info_group_get_sub_info(gli_data->handle, &artist);
		mp_media_info_group_get_thumbnail_path(gli_data->handle, &thumbnail);
		DEBUG_TRACE("thumbnail=%s", thumbnail);
		mp_retm_if(ret != 0, "Fail to get value");
		mp_retm_if(name == NULL, "Fail to get value");

		title = name;
	}


	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpAlbumDetailView_t *view_album_detail = mp_album_detail_view_create(view_manager->navi, name, artist, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_album_detail, NULL);

	mp_view_update_options((MpView_t *)view_album_detail);
	mp_view_set_title((MpView_t *)view_album_detail, title);
}

static char *
_mp_search_artist_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	startfunc;
	char *name = NULL;
	int ret = 0;

	mp_list_item_data_t *item_data = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item_data);
	mp_media_info_h svc_item = (item_data->handle);
	mp_retv_if(svc_item == NULL, NULL);
	MpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(part);

	if (!strcmp(part, "elm.text")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}
		if (!strcmp(part, "elm.text")) {
			bool res = false;
			char *markup_name = (char *)mp_util_search_markup_keyword(name, list->filter_str, &res);
			return (res) ? g_strdup(markup_name) : elm_entry_utf8_to_markup(name);
		} else {
			return g_strdup(name);
		}
	}
	return NULL;
}

Evas_Object *
_mp_search_artist_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	startfunc;
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h svc_item = item_data->handle;
	mp_retv_if(svc_item == NULL, NULL);

	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);
		char *thumb_name = NULL;
		Evas_Object *icon = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumb_name, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);
		return content;
	}

	return NULL;
}

static void
_mp_search_artist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	//MpSearchList_t *list = (MpSearchList_t *)data;
	DEBUG_TRACE("");
	int ret = 0;
	char *name = NULL;
	char *artist = NULL;
	char *title = NULL;
	char *thumbnail = NULL;

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);
	elm_genlist_item_selected_set(gli, FALSE);

	DEBUG_TRACE("");
	mp_list_item_data_t *gli_data = elm_object_item_data_get(gli);
	MP_CHECK(gli_data);

	if (gli_data->handle) {
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		ret = mp_media_info_group_get_sub_info(gli_data->handle, &artist);
		mp_media_info_group_get_thumbnail_path(gli_data->handle, &thumbnail);
		DEBUG_TRACE("thumbnail=%s", thumbnail);
		mp_retm_if(ret != 0, "Fail to get value");
		mp_retm_if(name == NULL, "Fail to get value");

		title = name;
	}

	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpArtistDetailView_t *view_artist_detail = mp_artist_detail_view_create(view_manager->navi, name, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_artist_detail, NULL);

	mp_view_update_options((MpView_t *)view_artist_detail);
	mp_view_set_title((MpView_t *)view_artist_detail, title);
}

char *
_mp_search_list_gl_label_get_title(void *data, Evas_Object * obj, const char *part)
{
	startfunc;
	char *text = NULL;

	if (!strcmp(part, "elm.text")) {
		text = GET_STR(data);
		return g_strdup(text);
	}
	return NULL;
}

static void
_mp_search_list_item_del(void *data, Evas_Object * obj)
{
	startfunc;
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);
	mp_language_mgr_unregister_genlist_item(item_data->it);
	free(item_data);
}

static void _mp_search_list_set_itc(void *thiz)
{
	MpSearchList_t *list = thiz;
	MP_CHECK(list);
	if (!list->itc_track) {
		list->itc_track = elm_genlist_item_class_new();
		MP_CHECK(list->itc_track);
		list->itc_track->item_style =  "type1";
		list->itc_track->func.text_get = _mp_search_track_list_label_get;
		list->itc_track->func.content_get = _mp_search_track_list_icon_get;
		list->itc_track->func.del = _mp_search_list_item_del;
	}
	if (!list->itc_album) {
		list->itc_album = elm_genlist_item_class_new();
		MP_CHECK(list->itc_album);
		list->itc_album->item_style =  "type1";
		list->itc_album->func.text_get = _mp_search_album_list_label_get;
		list->itc_album->func.content_get = _mp_search_album_list_icon_get;
		list->itc_album->func.del = _mp_search_list_item_del;
	}
	if (!list->itc_artist) {
		list->itc_artist = elm_genlist_item_class_new();
		MP_CHECK(list->itc_artist);
		list->itc_artist->item_style = "type1";
		list->itc_artist->func.text_get = _mp_search_artist_list_label_get;
		list->itc_artist->func.content_get = _mp_search_artist_list_icon_get;
		list->itc_artist->func.del = _mp_search_list_item_del;
	}
	if (!list->itc_group_title) {
		list->itc_group_title = elm_genlist_item_class_new();
		MP_CHECK(list->itc_group_title);
		list->itc_group_title->item_style = "groupindex";
		list->itc_group_title->func.text_get = _mp_search_list_gl_label_get_title;
	}
}

static void
_mp_search_list_append_group_title(void * data, char *text_ID, int index)
{
	startfunc;
	MpSearchList_t *list = (MpSearchList_t *)data;
	MP_CHECK(list);
	list->search_group_git[index] =
	    elm_genlist_item_append(list->genlist, list->itc_group_title, text_ID, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
	elm_genlist_item_select_mode_set(list->search_group_git[index], ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

static void _mp_search_list_refresh_list(void *thiz)
{
	startfunc;
	MpSearchList_t *list = (MpSearchList_t *)thiz;
	MP_CHECK(list);

	int count = 0;
	gint track_count = 0;
	gint artist_count = 0;
	gint album_count = 0;
	gint index = 0;
	int ret = 0;
	mp_media_list_h svc_handle = NULL;

	ret = mp_media_info_group_list_count(MP_GROUP_BY_ARTIST, NULL, list->filter_str, &artist_count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to create structure");
		goto END;
	}
	ret = mp_media_info_group_list_count(MP_GROUP_BY_ALBUM, NULL, list->filter_str, &album_count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to create structure");
		goto END;
	}
	ret = mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, list->filter_str, 0, &track_count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to create structure");
		goto END;
	}
	DEBUG_TRACE("artist %d album %d  track %d", artist_count, album_count, track_count);
	count = artist_count + album_count + track_count;

	if (_mp_search_list_set_sentinel(list, count)) {
		goto END;
	}
	elm_genlist_clear(list->genlist);
	if (artist_count) {
		_mp_search_list_append_group_title(list, ("IDS_MUSIC_TAB4_ARTISTS"), MP_SEARCH_ARTIST_GROUP);
		ret = mp_media_info_group_list_create(&svc_handle, MP_GROUP_BY_ARTIST, NULL, list->filter_str, 0, artist_count);
		if (ret != 0) {
			DEBUG_TRACE("Fail to get items");
			if (svc_handle) {
				mp_media_info_group_list_destroy(svc_handle);
			}
			goto END;
		}

		for (index = 0; index < artist_count; index++) {
			mp_media_info_h item = NULL;
			item = mp_media_info_group_list_nth_item(svc_handle, index);
			if (item == NULL) {
				DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
				continue;
			}
			mp_list_item_data_t *item_data = calloc(1, sizeof(mp_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_BY_ARTIST;
			item_data->it = elm_genlist_item_append(list->genlist, list->itc_artist, (void *)item_data, NULL, ELM_GENLIST_ITEM_NONE, _mp_search_artist_select_cb, list);
			elm_object_item_data_set(item_data->it, item_data);

			char*name = NULL;
			mp_media_info_group_get_main_info(item, &name);
			list->artist_list = g_list_append(list->artist_list, g_strdup(name));
		}

		if (list->artist_handle) {
			mp_media_info_group_list_destroy(list->artist_handle);
			list->artist_handle = NULL;
		}
		list->artist_handle = svc_handle;
		svc_handle = NULL;
	}

	if (album_count) {
		_mp_search_list_append_group_title(list, ("IDS_MUSIC_TAB4_ALBUMS"), MP_SEARCH_ALBUM_GROUP);
		ret = mp_media_info_group_list_create(&svc_handle, MP_GROUP_BY_ALBUM, NULL, list->filter_str, 0, album_count);
		if (ret != 0) {
			DEBUG_TRACE("Fail to get items");
			if (svc_handle) {
				mp_media_info_group_list_destroy(svc_handle);
			}
			goto END;
		}

		for (index = 0; index < album_count; index++) {
			mp_media_info_h item = NULL;
			char *title = NULL;
			item = mp_media_info_group_list_nth_item(svc_handle, index);
			if (item == NULL) {
				DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
				continue;
			}
			ret = mp_media_info_group_get_main_info(item, &title);
			mp_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mp_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_BY_ALBUM;
			item_data->it = elm_genlist_item_append(list->genlist, list->itc_album, (void *)item_data, NULL, ELM_GENLIST_ITEM_NONE, _mp_search_album_select_cb, list);
			elm_object_item_data_set(item_data->it, item_data);
			char* name = NULL;
			mp_media_info_group_get_main_info(item, &name);
			list->album_list = g_list_append(list->album_list, g_strdup(name));
		}
		if (list->album_handle) {
			mp_media_info_group_list_destroy(list->album_handle);
			list->album_handle = NULL;
		}
		list->album_handle = svc_handle;
		svc_handle = NULL;
	}

	if (track_count) {
		_mp_search_list_append_group_title(list, (STR_MP_TRACKS), MP_SEARCH_TRACK_GROUP);
		ret = mp_media_info_list_create(&svc_handle, MP_TRACK_ALL, NULL, NULL, list->filter_str, 0, 0, track_count);
		if (ret != 0) {
			DEBUG_TRACE("Fail to get items");
			if (svc_handle) {
				mp_media_info_list_destroy(svc_handle);
			}
			goto END;
		}
		for (index = 0; index < track_count; index++) {
			mp_media_info_h item = NULL;
			item = mp_media_info_list_nth_item(svc_handle, index);
			if (item == NULL) {
				DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
				continue;
			}
			mp_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mp_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_NONE;
			item_data->it = elm_genlist_item_append(list->genlist, list->itc_track, (void *)item_data, NULL, ELM_GENLIST_ITEM_NONE, _mp_search_track_sel_cb, list);
			elm_object_item_data_set(item_data->it, item_data);
			char* title = NULL;
			mp_media_info_get_title(item,  &title);
			list->track_list = g_list_append(list->track_list, g_strdup(title));
		}
		if (list->track_handle) {
			mp_media_info_list_destroy(list->track_handle);
			list->track_handle = NULL;
		}
		list->track_handle = svc_handle;
		svc_handle = NULL;
	}
	evas_object_show(list->genlist);
END:
	endfunc;
}

static void _mp_search_list_load_list(void *thiz)
{
	startfunc;
	MpSearchList_t *list = (MpSearchList_t *)thiz;
	MP_CHECK(list);

	int count = 0;
	gint track_count = 0;
	gint artist_count = 0;
	gint album_count = 0;

	gint index = 0;
	int ret = 0;
	mp_media_list_h svc_handle = NULL;

	ret = mp_media_info_group_list_count(MP_GROUP_BY_ARTIST, NULL, list->filter_str, &artist_count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to create structure");
		goto END;
	}
	ret = mp_media_info_group_list_count(MP_GROUP_BY_ALBUM, NULL, list->filter_str, &album_count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to create structure");
		goto END;
	}
	ret = mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, list->filter_str, 0, &track_count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to create structure");
		goto END;
	}

	count = artist_count + album_count + track_count;
	list->track_count = track_count;

	if (_mp_search_list_set_sentinel(list, count)) {
		goto END;
	}


	/*create new genlist*/
	if (list->genlist != NULL) {
		evas_object_del(list->genlist);
		list->genlist = NULL;
	}

	list->genlist = mp_widget_genlist_create(list->box);
	elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_homogeneous_set(list->genlist, EINA_TRUE);
	elm_genlist_mode_set(list->genlist, ELM_LIST_COMPRESS);
	evas_object_show(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);

	evas_object_data_set(list->genlist, "list_handle", (void *)list);

	_mp_search_list_set_itc(list);

	if (artist_count) {
		DEBUG_TRACE("append artist list items");
		_mp_search_list_append_group_title(list, ("IDS_MUSIC_TAB4_ARTISTS"), MP_SEARCH_ARTIST_GROUP);
		ret = mp_media_info_group_list_create(&svc_handle, MP_GROUP_BY_ARTIST, NULL, list->filter_str, 0, artist_count);
		if (ret != 0) {
			DEBUG_TRACE("Fail to get items");
			if (svc_handle) {
				mp_media_info_group_list_destroy(svc_handle);
			}
			goto END;
		}

		for (index = 0; index < artist_count; index++) {
			mp_media_info_h item = NULL;

			item = mp_media_info_group_list_nth_item(svc_handle, index);
			if (item == NULL) {
				DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
				continue;
			}

			mp_list_item_data_t *item_data = calloc(1, sizeof(mp_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_BY_ARTIST;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc_artist, (void *)item_data,
			                                        NULL, ELM_GENLIST_ITEM_NONE,
			                                        _mp_search_artist_select_cb, list);
			elm_object_item_data_set(item_data->it, item_data);

			char*name = NULL;
			mp_media_info_group_get_main_info(item, &name);
			list->artist_list = g_list_append(list->artist_list, g_strdup(name));
		}

		if (list->artist_handle) {
			mp_media_info_group_list_destroy(list->artist_handle);
			list->artist_handle = NULL;
		}
		list->artist_handle = svc_handle;
		svc_handle = NULL;

	}

	if (album_count) {
		DEBUG_TRACE("append album_count list items");
		_mp_search_list_append_group_title(list, ("IDS_MUSIC_TAB4_ALBUMS"), MP_SEARCH_ALBUM_GROUP);

		ret = mp_media_info_group_list_create(&svc_handle, MP_GROUP_BY_ALBUM, NULL, list->filter_str, 0, album_count);
		if (ret != 0) {
			DEBUG_TRACE("Fail to get items");
			if (svc_handle) {
				mp_media_info_group_list_destroy(svc_handle);
			}
			goto END;
		}

		for (index = 0; index < album_count; index++) {
			mp_media_info_h item = NULL;
			char *title = NULL;
			item = mp_media_info_group_list_nth_item(svc_handle, index);
			if (item == NULL) {
				DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
				continue;
			}
			ret = mp_media_info_group_get_main_info(item, &title);

			mp_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mp_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_BY_ALBUM;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc_album, (void *)item_data,
			                                        NULL, ELM_GENLIST_ITEM_NONE,
			                                        _mp_search_album_select_cb, list);
			elm_object_item_data_set(item_data->it, item_data);
			char* name = NULL;
			mp_media_info_group_get_main_info(item, &name);
			list->album_list = g_list_append(list->album_list, g_strdup(name));
		}

		if (list->album_handle) {
			mp_media_info_group_list_destroy(list->album_handle);
			list->album_handle = NULL;
		}
		list->album_handle = svc_handle;
		svc_handle = NULL;

	}

	if (track_count) {
		DEBUG_TRACE("append track_count list items");
		_mp_search_list_append_group_title(list, (STR_MP_TRACKS), MP_SEARCH_TRACK_GROUP);

		ret = mp_media_info_list_create(&svc_handle, MP_TRACK_ALL, NULL, NULL, list->filter_str, 0, 0, track_count);
		if (ret != 0) {
			DEBUG_TRACE("Fail to get items");
			if (svc_handle) {
				mp_media_info_list_destroy(svc_handle);
			}
			goto END;
		}

		for (index = 0; index < track_count; index++) {
			mp_media_info_h item = NULL;
			item = mp_media_info_list_nth_item(svc_handle, index);
			if (!item) {
				continue;
			}

			mp_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mp_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_NONE;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc_track, (void *)item_data,
			                                        NULL, ELM_GENLIST_ITEM_NONE,
			                                        _mp_search_track_sel_cb, list);
			elm_object_item_data_set(item_data->it, item_data);
			char* title = NULL;
			mp_media_info_get_title(item,  &title);
			list->track_list = g_list_append(list->track_list, g_strdup(title));
		}

		if (list->track_handle) {
			mp_media_info_list_destroy(list->track_handle);
			list->track_handle = NULL;
		}
		list->track_handle = svc_handle;
		svc_handle = NULL;

	}
END:
	endfunc;
}

void _mp_search_list_destory_cb(void *thiz)
{
	startfunc;
	MpSearchList_t *list = thiz;
	MP_CHECK(list);

	IF_FREE(list->filter_str);
	if (list->itc_track) {
		elm_genlist_item_class_free(list->itc_track);
		list->itc_track = NULL;
	}
	if (list->itc_album) {
		elm_genlist_item_class_free(list->itc_album);
		list->itc_album = NULL;
	}
	if (list->itc_artist) {
		elm_genlist_item_class_free(list->itc_artist);
		list->itc_artist = NULL;
	}
	if (list->itc_group_title) {
		elm_genlist_item_class_free(list->itc_group_title);
		list->itc_group_title = NULL;
	}

	mp_media_info_group_list_destroy(list->artist_handle);
	mp_media_info_group_list_destroy(list->album_handle);
	mp_media_info_list_destroy(list->track_handle);

	g_list_free(list->artist_list);
	list->artist_list = NULL;
	g_list_free(list->album_list);
	list->album_list = NULL;
	g_list_free(list->track_list);
	list->track_list = NULL;

	free(list);
}

static void _mp_search_list_update(void *thiz)
{
	startfunc;
	MpSearchList_t *list = thiz;
	MP_CHECK(list);
	_mp_search_list_load_list(list);
}

MpSearchList_t * mp_search_list_create(Evas_Object *parent)
{
	startfunc;
	MP_CHECK_NULL(parent);

	MpSearchList_t *list = calloc(1, sizeof(MpSearchList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_GROUP);

	list->update = _mp_search_list_update;
	list->destory_cb = _mp_search_list_destory_cb;
	list->refresh = _mp_search_list_refresh_list;
	return list;
}

void mp_search_list_set_data(MpSearchList_t *list, ...)
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
		case MP_SEARCH_LIST_FILTER_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->filter_str);
			list->filter_str = g_strdup(val);
			DEBUG_TRACE("list->filter_str = %s", list->filter_str);

			break;
		}

		default:
			DEBUG_TRACE("Invalid arguments");
		}

	} while (field >= 0);

	va_end(var_args);
}

