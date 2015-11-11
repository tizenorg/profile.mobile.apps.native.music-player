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

#include "mc-search-list.h"
#include "music-chooser.h"
#include "mc-track-list.h"
#include "mc-common.h"

static void _mc_search_list_popup_to_bottom(struct app_data *ad)
{
	startfunc;
	MP_CHECK(ad);

	Elm_Object_Item *navi_item = elm_naviframe_bottom_item_get(ad->navi_bar);
	elm_naviframe_item_pop_to(navi_item);
}
static Evas_Object *
_mc_search_list_no_content_add(void *data)
{
	DEBUG_TRACE_FUNC();
	UgMpSearchList_t *list = (UgMpSearchList_t *)data;
	MP_CHECK_VAL(list, NULL);

	Evas_Object *no_contents = NULL;

	no_contents = elm_layout_add(list->box);
	evas_object_size_hint_weight_set(no_contents, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(no_contents, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_layout_file_set(no_contents, MC_EDJ_FILE, "no_result");

	if (list->genlist) {
		elm_box_unpack(list->box, list->genlist);
		evas_object_hide(list->genlist);
	}

	elm_box_pack_end(list->box, no_contents);
	evas_object_show(no_contents);

	return no_contents;
}

static int
_mc_search_list_set_sentinel(void *thiz, int count)
{
	UgMpSearchList_t *list = (UgMpSearchList_t *)thiz;
	MP_CHECK_VAL(list, -1);
	if (0 >= count && (list->filter_str && strlen(list->filter_str))) {
		ERROR_TRACE("no tracks");
		if (!list->no_content) {
			list->no_content = _mc_search_list_no_content_add(list);
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
_mc_search_track_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	//startfunc;
	mc_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h track = item_data->handle;
	mp_retvm_if(!track, NULL, "data is null");

	UgMpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(part);

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.slide.text.1")) {
		char *title = NULL;

		mp_media_info_get_title(track,  &title);

		mp_retv_if(!title, NULL);
		if (!strcmp(part, "elm.text.1")) {
			bool res = false;
			char *markup_name = (char *)mc_common_search_markup_keyword(title, list->filter_str, &res);
			return (res) ? g_strdup(markup_name) : elm_entry_utf8_to_markup(title);
		} else {
			return g_strdup(title);
		}
	} else if (!strcmp(part, "elm.text.2")) {
		char *artist = NULL;

		mp_media_info_get_artist(track, &artist);
		mp_retv_if(!artist, NULL);
		return g_strdup(artist);
	}
	return NULL;
}

Evas_Object *
_mc_search_track_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	//startfunc;
	mc_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h track = item_data->handle;
	mp_retvm_if(!track, NULL, "data is null");

	if (!strcmp(part, "elm.icon")) {
		char *thumbpath = NULL;
		Evas_Object *icon;

		mp_media_info_get_thumbnail_path(track, &thumbpath);
		icon = mc_common_create_thumb_icon(obj, thumbpath, MC_LIST_ICON_SIZE, MC_LIST_ICON_SIZE);
		return icon;
	}
	return NULL;
}


static void
_mc_search_track_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	char *uri = NULL;

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);

	UgMpSearchList_t *list = (UgMpSearchList_t *)data;
	MP_CHECK(list);
	MP_CHECK(list->ad);

	mc_list_item_data_t *item = (mc_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	mp_media_info_get_file_path(item->handle, &uri);

	_mc_search_list_popup_to_bottom(list->ad);

	mc_track_list_set_uri_selected(list->ad, uri);

	return;
}

static char *
_mc_search_album_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	//startfunc;
	char *name = NULL;
	int ret = 0;

	mc_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h svc_item = item_data->handle;
	mp_retv_if(svc_item == NULL, NULL);

	UgMpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(part);

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.slide.text.1")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}

		if (!strcmp(part, "elm.text.1")) {
			bool res = false;
			char *markup_name = (char *)mc_common_search_markup_keyword(name, list->filter_str, &res);
			return (res) ? g_strdup(markup_name) : elm_entry_utf8_to_markup(name);
		} else {
			return g_strdup(name);
		}

	} else if (!strcmp(part, "elm.text.2")) {
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
_mc_search_album_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	//startfunc;
	Evas_Object *icon = NULL;

	mc_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h svc_item = item_data->handle;
	mp_retv_if(svc_item == NULL, NULL);
	MP_CHECK_NULL(part);

	if (!strcmp(part, "elm.icon")) {
		char *thumb_name = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
		icon = mc_common_create_thumb_icon(obj, thumb_name, MC_LIST_ICON_SIZE, MC_LIST_ICON_SIZE);
	}

	return icon;
}

static void
_mc_search_album_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	//MpSearchList_t *list = (MpSearchList_t *)data;
	DEBUG_TRACE("");

	char *name = NULL;
	char *thumbnail = NULL;
	int playlist_id = 0;
	int ret = 0;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);
	elm_genlist_item_selected_set(gli, FALSE);

	DEBUG_TRACE("");
	mc_list_item_data_t *gli_data = elm_object_item_data_get(gli);
	MP_CHECK(gli_data);

	UgMpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK(list);
	MP_CHECK(list->ad);

	_mc_search_list_popup_to_bottom(list->ad);

	if (gli_data->handle) {
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		DEBUG_TRACE("thumbnail=%s", thumbnail);
		mp_retm_if(ret != 0, "Fail to get value");
		mp_retm_if(name == NULL, "Fail to get value");

		mc_common_push_track_view_by_group_name(list->ad, MP_TRACK_BY_ALBUM, name, playlist_id, NULL);
	}
}

static char *
_mc_search_artist_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	//startfunc;
	char *name = NULL;
	int ret = 0;

	mc_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h svc_item = item_data->handle;
	mp_retv_if(svc_item == NULL, NULL);

	UgMpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);

	if (!strcmp(part, "elm.text")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}

		bool res = false;
		char *markup_name = (char *)mc_common_search_markup_keyword(name, list->filter_str, &res);
		return (res) ? g_strdup(markup_name) : elm_entry_utf8_to_markup(name);
	}
	return NULL;
}

Evas_Object *
_mc_search_artist_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	//startfunc;
	Evas_Object *icon = NULL;

	mc_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);

	mp_media_info_h svc_item = item_data->handle;
	mp_retv_if(svc_item == NULL, NULL);

	if (!strcmp(part, "elm.icon")) {
		char *thumb_name = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
		icon = mc_common_create_thumb_icon(obj, thumb_name, MC_LIST_ICON_SIZE, MC_LIST_ICON_SIZE);
	}

	return icon;
}

static void
_mc_search_artist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE("");
	char *name = NULL;
	int playlist_id = 0;

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);
	elm_genlist_item_selected_set(gli, FALSE);

	DEBUG_TRACE("");
	mc_list_item_data_t *gli_data = elm_object_item_data_get(gli);
	MP_CHECK(gli_data);

	UgMpSearchList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK(list);
	MP_CHECK(list->ad);

	_mc_search_list_popup_to_bottom(list->ad);

	if (gli_data->handle) {
		mp_media_info_group_get_main_info(gli_data->handle, &name);
		mp_retm_if(name == NULL, "Fail to get value");

		mc_common_push_track_view_by_group_name(list->ad, MP_TRACK_BY_ARTIST, name, playlist_id, NULL);
	}
}

char *
_mc_search_list_gl_label_get_title(void *data, Evas_Object * obj, const char *part)
{
	//startfunc;
	char *text = NULL;

	if (!strcmp(part, "elm.text")) {
		text = GET_STR(data);
		return g_strdup(text);
	}
	return NULL;
}

static void
_mc_search_list_item_del(void *data, Evas_Object * obj)
{
	startfunc;
	mc_list_item_data_t *item_data = data;
	MP_CHECK(item_data);
	//mp_language_mgr_unregister_genlist_item(item_data->it);
	free(item_data);
}

static void _mc_search_list_set_itc(void *thiz)
{
	UgMpSearchList_t *list = thiz;
	MP_CHECK(list);
	if (!list->itc_track) {
		list->itc_track = elm_genlist_item_class_new();
		MP_CHECK(list->itc_track);
		list->itc_track->item_style = "music/2text.1icon.tb";
		list->itc_track->func.text_get = _mc_search_track_list_label_get;
		list->itc_track->func.content_get = _mc_search_track_list_icon_get;
		list->itc_track->func.del = _mc_search_list_item_del;
	}
	if (!list->itc_album) {
		list->itc_album = elm_genlist_item_class_new();
		MP_CHECK(list->itc_album);
		list->itc_album->item_style = "music/2text.1icon.tb";
		list->itc_album->func.text_get = _mc_search_album_list_label_get;
		list->itc_album->func.content_get = _mc_search_album_list_icon_get;
		list->itc_album->func.del = _mc_search_list_item_del;
	}
	if (!list->itc_artist) {
		list->itc_artist = elm_genlist_item_class_new();
		MP_CHECK(list->itc_artist);
		list->itc_artist->item_style = "music/1text.1icon.2.tb";
		list->itc_artist->func.text_get = _mc_search_artist_list_label_get;
		list->itc_artist->func.content_get = _mc_search_artist_list_icon_get;
		list->itc_artist->func.del = _mc_search_list_item_del;
	}
	if (!list->itc_group_title) {
		list->itc_group_title = elm_genlist_item_class_new();
		MP_CHECK(list->itc_group_title);
		list->itc_group_title->item_style = "music/groupindex";
		list->itc_group_title->func.text_get = _mc_search_list_gl_label_get_title;
	}
}

static void
_mc_search_list_append_group_title(void * data, char *text_ID)
{
	startfunc;
	UgMpSearchList_t *list = (UgMpSearchList_t *)data;
	MP_CHECK(list);
	list->search_group_git =
	    elm_genlist_item_append(list->genlist, list->itc_group_title, text_ID, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
	elm_genlist_item_select_mode_set(list->search_group_git, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

static void _mc_search_list_load_list(void *thiz)
{
	startfunc;
	UgMpSearchList_t *list = (UgMpSearchList_t *)thiz;
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

	if (_mc_search_list_set_sentinel(list, count)) {
		goto END;
	}


	/*create new genlist*/
	if (list->genlist != NULL) {
		evas_object_del(list->genlist);
		list->genlist = NULL;
	}

	list->genlist = mc_widget_genlist_create(list->box);
	elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);

	evas_object_data_set(list->genlist, "list_handle", (void *)list);

	_mc_search_list_set_itc(list);
	if (artist_count) {
		DEBUG_TRACE("append artist list items");
		_mc_search_list_append_group_title(list, ("IDS_MUSIC_TAB4_ARTISTS"));
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

			mc_list_item_data_t *item_data = calloc(1, sizeof(mc_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_BY_ARTIST;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc_artist, (void *)item_data,
			                                        list->search_group_git, ELM_GENLIST_ITEM_NONE,
			                                        _mc_search_artist_select_cb, list);
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
		_mc_search_list_append_group_title(list, ("IDS_MUSIC_TAB4_ALBUMS"));

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

			mc_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mc_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_BY_ALBUM;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc_album, (void *)item_data,
			                                        list->search_group_git, ELM_GENLIST_ITEM_NONE,
			                                        _mc_search_album_select_cb, list);

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
		_mc_search_list_append_group_title(list, (STR_MP_TRACKS));

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

			mc_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mc_list_item_data_t));
			mp_assert(item_data);
			item_data->handle = item;
			item_data->group_type = MP_GROUP_NONE;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc_track, (void *)item_data,
			                                        list->search_group_git, ELM_GENLIST_ITEM_NONE,
			                                        _mc_search_track_sel_cb, list);
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

void _mc_search_list_destory_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	startfunc;
	UgMpSearchList_t *list = data;
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

	free(list);
}

void mc_search_list_update(void *thiz)
{
	startfunc;
	UgMpSearchList_t *list = thiz;
	MP_CHECK(list);
	_mc_search_list_load_list(list);
}


UgMpSearchList_t * mc_search_list_create(Evas_Object *parent, struct app_data *ad)
{
	startfunc;
	MP_CHECK_NULL(parent);

	UgMpSearchList_t *list = calloc(1, sizeof(UgMpSearchList_t));
	MP_CHECK_NULL(list);

	list->ad = ad;

	list->layout = mc_common_load_edj(parent, MC_EDJ_FILE, "list_layout");
	if (list->layout == NULL) {
		IF_FREE(list);
		return NULL;
	}

	list->box = elm_box_add(list->layout);
	if (list->box == NULL) {
		IF_FREE(list);
		return NULL;
	}

	evas_object_size_hint_weight_set(list->box, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(list->box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_part_content_set(list->layout, "list_content", list->box);
	evas_object_show(list->box);

	evas_object_event_callback_add(list->layout, EVAS_CALLBACK_FREE, _mc_search_list_destory_cb, list);

	return list;
}

void mc_search_list_set_data(UgMpSearchList_t *list, ...)
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
		case MC_SEARCH_LIST_FILTER_STR: {
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

