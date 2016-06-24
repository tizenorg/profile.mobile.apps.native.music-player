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

#include "mp-playlist-list.h"
#include "mp-playlist-detail-view.h"
#include "mp-select-track-view.h"
#include "mp-common.h"
#include "mp-popup.h"
#include "mp-widget.h"
#include "mp-setting-ctrl.h"
#include "mp-util.h"
#include "mp-ug-launch.h"
#include "mp-menu.h"
#include "mp-player-view.h"
#include "mp-play.h"
#include "mp-edit-playlist.h"
#include "ms-key-ctrl.h"

#define ALBUM_GRID_W 233
#define ALBUM_GRID_H 319
#define ALBUM_GRID_LAND_W 252
#define ALBUM_GRID_LAND_H 320

static void _mp_playlist_list_update(void *thiz);
static void _mp_playlist_append_user_playlists(void *thiz, Elm_Object_Item *parent_item);
static void _mp_playlist_list_set_grid_style(MpPlaylistList_t *list);

static int
_mp_playlist_list_get_track_type_by_playlist_id(int playlist_id)
{
	int track_type;
	if (playlist_id == MP_SYS_PLST_MOST_PLAYED) {
		track_type = MP_TRACK_BY_PLAYED_COUNT;
	} else if (playlist_id == MP_SYS_PLST_RECENTELY_ADDED) {
		track_type = MP_TRACK_BY_ADDED_TIME;
	} else if (playlist_id == MP_SYS_PLST_RECENTELY_PLAYED) {
		track_type = MP_TRACK_BY_PLAYED_TIME;
	} else if (playlist_id == MP_SYS_PLST_QUICK_LIST) {
		track_type = MP_TRACK_BY_FAVORITE;
	} else {
		track_type = MP_TRACK_BY_PLAYLIST;
	}

	return track_type;
}

static char *
_mp_playlist_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h plst_item = (item->handle);
	MP_CHECK_NULL(plst_item);

	int ret = 0;
	if (!strcmp(part, "elm.text")) {

		char *name = NULL;
		ret = mp_media_info_group_get_main_info(plst_item, &name);
		mp_retvm_if(ret != 0, NULL, "Fail to get value");
		mp_retvm_if(name == NULL, NULL, "Fail to get value");

		return elm_entry_utf8_to_markup(GET_STR(name));
	} else if (!strcmp(part, "elm.text.sub")) {
		int count = -1;
		int plst_id = -1;
		int total_time = 0;
		char time[20] = {0,};
		char *format_text = NULL;

		// TODO:  fix performance issue
		ret = mp_media_info_group_get_playlist_id(plst_item, &plst_id);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");

		char *text = NULL;
		int track_type = _mp_playlist_list_get_track_type_by_playlist_id(plst_id);
		ret = mp_media_info_list_count(track_type, NULL, NULL, NULL, plst_id, &count);

		total_time = mp_common_get_playlist_totaltime(track_type, plst_id, count);
		mp_util_format_duration(time, total_time);

		if (count == 1) {
			text = g_strdup(GET_STR(STR_MP_1_SONG));
		} else {
			text = g_strdup_printf(GET_STR(STR_MP_PD_SONGS), count);
		}

		format_text = g_strdup_printf("%s | %s", text, time);
		IF_FREE(text);
		return format_text;
	}

	return NULL;
}

/*static char *
_mp_playlist_add_label_get(void *data, Evas_Object * obj, const char *part)
{
	return g_strdup(GET_STR(STR_MP_CREATE_PLAYLIST));
}*/

static mp_group_type_e _mp_playlist_list_get_group_type(void *thiz)
{
	MpPlaylistList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);
	return MP_GROUP_BY_PLAYLIST;
}

static void
_mp_playlist_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpPlaylistList_t *list = (MpPlaylistList_t *)data;
	MP_CHECK(list);
	if (list->edit_mode) {
		return ;
	}

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	bool pop_del_status = true;
	int pop_item_count = 5;
	int playlist_state = 0;
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
			if (item_data) {
				item_data->checked = false;
			}
			temp = elm_gengrid_item_next_get(temp);
		}
	} else {
		temp = elm_genlist_first_item_get(list->genlist);
		while (temp) {
			item_data = elm_object_item_data_get(temp);
			if (item_data) {
				item_data->checked = false;
			}
			temp = elm_genlist_item_next_get(temp);
		}
	}

	item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);

	item_data->checked = true;

	int item_index = 0;
	int playlist_auto_count = 0;
	int i = 0;
	pop_item_count = 2;

	if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		item_index = elm_gengrid_item_index_get(item);
	} else {
		item_index = elm_genlist_item_index_get(item);
	}


	mp_media_info_group_get_main_info(item_data->handle, &title);
	mp_setting_playlist_get_state(&playlist_state);
	for (i = 0; i < MP_SYS_PLST_COUNT; i++) {
		if (playlist_state & (1 << i)) {
			playlist_auto_count++;
		}
	}

	if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		if (item_index < (playlist_auto_count)) {
			pop_item_count = 1;
			pop_del_status = false;
			title = GET_SYS_STR(title);
		}
	} else {
		if (item_index <= (playlist_auto_count)) {
			pop_item_count = 1;
			pop_del_status = false;
			title = GET_SYS_STR(title);
		}
	}

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

	char *up_title = elm_entry_utf8_to_markup(title);

	elm_object_part_text_set(popup, "title,text", up_title);
	IF_FREE(up_title);

	mp_genlist_popup_item_append(popup, STR_MP_PLAY_ALL, NULL, NULL, NULL,
	                             mp_common_playall_cb, list);

	if (pop_del_status) {
		mp_genlist_popup_item_append(popup, STR_MP_DELETE, NULL, NULL, NULL,
		                             mp_common_list_delete_cb, list);
		mp_genlist_popup_item_append(popup, STR_MP_RENAME, NULL, NULL, NULL,
		                             mp_common_playlist_rename_cb, list);

	}

	if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		MP_GENGRID_ITEM_LONG_PRESSED(obj, popup, event_info);
	} else {
		MP_GENLIST_ITEM_LONG_PRESSED(obj, popup, event_info);
	}

}

void
mp_playlist_list_view_rename_done_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	char *text = NULL;
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK(item);

	Evas_Object *editfiled_entry = obj;
	MP_CHECK(editfiled_entry);
	/* save */

	//mp_genlist_item_data_t *item = (mp_genlist_item_data_t *) elm_object_item_data_get(layout_data->rename_git);
	//MP_CHECK(item);
	mp_media_info_h plst = (item->handle);
	MP_CHECK(plst);

	bool rename_success = FALSE;
	int ret = 0;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	text = mp_util_isf_get_edited_str(editfiled_entry, TRUE);

	if (!mp_util_is_playlist_name_valid((char *)text)) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_RENAME_PLAYLIST"));
	} else {
		bool exist = false;
		ret = mp_media_info_playlist_is_exist(text, &exist);
		if (ret != 0) {
			ERROR_TRACE("Fail to get playlist count by name: %d", ret);
			mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_CREATE_PLAYLIST"));
		} else {
			char *origin_name = NULL;
			mp_media_info_group_get_main_info(plst, &origin_name);

			if (exist) {
				if (origin_name && !g_strcmp0(origin_name, text)) {
					mp_debug("Not edited.. rename OK");
					rename_success = TRUE;
				} else {
					mp_widget_text_popup(ad, GET_STR(STR_MP_POP_EXISTS));
				}
			} else {
				ret = mp_media_info_playlist_rename(plst, text);
				if (ret == 0) {
					mp_debug("mp_media_info_playlist_rename().. OK");
					rename_success = TRUE;
				}
			}
		}
	}
	IF_FREE(text);

	if (rename_success) {
		mp_debug("playlist rename success");
		/* update content */
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MP_CHECK(view_mgr);
		MpListView_t *view_to_update = (MpListView_t *)mp_view_mgr_get_top_view(view_mgr);
		MP_CHECK(view_to_update);
		mp_list_update(view_to_update->content_to_show);
		mp_list_set_edit(view_to_update->content_to_show, TRUE);
	}

	elm_genlist_item_update(item->it);

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_RENAMED);

	//mp_view_manager_set_controlbar_visible(mp_view_manager_get_navi_item(layout_data->ad), true);

	return;
}

int
mp_playlist_list_set_playlist(mp_plst_mgr *plst_mgr, int playlist_id)
{
	MP_CHECK_VAL(plst_mgr, 0);
	mp_media_list_h svc_handle = NULL;
	int count = 0, track_type = 0;
	int ret;

	DEBUG_TRACE("playlist_id %d", playlist_id);

	track_type = _mp_playlist_list_get_track_type_by_playlist_id(playlist_id);
	mp_media_info_list_count(track_type, NULL, NULL, NULL, playlist_id, &count);

	/* get music item data */
	ret = mp_media_info_list_create(&svc_handle, track_type, NULL, NULL, NULL, playlist_id, 0, count);
	if (ret != 0) {
		DEBUG_TRACE("fail to get list item: %d", ret);
		ret = mp_media_info_list_destroy(svc_handle);
		svc_handle = NULL;
	}

	if (count) {
		mp_playlist_mgr_clear(plst_mgr);
		mp_util_append_media_list_item_to_playlist(plst_mgr, svc_handle, count, 0, NULL);
	}

	if (svc_handle) {
		mp_media_info_list_destroy(svc_handle);
	}

	return count;
}

Evas_Object *
_mp_playlist_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h plst = (item->handle);
	MP_CHECK_NULL(plst);

	Evas_Object *eo = NULL;
	int ret = 0;

	int playlist_id = 0;
	char *thumb_path = NULL;

	Evas_Object *content = NULL;
	content = elm_layout_add(obj);

	mp_media_info_group_get_playlist_id(plst, &playlist_id);

	if (!strcmp(part, "elm.icon.1") || !strcmp(part, "elm.swallow.icon")) {
		mp_common_playlist_album_update(plst);
		ret = mp_media_info_playlist_get_thumbnail_path(plst, &thumb_path);
		mp_retvm_if(ret != 0, NULL, "Fail to get value");
		if (playlist_id >= 0) {
			eo = mp_util_create_lazy_update_thumb_icon(obj, thumb_path, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		} else {
			eo = mp_util_create_thumb_icon(obj, thumb_path, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
		}

		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", eo);

		return content;
	}

	Evas_Object *check = NULL;
	MpPlaylistList_t *list = evas_object_data_get(obj, "list_handle");
	MP_CHECK_NULL(list);
	if (list->edit_mode) {
		if (!strcmp(part, "elm.swallow.end")) {
			// swallow checkbox or radio button
			check = elm_check_add(obj);
			if (MP_LIST_OBJ_IS_GENGRID(obj)) {
				elm_object_style_set(check, "grid");
			} else {
				elm_object_style_set(check, "default");
			}
			evas_object_propagate_events_set(check, EINA_FALSE);
			evas_object_smart_callback_add(check, "changed", mp_common_view_check_changed_cb, NULL);
			elm_check_state_pointer_set(check, &item->checked);
			return check;
		}
	}
	return NULL;
}

Evas_Object *
_mp_playlist_add_icon_get(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.icon.1") || !strcmp(part, "elm.swallow.icon")) {
		Evas_Object *eo = NULL;
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);

		char image_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(image_path, 1024, "%s%s/%s", path, "images/music_player", PLAYLIST_CREATE_THUMBNAIL);

		eo = mp_util_create_thumb_icon(obj, image_path, MP_LIST_ICON_SIZE,
		                               MP_LIST_ICON_SIZE);

		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", eo);

		return content;
	}

	return NULL;
}

static void
_mp_playlist_list_item_del_cb(void *data, Evas_Object * obj)
{
	startfunc;
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);
	if (item_data->unregister_lang_mgr) {
		mp_language_mgr_unregister_genlist_item(item_data->it);
	}
	free(item_data);
}

static void _mp_playlist_list_set_edit(void *thiz, bool edit)
{
	startfunc;
	MpPlaylistList_t *list = thiz;
	MP_CHECK(list);

	if (edit) {
		list->auto_playlist_count = 0;

		if (!MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
			elm_genlist_clear(list->genlist);
		} else {
			elm_gengrid_clear(list->genlist);
		}

		if (list->reorderable) {
			mp_list_reorder_mode_set(list->genlist, EINA_TRUE);
		}

		mp_list_select_mode_set(list->genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

		//mp_list_bottom_counter_item_append((MpList_t *)list);

		_mp_playlist_append_user_playlists(list, NULL);
	}
}

static void
_mp_playlist_user_playlist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpPlaylistList_t *list = (MpPlaylistList_t *)data;
	MP_CHECK(list);

	int ret = 0;
	char *name = NULL;
	int p_id = 0;
	mp_list_item_data_t *item_data = NULL;

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		elm_gengrid_item_selected_set(gli, EINA_FALSE);
	} else {
		elm_genlist_item_selected_set(gli, EINA_FALSE);
	}

	item_data = elm_object_item_data_get(gli);
	MP_CHECK(item_data);

	ret = mp_media_info_group_get_playlist_id(item_data->handle, &p_id);
	mp_retm_if(ret != 0, "Fail to get value");

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");


	if (list->function_type == MP_LIST_FUNC_ADD_TRACK) {
		MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
		MP_CHECK(view_manager);
		MpSelectTrackView_t *view_select_track = mp_select_track_view_create(view_manager->navi);
		MP_CHECK(view_select_track);
		mp_view_mgr_push_view(view_manager, (MpView_t *)view_select_track, NULL);

		mp_view_update_options((MpView_t *)view_select_track);
		mp_view_set_title((MpView_t *)view_select_track, STR_MP_TILTE_SELECT_ITEM);
		mp_track_list_set_data((MpTrackList_t *)view_select_track->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_BY_PLAYLIST, MP_TRACK_LIST_PLAYLIT_ID, p_id, -1);
		mp_list_update((MpList_t *)view_select_track->content_to_show);
		mp_list_set_edit((MpList_t *)view_select_track->content_to_show, TRUE);
		mp_list_view_set_select_all((MpListView_t *)view_select_track, TRUE);
		return;
	}

	if (list->edit_mode) {
		//mp_edit_view_genlist_sel_cb(data, obj, event_info);
		mp_list_edit_mode_sel((MpList_t *)list, item_data);

		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		ERROR_TRACE("update options of edit view");
		mp_view_update_options_edit(view);
		ERROR_TRACE("set selected count");
		return;
	}

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MP_CHECK(view_manager);
	MpPlaylistDetailView_t *view_plst_detail = mp_playlist_detail_view_create(view_manager->navi, MP_TRACK_BY_PLAYLIST, name, p_id);
	MP_CHECK(view_plst_detail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_plst_detail, NULL);

	mp_view_update_options((MpView_t *)view_plst_detail);
	mp_view_set_title((MpView_t *)view_plst_detail, name);

}

static void
_mp_playlist_create_auto_playlist(void *data, mp_list_item_data_t *item_data, char *name)
{
	MP_CHECK(item_data);
	MpPlaylistList_t *list = (MpPlaylistList_t *)data;
	MP_CHECK(list);

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

	ERROR_TRACE("type is %d", type);

	if (list->function_type == MP_LIST_FUNC_ADD_TRACK) {
		MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
		MP_CHECK(view_manager);
		MpSelectTrackView_t *view_select_track = mp_select_track_view_create(view_manager->navi);
		MP_CHECK(view_select_track);
		mp_view_mgr_push_view(view_manager, (MpView_t *)view_select_track, NULL);

		mp_view_update_options((MpView_t *)view_select_track);
		mp_view_set_title((MpView_t *)view_select_track, STR_MP_TILTE_SELECT_ITEM);
		mp_track_list_set_data((MpTrackList_t *)view_select_track->content_to_show, MP_TRACK_LIST_TYPE, type, -1);
		mp_list_update((MpList_t *)view_select_track->content_to_show);
		mp_list_set_edit((MpList_t *)view_select_track->content_to_show, TRUE);
		return;
	}

	if (list->edit_mode) {
		//mp_edit_view_genlist_sel_cb(data, obj, event_info);
		mp_list_edit_mode_sel((MpList_t *)list, item_data);

		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MP_CHECK(view_mgr);
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		MP_CHECK(view);
		ERROR_TRACE("update options of edit view");
		mp_view_update_options_edit((MpView_t *)view);
		ERROR_TRACE("set selected count");
		return;
	}
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MP_CHECK(view_manager);
	MpPlaylistDetailView_t *view_plst_detail = mp_playlist_detail_view_create(view_manager->navi, type, name, -1);
	MP_CHECK(view_plst_detail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_plst_detail, NULL);

	mp_view_update_options((MpView_t *)view_plst_detail);
	mp_view_set_title((MpView_t *)view_plst_detail, name);
}

static void
_mp_playlist_auto_playlist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	int ret = 0;
	char *name = NULL;
	mp_list_item_data_t *item_data = NULL;
	MpPlaylistList_t *list = (MpPlaylistList_t *)data;
	MP_CHECK(list);

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;

	if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		elm_gengrid_item_selected_set(gli, EINA_FALSE);
	} else {
		elm_genlist_item_selected_set(gli, EINA_FALSE);
	}

	item_data = elm_object_item_data_get(gli);
	MP_CHECK(item_data);
	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");

	SECURE_DEBUG("playlist name: %s", name);

	_mp_playlist_create_auto_playlist(data, item_data, name);
}

static void
_mp_playlist_append_auto_playlists(void *thiz, Elm_Object_Item *parent_item)
{
	int i;
	int playlist_state = 0;

	MpPlaylistList_t *plst = (MpPlaylistList_t *)thiz;
	MP_CHECK(plst);

	if (plst->playlists_auto) {
		mp_media_info_group_list_destroy(plst->playlists_auto);
	}

	mp_setting_playlist_get_state(&playlist_state);

	char* str =  NULL;
	ms_key_get_playlist_str(&str);

	int value = atoi(str);
	int playlist[4] = {0};
	DEBUG_TRACE("value %d", value);
	int j = 0;
	for (j = 3; j >= 0 ; j--) {
		playlist[j] = value % 10;
		value = value / 10;
		DEBUG_TRACE("index  %d  %d", j, playlist[j]);
	}

	mp_media_info_group_list_create(&(plst->playlists_auto), MP_GROUP_BY_SYS_PLAYLIST, NULL, NULL, 0, 0);
	for (i = 0; i < MP_SYS_PLST_COUNT; i++) {
		int enable = playlist_state & (1 << (playlist[i] - 1));
		DEBUG_TRACE("index: %d, state: %d", i, enable);
		if (!enable) {
			continue;
		}

		mp_media_info_h item;
		//item = mp_media_info_group_list_nth_item(plst->playlists_auto, i);
		item = mp_media_info_group_list_nth_item(plst->playlists_auto, playlist[i] - 1);

		mp_list_item_data_t *item_data;
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->unregister_lang_mgr = true;

		plst->auto_playlist_count++;
		if (MP_LIST_OBJ_IS_GENGRID(plst->genlist)) {
			item_data->it = elm_gengrid_item_append(plst->genlist, plst->gengrid_itc, item_data,
			                                        _mp_playlist_auto_playlist_select_cb, (void *)plst);
		} else {
			item_data->it = elm_genlist_item_append(plst->genlist, plst->itc_auto,
			                                        item_data, parent_item,
			                                        ELM_GENLIST_ITEM_NONE, _mp_playlist_auto_playlist_select_cb,
			                                        plst);
		}
		elm_object_item_data_set(item_data->it, item_data);
	}
}

static void
_mp_playlist_append_user_playlists(void *thiz, Elm_Object_Item *parent_item)
{
	startfunc;
	gint count = -1;
	gint index = 0;
	int ret = 0;

	MpPlaylistList_t *plst = (MpPlaylistList_t *)thiz;
	mp_retm_if(!plst, "plst is null");

	mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, plst->type_str, plst->filter_str, &count);

	if (count < 0) {
		goto END;
	}

	if (plst->playlists_user) {
		mp_media_info_group_list_destroy(plst->playlists_user);
		plst->playlists_user = NULL;
	}

	ret = mp_media_info_group_list_create(&plst->playlists_user, MP_GROUP_BY_PLAYLIST, plst->type_str, plst->filter_str, 0, count);
	if (ret != 0) {
		DEBUG_TRACE("Fail to get items");
		goto END;
	}

	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		char *title = NULL;

		item = mp_media_info_group_list_nth_item(plst->playlists_user, index);
		if (!item) {
			DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
			goto END;
		}
		mp_media_info_group_get_main_info(item, &title);
		mp_list_item_data_t *item_data;
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->group_type = plst->group_type;
		item_data->index = index;

		if (MP_LIST_OBJ_IS_GENGRID(plst->genlist)) {
			item_data->it = elm_gengrid_item_append(plst->genlist, plst->gengrid_itc, item_data,
			                                        _mp_playlist_user_playlist_select_cb, (void *)plst);
		} else {
			item_data->it = elm_genlist_item_append(plst->genlist, plst->itc_auto,
			                                        item_data, parent_item,
			                                        ELM_GENLIST_ITEM_NONE, _mp_playlist_user_playlist_select_cb,
			                                        plst);
		}
		elm_object_item_data_set(item_data->it, item_data);

	}

END:
	endfunc;
}

static Elm_Object_Item *
_mp_playlist_append_group_index(void *thiz, int index, Elm_Genlist_Item_Class *itc_group_index)
{
	MpPlaylistList_t *list = (MpPlaylistList_t *)thiz;
	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->genlist);

	Elm_Object_Item *group_index = NULL;
	mp_list_item_data_t *item_data = mp_list_item_data_create(MP_LIST_ITEM_TYPE_GROUP_TITLE);
	MP_CHECK_NULL(item_data);
	item_data->index = index;
	item_data->it = elm_genlist_item_append(list->genlist, itc_group_index, item_data, NULL, ELM_GENLIST_ITEM_GROUP, NULL, NULL);
	group_index = item_data->it;
	elm_genlist_item_select_mode_set(group_index, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	list->group_index[index] = group_index;
	return group_index;
}

static void _mp_playlist_list_load_list(void *thiz, int count_auto, int count_user)
{
	MpPlaylistList_t *list = thiz;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	list->auto_playlist_count = 0;

	Elm_Object_Item *group_index = NULL;

	if (count_auto) {
		if (!list->edit_mode && !MP_LIST_OBJ_IS_GENGRID(list->genlist) && (ad->del_cb_invoked == 0)) {
			group_index = _mp_playlist_append_group_index(list, MP_PLAYLIST_GROUP_INDEX_DEFAULT, list->itc_group_index_default);
		}
		_mp_playlist_append_auto_playlists(list, group_index);
		group_index = NULL;
	}

	if (!list->edit_mode && !MP_LIST_OBJ_IS_GENGRID(list->genlist) && (ad->del_cb_invoked == 0)) {
		group_index = _mp_playlist_append_group_index(list, MP_PLAYLIST_GROUP_INDEX_NUM, list->itc_group_index_user);
	}

	if (count_user) {
		_mp_playlist_append_user_playlists(list, group_index);
	}

	/*if (!list->edit_mode)
		_mp_playlist_append_add_playlist(list, group_index);*/

	/*if (count_user)
		mp_list_bottom_counter_item_append((MpList_t *)list);*/

	endfunc;
}

void _mp_playlist_list_destory_cb(void *thiz)
{
	eventfunc;
	MpPlaylistList_t *list = thiz;
	MP_CHECK(list);

	if (list->playlists_auto) {
		mp_media_info_group_list_destroy(list->playlists_auto);
	}
	if (list->playlists_user) {
		mp_media_info_group_list_destroy(list->playlists_user);
	}

	if (list->itc_user) {
		elm_genlist_item_class_free(list->itc_user);
		list->itc_user = NULL;
	}
	if (list->itc_auto) {
		elm_genlist_item_class_free(list->itc_auto);
		list->itc_auto = NULL;
	}
	if (list->gengrid_add_itc) {
		elm_genlist_item_class_free(list->gengrid_add_itc);
		list->gengrid_add_itc = NULL;
	}
	if (list->gengrid_itc) {
		elm_genlist_item_class_free(list->gengrid_itc);
		list->gengrid_itc = NULL;
	}

	if (list->itc_group_index_default) {
		elm_genlist_item_class_free(list->itc_group_index_default);
		list->itc_group_index_default = NULL;
	}

	if (list->itc_group_index_user) {
		elm_genlist_item_class_free(list->itc_group_index_user);
		list->itc_group_index_user = NULL;
	}

	IF_FREE(list->type_str);
	IF_FREE(list->filter_str);

	free(list);
}

static void
_mp_playlist_list_gengrid_create(MpPlaylistList_t *list)
{
	startfunc;
	MP_CHECK(list);

	/*create new genlist*/
	mp_evas_object_del(list->genlist);

	list->genlist = elm_gengrid_add(list->box);
	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(list->genlist);
	MP_LIST_OBJ_SET_AS_GENGRID(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);

	if (!list->gengrid_itc) {
		list->gengrid_itc = elm_gengrid_item_class_new();
		MP_CHECK(list->gengrid_itc);
		list->gengrid_itc->func.text_get = _mp_playlist_list_label_get;
		list->gengrid_itc->func.content_get = _mp_playlist_list_icon_get;
		list->gengrid_itc->func.del = _mp_playlist_list_item_del_cb;
	}

	/*if (!list->gengrid_add_itc) {
		list->gengrid_add_itc = elm_gengrid_item_class_new();
		MP_CHECK(list->gengrid_add_itc);
		list->gengrid_add_itc->func.text_get = _mp_playlist_add_label_get;
		list->gengrid_add_itc->func.content_get = _mp_playlist_add_icon_get;
		list->gengrid_add_itc->func.del = NULL;
	}*/
	_mp_playlist_list_set_grid_style(list);
	evas_object_smart_callback_add(list->genlist, "longpressed", _mp_playlist_list_item_longpressed_cb, list);

	elm_gengrid_align_set(list->genlist, 0.5, 0.0);
	endfunc;
}

static char *
_mp_playlist_genlist_group_index_default_text_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);
	const char *text = NULL;
	if (!strcmp(part, "elm.text")) {
		if (item_data->index == 0) {
			text = STR_MP_DEFAULT_PLAYLIST_GROUP_TITLE;
		}
	}
	return g_strdup(GET_STR(text));
}

static char *
_mp_playlist_genlist_group_index_user_text_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK_NULL(item_data);
	const char *text = NULL;
	if (!strcmp(part, "elm.text")) {
		if (item_data->index != 0) {
			text = g_strdup(GET_STR(STR_MP_MY_PLAYLIST_GROUP_TITLE));
		}
	} else if (!strcmp(part, "elm.text.end")) {
		MpPlaylistList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);
		unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));
		if (item_data->index != 0) {
			if (count <= 0) {
				text = g_strdup(GET_STR(STR_MP_NO_PLAYLISTS));
			} else if (count == 1) {
				text = g_strdup(GET_STR(STR_MP_1_PLAYLIST));
			} else {
				text = g_strdup_printf(GET_STR(STR_MP_PD_PLAYLISTS), count);
			}
		}
	}
	return text;
}

static void
_mp_playlist_genlist_group_index_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);

	MpPlaylistList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK(list);

	list->group_index[item_data->index] = NULL;

	free(item_data);
}

static void
_mp_playlist_list_genlist_create(MpPlaylistList_t *list)
{
	MP_CHECK(list);

	/*create new genlist*/
	list->genlist = mp_widget_genlist_create(list->box);
	elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_homogeneous_set(list->genlist, EINA_TRUE);
	elm_genlist_mode_set(list->genlist, ELM_LIST_COMPRESS);
	evas_object_show(list->genlist);
	/*packet genlist to box*/
	elm_box_pack_end(list->box, list->genlist);

	//evas_object_smart_callback_add(list->genlist, "longpressed", _mp_playlist_list_item_longpressed_cb, list);
	evas_object_data_set(list->genlist, "list_data", list);

	/* current not used
	if (!list->itc_user)
		list->itc_user = elm_genlist_item_class_new();
	list->itc_user->item_style = "music/musiclist/2text.1icon";
	list->itc_user->func.text_get = _mp_playlist_list_label_get;
	list->itc_user->func.content_get = _mp_playlist_list_icon_get;
	list->itc_user->func.del = _mp_playlist_list_item_del_cb; */

	if (!list->itc_auto) {
		list->itc_auto = elm_genlist_item_class_new();
		MP_CHECK(list->itc_auto);
		list->itc_auto->item_style = "type1";
		list->itc_auto->func.text_get = _mp_playlist_list_label_get;
		list->itc_auto->func.content_get = _mp_playlist_list_icon_get;
		list->itc_auto->func.del = _mp_playlist_list_item_del_cb;
	}

	if (!list->itc_group_index_default) {
		list->itc_group_index_default = elm_genlist_item_class_new();
		MP_CHECK(list->itc_group_index_default);
		list->itc_group_index_default->item_style = "group_index";
		list->itc_group_index_default->func.text_get = _mp_playlist_genlist_group_index_default_text_get;
		list->itc_group_index_default->func.del = _mp_playlist_genlist_group_index_del_cb;
	}

	if (!list->itc_group_index_user) {
		list->itc_group_index_user = elm_genlist_item_class_new();
		MP_CHECK(list->itc_group_index_user);
		list->itc_group_index_user->item_style = "group_index";
		list->itc_group_index_user->func.text_get = _mp_playlist_genlist_group_index_user_text_get;
		list->itc_group_index_user->func.del = _mp_playlist_genlist_group_index_del_cb;
	}

	endfunc;
}


static void _mp_playlist_list_update(void *thiz)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	ad->del_cb_invoked = 0;
	int count_user = 0, count_auto = 0, res = 0, i, playlist_state = 0;
	MpPlaylistList_t *list = thiz;
	MP_CHECK(list);

	res = mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, NULL, list->filter_str, &count_user);
	MP_CHECK(res == 0);

	if (!list->edit_mode) {
		mp_setting_playlist_get_state(&playlist_state);
		for (i = 0; i < MP_SYS_PLST_COUNT; i++) {
			if (playlist_state & (1 << i)) {
				count_auto++;
			}
		}
	}

	mp_evas_object_del(list->no_content);
	mp_evas_object_del(list->genlist);

	//if (count_auto + count_user)
	//{
	if (list->display_mode == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		_mp_playlist_list_gengrid_create(list);
	} else {
		_mp_playlist_list_genlist_create(list);
	}
	evas_object_data_set(list->genlist, "list_handle", list);

	/* load list */
	_mp_playlist_list_load_list(thiz, count_auto, count_user);

	//}
	/*
	else
	{
		DEBUG_TRACE("count is 0");
		if (!list->no_content)
		{
			list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_PLAYLIST, NULL, NULL);
			elm_box_pack_end(list->box, list->no_content);
		}
	}
	*/

}

static unsigned int
_mp_playlist_list_get_count(void *thiz, MpListEditType_e type)
{
	MpPlaylistList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);

	int count = MP_LIST_OBJ_IS_GENGRID(list->genlist) ? elm_gengrid_items_count(list->genlist) : elm_genlist_items_count(list->genlist);

	int group_index_count = 0;
	int i = 0;
	while (i < MP_PLAYLIST_GROUP_INDEX_NUM) {
		if (list->group_index[i]) {
			++group_index_count;
		}

		++i;
	}

	count = count - list->auto_playlist_count - group_index_count;

	if (!list->edit_mode) {
		--count;    /// create playlist item
	}

	if (list->bottom_counter_item) {
		--count;
	}

	return count;
}

static void
_mp_playlist_list_set_grid_style(MpPlaylistList_t *list)
{
	bool landscape = mp_util_is_landscape();

	MP_CHECK(list->gengrid_itc);

	if (landscape) {
		list->gengrid_add_itc->item_style = list->gengrid_itc->item_style = "music/landscape/album_grid";
	} else {
		list->gengrid_add_itc->item_style = list->gengrid_itc->item_style = "music/album_grid2";
	}

	double scale = elm_config_scale_get();
	int w;
	int h;
	if (landscape) {
		w = (int)(ALBUM_GRID_LAND_W * scale);
		h = (int)(ALBUM_GRID_LAND_H * scale);
	} else {
		w = (int)(ALBUM_GRID_W * scale);
		h = (int)(ALBUM_GRID_H * scale);
	}
	elm_gengrid_item_size_set(list->genlist, w, h);
}

void _mp_playlist_list_rotate(void *thiz)
{
	MpPlaylistList_t * list = thiz;
	if (mp_list_get_display_mode((MpList_t *)list) == MP_LIST_DISPLAY_MODE_THUMBNAIL) {
		_mp_playlist_list_set_grid_style(list);
	}
}

static char * _mp_playlist_list_bottom_counter_text_get_cb(void *thiz)
{
	MpPlaylistList_t * list = thiz;
	MP_CHECK_NULL(list);

	char *text = NULL;

	unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));
	if (count == 1) {
		text = g_strdup(GET_STR(STR_MP_1_PLAYLIST));
	} else {
		text = g_strdup_printf(GET_STR(STR_MP_PD_PLAYLISTS), count);
	}
	return text;
}

MpPlaylistList_t * mp_playlist_list_create(Evas_Object *parent)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpPlaylistList_t *list = calloc(1, sizeof(MpPlaylistList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_PLAYLIST);

	list->get_count = _mp_playlist_list_get_count;
	list->update = _mp_playlist_list_update;
	list->destory_cb = _mp_playlist_list_destory_cb;
	list->set_edit = _mp_playlist_list_set_edit;
	list->get_group_type = _mp_playlist_list_get_group_type;
	list->rotate = _mp_playlist_list_rotate;
	list->group_type = MP_GROUP_BY_PLAYLIST;

	list->display_mode_changable = true;

	list->bottom_counter_text_get_cb = _mp_playlist_list_bottom_counter_text_get_cb;

	list->update(list);
	return list;
}

void mp_playlist_list_set_data(MpPlaylistList_t *list, ...)
{
	startfunc;
	MP_CHECK(list);

	va_list var_args;
	int field;



	va_start(var_args, list);
	do {
		field = va_arg(var_args, int);
		if (field < 0) {
			break;
		}

		switch (field) {
		case MP_PLAYLIST_LIST_TYPE: {
			int val = va_arg((var_args), int);

			list->group_type = val;
			DEBUG_TRACE("list->group_type = %d", list->group_type);
			break;
		}

		case MP_PLAYLIST_LIST_FUNC: {
			int val = va_arg((var_args), int);

			list->function_type = val;
			DEBUG_TRACE("list->function_type = %d", list->function_type);
			break;
		}

		case MP_PLAYLIST_LIST_TYPE_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->type_str);
			list->type_str = g_strdup(val);
			DEBUG_TRACE("list->type_str = %s", list->type_str);

			break;
		}
		case MP_PLAYLIST_LIST_FILTER_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->filter_str);
			list->filter_str = g_strdup(val);
			DEBUG_TRACE("list->filter_str = %s", list->filter_str);

			break;
		}
		case MP_PLAYLIST_LIST_DISPLAY_MODE: {
			int val = va_arg((var_args), int);
			list->display_mode = val;
			DEBUG_TRACE("list->display_mode = %d", list->display_mode);

			break;
		}

		default:
			DEBUG_TRACE("Invalid arguments");
		}

	} while (field);

	va_end(var_args);
}

void mp_playlist_list_copy_data(MpPlaylistList_t *src, MpPlaylistList_t *dest)
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
