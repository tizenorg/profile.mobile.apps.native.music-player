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

#include "mp-all-list.h"
#include "mp-util.h"
#include "mp-common.h"
#include "mp-playlist-detail-view.h"
#include "mp-setting-ctrl.h"
#include "mp-shortcut.h"
#include "mp-widget.h"
#include "mp-album-detail-view.h"
#include "mp-artist-detail-view.h"
#include "mp-all-view.h"
#include "mp-play.h"
#include "mp-player-view.h"
#include "mp-menu.h"

#define INITIAL_LOAD_COUNT 4
#define MP_GRID_ITEMS_IN_ROW 3
#define MP_SHUFFLE_IMG_SIZE 90

enum {
	MP_ALL_LIST_SHORTCUT,
	MP_ALL_LIST_TABBAR,
	MP_ALL_LIST_NOCONTENT,
	MP_ALL_LIST_SEPERATOR,
};

static void _append_shortcut(MpAllList_t *list);
static void _append_tabbar(MpAllList_t *list);
static void _mp_all_list_append_album_items(MpAllList_t *list, int count);

static void _mp_all_list_clear_list(MpAllList_t *list)
{
	startfunc;
	MP_CHECK(list);

	Elm_Object_Item *it = NULL, *next = NULL;
	next = it = elm_genlist_item_next_get(list->tabbar_it);;

	while (next) {
		next = elm_genlist_item_next_get(it);
		elm_object_item_del(it);
		it = next;
	}

	if (list->shuffle_it) {
		list->shuffle_it = NULL;
	}
	/*
		//destroy gengrid
		if (list->gengrid) {
			evas_object_unref(list->gengrid);
			list->gengrid = NULL;
		}
		*/
}

static void _mp_all_list_destory_cb(void *thiz)
{
	eventfunc;
	MpAllList_t *list = thiz;
	MP_CHECK(list);

	if (list->FwMgr) {
		mp_floating_widget_mgr_destroy(list->FwMgr);
		list->FwMgr = NULL;
	}

	elm_genlist_item_class_free(list->itc);
	list->itc = NULL;
	elm_genlist_item_class_free(list->itc_icon);
	list->itc_icon = NULL;
	elm_gengrid_item_class_free(list->gengrid_itc);
	list->gengrid_itc = NULL;
	elm_genlist_item_class_free(list->itc_shuffle);
	list->itc_shuffle = NULL;

	mp_media_info_list_destroy(list->track_list[0]);
	mp_media_info_list_destroy(list->track_list[1]);

	mp_media_info_group_list_destroy(list->group_list);
	list->group_list = NULL;

	mp_media_info_group_list_destroy(list->playlists_user);
	list->playlists_user = NULL;

	mp_media_info_group_list_destroy(list->playlists_auto);
	list->playlists_auto = NULL;

	mp_ecore_timer_del(list->load_timer);

	/*evas_object_unref(list->tabbar_layout);
	evas_object_unref(list->gengrid);*/

	free(list);
}

static mp_track_type_e _mp_track_list_get_track_type(void *thiz)
{
	return MP_TRACK_ALL;
}

static char *
_mp_all_list_songs_label_get(void *data, Evas_Object * obj, const char *part)
{
	/*startfunc;*/
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = (mp_media_info_h)(item->handle);
	mp_retvm_if(!track, NULL, "data is null");

	if (!strcmp(part, "elm.text.1")) {
		char *title = NULL;
		PROFILE_IN("_mp_all_list_songs_label_get:title");
		mp_media_info_get_title(track,  &title);
		char *markup = NULL;
		if (title) {
			markup = elm_entry_utf8_to_markup(title);
		}
		PROFILE_OUT("_mp_all_list_songs_label_get:title");
		return markup;
	} else if (!strcmp(part, "elm.text.2")) {
		char *artist = NULL;

		mp_media_info_get_artist(track, &artist);
		mp_retv_if(!artist, NULL);
		return g_strdup(artist);
	}
	return NULL;
}


Evas_Object *
_mp_all_list_songs_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = item->handle;
	mp_retvm_if(!track, NULL, "data is null");

	if (!strcmp(part, "elm.icon")) {
		char *thumbpath = NULL;
		PROFILE_IN("_mp_all_list_songs_icon_get");
		mp_media_info_get_thumbnail_path(track, &thumbpath);
#ifdef MP_FEATURE_PERSONAL_PAGE
		char *filepath = NULL;
		mp_media_info_get_file_path(track, &filepath);

		Evas_Object *icon = NULL;
		if (mp_util_is_in_personal_page(filepath)) {
			icon = mp_widget_lock_icon_create(obj, (const char *)thumbpath);
		} else {
			icon = mp_util_create_thumb_icon(obj, (const char *)thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		}

		PROFILE_OUT("_mp_all_list_songs_icon_get");
		return icon;
#else
		Evas_Object *icon = NULL;
		icon = mp_util_create_thumb_icon(obj, thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		PROFILE_OUT("_mp_all_list_songs_icon_get");
		return icon;
#endif
	}

	return NULL;
}


static void
_mp_all_list_songs_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	PROFILE_IN("mp_common_play_track_list");
	mp_common_play_track_list(item, obj);
	PROFILE_OUT("mp_common_play_track_list");

	return;
}

static void _mp_all_list_append_songs_item(MpAllList_t *list, mp_media_list_h svc_handle, int count)
{

	int index = 0;

	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		item = mp_media_info_list_nth_item(svc_handle, index);
		mp_list_item_data_t *item_data;
#ifdef MP_FEATURE_CLOUD
		if (list->cloud_view_type != MP_TRACK_LIST_VIEW_ALL) {
			mp_storage_type_e storage;
			int ret = mp_media_info_get_storage_type(item, &storage);
			if (ret != 0) {
				DEBUG_TRACE("Fail to mp_media_info_get_title, ret[%d], index[%d]", ret, index);
				goto END;
			}
			if (storage == MP_STORAGE_CLOUD) {
				if (list->cloud_view_type == MP_TRACK_LIST_VIEW_LOCAL) {
					continue;
				}
			} else {
				if (list->cloud_view_type == MP_TRACK_LIST_VIEW_CLOUD) {
					continue;
				}
			}
		}
#endif
		/* check DRM FL */
		if (mp_list_get_edit((MpList_t *)list) && mp_list_get_edit_type((MpList_t*)list) == MP_LIST_EDIT_TYPE_SHARE) {
			char *file_name = NULL;
			mp_media_info_get_file_path(item,  &file_name);
		}

#ifdef MP_FEATURE_PERSONAL_PAGE
		char *path = NULL;
		mp_media_info_get_file_path(item, &path);

		if (mp_util_is_in_personal_page((const char *)path)) {
			if (list->personal_page_status == FALSE) {
				continue;
			}
		}
#endif
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->index = index;
		/*item_data->group_type = MP_GROUP_NONE;*/

		char *title = NULL;
		mp_media_info_get_title(item_data->handle, &title);

		item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
		                                        ELM_GENLIST_ITEM_NONE, _mp_all_list_songs_sel_cb, list);
	}
#ifdef MP_FEATURE_CLOUD
END:
#endif
	endfunc;

}

static void
_mp_all_list_songs_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static Eina_Bool
_mp_all_list_track_lazy_load(void *thiz)
{
	startfunc;
	int count = 0, res = 0;
	MpAllList_t *list = thiz;
	mp_media_list_h svc_handle = NULL;
	MP_CHECK_FALSE(list);

	res = mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	MP_CHECK_FALSE(res == 0);

	count = count - INITIAL_LOAD_COUNT;

	mp_media_info_list_create(&svc_handle, MP_TRACK_ALL, NULL, NULL, NULL, 0, INITIAL_LOAD_COUNT, count);
	_mp_all_list_append_songs_item(list, svc_handle, count);

	if (list->track_list[1]) {
		mp_media_info_list_destroy(list->track_list[1]);
	}
	list->track_list[1] = svc_handle;

	list->load_timer = NULL;
	return EINA_FALSE;
}

static void
_mp_all_list_selected_item_data_get(void *thiz, GList **selected)
{
	startfunc;
	MpAllList_t *list = thiz;
	GList *sel_list = NULL;

	if (!list->genlist) {
		goto END;
	}

	Elm_Object_Item *item = elm_genlist_item_next_get(list->tabbar_it);
	mp_list_item_data_t *gl_item = NULL;

	if (!item) {
		goto END;
	}

	while (item) {
		if (mp_list_item_select_mode_get(item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			gl_item = elm_object_item_data_get(item);
			if (gl_item && gl_item->checked) {
				sel_list = g_list_append(sel_list, gl_item);
			}
		}
		item = mp_list_item_next_get(item);
	}
END:
	if (selected) {
		*selected = sel_list;
	}
}

static unsigned int
_mp_all_list_get_select_count(void *thiz)
{
	startfunc;
	MpAllList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);
	unsigned int count = 0;
	Elm_Object_Item *item;
	mp_list_item_data_t *data = NULL;

	item = mp_list_item_next_get(list->tabbar_it);
	while (item) {
		data = elm_object_item_data_get(item);
		item = mp_list_item_next_get(item);
		if (data && data->checked) {
			count++;
		}
	}
	return count;
}

static char *
_mp_all_list_shuffle_text_get(void *data, Evas_Object *obj, const char *part)
{
	startfunc;
	char *result = NULL;

	if (!strcmp(part, "elm.text")) {
		MpAllList_t *list  = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);

		int count = 0;
		mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
		list->track_count = count;
		result = (list->track_count == 1) ? g_strdup(GET_STR(STR_MP_SHUFFLE_1_TRACK)) : g_strdup_printf(GET_STR(STR_MP_SHUFFLE_PD_TRACKS), list->track_count);
	}
	return result;
}

Evas_Object *
_mp_all_list_shuffle_icon_get(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.icon.2")) {
		Evas_Object *icon;
		icon = mp_util_create_image(obj, IMAGE_EDJ_NAME, MP_ICON_SHUFFLE, MP_LIST_SHUFFLE_ICON_SIZE, MP_LIST_SHUFFLE_ICON_SIZE);
		return icon;
	}
	return NULL;
}

static void
_mp_all_list_shuffle_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	mp_play_control_shuffle_set(NULL, true);
	mp_common_play_track_list(item, obj);

	return;
}

void _mp_all_list_append_shuffle_item(MpAllList_t *list)
{
	startfunc;
	MP_CHECK(list);

	if (list->tab_status != MP_TAB_SONGS) {
		return;
	}

	if (NULL == list->itc_shuffle) {
		list->itc_shuffle = elm_genlist_item_class_new();
		if (list->itc_shuffle) {
			list->itc_shuffle->item_style = "music/1text.2icon.3";/*"music/3text.1icon.2"*/
			list->itc_shuffle->func.text_get = _mp_all_list_shuffle_text_get;
			list->itc_shuffle->decorate_all_item_style = NULL;
			list->itc_shuffle->func.content_get = _mp_all_list_shuffle_icon_get;
			list->itc_shuffle->func.del = _mp_all_list_songs_item_del_cb;
		}
	}

	mp_list_item_data_t *item_data;
	item_data = calloc(1, sizeof(mp_list_item_data_t));
	MP_CHECK(item_data);
	item_data->item_type = MP_LIST_ITEM_TYPE_SHUFFLE;

	item_data->it = list->shuffle_it = elm_genlist_item_insert_after(list->genlist, list->itc_shuffle, item_data, NULL, list->tabbar_it,
	                                   ELM_GENLIST_ITEM_NONE, _mp_all_list_shuffle_cb, list);

	endfunc;
}

static void _mp_all_list_load_track_list(MpAllList_t *list)
{
	startfunc;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle = NULL;
	int count = 0;

	list->itc->item_style = "music/tracklist/2text.1icon.4";/*"music/3text.1icon.2"*/
	list->itc->decorate_all_item_style = "createlist/edit_default";/*"tracklist/edit_default"*/
	list->itc->func.text_get = _mp_all_list_songs_label_get;
	list->itc->func.content_get = _mp_all_list_songs_icon_get;
	list->itc->func.del = _mp_all_list_songs_item_del_cb;

	list->list_type = MP_LIST_TYPE_ALL;

	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	list->track_count = count;

	if (count) {
		_mp_all_list_append_shuffle_item(list);

		static int initial = 1;

		if (initial) {
			if (list->cloud_view_type == MP_TRACK_LIST_VIEW_ALL && count > INITIAL_LOAD_COUNT) {
				count = INITIAL_LOAD_COUNT;
				list->load_timer = ecore_timer_add(0.3, _mp_all_list_track_lazy_load, list);
			}
			initial = 0;
		}

		/*get data from DB*/
		PROFILE_IN("mp_media_info_list_create");
		mp_media_info_list_create(&svc_handle, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, count);
		PROFILE_OUT("mp_media_info_list_create");

		PROFILE_IN("_mp_all_list_append_songs_item");
		_mp_all_list_append_songs_item(list, svc_handle, count);
		PROFILE_OUT("_mp_all_list_append_songs_item");

		if (list->track_list[0]) {
			mp_media_info_list_destroy(list->track_list[0]);
		}
		list->track_list[0] = svc_handle;

		list->get_select_count = _mp_all_list_get_select_count;
	} else {
		Elm_Object_Item *it =
		    elm_genlist_item_append(list->genlist, list->itc_icon,
		                            (void *)MP_ALL_LIST_NOCONTENT, NULL,
		                            ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
}

static void
_mp_all_list_create_auto_playlist_detail_view(void *data, mp_list_item_data_t *item_data, char *name)
{
	MP_CHECK(item_data);

	mp_track_type_e type = MP_TRACK_ALL;
	if (!strcmp(STR_MP_MOST_PLAYED, name)) {
		type = MP_TRACK_BY_PLAYED_COUNT;
	} else if (!strcmp((STR_MP_RECENTLY_ADDED), name)) {
		type = MP_TRACK_BY_ADDED_TIME;
	} else if (!strcmp((STR_MP_RECENTLY_PLAYED), name)) {
		type = MP_TRACK_BY_PLAYED_TIME;
	} else if (!strcmp((STR_MP_FAVOURITES), name)) {
		type = MP_TRACK_BY_FAVORITE;
	} else {
		SECURE_ERROR("Invalid type: %s", name);
	}

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpPlaylistDetailView_t *view_plst_detail = mp_playlist_detail_view_create(view_manager->navi, type, name, mp_media_info_get_auto_playlist_id_by_name(name));
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_plst_detail, NULL);

	mp_view_update_options((MpView_t *)view_plst_detail);
	mp_view_set_title((MpView_t *)view_plst_detail, name);
}


static void
_mp_all_list_auto_playlist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	int ret = 0;
	char *name = NULL;
	mp_list_item_data_t *item_data = NULL;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);

	item_data = elm_object_item_data_get(gli);
	MP_CHECK(item_data);
	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");


	_mp_all_list_create_auto_playlist_detail_view(data, item_data, name);
}

static void
_mp_all_list_append_auto_playlists(void *thiz)
{
	int i;
	int playlist_state = 0;

	MpAllList_t *list = thiz;
	MP_CHECK(list);

	if (list->playlists_auto) {
		mp_media_info_group_list_destroy(list->playlists_auto);
	}

	mp_setting_playlist_get_state(&playlist_state);

	mp_media_info_group_list_create(&(list->playlists_auto), MP_GROUP_BY_SYS_PLAYLIST, NULL, NULL, 0, 0);
	for (i = 0; i < MP_SYS_PLST_COUNT; i++) {
		int enable = playlist_state & (1 << i);
		/*DEBUG_TRACE("index: %d, state: %d",i, enable);*/
		if (!enable) {
			continue;
		}

		mp_media_info_h item;
		item = mp_media_info_group_list_nth_item(list->playlists_auto, i);

		mp_list_item_data_t *item_data;
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->unregister_lang_mgr = true;

		list->auto_playlist_count++;
		item_data->it = elm_genlist_item_append(list->genlist, list->itc,
		                                        item_data, NULL,
		                                        ELM_GENLIST_ITEM_NONE, _mp_all_list_auto_playlist_select_cb,
		                                        list);
	}
}

static void
_mp_all_list_user_playlist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	int ret = 0;
	char *name = NULL;
	int p_id = 0;
	mp_list_item_data_t *item_data = NULL;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);
	if (elm_genlist_item_flip_get(gli)) {
		return;
	}

	item_data = elm_object_item_data_get(gli);
	MP_CHECK(item_data);

	ret = mp_media_info_group_get_playlist_id(item_data->handle, &p_id);
	mp_retm_if(ret != 0, "Fail to get value");

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");


	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpPlaylistDetailView_t *view_plst_detail = mp_playlist_detail_view_create(view_manager->navi, MP_TRACK_BY_PLAYLIST, name, p_id);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_plst_detail, NULL);

	mp_view_update_options((MpView_t *)view_plst_detail);
	mp_view_set_title((MpView_t *)view_plst_detail, name);
}

static void
_mp_all_list_append_user_playlists(void *thiz)
{
	startfunc;
	gint count = -1;
	gint index = 0;
	int ret = 0;

	MpAllList_t *list = (MpAllList_t *)thiz;
	mp_retm_if(!list, "plst is null");

	mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, NULL, NULL, &count);

	if (count < 0) {
		goto END;
	}

	ret = mp_media_info_group_list_create(&list->playlists_user, MP_GROUP_BY_PLAYLIST, NULL, NULL, 0, count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to get items");
		goto END;
	}

	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		char *title = NULL;

		item = mp_media_info_group_list_nth_item(list->playlists_user, index);
		if (!item) {
			DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
			goto END;
		}
		mp_media_info_group_get_main_info(item, &title);
		mp_list_item_data_t *item_data;
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		/*item_data->group_type = list->group_type;*/
		item_data->index = index;

		item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
		                                        ELM_GENLIST_ITEM_NONE, _mp_all_list_user_playlist_select_cb, list);

	}

END:
	endfunc;
}

static char *
_mp_all_list_playlist_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h plst_item = (item->handle);
	MP_CHECK_NULL(plst_item);

	int ret = 0;
	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.slide.text.1")) {

		char *name = NULL;
		ret = mp_media_info_group_get_main_info(plst_item, &name);
		mp_retvm_if(ret != 0, NULL, "Fail to get value");
		mp_retvm_if(name == NULL, NULL, "Fail to get value");

		if (!strcmp(part, "elm.text.1")) {
			return elm_entry_utf8_to_markup(GET_STR(name));
		} else {
			return g_strdup(GET_STR(name));
		}
	} else if (!strcmp(part, "elm.text.2")) {
		int count = -1;
		int plst_id = -1;

		/* TODO:  fix performance issue*/
		ret = mp_media_info_group_get_playlist_id(plst_item, &plst_id);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (plst_id < 0) {
			return NULL;
		}

		ret = mp_media_info_list_count(MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, plst_id, &count);
		mp_retvm_if(ret != 0, NULL, "Fail to get count");
		mp_retvm_if(count < 0, NULL, "Fail to get count");
		return g_strdup_printf("(%d)", count);
	}

	return NULL;
}

Evas_Object *
_mp_all_list_playlist_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h plst = (item->handle);
	MP_CHECK_NULL(plst);

	Evas_Object *eo = NULL;
	int ret = 0;

	int playlist_id = 0;
	char *thumb_path = NULL;

	mp_media_info_group_get_playlist_id(plst, &playlist_id);

	if (!strcmp(part, "elm.icon")) {
		if (!playlist_id) {
			char default_thumbnail[1024] = {0};
			char *shared_path = app_get_shared_resource_path();
			snprintf(default_thumbnail, 1024, "%s%s/%s", shared_path, "shared_images", DEFAULT_THUMBNAIL);
			free(shared_path);
			eo = mp_util_create_thumb_icon(obj, default_thumbnail, MP_LIST_ICON_SIZE,
			                               MP_LIST_ICON_SIZE);
		} else {
			ret = mp_media_info_playlist_get_thumbnail_path(plst, &thumb_path);
			mp_retvm_if(ret != 0, NULL, "Fail to get value");
			eo = mp_util_create_thumb_icon(obj, thumb_path, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		}
	}

	return eo;
}

static void
_mp_all_list_playlist_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);
	if (item_data->unregister_lang_mgr) {
		mp_language_mgr_unregister_genlist_item(item_data->it);
	}
	free(item_data);
}

static void _mp_all_list_load_playlists(MpAllList_t *list)
{
	startfunc;
	int count_user = 0, count_auto = 0, res = 0, i, playlist_state = 0;
	MP_CHECK(list);

	list->itc->item_style = "music/musiclist/2text.1icon";
	list->itc->decorate_all_item_style = "createlist/edit_default";
	list->itc->func.text_get = _mp_all_list_playlist_label_get;
	list->itc->func.content_get = _mp_all_list_playlist_icon_get;
	list->itc->func.del = _mp_all_list_playlist_item_del_cb;

	list->list_type = MP_LIST_TYPE_PLAYLIST;

	res = mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, NULL, NULL, &count_user);
	MP_CHECK(res == 0);

	if (!list->edit_mode) {
		mp_setting_playlist_get_state(&playlist_state);
		for (i = 0; i < MP_SYS_PLST_COUNT; i++) {
			if (playlist_state & (1 << i)) {
				count_auto++;
			}
		}
	}

	if (count_auto + count_user) {
		/* load list */
		list->auto_playlist_count = 0;
		if (count_auto) {
			_mp_all_list_append_auto_playlists(list);
		}
		if (count_user) {
			_mp_all_list_append_user_playlists(list);
		}

	} else {
		Elm_Object_Item *it =
		    elm_genlist_item_append(list->genlist, list->itc_icon,
		                            (void *)MP_ALL_LIST_NOCONTENT, NULL,
		                            ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

}

static char *
_mp_all_list_album_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *name = NULL;
	int ret = 0;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);

	mp_retv_if(svc_item == NULL, NULL);

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.slide.text.1") || !strcmp(part, "elm.text")) {
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

	} else if (!strcmp(part, "elm.text.2")) {
		ret = mp_media_info_group_get_sub_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}
		return g_strdup(name);
	} else if (!strcmp(part, "elm.text.3")) {
		int count = 0;
		mp_media_info_group_get_track_count(svc_item, &count);
		return g_strdup_printf("(%d)", count);
	}

	DEBUG_TRACE("Unusing part: %s", part);
	return NULL;
}


Evas_Object *
_mp_all_list_album_icon_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *icon = NULL;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	mp_retv_if(svc_item == NULL, NULL);

	bool landscape = mp_util_is_landscape();

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
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
		icon = mp_util_create_thumb_icon(obj, thumb_name, w, h);
	}

	return icon;
}

static void
_mp_all_list_album_grid_item_select_tts_double_action_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	eventfunc;
	mp_list_item_data_t *item_data = data;
	int ret = 0;
	char *name = NULL;
	char *artist = NULL;
	char *thumbnail = NULL;

	if (mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ALBUM_DETAIL)) {
		ERROR_TRACE("album detail view already exist..");
		return;
	}

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	ret = mp_media_info_group_get_sub_info(item_data->handle, &artist);
	mp_media_info_group_get_thumbnail_path(item_data->handle, &thumbnail);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");

	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpAlbumDetailView_t *view_album_detail = mp_album_detail_view_create(view_manager->navi, name, artist, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_album_detail, NULL);

	mp_view_update_options((MpView_t *)view_album_detail);
	mp_view_set_title((MpView_t *)view_album_detail, name);

}


static void
_mp_all_list_album_grid_item_select_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	eventfunc;
	mp_list_item_data_t *item_data = data;
	int ret = 0;
	char *name = NULL;
	char *artist = NULL;
	char *thumbnail = NULL;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	if (mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ALBUM_DETAIL)) {
		ERROR_TRACE("album detail view already exist..");
		return;
	}

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	ret = mp_media_info_group_get_sub_info(item_data->handle, &artist);
	mp_media_info_group_get_thumbnail_path(item_data->handle, &thumbnail);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");

	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpAlbumDetailView_t *view_album_detail = mp_album_detail_view_create(view_manager->navi, name, artist, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_album_detail, NULL);

	mp_view_update_options((MpView_t *)view_album_detail);
	mp_view_set_title((MpView_t *)view_album_detail, name);

}


Evas_Object *
_mp_album_list_grid_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *content = NULL;
	mp_grid_item_data_t *grid_data = data;
	int thumbnail_size = 0;

	MP_CHECK_NULL(grid_data);
	MP_CHECK_NULL(grid_data->item_data);

	bool landscape = mp_util_is_landscape();

	if (!landscape) {
		content = mp_common_load_edj(obj, MP_EDJ_NAME, "artist_album_grid_layout");
		evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_min_set(content, 0, 319 * elm_config_scale_get());
		thumbnail_size = MP_ALBUM_THUMB_ICON_SIZE;
	} else {
		content = mp_common_load_edj(obj, MP_EDJ_NAME, "album_grid_layout_ld");
		evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_min_set(content, 0, 322 * elm_config_scale_get());
		thumbnail_size = 230;
	}

	int i;
	for (i = 0; i < grid_data->item_count; i++) {
		if (!grid_data->item_data[i]) {
			break;
		}

		char *main_text = NULL, *sub_text = NULL;
		char *thumb_name = NULL;
		char *part = g_strdup_printf("item_%d", i);
		Evas_Object *sub_item = NULL;
		if (!landscape) {
			sub_item = mp_common_load_edj(content, MP_EDJ_NAME, "artist_album_grid_item");
		} else {
			sub_item = mp_common_load_edj(content, MP_EDJ_NAME, "album_grid_item_ld");
		}

		mp_media_info_group_get_thumbnail_path(grid_data->item_data[i]->handle, &thumb_name);
		mp_media_info_group_get_main_info(grid_data->item_data[i]->handle, &main_text);
		mp_media_info_group_get_sub_info(grid_data->item_data[i]->handle, &sub_text);

		elm_object_part_text_set(sub_item, "elm.text.1", main_text);
		elm_object_part_text_set(sub_item, "elm.text.2", sub_text);
		elm_object_signal_callback_add(sub_item, "clicked", "*", _mp_all_list_album_grid_item_select_cb, grid_data->item_data[i]);

		Evas_Object *icon = mp_util_create_thumb_icon(sub_item, thumb_name, thumbnail_size, thumbnail_size);
		elm_object_part_content_set(sub_item, "albumart", icon);

		elm_object_part_content_set(content, part, sub_item);
		evas_object_show(sub_item);
	}

	return content;
}

static void
_mp_all_list_group_item_del_cb(void *data, Evas_Object *obj)
{
	mp_list_item_data_t *item_data = data;
	IF_FREE(item_data);
}

static void
_mp_all_list_grid_item_del_cb(void *data, Evas_Object *obj)
{
	mp_grid_item_data_t *grid_data = data;
	MP_CHECK(grid_data);

	int i;
	for (i = 0; i < grid_data->item_count; i++) {
		IF_FREE(grid_data->item_data[i]);
	}
	IF_FREE(grid_data->item_data);

	free(grid_data);
}

static void
_mp_all_list_album_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpAllList_t *list = data;
	int ret = 0;
	int index = 0;/*(int)data;*/
	char *name = NULL;
	char *artist = NULL;
	char *title = NULL;
	char *thumbnail = NULL;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

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
	if (index >= 0) {
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		ret = mp_media_info_group_get_sub_info(gli_data->handle, &artist);
		mp_media_info_group_get_thumbnail_path(gli_data->handle, &thumbnail);
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

static void _mp_all_list_append_album_items(MpAllList_t *list, int count)
{
	startfunc;
	MP_CHECK(list);

	/*media-svc related*/
	gint index = 0;
	int ret = 0;
	bool landscape = mp_util_is_landscape();

	if (count < 0) {
		goto END;
	}

	if (list->group_list) {
		mp_media_info_group_list_destroy(list->group_list);
		list->group_list = NULL;
	}

	ret = mp_media_info_group_list_create(&list->group_list, MP_GROUP_BY_ALBUM, NULL, NULL, 0, count);

	if (ret != 0) {
		DEBUG_TRACE("Fail to get items");
		goto END;
	}

	for (index = 0; index < count;) {
		mp_media_info_h item = NULL;

		if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			mp_grid_item_data_t *grid_data = calloc(1, sizeof(mp_grid_item_data_t));
			MP_CHECK(grid_data);

			int item_count = 3;
			if (landscape) {
				item_count = 5;
			}

			grid_data->item_count = item_count;
			grid_data->item_data = calloc(item_count, sizeof(mp_list_item_data_t *));

			int j;
			for (j = 0; j < item_count; j++) {
				item = mp_media_info_group_list_nth_item(list->group_list, index);
				if (!item) {
					DEBUG_TRACE("No more items");
					index++;
					break;
				}

				mp_list_item_data_t *item_data;
				item_data = calloc(1, sizeof(mp_list_item_data_t));
				if (item_data) {
					item_data->handle = item;
					item_data->index = index;
					grid_data->item_data[j] = item_data;
				}
				index++;

			}
			DEBUG_TRACE("index: %d, count: %d", index, count);
			if (grid_data->item_data[0] == NULL) {
				IF_FREE(grid_data->item_data);
				IF_FREE(grid_data);
				goto END;
			}
			elm_genlist_item_append(list->genlist, list->itc, grid_data, NULL,
			                        ELM_GENLIST_ITEM_NONE, NULL, NULL);
		} else {
			item = mp_media_info_group_list_nth_item(list->group_list, index);
			if (!item) {
				WARN_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
				goto END;
			}
			mp_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mp_list_item_data_t));
			MP_CHECK(item_data);
			item_data->handle = item;
			item_data->index = index;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
			                                        ELM_GENLIST_ITEM_NONE, _mp_all_list_album_select_cb, (void *)list);
			index++;
		}

	}

END:
	endfunc;
}

static void _mp_all_list_load_album_list(MpAllList_t *list)
{
	startfunc;
	int count = 0, res = 0;
	MP_CHECK(list);

	res = mp_media_info_group_list_count(MP_GROUP_BY_ALBUM, NULL, NULL, &count);
	MP_CHECK(res == 0);

	list->list_type = MP_LIST_TYPE_GROUP;

	if (count) {
		if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			list->itc->item_style = "1icon/no_padding_line";
			list->itc->func.content_get = _mp_album_list_grid_get;
			list->itc->func.text_get = NULL;
			list->itc->func.del = _mp_all_list_grid_item_del_cb;
			/*
						if (!list->gengrid_itc) {
							list->gengrid_itc= elm_gengrid_item_class_new();
							MP_CHECK(list->gengrid_itc);
						}

						bool landscape = mp_util_is_landscape();
						DEBUG_TRACE("landscape: %d", landscape);

						if (landscape)
							list->gengrid_itc->item_style = "music/landscape/album_grid";
						else
							list->gengrid_itc->item_style = "music/album_grid";
						list->gengrid_itc->func.text_get = _mp_all_list_album_label_get;
						list->gengrid_itc->func.content_get = _mp_all_list_album_icon_get;
						list->gengrid_itc->func.del = _mp_all_list_group_item_del_cb;
			*/
			/*
						Elm_Object_Item* item= elm_genlist_item_append(list->genlist, list->itc, NULL, list->tabbar_it,
												    ELM_GENLIST_ITEM_NONE, NULL, NULL);
						elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			*/
			_mp_all_list_append_album_items(list, count);

		} else {
			list->itc->item_style = "music/3text.1icon.2";
			list->itc->func.text_get = _mp_all_list_album_label_get;
			list->itc->func.content_get = _mp_all_list_album_icon_get;
			list->itc->func.del = _mp_all_list_group_item_del_cb;

			/* load list */
			_mp_all_list_append_album_items(list, count);
		}

	} else {
		Elm_Object_Item *it =
		    elm_genlist_item_append(list->genlist, list->itc_icon,
		                            (void *)MP_ALL_LIST_NOCONTENT, NULL,
		                            ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

}

static void
_mp_all_list_artist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	MpAllList_t *list = data;
	eventfunc;
	int ret = 0;
	int index = 0;/*(int)data;*/
	char *name = NULL;
	char *thumbnail = NULL;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

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
	if (index >= 0) {
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		mp_media_info_group_get_thumbnail_path(gli_data->handle, &thumbnail);
		mp_retm_if(ret != 0, "Fail to get value");
		mp_retm_if(name == NULL, "Fail to get value");
	}

	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpArtistDetailView_t *view_artist_detail = mp_artist_detail_view_create(view_manager->navi, name, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_artist_detail, NULL);

	mp_view_update_options((MpView_t *)view_artist_detail);
	mp_view_set_title((MpView_t *)view_artist_detail, name);

}

static void _mp_all_list_append_artist_items(void *thiz, int count)
{
	MpAllList_t *list = thiz;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle;
	gint index = 0;
	int ret = 0;
	bool landscape = mp_util_is_landscape();

	DEBUG_TRACE("count: %d", count);

	if (count < 0) {
		goto END;
	}

	ret = mp_media_info_group_list_create(&svc_handle, MP_GROUP_BY_ARTIST, NULL, NULL, 0, count);

	if (ret != 0) {
		DEBUG_TRACE("Fail to get items");
		goto END;
	}

	if (list->group_list) {
		mp_media_info_group_list_destroy(list->group_list);
	}
	list->group_list = svc_handle;

	for (index = 0; index < count;) {
		mp_media_info_h item = NULL;

		if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			mp_grid_item_data_t *grid_data = calloc(1, sizeof(mp_grid_item_data_t));
			MP_CHECK(grid_data);

			int item_count = 3;

			if (landscape) {
				item_count = 7;
			}

			grid_data->item_count = item_count;
			grid_data->item_data = calloc(item_count, sizeof(mp_list_item_data_t *));

			int j;
			for (j = 0; j < item_count; j++) {
				item = mp_media_info_group_list_nth_item(list->group_list, index);
				if (!item) {
					DEBUG_TRACE("No more items");
					index++;
					break;
				}

				mp_list_item_data_t *item_data;
				item_data = calloc(1, sizeof(mp_list_item_data_t));
				if (item_data) {
					item_data->handle = item;
					item_data->index = index;
					grid_data->item_data[j] = item_data;
				}
				index++;

			}
			DEBUG_TRACE("index: %d, count: %d", index, count);
			if (grid_data->item_data[0] == NULL) {
				IF_FREE(grid_data->item_data);
				IF_FREE(grid_data);
				goto END;
			}
			elm_genlist_item_append(list->genlist, list->itc, grid_data, NULL,
			                        ELM_GENLIST_ITEM_NONE, NULL, NULL);
		} else {
			item = mp_media_info_group_list_nth_item(svc_handle, index);
			if (!item) {
				DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
				goto END;
			}

			mp_list_item_data_t *item_data;
			item_data = calloc(1, sizeof(mp_list_item_data_t));
			MP_CHECK(item_data);
			item_data->handle = item;
			item_data->index = index;
			item_data->display_mode = list->display_mode;

			item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
			                                        ELM_GENLIST_ITEM_NONE, _mp_all_list_artist_select_cb, (void *)list);
			index++;
		}

	}


END:
	endfunc;
}

static void
_mp_all_list_artist_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	mp_list_item_data_t *item = data;
	MP_CHECK(item);
	MP_CHECK(item->it);
	item->artist_album_page++;
	mp_debug("next artist album page = %d", item->artist_album_page);
	elm_genlist_item_fields_update(item->it, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
}

static Evas_Object *
_mp_artist_list_album_icon_get(Evas_Object *obj, mp_list_item_data_t *item)
{
	MP_CHECK_NULL(item);
	MP_CHECK_NULL(item->handle);

	bool landscape = mp_util_is_landscape();
	Evas_Object *layout = NULL;

	if (landscape) {
		layout = mp_common_load_edj(obj, MP_EDJ_NAME, "landscape_artist_list_default");
	} else {
		layout = mp_common_load_edj(obj, MP_EDJ_NAME, "artist_list_default");
	}
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	if (landscape) {
		evas_object_size_hint_min_set(layout, 0, 236 * elm_config_scale_get());
	} else {
		evas_object_size_hint_min_set(layout, 0, 232 * elm_config_scale_get());
	}

	char *artist_name = NULL;
	mp_media_info_group_get_main_info(item->handle, &artist_name);
	elm_object_part_text_set(layout, "elm.text.1", artist_name);

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
	mp_util_domain_translatable_part_text_set(layout, "elm.text.2", sub_text);
	SAFE_FREE(sub_text);

	if (landscape) {
		int offset = item->artist_album_page * 7;
		if (offset >= album_count) {
			item->artist_album_page = 0;
			offset = 0;
		}

		int i;
		int diff = album_count - offset;
		int count = (diff > 7) ? 7 : diff;
		for (i = offset; i < (offset + count) ; i++) {
			char *path = album_thumbs[i];
			Evas_Object *thumb = mp_util_create_thumb_icon(obj, path, 166 * elm_config_scale_get(), 166 * elm_config_scale_get());
			char *part = g_strdup_printf("elm.icon.%d", (i - offset + 1));
			elm_object_part_content_set(layout, part, thumb);
			IF_FREE(part);
		}
	} else {
		int offset = item->artist_album_page * 4;
		if (offset >= album_count) {
			item->artist_album_page = 0;
			offset = 0;
		}

		int i;
		int diff = album_count - offset;
		int count = (diff > 4) ? 4 : diff;
		for (i = offset; i < (offset + count) ; i++) {
			char *path = album_thumbs[i];
			Evas_Object *thumb = mp_util_create_thumb_icon(obj, path, 162 * elm_config_scale_get(), 162 * elm_config_scale_get());
			char *part = g_strdup_printf("elm.icon.%d", (i - offset + 1));
			elm_object_part_content_set(layout, part, thumb);
			IF_FREE(part);
		}
	}
	evas_object_show(layout);
	return layout;
}

Evas_Object *
_mp_all_list_artist_icon_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *icon = NULL;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	mp_retv_if(svc_item == NULL, NULL);

	bool landscape = mp_util_is_landscape();

	if (item->display_mode == MP_LIST_DISPLAY_MODE_NORMAL) {
		if (!g_strcmp0(part, "elm.icon")) {
			return _mp_artist_list_album_icon_get(obj, item);
		} else if (!g_strcmp0(part, "elm.icon.more")) {
			int album_count = 0;
			mp_media_info_group_get_album_thumnail_paths(svc_item, NULL, &album_count);
			if (landscape) {
				if (album_count <= 7) {
					return NULL;
				}
			} else {
				if (album_count <= 4) {
					return NULL;
				}
			}

			Evas_Object *btn = elm_button_add(obj);
			elm_object_style_set(btn, "music/artist/more");
			evas_object_propagate_events_set(btn, EINA_FALSE);
			evas_object_smart_callback_add(btn, "clicked", _mp_all_list_artist_more_btn_cb, item);
			return btn;
		}
	}

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {
		char *thumb_name = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
		int w, h;
		if (item->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			w = MP_ARTIST_THUMB_ICON_SIZE * elm_config_scale_get();
		} else {
			w = MP_LIST_ICON_SIZE;
		}
		h = w;
		icon = mp_util_create_thumb_icon(obj, thumb_name, w, h);
		return icon;
	}
	return icon;
}

static char *
_mp_all_list_artist_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *name = NULL;
	int ret = 0;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	mp_retv_if(svc_item == NULL, NULL);

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.slide.text.1") || !strcmp(part, "elm.text")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}
		return g_strdup(name);
	} else if (!strcmp(part, "elm.text.2")) {
		int count = -1;
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");


		ret = mp_media_info_list_count(MP_TRACK_BY_ARTIST, name, NULL, NULL, 0, &count);
		mp_retvm_if(ret != 0, NULL, "Fail to get count");
		mp_retvm_if(count < 0, NULL, "Fail to get count");
		return g_strdup_printf("(%d)", count);
	}

	return NULL;
}

static void
_mp_all_list_artist_grid_item_select_tts_double_action_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	eventfunc;
	mp_list_item_data_t *item_data = data;
	int ret = 0;
	char *name = NULL;
	char *thumbnail = NULL;

	if (mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ARTIST_DETAIL)) {
		ERROR_TRACE("album detail view already exist..");
		return;
	}

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_media_info_group_get_thumbnail_path(item_data->handle, &thumbnail);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");

	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpArtistDetailView_t *view_artist_detail = mp_artist_detail_view_create(view_manager->navi, name, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_artist_detail, NULL);

	mp_view_update_options((MpView_t *)view_artist_detail);
	mp_view_set_title((MpView_t *)view_artist_detail, name);

}

static void
_mp_all_list_artist_grid_item_select_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	eventfunc;
	mp_list_item_data_t *item_data = data;
	int ret = 0;
	char *name = NULL;
	char *thumbnail = NULL;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	if (mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ARTIST_DETAIL)) {
		ERROR_TRACE("album detail view already exist..");
		return;
	}

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_media_info_group_get_thumbnail_path(item_data->handle, &thumbnail);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");

	/* create the view of album detail */
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpArtistDetailView_t *view_artist_detail = mp_artist_detail_view_create(view_manager->navi, name, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_artist_detail, NULL);

	mp_view_update_options((MpView_t *)view_artist_detail);
	mp_view_set_title((MpView_t *)view_artist_detail, name);

}

Evas_Object *
_mp_artist_list_grid_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *content = NULL;
	mp_grid_item_data_t *grid_data = data;
	int thumbnail_size = 0;

	MP_CHECK_NULL(grid_data);
	MP_CHECK_NULL(grid_data->item_data);

	bool landscape = mp_util_is_landscape();

	if (!landscape) {
		content = mp_common_load_edj(obj, MP_EDJ_NAME, "artist_album_grid_layout");
		evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_min_set(content, 0, 319 * elm_config_scale_get());
		thumbnail_size = MP_ALBUM_THUMB_ICON_SIZE;
	} else {
		content = mp_common_load_edj(obj, MP_EDJ_NAME, "artist_album_grid_layout_ld");
		evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_min_set(content, 0, 230 * elm_config_scale_get());
		thumbnail_size = 166;
	}

	int i;
	for (i = 0; i < grid_data->item_count; i++) {
		if (!grid_data->item_data[i]) {
			break;
		}

		char *main_text = NULL, *sub_text = NULL;
		char *thumb_name = NULL;
		char *part = g_strdup_printf("item_%d", i);
		Evas_Object *sub_item = NULL;

		if (!landscape) {
			sub_item = mp_common_load_edj(content, MP_EDJ_NAME, "artist_album_grid_item");
		} else {
			sub_item = mp_common_load_edj(content, MP_EDJ_NAME, "artist_album_grid_item_ld");
		}

		mp_media_info_group_get_thumbnail_path(grid_data->item_data[i]->handle, &thumb_name);
		mp_media_info_group_get_main_info(grid_data->item_data[i]->handle, &main_text);
		mp_media_info_group_get_sub_info(grid_data->item_data[i]->handle, &sub_text);

		elm_object_part_text_set(sub_item, "elm.text.1", main_text);
		elm_object_part_text_set(sub_item, "elm.text.2", sub_text);
		elm_object_signal_callback_add(sub_item, "clicked", "*", _mp_all_list_artist_grid_item_select_cb, grid_data->item_data[i]);

		Evas_Object *icon = mp_util_create_thumb_icon(sub_item, thumb_name, thumbnail_size, thumbnail_size);
		elm_object_part_content_set(sub_item, "albumart", icon);

		elm_object_part_content_set(content, part, sub_item);
		evas_object_show(sub_item);
	}

	return content;
}

static void mp_all_list_playall_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpAllList_t *list = data;
	MP_CHECK(list);

	int count = 0;
	char *type_str = NULL;
	int ret = 0;
	int playlist_id = -1;
	char *playlist_name = NULL;

	mp_popup_destroy(ad);

	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;
	mp_media_list_h svc_handle = NULL;/*= handle;*/

	mp_list_selected_item_data_get((MpList_t *)list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}

	if (!ad->playlist_mgr) {
		mp_common_create_playlist_mgr();
	}

	mp_playlist_mgr_clear(ad->playlist_mgr);

	mp_group_type_e group_type = mp_list_get_group_type((MpList_t *)list);

	DEBUG_TRACE("group_type: %d", group_type);
	if (group_type == MP_GROUP_BY_PLAYLIST) {
		/* get playlist name */
		ret = mp_media_info_group_get_main_info(item_data->handle, &playlist_name);
		ret = mp_media_info_group_get_playlist_id(item_data->handle, &playlist_id);
		mp_retm_if(playlist_name == NULL, "Fail to get playlist_name");

		/* create playlist */
		mp_playlist_list_set_playlist(ad->playlist_mgr, playlist_id);
		ad->paused_by_user = FALSE;
	} else if (group_type == MP_GROUP_BY_ALBUM) {
		/* get playlist name */
		ret = mp_media_info_group_get_main_info(item_data->handle, &type_str);

		mp_media_info_list_count(MP_TRACK_BY_ALBUM, type_str, NULL, NULL, 0, &count);
		mp_media_info_list_create(&svc_handle,
		                          MP_TRACK_BY_ALBUM, type_str, NULL, NULL, 0, 0, count);

		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);

	} else if (group_type == MP_GROUP_BY_ARTIST) {
		/* get playlist name */
		ret = mp_media_info_group_get_main_info(item_data->handle, &type_str);

		mp_media_info_list_count(MP_TRACK_BY_ARTIST, type_str, NULL, NULL, 0, &count);
		mp_media_info_list_create(&svc_handle,
		                          MP_TRACK_BY_ARTIST, type_str, NULL, NULL, 0, 0, count);

		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);
	}

	ret = mp_play_new_file(ad, TRUE);
	if (ret) {
		ERROR_TRACE("Error: mp_play_new_file..");
		if (ret == MP_PLAY_ERROR_NO_SONGS) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_NO_SONGS));
		}
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
		}
#endif
		goto END;
	}

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpPlayerView_t *player_view = mp_player_view_create(view_manager->navi, MP_PLAYER_NORMAL, false);
	mp_view_mgr_push_view(view_manager, (MpView_t *)player_view, NULL);
	mp_view_update((MpView_t *)player_view);
	mp_view_update_options((MpView_t *)player_view);


END:

	if (svc_handle) {
		mp_media_info_list_destroy(svc_handle);
	}

	if (sel_list) {
		g_list_free(sel_list);
		sel_list = NULL;
	}

	endfunc;
}

#if 0
static char *_mp_media_info_get_live_auto_playlist_thumbnail_by_name(const char *name)
{
	MP_CHECK_VAL(name, NULL);

	char *thumb_path = (char *)malloc(1024*sizeof(char));
	char *shared_path = app_get_shared_resource_path();

	if (!g_strcmp0(name, STR_MP_FAVOURITES)) {
		snprintf(thumb_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_THUMBNAIL_QUICK_LIST);
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_PLAYED)) {
		snprintf(thumb_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_THUMBNAIL_RECENTLY_PLAYED);
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_ADDED)) {
		snprintf(thumb_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_THUMBNAIL_RECENTLY_ADDED);
	} else if (!g_strcmp0(name, STR_MP_MOST_PLAYED)) {
		snprintf(thumb_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_THUMBNAIL_MOST_PLAYED);
	}
	free(shared_path);

	return thumb_path;
}

static char *_mp_media_info_get_live_auto_playlist_icon_by_name(const char *name)
{
	MP_CHECK_VAL(name, NULL);

	char *icon_path = (char *)malloc(1024*sizeof(char));
	char *shared_path = app_get_shared_resource_path();

	if (!g_strcmp0(name, STR_MP_FAVOURITES)) {
		snprintf(icon_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_ICON_QUICK_LIST);
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_PLAYED)) {
		snprintf(icon_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_ICON_RECENTLY_PLAYED);
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_ADDED)) {
		snprintf(icon_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_ICON_RECENTLY_ADDED);
	} else if (!g_strcmp0(name, STR_MP_MOST_PLAYED)) {
		snprintf(icon_path, 1024, "%s%s/%s", shared_path, "shared_images", LIVE_ICON_MOST_PLAYED);
	}

	return icon_path;
}
#endif

#ifdef MP_FEATURE_ADD_TO_HOME
static void mp_all_add_to_home_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpAllList_t *list = data;
	MP_CHECK(list);

	GList *sel_list = NULL;
	char *name = NULL;
	char *thumbnail = NULL;
	int ret = 0;
	int p_id = -1;
	mp_list_item_data_t *item_data = NULL;

	mp_popup_destroy(ad);

	mp_list_selected_item_data_get((MpList_t *)list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");
	mp_media_info_group_get_thumbnail_path(item_data->handle, &thumbnail);

	int type = 0;
	const char *extra1 = NULL;
	const char *extra2 = NULL;
	if (list->tab_status == MP_TAB_PLAYLISTS) {
		ret = mp_media_info_group_get_playlist_id(item_data->handle, &p_id);
		mp_retm_if(ret != 0, "Fail to get value");
		DEBUG_TRACE("p_id: %d", p_id);
		if (p_id < 0) {
			type = MP_ADD_TO_HOME_SHORTCUT_TYPE_SYS_PLAYLIST;
			extra1 = _mp_media_info_get_live_auto_playlist_thumbnail_by_name(name);
			extra2 = _mp_media_info_get_live_auto_playlist_icon_by_name(name);
		} else {
			type = MP_ADD_TO_HOME_SHORTCUT_TYPE_USER_PLAYLIST;
			name = (void *)p_id;
		}
		mp_menu_add_to_home(type, name, (void *)extra1, (void *)extra2);
	} else if (list->tab_status == MP_TAB_ALBUMS) {
		mp_menu_add_to_home(MP_ADD_TO_HOME_SHORTCUT_TYPE_ALBUM, name, thumbnail, NULL);
	}

	if (sel_list) {
		g_list_free(sel_list);
		sel_list = NULL;
	}
}
#endif

static void
_mp_all_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpAllList_t *list = (MpAllList_t *)data;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	bool pop_del_status = true;
	bool pop_add_to_home_status = false;
	bool popup_group_status = false;
	int pop_item_count = 5;
	int playlist_state = 0;
	char *title = NULL;
	Elm_Object_Item *temp = NULL;
	Evas_Object *popup = NULL;
	mp_list_item_data_t *item_data = NULL;

	if ((list->tab_status == MP_TAB_ALBUMS && list->album_disp_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) ||
	        (list->tab_status == MP_TAB_ARTISTS && list->artist_disp_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) ||
	        list->scroll_drag_status || list->shuffle_it == item) {
		return;
	}

	if (list->tab_status == MP_TAB_SONGS) {
		temp = elm_genlist_item_next_get(list->shuffle_it);
	} else {
		temp = elm_genlist_item_next_get(list->tabbar_it);
	}

	while (temp) {
		item_data = elm_object_item_data_get(temp);
		MP_CHECK(item_data);
		item_data->checked = false;
		temp = elm_genlist_item_next_get(temp);
	}

	item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);

	item_data->checked = true;

	if (list->tab_status == MP_TAB_PLAYLISTS) {
		int item_index = elm_genlist_item_index_get(item);
		int playlist_auto_count = 0;
		int i = 0;
		pop_item_count = 2;
		popup_group_status = true;
		pop_add_to_home_status = true;

		mp_media_info_group_get_main_info(item_data->handle, &title);
		mp_setting_playlist_get_state(&playlist_state);
		for (i = 0; i < MP_SYS_PLST_COUNT; i++) {
			if (playlist_state & (1 << i)) {
				playlist_auto_count++;
			}
		}
		if (item_index <= (playlist_auto_count + 1)) {
			pop_item_count = 1;
			pop_del_status = false;
			title = GET_SYS_STR(title);
		}
	} else if (list->tab_status == MP_TAB_SONGS) {
		mp_media_info_get_title(item_data->handle, &title);
	} else if (list->tab_status == MP_TAB_ALBUMS) {
		pop_item_count = 4;
		popup_group_status = true;
		pop_add_to_home_status = true;
		mp_media_info_group_get_main_info(item_data->handle, &title);
	} else if (list->tab_status == MP_TAB_ARTISTS) {
		pop_item_count = 3;
		popup_group_status = true;
		mp_media_info_group_get_main_info(item_data->handle, &title);
	}

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

	char *up_title = g_strdup(title);

	elm_object_part_text_set(popup, "title,text", up_title);
	IF_FREE(up_title);

	if (!popup_group_status)
		mp_genlist_popup_item_append(popup, STR_MP_SET_AS, NULL, NULL, NULL,
		                             mp_common_list_set_as_cb, list);
	if (popup_group_status)
		mp_genlist_popup_item_append(popup, STR_MP_PLAY_ALL, NULL, NULL, NULL,
		                             mp_all_list_playall_cb, list);
	if (list->list_type != MP_LIST_TYPE_PLAYLIST)
		mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PLAYLIST, NULL, NULL, NULL,
		                             mp_common_list_add_to_playlist_cb, list);
	if (!popup_group_status) {
		bool favourite = false;
		char *str = NULL;
		Evas_Smart_Cb cb = NULL;

		mp_media_info_get_favorite(item_data->handle, &favourite);
		if (favourite) {
			str = STR_MP_UNFAVOURITES;
			cb = mp_common_list_unfavorite_cb;
		} else {
			str = STR_MP_FAVOURITES;
			cb = mp_common_list_add_to_favorite_cb;
		}

		mp_genlist_popup_item_append(popup, str, NULL, NULL, NULL, cb, list);
	}
	if (pop_del_status)
		mp_genlist_popup_item_append(popup, STR_MP_DELETE, NULL, NULL, NULL,
		                             mp_common_list_delete_cb, list);
	if (!popup_group_status)
		mp_genlist_popup_item_append(popup, STR_MP_POPUP_MORE_INFO, NULL, NULL, NULL,
		                             mp_common_list_more_info_cb, list);

	MP_GENLIST_ITEM_LONG_PRESSED(obj, popup, event_info);

}


static void
_mp_all_list_artist_list_item_highlighted_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpAllList_t *list = data;
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
_mp_all_list_artist_list_item_unhighlighted_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpAllList_t *list = data;
	MP_CHECK(list);
	MP_CHECK(!MP_LIST_OBJ_IS_GENGRID(obj));

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	Evas_Object *layout = elm_object_item_part_content_get(item, "elm.icon");
	if (layout) {
		elm_object_signal_emit(layout, "elm,state,unselected", "elm");
	}
}

static void _mp_all_list_load_artist_list(MpAllList_t *list)
{
	startfunc;
	int count = 0, res = 0;
	MP_CHECK(list);

	res = mp_media_info_group_list_count(MP_GROUP_BY_ARTIST, NULL, NULL, &count);
	MP_CHECK(res == 0);

	list->list_type = MP_LIST_TYPE_GROUP;

	if (count) {
		if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
			list->itc->item_style = "1icon/no_padding_line";
			list->itc->func.content_get = _mp_artist_list_grid_get;
			list->itc->func.text_get = NULL;
			list->itc->func.del = _mp_all_list_grid_item_del_cb;

			if (!list->gengrid_itc) {
				list->gengrid_itc = elm_gengrid_item_class_new();
				MP_CHECK(list->gengrid_itc);
			}

			list->gengrid_itc->item_style = "default_gridtext";
			list->gengrid_itc->func.text_get = _mp_all_list_album_label_get;
			list->gengrid_itc->func.content_get = _mp_all_list_album_icon_get;
			list->gengrid_itc->func.del = _mp_all_list_group_item_del_cb;
			/*
						Elm_Object_Item* item= elm_genlist_item_append(list->genlist, list->itc, NULL, list->tabbar_it,
												    ELM_GENLIST_ITEM_NONE, NULL, NULL);
						elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
			*/
			_mp_all_list_append_artist_items(list, count);
		} else {
			list->itc->item_style = "music/artist/2icon";
			list->itc->func.text_get = _mp_all_list_artist_label_get;
			list->itc->func.content_get = _mp_all_list_artist_icon_get;
			list->itc->func.del = _mp_all_list_group_item_del_cb;

			/* load list */
			_mp_all_list_append_artist_items(list, count);

			evas_object_smart_callback_add(list->genlist, "highlighted", _mp_all_list_artist_list_item_highlighted_cb, list);
			evas_object_smart_callback_add(list->genlist, "unhighlighted", _mp_all_list_artist_list_item_unhighlighted_cb, list);
		}

	} else {
		DEBUG_TRACE("count is 0");
		Elm_Object_Item *it =
		    elm_genlist_item_append(list->genlist, list->itc_icon,
		                            (void *)MP_ALL_LIST_NOCONTENT, NULL,
		                            ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

}

static mp_group_type_e _mp_all_list_get_group_type(void *thiz)
{
	MpAllList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);

	if (list->tab_status == MP_TAB_ALBUMS) {
		return MP_GROUP_BY_ALBUM;
	} else if (list->tab_status == MP_TAB_ARTISTS) {
		return MP_GROUP_BY_ARTIST;
	} else if (list->tab_status == MP_TAB_PLAYLISTS) {
		return MP_GROUP_BY_PLAYLIST;
	}

	return MP_GROUP_NONE;
}

static void _mp_all_list_update(void *thiz)
{
	startfunc;
	MP_CHECK(thiz);
	MpAllList_t *list = thiz;
	MP_CHECK(list->itc);

	_mp_all_list_clear_list(list);

	mp_ecore_timer_del(list->load_timer);

	mp_list_hide_fast_scroll((MpList_t *)list);

	/*clear media_info handles*/
	if (list->track_list[0]) {
		mp_media_info_list_destroy(list->track_list[0]);
		list->track_list[0] = NULL;
	}

	if (list->track_list[1]) {
		mp_media_info_list_destroy(list->track_list[1]);
		list->track_list[1] = NULL;
	}

	/*del timer*/
	mp_ecore_timer_del(list->load_timer);
	list->display_mode_changable = false;

	if (list->tab_status == MP_TAB_SONGS) {
		PROFILE_IN("_mp_all_list_load_list");
		list->display_mode = MP_LIST_DISPLAY_MODE_NORMAL;
		_mp_all_list_load_track_list(thiz);
		PROFILE_OUT("_mp_all_list_load_list");
		/*mp_list_show_fast_scroll((MpList_t *)list);*/
	} else if (list->tab_status == MP_TAB_PLAYLISTS) {
		list->display_mode = MP_LIST_DISPLAY_MODE_NORMAL;
		_mp_all_list_load_playlists(thiz);
	} else if (list->tab_status == MP_TAB_ALBUMS) {
		list->display_mode = list->album_disp_mode;
		_mp_all_list_load_album_list(thiz);

		list->display_mode_changable = true;
	} else if (list->tab_status == MP_TAB_ARTISTS) {
		list->display_mode = list->artist_disp_mode;
		_mp_all_list_load_artist_list(thiz);

		list->display_mode_changable = true;
	} else {
		ERROR_TRACE("Invalid type: tab_status[%d]", list->tab_status);
	}

	endfunc;
}

void mp_all_list_update_genlist(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpAllList_t *list = thiz;
	MP_CHECK(list->genlist);

	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);

	if (list->tab_status == MP_TAB_SONGS && count <= 0) {
		mp_list_update(thiz);
	} else {
		elm_genlist_realized_items_update(list->genlist);
	}
}

void mp_all_list_update_data(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpAllList_t *list = thiz;
	MP_CHECK(list->itc);
	MP_CHECK(list->genlist);

	MpListDisplayMode_e current = mp_list_get_display_mode((MpList_t *)list);
	if (MP_LIST_DISPLAY_MODE_THUMBNAIL == current || MP_TAB_ARTISTS == list->tab_status) {
		mp_all_list_set_display_mode(list, current);
		Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
		elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	} else {
		elm_genlist_realized_items_update(list->genlist);
	}

	endfunc;
}

static const char *_get_label(void *thiz, void *event_info)
{
	MpAllList_t *list = thiz;
	MP_CHECK_NULL(list);
	char *title = NULL;

	MP_CHECK_NULL(list->tab_status == MP_TAB_SONGS);
	MP_CHECK_NULL(elm_genlist_item_select_mode_get(event_info) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	MP_CHECK_NULL(elm_genlist_item_select_mode_get(event_info) != ELM_OBJECT_SELECT_MODE_NONE);

	mp_list_item_data_t *track =  elm_object_item_data_get(event_info);
	MP_CHECK_NULL(track);

	mp_media_info_get_title(track->handle, &title);
	return title;
}


static unsigned int
_mp_all_list_get_editable_count(void *thiz, MpListEditType_e type)
{
	MpAllList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);
	int count = 0;
	Elm_Object_Item *item = NULL;

	/*if (list->display_mode == MP_LIST_DISPLAY_MODE_NORMAL)*/
	{
		if (list->tab_status == MP_TAB_SONGS) {
			item = mp_list_item_next_get(list->shuffle_it);
		} else {
			item = mp_list_item_next_get(list->tabbar_it);
		}
		while (item) {
			item = mp_list_item_next_get(item);
			count++;
		}

		if (list->tab_status == MP_TAB_PLAYLISTS) {
			count = count - list->auto_playlist_count;
		}
	}

	return count;
}

static void _tab_change_cb(void *data, Evas_Object * obj, void *event_info)
{
	MpAllList_t *list = (MpAllList_t *)data;
	Elm_Object_Item *it, *it2;
	eventfunc;

	if (list->first_change) {
		list->first_change = false;
		return;
	}

	it = elm_toolbar_selected_item_get(obj);
	mp_retm_if(it == NULL, "tab item is NULL");

	it2 = elm_toolbar_first_item_get(obj);

	int i = 0;
	for (i = 0; i < MP_TAB_MAX; i++) {
		if (it == it2) {
			break;
		}
		it2 = elm_toolbar_item_next_get(it2);
	}

	if (list->tab_status == i) {
		return;
	}

	list->tab_status = i;

	_mp_all_list_update(list);

	if (mp_floating_widget_mgr_visible_get(list->FwMgr, 0)) {
		/*mp_floating_widget_mgr_widget_deleted(list->FwMgr, 0);*/
		elm_genlist_item_show(list->tabbar_it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
		elm_genlist_item_update(list->tabbar_it);
	}
}


static Evas_Object *_create_tabbar(Evas_Object *parent, MpAllList_t *list)
{
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(list);

	PROFILE_IN("mp_widget_create_tabbar");
	Evas_Object *obj = mp_widget_create_tabbar(parent);
	PROFILE_OUT("mp_widget_create_tabbar");
	list->first_change = true;

	PROFILE_IN("mp_util_toolbar_item_append");
	mp_util_toolbar_item_append(obj, NULL, (STR_MP_TRACKS), _tab_change_cb, list);
	PROFILE_OUT("mp_util_toolbar_item_append");
	PROFILE_IN("mp_util_toolbar_item_append");
	mp_util_toolbar_item_append(obj, NULL, (STR_MP_PLAYLISTS), _tab_change_cb, list);
	PROFILE_OUT("mp_util_toolbar_item_append");
	PROFILE_IN("mp_util_toolbar_item_append");
	mp_util_toolbar_item_append(obj, NULL, (STR_MP_ALBUMS), _tab_change_cb, list);
	PROFILE_OUT("mp_util_toolbar_item_append");
	PROFILE_IN("mp_util_toolbar_item_append");
	mp_util_toolbar_item_append(obj, NULL, (STR_MP_ARTISTS), _tab_change_cb, list);
	PROFILE_OUT("mp_util_toolbar_item_append");

	PROFILE_IN("elm_toolbar_item_selected_set");
	elm_toolbar_item_selected_set(mp_util_toolbar_nth_item(obj, list->tab_status), EINA_TRUE);
	PROFILE_OUT("elm_toolbar_item_selected_set");

	evas_object_show(obj);

	return obj;
}

static void
_mp_all_list_content_shortcut_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MpAllList_t *list = data;
	MP_CHECK(list);
	list->shortcut_layout = NULL;
}

Evas_Object *
_mp_all_list_content_get(void *data, Evas_Object * obj, const char *part)
{
	int type = (int)data;
	Evas_Object *content = NULL;

	MpAllList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);
	PROFILE_IN("_mp_all_list_content_get");
	if (type == MP_ALL_LIST_SHORTCUT) {
		PROFILE_IN("mp_shortcut_add");
		static int calc_size = true;
		if (calc_size) {
			calc_size = false;
			content = evas_object_rectangle_add(evas_object_evas_get(obj));
			evas_object_size_hint_min_set(content, 1, mp_shortcut_get_height());
		} else {
			content = mp_shortcut_add(obj, list->shortcut_index);
			list->shortcut_layout = content;
			evas_object_event_callback_add(list->shortcut_layout, EVAS_CALLBACK_DEL, _mp_all_list_content_shortcut_del_cb, list);
		}
		PROFILE_OUT("mp_shortcut_add");
	} else if (type == MP_ALL_LIST_TABBAR) {
		PROFILE_IN("_create_tabbar");
		static int calc_size = true;
		if (calc_size) {
			calc_size = false;
			content = evas_object_rectangle_add(evas_object_evas_get(obj));
			/*evas_object_resize(content, 0, 75*elm_config_scale_get());*/
			evas_object_size_hint_min_set(content, 1, 75 * elm_config_scale_get());
		} else {
			content = _create_tabbar(obj, list);
			list->tabbar_layout = content;
		}
		PROFILE_OUT("_create_tabbar");
	} else if (type == MP_ALL_LIST_SEPERATOR) {
		content = elm_layout_add(obj);
		evas_object_size_hint_min_set(content, 720 * elm_config_scale_get(), 2560 * elm_config_scale_get());
	} else if (type == MP_ALL_LIST_NOCONTENT) {
		content = mp_widget_create_no_contents(obj, MP_NOCONTENT_TRACKS, NULL, list);
		evas_object_size_hint_min_set(content, 720 * elm_config_scale_get(), 660 * elm_config_scale_get());
	}
	PROFILE_OUT("_mp_all_list_content_get");
	return content;
}

void _floating_tabbar_cb(bool show, int x, int y, int w, int h, void *data)
{
	DEBUG_TRACE("x: %d, y: %d, w: %d, h: %d, show: %d", x, y, w, h, show);
	MpAllList_t *list = data;
	if (show) {
		if (!list->floating_tabbar) {
			Evas_Object *obj = _create_tabbar(list->genlist, list);
			list->floating_tabbar = obj;
			elm_object_part_content_set(list->parent, "tabbar", obj);
		}

		edje_object_signal_emit(_EDJ(list->parent), "show,tabbar", "*");
		elm_toolbar_item_selected_set(mp_util_toolbar_nth_item(list->floating_tabbar, list->tab_status), EINA_TRUE);
		if (list->tab_status == MP_TAB_SONGS) {
			mp_list_show_fast_scroll((MpList_t *)list);
		}
	} else {
		edje_object_signal_emit(_EDJ(list->parent), "hide,tabbar", "*");
		mp_list_hide_fast_scroll((MpList_t *)list);
	}

}

static void _realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpAllList_t *list = data;
	Elm_Object_Item *item = event_info;
	MP_CHECK(list);

	DEBUG_TRACE("list->list_type=%d", list->list_type);
	Eina_List *genlist_items = NULL;
	if (list->tabbar_it == event_info) {
		list->tabbar_realized = true;
		elm_object_item_access_register(list->tabbar_it);
		Evas_Object *content;

		content = elm_object_item_part_content_get(item, "elm.icon");
		genlist_items = eina_list_append(genlist_items, content);
		elm_object_item_access_order_set(item, genlist_items);

		mp_floating_widget_callback_add(list->FwMgr, 75 * elm_config_scale_get(), 1, 0, _floating_tabbar_cb, list);
	} else if (list->shortcut_it == event_info) {
		elm_object_item_access_register(list->shortcut_it);
		Evas_Object *content;

		content = elm_object_item_part_content_get(item, "elm.icon");
		genlist_items = eina_list_append(genlist_items, content);
		elm_object_item_access_order_set(item, genlist_items);
	} else if ((list->tab_status == MP_TAB_ALBUMS || list->tab_status == MP_TAB_ARTISTS) &&
	           (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL)) {
		elm_object_item_access_register(item);
		Evas_Object *content;

		content = elm_object_item_part_content_get(item, "elm.icon");
		genlist_items = eina_list_append(genlist_items, content);
		elm_object_item_access_order_set(item, genlist_items);
	} else if (list->list_type == MP_LIST_TYPE_ALL) {
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);
		MP_CHECK(item);

		bool set_color = false;
		if ((ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED)) {
			mp_list_item_data_t *item_data = (mp_list_item_data_t *)elm_object_item_data_get(item);
			MP_CHECK(item_data);
			MP_CHECK(item_data->handle);

			mp_plst_item *cur = mp_playlist_mgr_get_current(ad->playlist_mgr);
			MP_CHECK(cur);

			char *media_id = NULL;
			mp_media_info_get_media_id(item_data->handle, &media_id);

			if (!g_strcmp0(cur->uid, media_id)) {
				set_color = true;
				DEBUG_TRACE("media_id=%s: Change color", media_id);
			}
		}

		if (set_color) {
			elm_object_item_signal_emit(item, "elm.text.1.color", "elm.text.1");
		} else {
			elm_object_item_signal_emit(item, "elm.text.1.default", "elm.text.1");
		}
	}
}

static void _mp_all_list_create_genlist(Evas_Object *parent, MpAllList_t *list)
{
#ifdef MP_CREATE_FAKE_IMAGE
	return ;
#endif
	PROFILE_IN("elm_genlist_add");
	list->genlist = mp_widget_genlist_create(list->box);
	elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	evas_object_smart_callback_add(list->genlist, "realized", _realized_cb, list);
	evas_object_smart_callback_add(list->genlist, "longpressed", _mp_all_list_item_longpressed_cb, list);
	evas_object_smart_callback_add(list->genlist, "scroll,drag,start", list->drag_start_cb, list);
	evas_object_smart_callback_add(list->genlist, "scroll,drag,stop", list->drag_stop_cb, list);

	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);

	evas_object_data_set(list->genlist, "list_data", list);

	list->itc = elm_genlist_item_class_new();

	list->itc_icon = elm_genlist_item_class_new();
	if (list->itc_icon) {
		list->itc_icon->item_style = "music/1icon/no_padding";/*"music/3text.1icon.2"*/
		list->itc_icon->func.content_get = _mp_all_list_content_get;
		list->itc_icon->func.del = NULL;
	}

	/*create floating widget manager*/
	list->FwMgr = mp_floating_widget_mgr_create(list->genlist);

	PROFILE_OUT("elm_genlist_add");
}

static void _append_shortcut(MpAllList_t *list)
{
#ifdef MP_CREATE_FAKE_IMAGE
	return ;
#endif
	startfunc;
	list->shortcut_it = elm_genlist_item_prepend(list->genlist, list->itc_icon, (void *)MP_ALL_LIST_SHORTCUT, NULL,
	                    ELM_GENLIST_ITEM_NONE, NULL, list);
	elm_genlist_item_select_mode_set(list->shortcut_it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

static void _append_tabbar(MpAllList_t *list)
{
#ifdef MP_CREATE_FAKE_IMAGE
	return ;
#endif
	startfunc;
	list->tabbar_it = elm_genlist_item_append(list->genlist, list->itc_icon, (void *)MP_ALL_LIST_TABBAR, NULL,
	                  ELM_GENLIST_ITEM_NONE, NULL, list);
	elm_genlist_item_select_mode_set(list->tabbar_it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
}

MpAllList_t *mp_all_list_create(Evas_Object *parent, MpTab_e init_tab)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpAllList_t *list = calloc(1, sizeof(MpAllList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_ALL);

	list->parent = parent;

	list->tab_status = init_tab; /* for shortcut */
	list->shortcut_index = 0;

#ifdef MP_FEATURE_PERSONAL_PAGE
	list->personal_page_status = mp_util_is_personal_page_on();
#endif
	_mp_all_list_create_genlist(parent, list);
	_append_shortcut(list);
	_append_tabbar(list);

	list->album_disp_mode = MP_LIST_DISPLAY_MODE_THUMBNAIL;
	list->artist_disp_mode = MP_LIST_DISPLAY_MODE_NORMAL;

	list->update = _mp_all_list_update;
	list->destory_cb = _mp_all_list_destory_cb;
	list->get_track_type = _mp_track_list_get_track_type;
	list->get_group_type = _mp_all_list_get_group_type;

	list->get_label = _get_label;

	list->get_count = _mp_all_list_get_editable_count;
	/*list->longpressed_cb = _mp_all_list_item_longpressed_cb;*/
	list->selected_item_data_get = _mp_all_list_selected_item_data_get;

	return list;
}

void mp_all_list_update_favourite(MpAllList_t *list)
{
	MP_CHECK(list);

	int count = 0;
	mp_media_info_list_count(MP_TRACK_BY_FAVORITE, NULL, NULL, NULL, 0, &count);
	if ((count == 0) || (count > 0 && ((count - 1) == list->shortcut_index))) {
		list->shortcut_index = 0;
	} else {
		list->shortcut_index = list->shortcut_index + 1;
	}

	mp_shortcut_update_cache(list->shortcut_layout, list->shortcut_index);
	elm_genlist_item_update(list->shortcut_it);
}

void mp_all_list_rotate_shortcut(MpAllList_t *list)
{
	MP_CHECK(list);

	elm_object_item_del(list->shortcut_it);
	list->shortcut_it = NULL;

	_append_shortcut(list);
}

void mp_all_list_update_shortcut(MpAllList_t *list)
{
	MP_CHECK(list);

	if (list->scroll_drag_status) {
		return;
	}

	mp_shortcut_update_cache(list->shortcut_layout, list->shortcut_index);
	elm_genlist_item_update(list->shortcut_it);
}

MpTab_e mp_all_list_get_selected_tab(MpAllList_t *list)
{
	MP_CHECK_VAL(list, MP_TAB_SONGS);
	return list->tab_status;
}

void mp_all_list_select_tab(MpAllList_t *list, MpTab_e tab)
{
	EVENT_TRACE("Select tab : %d", tab);
	MP_CHECK(list);
	list->tab_status = tab;
	elm_genlist_item_update(list->tabbar_it);

	_mp_all_list_update(list);

	if (list->floating_tabbar) {
		elm_toolbar_item_selected_set(mp_util_toolbar_nth_item(list->floating_tabbar, tab), EINA_TRUE);
	}

	endfunc;
}

void mp_all_list_set_display_mode(MpAllList_t *list, MpListDisplayMode_e mode)
{
	startfunc;
	MP_CHECK(list);

	if (list->tab_status == MP_TAB_ALBUMS) {
		list->album_disp_mode = mode;
	} else {
		list->artist_disp_mode = mode;
	}

	list->display_mode = mode;

	_mp_all_list_update(list);
}

