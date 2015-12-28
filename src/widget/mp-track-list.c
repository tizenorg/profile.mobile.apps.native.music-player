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
#include "mp-create-playlist-view.h"
#include "mp-ctxpopup.h"
#include "mp-popup.h"
#include "mp-util.h"
#include "mp-common.h"
#include "mp-widget.h"
#include "mp-play.h"
#include "mp-edit-callback.h"
#include "mp-player-mgr.h"
#include <media_content.h>
#include <player.h>

#define INITIAL_LOAD_COUNT 8
#define ALBUMART_INDEX_SIZE		(720  * elm_config_scale_get())
#ifdef MP_FEATURE_LANDSCAPE
#define LANDSCAPE_ALBUMART_INDEX_SIZE		(256  * elm_config_scale_get())
#endif

typedef struct {
	char *name;
	char *path;
	int count;
} albumart_info_s;

static char *
_mp_track_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = (mp_media_info_h)(item->handle);
	mp_retvm_if(!track, NULL, "data is null");

	if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.sub")) {
		MpTrackList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);

		struct appdata *ad = mp_util_get_appdata();
		mp_track_info_t* current = ad->current_track_info;


		char *title = NULL;

		if (!strcmp(part, "elm.text")) {

			if (list->track_type == MP_TRACK_BY_FOLDER) {
				mp_media_info_get_display_name(track, &title);
			} else {
				mp_media_info_get_title(track,  &title);
			}
		} else {
			mp_media_info_get_artist(track, &title);
		}

		char *markup = NULL;
		char *uri = NULL;
		static char result[DEF_STR_LEN + 1] = { 0, };
		mp_media_info_get_file_path(track, &uri);
		mp_retv_if(!uri, NULL);

		bool match = false;
		if (current && !g_strcmp0(current->uri, uri) && list->edit_mode == 0) {
			match = true;
		}

		if (match && list->track_type == MP_TRACK_BY_PLAYLIST) {
			int member_id = 0;
			mp_media_info_get_playlist_member_id(track, &member_id);
			if (member_id != current->playlist_member_id) {
				match = false;
			}
		}

		if (match) {
			char *markup_title = elm_entry_utf8_to_markup(title);

			int r, g, b, a;
			//Apply RGB equivalent of color
			r = 21;
			g = 108;
			b = 148;
			a = 255;
			memset(result, 0x00, DEF_STR_LEN + 1);
			snprintf(result, DEF_STR_LEN,
			         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, markup_title);
			IF_FREE(markup_title);

			return g_strdup(result);
		} else {
			markup = elm_entry_utf8_to_markup(title);
		}

		return markup;
	}
	/*else if (!strcmp(part, "elm.text.3") )
	{
		int duration;
		char time[16] = "";

		mp_media_info_get_duration(track, &duration);

		mp_util_format_duration(time, duration);
		time[15] = '\0';
		return g_strdup(time);
	}*/
	return NULL;
}

Evas_Object *
_mp_track_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = item->handle;
	mp_retvm_if(!track, NULL, "data is null");
	MP_CHECK_NULL(obj);
	MpTrackList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	Evas_Object *content = NULL;
	content = elm_layout_add(obj);

	//get player status
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	mp_track_info_t* current = ad->current_track_info;
	char *uri = NULL;
	mp_media_info_get_file_path(track, &uri);

	Evas_Object *part_content = elm_object_item_part_content_get(item->it, "elm.swallow.icon");
	if (part_content) {
		elm_object_signal_emit(part_content, "show_default", "*");
	}

	bool match = false;
	if (current && !g_strcmp0(current->uri, uri) && list->edit_mode == 0) {
		match = true;
	}

	/*It make the same tracks will not be matched in playlist using the condition */
	if (match && list->track_type == MP_TRACK_BY_PLAYLIST) {
		int member_id = 0;
		mp_media_info_get_playlist_member_id(track, &member_id);
		if (current && (member_id != current->playlist_member_id)) {
			match = false;
		}
	}

	if (match && part_content) {
		if (((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PLAYING)) {
			elm_object_signal_emit(part_content, "show_play", "*");
		} else if (((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PAUSED) || ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_READY)) {
			elm_object_signal_emit(part_content, "show_pause", "*");
		}
	}

	if (!strcmp(part, "elm.swallow.icon")) {
		char *thumbpath = NULL;
		Evas_Object *icon;

		PROFILE_IN("_mp_track_list_icon_get");
		mp_media_info_get_thumbnail_path(track, &thumbpath);

#ifdef MP_FEATURE_PERSONAL_PAGE
		char *filepath = NULL;
		mp_media_info_get_file_path(track, &filepath);
		if (mp_util_is_in_personal_page(filepath)) {
			icon = mp_widget_lock_icon_create(obj, (const char *)thumbpath);
		} else {
			icon = mp_util_create_lazy_update_thumb_icon(obj, thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		}
#else
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
#endif
		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);
		PROFILE_OUT("_mp_track_list_icon_get");

		if (match && content) {
			if ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PLAYING) {
				elm_object_signal_emit(content, "show_play", "*");
			} else {
				elm_object_signal_emit(content, "show_pause", "*");
			}
		}

		return content;
	}


	Evas_Object *check = NULL;

	if (list->edit_mode) {
		// if edit mode
		if (!strcmp(part, "elm.swallow.end")) {
			// swallow checkbox or radio button

			check = elm_check_add(obj);
			elm_object_style_set(check, "default");
			evas_object_propagate_events_set(check, EINA_FALSE);
			evas_object_smart_callback_add(check, "changed", mp_common_view_check_changed_cb, NULL);
			elm_check_state_pointer_set(check, &item->checked);

			return check;
		}
	}

	if (list->reorderable) {
		if (!strcmp(part, "elm.swallow.end")) {
			Evas_Object *thumbnail = NULL;
			thumbnail = elm_image_add(obj);
			elm_image_file_set(thumbnail, IMAGE_EDJ_NAME, MP_LITE_REORDER_ICON);
			evas_object_color_set(thumbnail, 8, 8, 8, 204);
			evas_object_size_hint_align_set(thumbnail, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

			return thumbnail;
		}

	}

	return NULL;
}

static void
_mp_track_list_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);
	SAFE_FREE(item_data);
}

static void
_mp_track_genlist_sel_cb(void *data, Evas_Object * obj, void *event_info)
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

	if (!list->reorderable) {
		mp_common_play_track_list_with_playlist_id(item, obj, list->playlist_id);
	}

	return;
}

static void
_mp_track_list_albumart_index_list_destroy(MpTrackList_t *list)
{
	MP_CHECK(list);
	MP_CHECK(list->albumart_index_list);

	GList *info_list = list->albumart_index_list;
	while (info_list) {

		albumart_info_s *albumart = info_list->data;
		if (albumart) {
			IF_FREE(albumart->name);
			IF_FREE(albumart->path);
		}

		info_list = info_list->next;
	}

	list->albumart_index_list = NULL;
}

static void
_mp_track_list_albumart_index_shortcut_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MpTrackList_t *list = data;
	MP_CHECK(list);
	MP_CHECK(list->albumart_index_list);

	int index = (int)evas_object_data_get(obj, "index");
	EVENT_TRACE("shortcut index = %d", index);
	albumart_info_s *albumart = g_list_nth_data(list->albumart_index_list, index);
	MP_CHECK(albumart);

	MP_CHECK(list->genlist);

	char *album = NULL;
	Elm_Object_Item *item = mp_list_first_item_get(list->genlist);
	while (item) {
		mp_list_item_data_t *data = elm_object_item_data_get(item);
		if (data && data->handle && data->item_type == MP_LIST_ITEM_TYPE_NORMAL) {
			mp_media_info_get_album(data->handle, &album);
			if (!g_strcmp0(album, albumart->name)) {
				mp_list_item_selected_set(item, EINA_TRUE);
				return;
			}
		}
		item = mp_list_item_next_get(item);
	}
}

static Evas_Object *
_mp_track_list_albumart_index_contnet_get_cb(void *data, Evas_Object *obj, const char *part)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);
	MpTrackList_t *list = (MpTrackList_t *)item_data->handle;
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->albumart_index_list);

	int albumart_count = g_list_length(list->albumart_index_list);

	int shortcut_count = 1;
	if (albumart_count >= 6) {
		shortcut_count = 6;
	} else if (albumart_count >= 4) {
		shortcut_count = 4;
	}

#ifdef MP_FEATURE_LANDSCAPE
	bool landscape = mp_util_is_landscape();
	int landscape_shortcut_count = 5;
#endif
	Evas_Object *layout = NULL;

	char *group = g_strdup_printf("playlist_shortcut_layout_%d", shortcut_count);
#ifdef MP_FEATURE_LANDSCAPE
	if (landscape) {
		layout = mp_common_load_edj(obj, MP_EDJ_NAME, "landscape_playlist_shortcut_layout");
	} else
#endif
		layout = mp_common_load_edj(obj, MP_EDJ_NAME, group);
	IF_FREE(group);

#ifdef MP_FEATURE_LANDSCAPE
	if (landscape) {
		evas_object_size_hint_min_set(layout, LANDSCAPE_ALBUMART_INDEX_SIZE, LANDSCAPE_ALBUMART_INDEX_SIZE * elm_config_scale_get());
	} else
#endif
		evas_object_size_hint_min_set(layout, ALBUMART_INDEX_SIZE, ALBUMART_INDEX_SIZE * elm_config_scale_get());

	int i = 0;
	int size = 0;
#ifdef MP_FEATURE_LANDSCAPE
	if (landscape) {
		for (; i < landscape_shortcut_count; i++) {
			albumart_info_s *info = g_list_nth_data(list->albumart_index_list, i);
			if (info) {
				size = LANDSCAPE_ALBUMART_INDEX_SIZE;
				Evas_Object *shortcut = mp_widget_shorcut_box_add(layout, NULL, info->path, NULL, size, size, _mp_track_list_albumart_index_shortcut_clicked_cb, list);
				evas_object_data_set(shortcut, "index", (void *)i);
				char *part = g_strdup_printf("elm.icon.%d", i + 1);
				elm_object_part_content_set(layout, part, shortcut);
				IF_FREE(part);
			}
		}
	} else
#endif
	{
		for (; i < shortcut_count; i++) {
			albumart_info_s *info = g_list_nth_data(list->albumart_index_list, i);
			if (info) {
				if (shortcut_count == 1) {
					size = ALBUMART_INDEX_SIZE;
				} else if (shortcut_count == 4) {
					size = ALBUMART_INDEX_SIZE / 2;
				} else {
					size = ALBUMART_INDEX_SIZE / 3;
					if (i == 0) {
						size = size * 2;
					}
				}
				Evas_Object *shortcut = mp_widget_shorcut_box_add(layout, NULL, info->path, NULL, size, size, _mp_track_list_albumart_index_shortcut_clicked_cb, list);
				evas_object_data_set(shortcut, "index", (void *)i);
				char *part = g_strdup_printf("elm.icon.%d", i + 1);
				elm_object_part_content_set(layout, part, shortcut);
				IF_FREE(part);
			}
		}
	}

	return layout;
}

static void
_mp_track_list_albumart_index_item_del_cb(void *data, Evas_Object *obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);

	MpTrackList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK(list);

	if (list->albumart_index_item  == item_data->it) {
		list->albumart_index_item = NULL;
	}
}

static int
_mp_track_list_album_index_list_with_sort(const void *a, const void *b)
{
	if (!a || !b) {
		mp_error("Invalid parameter");
		return -1;
	}

	albumart_info_s *a_album = (albumart_info_s *)a;
	albumart_info_s *b_album = (albumart_info_s *)b;

	return (b_album->count - a_album->count);
}

static void
_mp_track_list_albumart_index_list_append(MpTrackList_t *list, const mp_media_info_h media)
{
	MP_CHECK(list);
	MP_CHECK(media);

	char *name = NULL;
	mp_media_info_get_album(media, &name);
	MP_CHECK(name);

	albumart_info_s *album = NULL;
	GList *album_list = list->albumart_index_list;
	while (album_list) {
		album = album_list->data;
		if (album) {
			if (!g_strcmp0(name, album->name)) {
				++album->count;
				return;
			}
		}

		album_list = album_list->next;
	}

	char *path = NULL;
	mp_media_info_get_thumbnail_path(media, &path);
	if (path == NULL || strlen(path) == 0) {
		path = g_strdup(DEFAULT_THUMBNAIL);
	}
	MP_CHECK(path);
	//mp_debug("path = %s", path);

	album = calloc(1, sizeof(albumart_info_s));
	MP_CHECK(album);
	album->name = g_strdup(name);
	album->path = g_strdup(path);
	album->count = 1;

	list->albumart_index_list = g_list_append(list->albumart_index_list, album);
}

void mp_track_list_update_albumart_index(MpTrackList_t *list)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->genlist);

	if (list->albumart_index_item) {
		elm_object_item_del(list->albumart_index_item);
		list->albumart_index_item = NULL;
	}
	_mp_track_list_albumart_index_list_destroy(list);

	MP_CHECK(!list->edit_mode);
	MP_CHECK(list->index_type == MP_TRACK_LIST_INDEX_ALBUM_ART_LIST);

	int count = 0;
	mp_media_info_list_count(list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, &count);
	MP_CHECK(count);

	static Elm_Genlist_Item_Class itc;
	itc.item_style = "1icon/no_padding";
	itc.func.content_get = _mp_track_list_albumart_index_contnet_get_cb;
	itc.func.del = _mp_track_list_albumart_index_item_del_cb;

	mp_media_list_h svc_handle = NULL;
	mp_media_info_list_create(&svc_handle, list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, 0, count);
	MP_CHECK(svc_handle);
	int i = 0;
	for (; i < count ; i++) {
		mp_media_info_h media = mp_media_info_list_nth_item(svc_handle, i);
		_mp_track_list_albumart_index_list_append(list, media);
	}
	mp_media_info_list_destroy(svc_handle);

	if (list->albumart_index_list) {
		if (list->playlist_id > 0) {
			list->albumart_index_list = g_list_sort(list->albumart_index_list, _mp_track_list_album_index_list_with_sort);
		}

		mp_list_item_data_t *item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->index = -1;
		item_data->item_type = MP_LIST_ITEM_TYPE_ALBUMART_INDEX;
		item_data->handle = (mp_media_info_h)list;

		item_data->it = elm_genlist_item_prepend(list->genlist, &itc, item_data, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
		elm_genlist_item_select_mode_set(item_data->it, ELM_OBJECT_SELECT_MODE_NONE);
		list->albumart_index_item = item_data->it;
	}
	endfunc;
}

MpCloudView_e mp_track_list_get_cloud_view(MpTrackList_t *list)
{
	return list->cloud_view_type;
}

static void _mp_track_list_append_item(MpTrackList_t *list, mp_media_list_h svc_handle, int count)
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
				ERROR_TRACE("Fail to mp_media_info_get_title, ret[%d], index[%d]", ret, index);
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

		if (mp_util_is_in_personal_page((const char *)path)) {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_ADD || !mp_util_is_personal_page_on()) {
				DEBUG_TRACE("ignore the items out of personal storage");
				continue;
			}
		} else {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_REMOVE) {
				DEBUG_TRACE("ignore the items in personal storage");
				continue;
			}
		}
#endif
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->index = index;
		item_data->group_type = MP_GROUP_NONE;

		char *file_path = NULL;
		mp_media_info_get_file_path(item, &file_path);
		item_data->checked = mp_list_is_in_checked_path_list(list->checked_path_list, file_path);

		item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
		                                        ELM_GENLIST_ITEM_NONE, _mp_track_genlist_sel_cb, list);
		elm_object_item_data_set(item_data->it, item_data);
	}
#ifdef MP_FEATURE_CLOUD
END:
#endif
	endfunc;

}

static Eina_Bool
_mp_track_list_lazy_load(void *thiz)
{
	startfunc;
	int count = 0, res = 0;
	MpTrackList_t *list = thiz;
	mp_media_list_h svc_handle = NULL;
	MP_CHECK_FALSE(list);

	res = mp_media_info_list_count(list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, &count);
	MP_CHECK_FALSE(res == 0);

	count = count - INITIAL_LOAD_COUNT;

	mp_media_info_list_create(&svc_handle, list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, INITIAL_LOAD_COUNT, count);
	_mp_track_list_append_item(list, svc_handle, count);

//	mp_list_bottom_counter_item_append((MpList_t *)list);

	if (list->track_list[1]) {
		mp_media_info_list_destroy(list->track_list[1]);
	}
	list->track_list[1] = svc_handle;

	list->load_timer = NULL;
	if (mp_list_get_edit((MpList_t *)list) || list->get_by_view) {
		MpView_t *view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
		if (view) {
			mp_view_update_options(view);
		}
	}
	return EINA_FALSE;
}


static char *
_mp_track_list_shuffle_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *markup = NULL;
	static char result[DEF_STR_LEN + 1] = { 0, };
	if (!strcmp(part, "elm.text")) {
		MpTrackList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);

		int r = 21;
		int g = 108;
		int b = 148;
		int a = 255 ;

		markup = (list->track_count == 1) ? g_strdup(GET_STR(STR_MP_SHUFFLE_1_TRACK)) : g_strdup_printf(GET_STR(STR_MP_SHUFFLE_PD_TRACKS), list->track_count);

		memset(result, 0x00, DEF_STR_LEN + 1);
		snprintf(result, DEF_STR_LEN,
		         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, markup);
		IF_FREE(markup);

		return g_strdup(result);
	}
	return NULL;
}

Evas_Object *
_mp_track_list_shuffle_icon_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *content = NULL;
	content = elm_layout_add(obj);

	if (!strcmp(part, "elm.swallow.icon")) {
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
_mp_track_list_shuffle_cb(void *data, Evas_Object * obj, void *event_info)
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
	mp_common_play_track_list_with_playlist_id(item, obj, list->playlist_id);

	return;
}

static void
_mp_track_list_shuffle_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static void _mp_track_list_append_shuffle_item(MpTrackList_t *list)
{
	startfunc;
	MP_CHECK(list);

	if (list->itc_shuffle == NULL) {
		list->itc_shuffle = elm_genlist_item_class_new();
		MP_CHECK(list->itc_shuffle);
		//list->itc_shuffle->item_style = "music/1line";//"music/3text.1icon.2"
		list->itc_shuffle->item_style = "default";//"music/3text.1icon.2"
		list->itc_shuffle->func.text_get = _mp_track_list_shuffle_text_get;
		list->itc_shuffle->decorate_all_item_style = NULL;
		list->itc_shuffle->func.content_get = _mp_track_list_shuffle_icon_get;
		list->itc_shuffle->func.del = _mp_track_list_shuffle_item_del_cb;
	}

	mp_list_item_data_t *item_data;
	item_data = calloc(1, sizeof(mp_list_item_data_t));
	MP_CHECK(item_data);
	item_data->item_type = MP_LIST_ITEM_TYPE_SHUFFLE;

	item_data->it = list->shuffle_it = elm_genlist_item_append(list->genlist, list->itc_shuffle, item_data, NULL,
	                                   ELM_GENLIST_ITEM_NONE, _mp_track_list_shuffle_cb, list);

	elm_object_item_data_set(item_data->it, item_data);

	endfunc;
}

void mp_track_list_show_shuffle(void *thiz, bool show)
{
	startfunc;
	MP_CHECK(thiz);
	MpTrackList_t *list = thiz;
	MP_CHECK(list->genlist);

	DEBUG_TRACE("show shuffle: %d   list->shuffle_it: %0x", show, list->shuffle_it);
	if (show) {
		_mp_track_list_append_shuffle_item(list);
	} else if (list->shuffle_it) {
		elm_object_item_del(list->shuffle_it);
		list->shuffle_it = NULL;
	}
}

void mp_track_list_popup_delete_genlist_item(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpTrackList_t *list = thiz;
	MP_CHECK(list->genlist);

	if (list->track_count > 0) {
		list->track_count--;
	}
}

void mp_track_list_update_genlist(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpTrackList_t *list = thiz;
	MP_CHECK(list->genlist);

	if (list->track_count <= 0) {
		mp_list_update(thiz);
	} else {
		elm_genlist_realized_items_update(list->genlist);
	}
}

static void _mp_track_list_load_list(void *thiz, int count)
{
	startfunc;
	MpTrackList_t *list = thiz;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle = NULL;

	/*clear genlist*/
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	if (item) {
		elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
		elm_genlist_clear(list->genlist);
	}

	mp_track_list_update_albumart_index(list);

	mp_ecore_timer_del(list->load_timer);
	if (list->track_type == MP_TRACK_ALL && count > INITIAL_LOAD_COUNT) {
		count = INITIAL_LOAD_COUNT;
		list->load_timer = ecore_timer_add(0.4, _mp_track_list_lazy_load, list);
	}


	/*get data from DB*/
	PROFILE_IN("mp_media_info_list_create");
	mp_media_info_list_create(&svc_handle, list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, 0, count);
	PROFILE_OUT("mp_media_info_list_create");

	mp_track_list_show_shuffle(list, true);

//	if ((!list->reorderable) && ((list->track_type != MP_TRACK_ALL) || (count <= INITIAL_LOAD_COUNT)))
//			mp_list_bottom_counter_item_append((MpList_t *)list);

	PROFILE_IN("_mp_track_list_append_item");
	_mp_track_list_append_item(list, svc_handle, count);
	PROFILE_OUT("_mp_track_list_append_item");

	if (list->track_list[0]) {
		mp_media_info_list_destroy(list->track_list[0]);
	}
	list->track_list[0] = svc_handle;

}
static void _mp_track_list_destory_cb(void *thiz)
{
	eventfunc;
	MpTrackList_t *list = thiz;
	MP_CHECK(list);

	mp_ecore_timer_del(list->load_timer);

	if (list->playlists) {
		mp_media_info_group_list_destroy(list->playlists);
	}

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

	if (list->itc_shuffle) {
		elm_genlist_item_class_free(list->itc_shuffle);
		list->itc_shuffle = NULL;
	}

	IF_FREE(list->type_str);
	IF_FREE(list->type_str2);
	IF_FREE(list->filter_str);

	elm_genlist_item_class_free(list->itc);

	_mp_track_list_albumart_index_list_destroy(list);
	mp_list_free_checked_path_list(list->checked_path_list);

	free(list);
}

/*
static void
_mp_track_list_item_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpTrackList_t *list = data;
	MP_CHECK(list);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	int index = -1;
        int ret = 0;
        int member_id = 0;

        void *playlist_handle = mp_list_get_playlist_handle((MpList_t *)list);
	Elm_Object_Item *temp = elm_genlist_first_item_get(obj);
	while (temp) {
                index = elm_genlist_item_index_get(temp);
	        mp_list_item_data_t *item_data = elm_object_item_data_get(temp);
	        MP_CHECK(item_data);

                ret = mp_media_info_get_playlist_member_id(item_data->handle, &member_id);
                MP_CHECK(ret==MEDIA_CONTENT_ERROR_NONE);

                ret = mp_media_info_playlist_set_play_order(playlist_handle, member_id, index);
                MP_CHECK(ret==MEDIA_CONTENT_ERROR_NONE);
                temp = elm_genlist_item_next_get(temp);
        }

  	ret = mp_media_info_playlist_update_db(playlist_handle);
	MP_CHECK(ret==MEDIA_CONTENT_ERROR_NONE);

	//mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_REORDER);
}
*/

/*static void
_mp_track_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpTrackList_t *list = (MpTrackList_t*)data;
	MP_CHECK(list);

	 if (list->edit_mode)
	 {
		return ;
	 }

        struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

        char *title = NULL;

        int pop_item_count = 5;
#ifdef MP_FEATURE_ALBUMART_UPDATE
	pop_item_count = 6;
#endif
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
	if (item_data->item_type == MP_LIST_ITEM_TYPE_ALBUMART_INDEX)
			return;

        item_data->checked = true;

        mp_media_info_get_title(item_data->handle, &title);

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

        char *up_title = g_strdup(title);

        MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
        MpView_t *view = mp_view_mgr_get_top_view(view_mgr);

        elm_object_part_text_set(popup, "title,text", up_title);
        IF_FREE(up_title);

#ifdef MP_FEATURE_PERSONAL_PAGE
	char *file_path = NULL;
	bool personal_page_flag = false;
	mp_media_info_get_file_path(item_data->handle, &file_path);
	if (file_path)
	{
		personal_page_flag = mp_util_is_in_personal_page((const char *)file_path);
	}

	if (!personal_page_flag)
	{
#endif
        mp_genlist_popup_item_append(popup, STR_MP_SET_AS, NULL, NULL, NULL,
                                     mp_common_list_set_as_cb, list);
#ifdef MP_FEATURE_PERSONAL_PAGE
	}
#endif
        mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PLAYLIST, NULL, NULL, NULL,
                                     mp_common_list_add_to_playlist_cb, list);
        if (list->track_type != MP_TRACK_BY_FAVORITE) {
        	char *str = NULL;
        	Evas_Smart_Cb cb = NULL;
		mp_media_info_h favourite_handle = NULL;
		char *uid = NULL;
		bool favorite = false;
		mp_media_info_get_media_id(item_data->handle, &uid);
		mp_media_info_create(&favourite_handle, uid);
		mp_media_info_get_favorite(favourite_handle, &favorite);
		SAFE_FREE(favourite_handle);
		if (favorite)
		{
        		str = STR_MP_POPUP_REMOVE_FROM_FAVORITE;
        		cb = mp_common_list_unfavorite_cb;
        	}
        	else {
        		str = STR_MP_ADD_TO_FAVOURITES;
        		cb = mp_common_list_add_to_favorite_cb;
        	}

        	mp_genlist_popup_item_append(popup, str, NULL, NULL, NULL, cb, list);
        }
        if (MP_VIEW_PLAYLIST_DETAIL == view->view_type)
        {
                mp_genlist_popup_item_append(popup, STR_MP_REMOVE, NULL, NULL, NULL,
                                             mp_common_list_remove_cb, list);
        }
        else
        {
                mp_genlist_popup_item_append(popup, STR_MP_DELETE, NULL, NULL, NULL,
                                             mp_common_list_delete_cb, list);
        }

#ifdef MP_FEATURE_PERSONAL_PAGE
	if (mp_util_is_personal_page_on() && top_view->view_type != MP_VIEW_PLAYLIST_DETAIL) // personal storage function is not provided in playlist detail view
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
}

static void mp_track_list_add_tracks_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpTrackList_t *list = data;
	MP_CHECK(list);
	mp_common_show_add_tracks_view(list->playlist_id);
}*/

static void _mp_track_list_item_highlighted(void *data, Evas_Object *obj, void *event_info)
{
	MpTrackList_t *list = data;
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

static void _mp_track_list_item_unhighlighted(void *data, Evas_Object *obj, void *event_info)
{
	MpTrackList_t *list = data;
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

static void _mp_track_list_update(void *thiz)
{
	startfunc;
	int count = 0, res = 0;
	MpTrackList_t *list = thiz;
	MP_CHECK(list);

	PROFILE_IN("mp_media_info_list_count");
	res = mp_media_info_list_count(list->track_type, list->type_str, list->type_str2, list->filter_str, list->playlist_id, &count);
	PROFILE_OUT("mp_media_info_list_count");
	MP_CHECK(res == 0);
	list->track_count = count;

	if (list->get_by_view == false) {
		mp_list_free_checked_path_list(list->checked_path_list);
		list->checked_path_list = mp_list_get_checked_path_list((MpList_t *)list);
	}
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
				list->itc->item_style = "type1";
				list->itc->func.text_get = _mp_track_list_label_get;
				list->itc->func.content_get = _mp_track_list_icon_get;
				list->itc->func.del = _mp_track_list_item_del_cb;
			}
		}

		if (list->reorderable) {
			evas_object_smart_callback_add(list->genlist, "moved", mp_list_item_reorder_moved_cb, list);
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
		//evas_object_smart_callback_add(list->genlist, "longpressed", _mp_track_list_item_longpressed_cb, list);
		evas_object_smart_callback_add(list->genlist, "scroll,drag,start", list->drag_start_cb, list);
		evas_object_smart_callback_add(list->genlist, "scroll,drag,stop", list->drag_stop_cb, list);
		evas_object_smart_callback_add(list->genlist, "highlighted", _mp_track_list_item_highlighted, list);
		evas_object_smart_callback_add(list->genlist, "unhighlighted", _mp_track_list_item_unhighlighted, list);

		PROFILE_OUT("elm_genlist_add");
		/* load list */
		PROFILE_IN("_mp_track_list_load_list");
		_mp_track_list_load_list(thiz, count);
		list->show_fastscroll(list);
		PROFILE_OUT("_mp_track_list_load_list");

		if (!mp_list_get_editable_count(thiz, mp_list_get_edit_type(thiz))) {
			goto NoContents;
		}

		return;
	}

NoContents:
	list->hide_fastscroll(list);
	mp_evas_object_del(list->genlist);
	if (!list->no_content) {
		if (list->track_type > MP_TRACK_TYPE_PLAYLIST_MIN && list->track_type < MP_TRACK_TYPE_PLAYLIST_MAX) {
			char *helptext = NULL;
			Evas_Smart_Cb callback = NULL;
			void *cb_data = NULL;

			if (list->track_type == MP_TRACK_BY_PLAYLIST) {
				char *playlist_name = NULL;
				mp_media_info_group_get_main_info(list->playlist_handle, &playlist_name);
				char *title = elm_entry_utf8_to_markup(playlist_name);
				helptext = g_strdup_printf(GET_STR(STR_MP_YOU_CAN_ADD_TRACKS_THE_PLAYLIST_PD), title);
				IF_FREE(title);
				//callback = mp_track_list_add_tracks_to_playlist_cb;
				//cb_data = list;
			} else if (list->track_type == MP_TRACK_BY_FAVORITE) {
				helptext = g_strdup(STR_MP_AFTER_YOU_ADD_TRACK_TO_FAVOURITE);
			} else if (list->track_type == MP_TRACK_BY_PLAYED_COUNT || list->track_type == MP_TRACK_BY_PLAYED_TIME) {
				helptext = g_strdup(STR_MP_AFTER_YOU_PLAY_TRACKS_THEY_WILL_BE_SHOWN);
			} else if (list->track_type == MP_TRACK_BY_ADDED_TIME) {
				helptext = g_strdup(STR_MP_AFTER_YOU_DOWNLOAD_TRACKS);
			}

			list->no_content = mp_widget_create_no_content_playlist(list->box, helptext, callback, cb_data);
			IF_FREE(helptext);
		} else {
			list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_TRACKS, NULL, list);
		}
		elm_box_pack_end(list->box, list->no_content);
	}
}

static mp_track_type_e _mp_track_list_get_track_type(void *thiz)
{
	MpTrackList_t *list = thiz;
	MP_CHECK_VAL(list, MP_TRACK_ALL);
	return list->track_type;
}

static void *_mp_track_list_get_handle(void *thiz)
{
	MpTrackList_t *list = thiz;
	MP_CHECK_NULL(list);
	return list->playlist_handle;
}

static const char *_get_label(void *thiz, void *event_info)
{
	MpTrackList_t *list = thiz;
	MP_CHECK_NULL(list);
	char *title = NULL;

	mp_list_item_data_t *track =  elm_object_item_data_get(event_info);
	MP_CHECK_NULL(track);

	mp_media_info_get_title(track->handle, &title);
	return title;
}

static void _mp_track_list_set_edit(void *thiz, bool edit)
{
	startfunc;
	MpTrackList_t *list = thiz;
	MP_CHECK(list);

	mp_track_list_show_shuffle(list, false);

	/* check DRM FL */
	if (mp_list_get_edit_type((MpList_t*)list) == MP_LIST_EDIT_TYPE_SHARE) {
		_mp_track_list_update(list);
	}

	if (edit && list->albumart_index_item) {
		elm_object_item_del(list->albumart_index_item);
		list->albumart_index_item = NULL;
	}

	if (list->set_edit_default) {
		list->set_edit_default(list, edit);
	}

	if (!edit) {
		mp_track_list_update_albumart_index(list);
	}
}

static unsigned int
_mp_track_list_get_editable_count(void *thiz, MpListEditType_e type)
{
	MpTrackList_t *list = thiz;
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

	return count;
}

static char *
_mp_track_list_bottom_counter_text_cb(void *thiz)
{
	MpTrackList_t *list = thiz;
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

static mp_group_type_e _mp_track_list_get_group_type(void *thiz)
{
	MpTrackList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);
	return MP_GROUP_NONE;
}


MpTrackList_t * mp_track_list_create(Evas_Object *parent)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpTrackList_t *list = calloc(1, sizeof(MpTrackList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_TRACK);

	list->update = _mp_track_list_update;
	list->destory_cb = _mp_track_list_destory_cb;
	list->get_track_type = _mp_track_list_get_track_type;
	list->get_group_type = _mp_track_list_get_group_type;
	list->get_playlist_handle = _mp_track_list_get_handle;
	list->get_label = _get_label;

	list->set_edit_default = list->set_edit;
	list->set_edit = _mp_track_list_set_edit;
	list->get_count = _mp_track_list_get_editable_count;

	list->bottom_counter_text_get_cb = _mp_track_list_bottom_counter_text_cb;

	return list;
}

static void
_set_playlist_handle(MpTrackList_t *list)
{
	int res = 0;
	int i, count = 0;
	mp_media_list_h media_list = NULL;
	mp_media_info_h media = NULL;

	res = mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, NULL, NULL, &count);
	MP_CHECK(res == 0);

	res = mp_media_info_group_list_create(&media_list, MP_GROUP_BY_PLAYLIST, NULL, NULL, 0, count);
	MP_CHECK(res == 0);

	for (i = 0; i < count; i++) {
		int playlist_id;
		media = mp_media_info_group_list_nth_item(media_list, i);
		mp_media_info_group_get_playlist_id(media, &playlist_id);
		if (playlist_id == list->playlist_id) {
			break;
		}
	}
	if (list->playlists) {
		mp_media_info_group_list_destroy(list->playlists);
	}

	list->playlists = media_list;
	list->playlist_handle = media;
}


void mp_track_list_set_data(MpTrackList_t *list, ...)
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
		case MP_TRACK_LIST_TYPE: {
			int val = va_arg((var_args), int);

			list->track_type = val;
			DEBUG_TRACE("list->track_type = %d", list->track_type);
			break;
		}

		case MP_TRACK_LIST_PLAYLIT_ID: {
			int val = va_arg((var_args), int);
			list->playlist_id = val;
			DEBUG_TRACE("list->playlist_id = %d", list->playlist_id);

			_set_playlist_handle(list);

			break;
		}

		case MP_TRACK_LIST_TYPE_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->type_str);
			list->type_str = g_strdup(val);
			DEBUG_TRACE("list->type_str = %s", list->type_str);

			break;
		}
		case MP_TRACK_LIST_TYPE_STR2: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->type_str2);
			list->type_str2 = g_strdup(val);
			DEBUG_TRACE("list->type_str = %s", list->type_str2);

			break;
		}
		case MP_TRACK_LIST_FILTER_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->filter_str);
			list->filter_str = g_strdup(val);
			DEBUG_TRACE("list->filter_str = %s", list->filter_str);

			break;
		}
		case MP_TRACK_LIST_INDEX_TYPE: {
			int val = va_arg((var_args), int);
			list->index_type = (MpTrackListIndex_t)val;
			DEBUG_TRACE("list->index_type = %d", list->index_type);
			break;
		}
		case MP_TRACK_LIST_CLOUD_TYPE: {
			int val = va_arg((var_args), int);
			list->cloud_view_type = val;
			DEBUG_TRACE("list->index_type = %d", list->index_type);
			break;
		}
		case MP_TRACK_LIST_CHECKED_LIST: {
			GList *val = va_arg((var_args), GList*);
			list->checked_path_list = val;
			list->get_by_view = true;
			break;
		}

		default:
			DEBUG_TRACE("Invalid arguments");
		}

	} while (field >= 0);

	va_end(var_args);
}

void mp_track_list_copy_data(MpTrackList_t *src, MpTrackList_t *dest)
{
	MP_CHECK(src);
	MP_CHECK(dest);

	dest->track_type = src->track_type;
	dest->playlist_id = src->playlist_id;
	_set_playlist_handle(dest);

	SAFE_FREE(dest->type_str);
	dest->type_str = g_strdup(src->type_str);

	SAFE_FREE(dest->type_str2);
	dest->type_str2 = g_strdup(src->type_str2);

	SAFE_FREE(dest->filter_str);
	dest->filter_str = g_strdup(src->filter_str);

	dest->index_type = src->index_type;
	dest->cloud_view_type = src->cloud_view_type;
}

