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

#include "mp-edit-callback.h"
#include "mp-popup.h"
#include "mp-menu.h"
#include "mp-util.h"
#include "mp-widget.h"
#include "mp-ctxpopup.h"
#include "mp-create-playlist-view.h"
#include "mp-list.h"
#include "mp-define.h"
#include "mp-playlist-mgr.h"
#include "mp-player-mgr.h"
#include "mp-play.h"
#include "mp-playlist-detail-view.h"
#include "mp-edit-playlist.h"
#include "mp-common.h"
#include "mp-minicontroller.h"
#include "mp-lockscreenmini.h"
#include "mp-file-util.h"

#ifdef MP_FEATURE_PERSONAL_PAGE
#include <sys/stat.h>
#include <sys/statfs.h>
#include "mp-track-list.h"
#endif

typedef enum {
	MP_EDIT_THREAD_FEEDBACK_UNABLE_TO_ADD_PLST,
	MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION,
	MP_EDIT_THREAD_FEEDBACK_TRACK_DELETED,
#ifdef MP_FEATURE_PERSONAL_PAGE
	MP_EDIT_THREAD_FEEDBACK_TRACK_MOVE,
#endif
} mp_edit_thread_feedback_e;

#ifdef MP_FEATURE_PERSONAL_PAGE
static int g_total_count;
#endif

static Ecore_Thread *g_edit_thread;
static Ecore_Thread *g_delete_thread;
static Ecore_Thread *g_personal_storage_thread;
static int g_playlist_id;
static int g_playlist_track_count;
static GList *g_selected_list;
static MpListType_e g_list_type;
static int g_group_type;
static int g_track_type;
static int g_edit_operation;
static int g_error_count;
static int g_selected_count;
static int g_selected_tracklist_count;
static void *g_playlist_handle;
static char *g_playlist_name;

void
mp_edit_cb_excute_track_delete(void *data);

static void
_mp_edit_cb_create_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	evas_object_del(obj);
	int response = (int)event_info;
	if (response == MP_POPUP_NO) {
		DEBUG_TRACE("cancel btn click");
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_POPUP_CANCEL);
		return;
	}
	MP_CHECK(response);

	Mp_Playlist_Data *mp_playlist_data = mp_edit_playlist_create(MP_PLST_CREATE);
	MP_CHECK(mp_playlist_data);
	mp_edit_playlist_set_edit_list(mp_playlist_data, data);
	mp_edit_playlist_content_create(mp_playlist_data);

	endfunc;
}

void
_mp_edit_delete_track_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_evas_object_del(obj);
	ad->popup_delete = NULL;

	int response = (int)event_info;
	if (response == MP_POPUP_NO) {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_POPUP_CANCEL);
		return;
	}

	mp_edit_cb_excute_track_delete(data);

	endfunc;
	return;
}

void mp_edit_create_track_delete_popup(void *data)
{
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	char *title_txt = NULL;
	char *title = NULL;
	char *help_txt = NULL;
	title = STR_MP_DELETE;

	title_txt = g_strconcat("<align=center>", GET_STR(title), "</align>", NULL);
	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, title_txt, data, _mp_edit_delete_track_popup_response_cb, ad);
	ad->popup_delete = popup;
	//making help_txt
	char *markup = NULL;
	markup = g_strdup(GET_STR(STR_MP_ONE_TRACK_DETELED));
	help_txt = g_strconcat("<align=left>", markup, "</align>", NULL);
	mp_util_domain_translatable_text_set(popup, help_txt);
	IF_FREE(title_txt);
	IF_FREE(help_txt);
	IF_FREE(markup);

	mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_CANCEL, MP_POPUP_NO);
	mp_popup_button_set(popup, MP_POPUP_BTN_2, STR_MP_DELETE, MP_POPUP_YES);

	evas_object_show(popup);
}

static void
_mp_edit_cb_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *selected_item = event_info;
	Popup_genlist_item *gli_data = (Popup_genlist_item *)elm_object_item_data_get(selected_item);

	char *playlist_name;
	mp_media_info_h item = data;
	MP_CHECK(item);
	mp_media_info_group_get_playlist_id(item, &g_playlist_id);
	mp_media_info_group_get_main_info(item, &playlist_name);

	mp_popup_destroy(ad);
	mp_edit_cb_excute_add_to_playlist(gli_data->item_data, g_playlist_id, playlist_name, true);
}

static void _mp_edit_cb_popup_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	mp_media_list_h list = data;
	mp_media_info_group_list_destroy(list);
}

void
mp_edit_create_add_to_playlist_popup(void *data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	Evas_Object *popup = NULL;
	if (mp_list_get_checked_count(list) <= 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_ADD_TO_PLST, data, ad);
	mp_retm_if(!popup, "popup is NULL !!!");

	//mp_genlist_popup_item_append(popup, GET_STR("IDS_MUSIC_OPT_CREATE_PLAYLIST"), NULL, NULL, NULL, _mp_edit_cb_create_playlist_cb, data);
	mp_popup_response_callback_set(popup, _mp_edit_cb_create_playlist_cb, data);

	int i = 0, count = -1, ret = -1;

	ret = mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, NULL, NULL, &count);
	if (ret != 0) {
		ERROR_TRACE("Error in mp_media_info_group_list_count (%d)", ret);
		return;
	}

	if (count) {
		mp_media_list_h media_list = NULL;

		ret = mp_media_info_group_list_create(&media_list, MP_GROUP_BY_PLAYLIST, NULL, NULL, 0, count);
		if (ret != 0) {
			WARN_TRACE("Fail to get playlist");
			return;
		}
		for (i = 0; i < count; i++) {
			char *name = NULL;
			mp_media_info_h item = NULL;
			item = mp_media_info_group_list_nth_item(media_list, i);

			ret = mp_media_info_group_get_main_info(item, &name);
			mp_retm_if(ret != 0, "Fail to get value");
			mp_genlist_popup_item_append(popup, name, NULL, NULL, (void *)list, _mp_edit_cb_add_to_playlist_cb, (void *)item);
		}

		evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_edit_cb_popup_del_cb, media_list);
	} else {
		Elm_Object_Item *it = mp_genlist_popup_item_append(popup, GET_STR(STR_MP_NO_PLAYLISTS), NULL, NULL, NULL, NULL, ad);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	evas_object_show(popup);
}

static void
_mp_edit_progress_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MP_CHECK(obj);
	mp_evas_object_del(obj);

	if (g_edit_thread) {
		ecore_thread_cancel(g_edit_thread);
		g_edit_thread = NULL;
	}

	WARN_TRACE("EDIT progress has been completed. Now update views..");
	DEBUG_TRACE("selected_count, %d, error_count: %d", g_selected_count, g_error_count);

	if (g_edit_operation == MP_EDIT_ADD_TO_PLAYLIST) {
		/*mp_util_post_add_to_playlist_popup_message(g_selected_tracklist_count);*/

		mp_view_update(mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYLIST_DETAIL));

		mp_view_mgr_post_event(GET_VIEW_MGR, MP_ADD_TO_PLAYLIST_DONE);
	} else if (g_edit_operation == MP_EDIT_DELETE) {
		if ((g_selected_count == 1) && g_error_count) {
			mp_util_post_status_message(ad, GET_SYS_STR("IDS_COM_POP_FAILED"));
		}
		/* notification was removed from UX
		else
		{
			if (g_track_type > MP_TRACK_TYPE_PLAYLIST_MIN && g_track_type < MP_TRACK_TYPE_PLAYLIST_MAX)
				mp_util_post_status_message(ad, GET_SYS_STR(STR_MP_REMOVED));
			else
				mp_util_post_status_message(ad, GET_SYS_STR(STR_MP_DELETED));
		}
		*/
		if (g_group_type == MP_GROUP_BY_FOLDER) {
			DEBUG_TRACE("update all view");
			mp_view_update(mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ALL));
		}

		//set selection info && update buttons
		MpListView_t *view = (MpListView_t *)mp_view_mgr_get_top_view(GET_VIEW_MGR);
		MP_CHECK(view);
		bool list_view = false;
		mp_list_view_is_list_view(view, &list_view);
		if (list_view && mp_list_get_edit(view->content_to_show)) {
			mp_view_update_options_edit((MpView_t *)view);
			view->selection_info = mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count(view->content_to_show));
		}

		//if (mp_list_get_edit(view->content_to_show))
		{
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_DELETE_DONE);
		}
		/*
		else
		{
		        mp_view_mgr_post_event(GET_VIEW_MGR, MP_POPUP_DELETE_DONE);
		}*/
	}

	ad->edit_in_progress = false;

}

static void
_mp_edit_cb_add_to_plst_thread(void *data, Ecore_Thread *thread)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_list_item_data_t *item = NULL;
	mp_media_info_h item_handle = NULL;
	mp_media_list_h group_track_handle = NULL;
	int ret = 0;
	g_selected_tracklist_count = 0;

	mp_media_info_connect();

	GList *node = g_list_last(g_selected_list);
	MP_CHECK_EXCEP(node);

	mp_playlist_h playlist_h = NULL;
	mp_media_info_playlist_handle_create(&playlist_h, g_playlist_id);

	char *title = NULL;
	while (node) {
		if (ecore_thread_check(thread)) {	// pending cancellation
			WARN_TRACE("popup cancel clicked");
			goto mp_exception;
		}

		char *fid = NULL;
		char *thumbnail_path = NULL;

		item = node->data;
		node = g_list_previous(node);
		if (!item) {
			WARN_TRACE("CHECK here...");
			ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
			goto mp_exception;
		}
		item_handle = (item->handle);
		if (!item_handle) {
			continue;
		}

		switch (g_list_type) {
		case MP_LIST_TYPE_TRACK:
		case MP_LIST_TYPE_ARTIST_DETAIL:
		case MP_LIST_TYPE_ALBUM_DETAIL:
		case MP_LIST_TYPE_ALL: {
			ret = mp_media_info_get_media_id(item_handle,  &fid);
			if (ret != 0) {
				ERROR_TRACE("CRITICAL ERROR ## CHECK BELOW ITEM");

				goto mp_exception;
			}

			int res = 0;
			if (g_playlist_id == -1) { /*favoriate playlist*/
				res = mp_media_info_set_favorite(item_handle, true);

			} else {
				mp_media_info_get_thumbnail_path(item_handle, &thumbnail_path);
				res = mp_media_info_playlist_add_item(playlist_h, fid, thumbnail_path);
			}
			if (res) {
				WARN_TRACE("");
				ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_UNABLE_TO_ADD_PLST);
				ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
				goto mp_exception;
			} else {
				g_playlist_track_count++;
			}

#ifdef MP_PLAYLIST_MAX_ITEM_COUNT
			if (g_playlist_track_count >= MP_PLAYLIST_MAX_ITEM_COUNT) {
				DEBUG_TRACE("unable to add more tracks...");
				//ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
				goto mp_exception;
			}
#endif
			g_selected_tracklist_count += 1;

			break;
		}
		case MP_LIST_TYPE_GROUP: {
			mp_track_type_e item_type = MP_TRACK_ALL;
			int count = 0, i;
			mp_media_info_h item = NULL;

			if (g_group_type == MP_GROUP_BY_FOLDER) {
				ret = mp_media_info_group_get_folder_id(item_handle, &title);
			} else {
				ret = mp_media_info_group_get_main_info(item_handle, &title);
			}
			MP_CHECK_EXCEP(ret == 0);

			item_type = mp_menu_get_track_type_by_group(g_group_type);
			ret = mp_media_info_list_count(item_type, title, NULL, NULL, 0, &count);
			MP_CHECK_EXCEP(ret == 0);
			if (group_track_handle) {
				mp_media_info_list_destroy(group_track_handle);
				group_track_handle = NULL;
			}
			ret = mp_media_info_list_create(&group_track_handle, item_type, title, NULL, NULL, 0, 0, count);
			MP_CHECK_EXCEP(ret == 0);

			for (i = 0; i < count; i++) {
				char *fid = NULL;
				char *thumbnail_path = NULL;
				item = mp_media_info_list_nth_item(group_track_handle, i);
				ret = mp_media_info_get_media_id(item, &fid);
				MP_CHECK_EXCEP(ret == 0);

				int res = 0;
				if (g_playlist_id == -1) { /*favoriate playlist*/
					res = mp_media_info_set_favorite(item, true);
				} else {
					mp_media_info_get_thumbnail_path(item, &thumbnail_path);
					res = mp_media_info_playlist_add_item(playlist_h, fid, thumbnail_path);
				}
				if (res) {
					WARN_TRACE("");
					ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_UNABLE_TO_ADD_PLST);
					ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
					goto mp_exception;
				} else {
					g_playlist_track_count++;
				}

#ifdef MP_PLAYLIST_MAX_ITEM_COUNT
				if (g_playlist_track_count >= MP_PLAYLIST_MAX_ITEM_COUNT) {
					DEBUG_TRACE("unable to add more tracks...");
					ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
					goto mp_exception;
				}
#endif
			}
			g_selected_tracklist_count += count;
			break;
		}
		default:
			ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
			WARN_TRACE("unexpected case... [%d]", g_list_type);
			goto mp_exception;
		}
		IF_FREE(title);
	}
mp_exception:
	mp_media_info_playlist_handle_destroy(playlist_h);

	if (group_track_handle) {
		mp_media_info_list_destroy(group_track_handle);
		group_track_handle = NULL;
	}
	mp_media_info_disconnect();
	IF_FREE(title);

	//To make progress popup visible if only one item deleted.
	sleep(1);
}

inline static int
_delete_track(mp_media_info_h item_handle)
{
	int ret = 0;
	char *uri = NULL, *fid = NULL;

	MP_CHECK_VAL(item_handle, -1);

	switch (g_track_type) {
	case MP_TRACK_BY_PLAYLIST: {
		int member_id = 0;
		ret = mp_media_info_get_playlist_member_id(item_handle, &member_id);
		MP_CHECK_VAL(ret == 0, -1);
		ret = mp_media_info_playlist_remove_media(g_playlist_handle, member_id);
		MP_CHECK_VAL(ret == 0, -1);

		mp_common_playlist_album_update(g_playlist_handle);

		break;
	}
	case MP_TRACK_BY_ADDED_TIME: {
		ret = mp_media_info_set_added_time(item_handle, 0);
		MP_CHECK_VAL(ret == 0, -1);
		break;
	}
	case MP_TRACK_BY_PLAYED_TIME: {
		ret = mp_media_info_set_played_time(item_handle, 0);
		MP_CHECK_VAL(ret == 0, -1);
		break;
	}
	case MP_TRACK_BY_FAVORITE: {
		ret = mp_media_info_set_favorite(item_handle, 0);
		MP_CHECK_VAL(ret == 0, -1);
		break;
	}
	case MP_TRACK_BY_PLAYED_COUNT: {
		ret = mp_media_info_set_played_count(item_handle, 0);
		MP_CHECK_VAL(ret == 0, -1);
		break;
	}
	default: {
#ifdef MP_FEATURE_CLOUD
		mp_storage_type_e storage;
		ret = mp_media_info_get_media_id(item_handle,  &fid);
		ret = mp_media_info_get_storage_type(item_handle, &storage);
		if (storage == MP_STORAGE_CLOUD) {
			return mp_cloud_delete_content(fid, true);
		} else {
			ret = mp_media_info_get_file_path(item_handle, &uri);
			MP_CHECK_VAL(ret == 0, -1);

			if (mp_util_delete_track(NULL, fid, uri) != MP_FILE_DELETE_ERR_NONE) {
				DEBUG_TRACE("Fail to delete item, fid: %d, path: %s", fid, uri);
				return -1;
			}
		}
#else
		ret = mp_media_info_get_media_id(item_handle,  &fid);
		ret = mp_media_info_get_file_path(item_handle, &uri);
		MP_CHECK_VAL(ret == 0, -1);

		if (mp_util_delete_track(NULL, fid, uri) != MP_FILE_DELETE_ERR_NONE) {
			DEBUG_TRACE("Fail to delete item, fid: %d, path: %s", fid, uri);
			return -1;
		}
#endif
		ecore_thread_feedback(g_edit_thread, (void *)MP_EDIT_THREAD_FEEDBACK_TRACK_DELETED);
		break;
	}
	}
	return 0;
}

inline static int
_delete_playlist(mp_media_info_h item_handle)
{
	int ret = 0;
	int plst_id;
	ret = mp_media_info_group_get_playlist_id(item_handle, &plst_id);
	ret = mp_media_info_playlist_delete_from_db(plst_id);
	MP_CHECK_VAL(ret == 0, -1);

	return 0;
}

inline static int
_delete_group(mp_media_info_h item_handle)
{
	int ret = 0;
	mp_track_type_e item_type = MP_TRACK_ALL;
	int count = 0, i;
	mp_media_info_h item = NULL;
	char *title = NULL, *uri = NULL, *fid = NULL;

	mp_media_list_h group_track_handle = NULL;

	if (g_group_type == MP_GROUP_BY_FOLDER) {
		ret = mp_media_info_group_get_folder_id(item_handle, &title);
	} else {
		ret = mp_media_info_group_get_main_info(item_handle, &title);
	}
	if (ret != 0) {
		IF_FREE(title);
		return -1;
	}

	item_type = mp_menu_get_track_type_by_group(g_group_type);
	ret = mp_media_info_list_count(item_type, title, NULL, NULL, 0, &count);
	if (ret != 0) {
		IF_FREE(title);
		return -1;
	}

	ret = mp_media_info_list_create(&group_track_handle, item_type, title, NULL, NULL, 0, 0, count);
	if (ret != 0) {
		IF_FREE(title);
		return -1;
	}

	for (i = 0; i < count; i++) {
		item = mp_media_info_list_nth_item(group_track_handle, i);
		mp_media_info_get_media_id(item, &fid);
		mp_media_info_get_file_path(item, &uri);
		if (mp_util_delete_track(NULL, fid, uri) != MP_FILE_DELETE_ERR_NONE) {
			WARN_TRACE("Fail to delete group");
			ret = -1;
		}
	}

	IF_FREE(title);
	ecore_thread_feedback(g_edit_thread, (void *)MP_EDIT_THREAD_FEEDBACK_TRACK_DELETED);
	//mp_view_mgr_post_event(GET_VIEW_MGR, MP_DELETE_DONE);
	mp_media_info_list_destroy(group_track_handle);
	return ret;
}


static void
_mp_edit_cb_delete_thread(void *data, Ecore_Thread *thread)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_list_item_data_t *item = NULL;
	mp_media_info_h item_handle = NULL;

	DEBUG_TRACE("g_list_type: %d, track_type: %d, g_group_type: %d", g_list_type, g_track_type, g_group_type);

	mp_media_info_connect();

	GList *node = g_list_last(g_selected_list);
	MP_CHECK_EXCEP(node);
	while (node) {
		if (ecore_thread_check(thread)) {	// pending cancellation
			WARN_TRACE("popup cancel clicked");
			goto mp_exception;
		}

		item = node->data;
		node = g_list_previous(node);
		if (!item) {
			WARN_TRACE("CHECK here...");
			ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
			goto mp_exception;
		}
		item_handle = (item->handle);
		if (!item_handle) {
			continue;
		}

		switch (g_list_type) {
		case MP_LIST_TYPE_TRACK:
		case MP_LIST_TYPE_ALBUM_DETAIL:
		case MP_LIST_TYPE_ARTIST_DETAIL:
		case MP_LIST_TYPE_ALL: {
			if (_delete_track(item_handle)) {
				g_error_count++;
			} else {
				ecore_thread_feedback(thread, item->it);
			}
			break;
		}
		case MP_LIST_TYPE_PLAYLIST: {
			if (!_delete_playlist(item_handle)) {
				ecore_thread_feedback(thread, item->it);
			}
			break;
		}
		case MP_LIST_TYPE_GROUP: {
			if (!_delete_group(item_handle)) {
				ecore_thread_feedback(thread, item->it);
			}
			break;
		}
		default:
			ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
			WARN_TRACE("unexpected case...");
			goto mp_exception;
		}
	}
mp_exception:

	//To make progress popup visible if only one item deleted.
	sleep(1);
	mp_media_info_disconnect();
}

static void _mp_edit_cb_check_playlist()
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	bool current_removed = false;
	bool next_play = false;

	if (mp_player_mgr_get_state() == PLAYER_STATE_PLAYING) {
		next_play = true;
	}

	mp_playlist_mgr_check_existance_and_refresh(ad->playlist_mgr, &current_removed);
	if (current_removed) {
		mp_play_destory(ad);
		if (mp_playlist_mgr_count(ad->playlist_mgr) == 0) {
			if (ad->current_track_info) {
				mp_util_free_track_info(ad->current_track_info);
				ad->current_track_info = NULL;
			}
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_UNSET_NOW_PLAYING);
			if (ad->b_minicontroller_show) {
				mp_minicontroller_hide(ad);
			}
#ifdef MP_FEATURE_LOCKSCREEN
			if (ad->b_lockmini_show) {
				mp_lockscreenmini_hide(ad);
			}
#endif

			char *data_path = app_get_data_path();
			/*as all the items are removed, remove now-playing.ini to avoid copy the same track but in DB, they are different*/
			char nowplaying_ini[1024] = {0};
			snprintf(nowplaying_ini, 1024, "%s%s", data_path, MP_NOWPLAYING_INI_FILE_NAME);
			mp_file_remove(nowplaying_ini);
			/* remove playing_track.ini to avoid lockscreen still using the file content*/

			char playing_ini[1024] = {0};
#ifndef MP_SOUND_PLAYER
			snprintf(playing_ini, 1024, "%s%s", data_path, MP_PLAYING_INI_FILE_NAME_MUSIC);
			free(data_path);
			mp_file_remove(playing_ini);
#else
			snprintf(playing_ini, 1024, "%s%s", data_path, MP_PLAYING_INI_FILE_NAME_SOUND);
			free(data_path);
			mp_file_remove(playing_ini);
#endif
		} else if (next_play) {
			mp_play_new_file(ad, true);
		}
	}
}


static void
_mp_edit_cb_edit_thread_notify_cb(void *data, Ecore_Thread *thread, void *msg_data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_edit_thread_feedback_e feedback = (mp_edit_thread_feedback_e)msg_data;
	switch (feedback) {
	case MP_EDIT_THREAD_FEEDBACK_UNABLE_TO_ADD_PLST:
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_TO_ADD"));
		break;

	case MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION:
		mp_popup_response(ad->popup[MP_POPUP_PROGRESS], MP_POPUP_NO);
		break;

	case MP_EDIT_THREAD_FEEDBACK_TRACK_DELETED: {
		_mp_edit_cb_check_playlist();
	}
	break;

	default:
		DEBUG_TRACE("delete genlist item");
		if (g_track_type == MP_TRACK_BY_PLAYLIST && g_playlist_handle && ad->playlist_mgr) {
			int item_playlist_id = 0;
			mp_media_info_group_get_playlist_id(g_playlist_handle, &item_playlist_id);
			int current_playlist_id = mp_playlist_mgr_get_playlist_id(ad->playlist_mgr);
			if (current_playlist_id && current_playlist_id == item_playlist_id) {
				mp_list_item_data_t *item_data = elm_object_item_data_get(msg_data);
				if (item_data && item_data->handle) {
					int member_id = 0;
					mp_media_info_get_playlist_member_id(item_data->handle, &member_id);
					mp_debug("item playlist_id = %d, member_id = %d", item_playlist_id, member_id);
					mp_plst_item *plst_item = mp_playlist_mgr_get_item_by_playlist_memeber_id(ad->playlist_mgr, member_id);
					if (plst_item) {
						if (plst_item == mp_playlist_mgr_get_current(ad->playlist_mgr)) {
							mp_play_destory(ad);
							mp_playlist_mgr_item_remove_item(ad->playlist_mgr, plst_item);
							mp_play_new_file(ad, true);
						} else {
							mp_playlist_mgr_item_remove_item(ad->playlist_mgr, plst_item);
						}
					}
				}
			}
		}
		elm_object_item_del(msg_data);
		break;
	}
}

/*
** use idle to delete the pervious playlist detail view
** don't need to record the idler handle, it will be called only once
** send event in idle to avoid update routine in the previous playlist detail view which will be deleted
*/
static Eina_Bool _del_old_playlist_detail_view_cb(void *data)
{
	startfunc;
	MpView_t *view = (MpView_t *)data;
	if (view) {
		elm_object_item_del(view->navi_it); //elm_naviframe_item_pop does not work
		//mp_view_mgr_pop_a_view(GET_VIEW_MGR, view);
	}

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_ADD_TO_PLAYLIST_DONE);
	return FALSE;
}

static void
_mp_edit_cb_edit_thread_end_cb(void *data, Ecore_Thread *thread)
{
	WARN_TRACE("thread_end");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	int playlist_id = g_playlist_id;

	g_edit_thread = NULL;
	g_playlist_id = 0;
	g_playlist_track_count = 0;
	g_list_free(g_selected_list);

	_mp_edit_cb_check_playlist();

	if (g_edit_operation == MP_EDIT_ADD_TO_PLAYLIST && g_playlist_name) {
		/*keep previous playlist detail view, which to be popped after new view pushed*/
		MpView_t *previous_view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYLIST_DETAIL);
		MpView_t *view = NULL;
		if (playlist_id == -1) { /*favoriate playlist*/
			view = (MpView_t *)mp_playlist_detail_view_create(GET_NAVIFRAME,
			        MP_TRACK_BY_FAVORITE, g_playlist_name, playlist_id);
		} else {
			view = (MpView_t *)mp_playlist_detail_view_create(GET_NAVIFRAME,
			        MP_TRACK_BY_PLAYLIST, g_playlist_name, playlist_id);
		}
		mp_view_mgr_push_view(GET_VIEW_MGR, view, NULL);
		mp_view_update_options(view);
		mp_view_set_title(view, g_playlist_name);

		IF_FREE(g_playlist_name);
		/*
		**in this case, only need to delete popup and send MP_ADD_TO_PLAYLIST_DONE,
		**don't need to update the view to be popped
		*/
		mp_evas_object_del(ad->popup[MP_POPUP_PROGRESS]);
		/*idler is used to delete old playlist detail view to avoid blink*/
		ecore_idler_add(_del_old_playlist_detail_view_cb, previous_view);
	} else {
		mp_popup_response(ad->popup[MP_POPUP_PROGRESS], MP_POPUP_YES);
	}
}

static void
_mp_edit_cb_edit_thread_cancel_cb(void *data, Ecore_Thread *thread)
{
	WARN_TRACE("thread_cancel");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	g_edit_thread = NULL;
	g_list_free(g_selected_list);

	_mp_edit_cb_check_playlist();

	mp_evas_object_del(ad->popup[MP_POPUP_PROGRESS]);

	mp_view_mgr_delete_view(GET_VIEW_MGR, MP_VIEW_EDIT);

	/*update top view*/
	MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
	MP_CHECK(top_view);
	mp_view_update(top_view);

	if (top_view->view_type != MP_VIEW_ALL) {
		mp_view_update(GET_ALL_VIEW);
	}

	if (top_view->view_type == MP_VIEW_FOLDER_DETAIL) {
		//update folder view
		MpView_t *folder_view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_FOLDER);
		MP_CHECK(folder_view);
		mp_view_update(folder_view);
	}
}


void
mp_edit_cb_excute_add_to_playlist(void *data, int playlist_id, char *playlist_name, bool selected)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	if (selected) {
		mp_list_selected_item_data_get(list, &g_selected_list);
	} else {
		mp_list_all_item_data_get(list, &g_selected_list);
	}
	MP_CHECK(g_selected_list);
	g_list_type = list->list_type;
	g_group_type = mp_list_get_group_type(list);
	g_selected_count = g_list_length(g_selected_list);
	g_error_count = 0;
	g_edit_operation = MP_EDIT_ADD_TO_PLAYLIST;
	g_playlist_id = playlist_id;
	IF_FREE(g_playlist_name);
	g_playlist_name = g_strdup(playlist_name);
	DEBUG_TRACE("playlist name = %s", g_playlist_name);

#ifdef MP_PLAYLIST_MAX_ITEM_COUNT
	mp_media_info_list_count(MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, g_playlist_id, &g_playlist_track_count);
	DEBUG_TRACE("number of tracks in playlist: %d", g_playlist_track_count);
	if (g_playlist_track_count >= MP_PLAYLIST_MAX_ITEM_COUNT) {
		char *fmt_str = GET_STR("IDS_MUSIC_POP_UNABLE_TO_ADD_MORE_THAN_PD_MUSIC_FILE");
		char *noti_str = g_strdup_printf(fmt_str, MP_PLAYLIST_MAX_ITEM_COUNT);
		mp_util_post_status_message(ad, noti_str);
		IF_FREE(noti_str);
		return;
	}
#endif

	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_PROGRESS, GET_STR("IDS_MUSIC_BODY_ADD_TO_PLAYLIST"), list,
	                                     _mp_edit_progress_popup_response_cb, ad);
	evas_object_show(popup);

	g_edit_thread = ecore_thread_feedback_run(
	                    _mp_edit_cb_add_to_plst_thread,
	                    _mp_edit_cb_edit_thread_notify_cb,
	                    _mp_edit_cb_edit_thread_end_cb,
	                    _mp_edit_cb_edit_thread_cancel_cb,
	                    (const void *)g_selected_list,
	                    EINA_TRUE);

	if (!g_edit_thread) {
		mp_popup_response(ad->popup[MP_POPUP_PROGRESS], MP_POPUP_NO);
	}
	ad->edit_in_progress = true;
}

#ifndef MP_SOUND_PLAYER
void *mp_edit_get_delete_thread()
{
	return g_edit_thread;
}
#endif

#ifdef MP_FEATURE_CLOUD

static void
_mp_edit_cb_offline_thread(void *data, Ecore_Thread *thread)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_list_item_data_t *item = NULL;
	mp_media_info_h item_handle = NULL;
	int ret = 0;
	char *fid = NULL;

	mp_media_info_connect();

	GList *node = g_list_last(g_selected_list);
	MP_CHECK_EXCEP(node);
	while (node) {
		if (ecore_thread_check(thread)) {	// pending cancellation
			WARN_TRACE("popup cancel clicked");
			goto mp_exception;
		}

		item = node->data;
		node = g_list_previous(node);
		if (!item) {
			WARN_TRACE("CHECK here...");
			ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
			goto mp_exception;
		}
		item_handle = (item->handle);
		if (!item_handle) {
			continue;
		}

		switch (g_list_type) {
		case MP_LIST_TYPE_TRACK:
		case MP_LIST_TYPE_ALBUM_DETAIL:
		case MP_LIST_TYPE_ARTIST_DETAIL: {
			ret = mp_media_info_get_media_id(item_handle,  &fid);
			MP_CHECK_EXCEP(ret == 0);

			ret = mp_cloud_make_offline_available(fid);
			MP_CHECK_EXCEP(ret == 0);

			break;
		}
		default:
			ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_CANCEL_BY_EXCEPTION);
			WARN_TRACE("unexpected case...");
			goto mp_exception;
		}
	}
mp_exception:
	mp_media_info_disconnect();
}

void
mp_edit_cb_excute_make_offline_available(void *data)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_selected_item_data_get(list, &g_selected_list);
	MP_CHECK(g_selected_list);
	g_list_type = list->list_type;
	g_group_type = mp_list_get_group_type(list);
	g_selected_count = g_list_length(g_selected_list);
	g_error_count = 0;
	g_edit_operation = MP_EDIT_ADD_TO_PLAYLIST;

	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_PROGRESS, GET_STR("IDS_MUSIC_BODY_ADD_TO_PLAYLIST"), list,
	                                     _mp_edit_progress_popup_response_cb, ad);
	evas_object_show(popup);

	g_edit_thread = ecore_thread_feedback_run(
	                    _mp_edit_cb_offline_thread,
	                    _mp_edit_cb_edit_thread_notify_cb,
	                    _mp_edit_cb_edit_thread_end_cb,
	                    _mp_edit_cb_edit_thread_cancel_cb,
	                    (const void *)g_selected_list,
	                    EINA_TRUE);

	if (!g_edit_thread) {
		mp_popup_response(ad->popup[MP_POPUP_PROGRESS], MP_POPUP_NO);
	}
	ad->edit_in_progress = true;
}
#endif

void
mp_edit_cb_excute_delete(void *data)
{
	startfunc;
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_selected_item_data_get(list, &g_selected_list);
	MP_CHECK(g_selected_list);

	g_list_type = list->list_type;
	g_group_type = mp_list_get_group_type(list);
	g_selected_count = g_list_length(g_selected_list);
	g_error_count = 0;
	g_edit_operation = MP_EDIT_DELETE;
	g_track_type = mp_list_get_track_type(list);
	g_playlist_handle = mp_list_get_playlist_handle(list);

	char *title = NULL;
	mp_track_type_e type = mp_list_get_track_type(list);
	if (type > MP_TRACK_TYPE_PLAYLIST_MIN && type < MP_TRACK_TYPE_PLAYLIST_MAX) {
		title = STR_MP_REMOVING;
	} else {
		title = MP_POPUP_DELETING;
	}

	Evas_Object *popup = mp_popup_message_create(ad->win_main, MP_POPUP_PROGRESS, NULL, title, list,
	                     _mp_edit_progress_popup_response_cb, ad);
	evas_object_show(popup);

	g_edit_thread = ecore_thread_feedback_run(
	                    _mp_edit_cb_delete_thread,
	                    _mp_edit_cb_edit_thread_notify_cb,
	                    _mp_edit_cb_edit_thread_end_cb,
	                    _mp_edit_cb_edit_thread_cancel_cb,
	                    (const void *)g_selected_list,
	                    EINA_TRUE);

	if (!g_edit_thread) {
		mp_popup_response(ad->popup[MP_POPUP_PROGRESS], MP_POPUP_NO);
	}
	ad->edit_in_progress = true;

}

static void
_mp_edit_cb_delete_track_thread(void *data, Ecore_Thread *thread)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);

	mp_media_info_connect();
	if (mp_util_delete_track(NULL, item->uid, item->uri) != MP_FILE_DELETE_ERR_NONE) {
		DEBUG_TRACE("Fail to delete item, fid: %d, path: %s", item->uid, item->uri);
	}

	ecore_thread_feedback(thread, (void *)MP_EDIT_THREAD_FEEDBACK_TRACK_DELETED);

	//To make progress popup visible if only one item deleted.
//	usleep(1000000);
	mp_media_info_disconnect();
}

static void
_mp_edit_cb_delete_track_thread_end_cb(void *data, Ecore_Thread *thread)
{
	WARN_TRACE("thread_end");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	g_delete_thread = NULL;
	mp_popup_response(ad->popup[MP_POPUP_PROGRESS], MP_POPUP_YES);
}

static void
_mp_edit_cb_delete_track_thread_notify_cb(void *data, Ecore_Thread *thread, void *msg_data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_edit_thread_feedback_e feedback = (mp_edit_thread_feedback_e)msg_data;

	switch (feedback) {

	case MP_EDIT_THREAD_FEEDBACK_TRACK_DELETED:
		DEBUG_TRACE("delete track in notify");
		mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
		MP_CHECK(item);

		mp_playlist_mgr_item_remove_item(ad->playlist_mgr, item);
		/*
		when the playlist has same track,then delete the track,
		the playlist should remove the deleted track
		*/
		mp_playlist_mgr_item_remove_deleted_item(ad->playlist_mgr);
		mp_play_destory(ad);

		//mp_play_new_file(ad, true);
		break;
	default:
		break;
	}
}

static void
_mp_edit_cb_delete_track_thread_cancel_cb(void *data, Ecore_Thread *thread)
{
	WARN_TRACE("thread_cancel");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	g_delete_thread = NULL;

	mp_evas_object_del(ad->popup[MP_POPUP_PROGRESS]);
}

static void
_mp_edit_message_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MP_CHECK(obj);
	mp_evas_object_del(ad->popup[MP_POPUP_PROGRESS]);

	if (g_delete_thread) {
		ecore_thread_cancel(g_delete_thread);
		g_delete_thread = NULL;
	}

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_DELETE_DONE);
	ad->edit_in_progress = false;

	mp_play_new_file(ad, true);
	_mp_edit_cb_check_playlist();

}

void
mp_edit_cb_excute_track_delete(void *data)
{
	startfunc;
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	char *title = NULL;
	title = MP_POPUP_DELETING;

	Evas_Object *popup = mp_popup_message_create(ad->win_main, MP_POPUP_PROGRESS, NULL, title, NULL,
	                     _mp_edit_message_popup_response_cb, ad);
	elm_popup_align_set(popup, 0.5, 0.99);
	evas_object_show(popup);

	g_delete_thread = ecore_thread_feedback_run(
	                      _mp_edit_cb_delete_track_thread,
	                      _mp_edit_cb_delete_track_thread_notify_cb,
	                      _mp_edit_cb_delete_track_thread_end_cb,
	                      _mp_edit_cb_delete_track_thread_cancel_cb,
	                      NULL,
	                      EINA_TRUE);

	if (!g_delete_thread) {
		mp_popup_response(ad->popup[MP_POPUP_PROGRESS], MP_POPUP_NO);
	}
	ad->edit_in_progress = true;

}

#ifdef MP_FEATURE_PERSONAL_PAGE
int _mp_edit_cb_get_item_size(const char *item, unsigned long long *size)
{
	struct stat info;
	if (!item || !size) {
		return -1;
	}
	*size = 0;
	if (stat(item, &info)) {
		ERROR_TRACE("Fail to stat item : %s", item);
		return -1;
	}

	if (S_ISREG(info.st_mode)) {
		*size = (unsigned long long)info.st_size;
	}
	return 0;
}

int _mp_edit_cb_get_remain_space(const char *path, unsigned long long *size)
{
	struct statfs dst_fs;

	if (!path || !size) {
		return -1;
	}

	if (statfs(path, &dst_fs) == 0) {
		*size = ((unsigned long long)(dst_fs.f_bsize) * (unsigned long long)(dst_fs.f_bavail));
	}
	return 0;
}

/*
@return:
-1,	error case
0,	Phone storage
1,	MMC storage
2,	personal storage
*/
static int _mp_edit_cb_get_store_type_by_full(const char *filepath)
{
	if (filepath == NULL) {
		return -1;
	}

	if (strncmp(filepath, MP_MUSIC_DIR, strlen(MP_MUSIC_DIR)) == 0) {
		return 0;
	} else if (strncmp(filepath, MP_MMC_DIR, strlen(MP_MMC_DIR)) == 0) {
		return 1;
	} else if (strncmp(filepath, MP_PERSONAL_PAGE_DIR, strlen(MP_PERSONAL_PAGE_DIR)) == 0) {
		return 2;
	} else {
		return -1;
	}
}

static int _mp_edit_cb_get_root_path_by_full(const char *full_path, char **path)
{
	assert(full_path);
	assert(path);
	int store_type = 0;

	store_type = _mp_edit_cb_get_store_type_by_full(full_path);

	switch (store_type) {
	case 0:
		*path = g_strdup(MP_MUSIC_DIR);
		break;
	case 1:
		*path = g_strdup(MP_MMC_DIR);
		break;
	case 2:
		*path = g_strdup(MP_PERSONAL_PAGE_DIR);
		break;
	default:
		*path = g_strdup(full_path);
		return -1;
	}
	return 0;
}

static int _mp_edit_cb_get_logical_path_by_full(const char *full_path, char **path)
{
	assert(full_path);
	assert(path);
	int store_type = 0;
	int root_len = 0;

	store_type = _mp_edit_cb_get_store_type_by_full(full_path);

	*path = g_strdup(full_path);
	if (*path == NULL) {
		return -1;
	}

	memset(*path, 0, strlen(*path));
	switch (store_type) {
	case 0:
		root_len = strlen(MP_MUSIC_DIR);
		break;
	case 1:
		root_len = strlen(MP_MMC_DIR);
		break;
	case 2:
		root_len = strlen(MP_PERSONAL_PAGE_DIR);
		break;
	default:
		return -1;
	}

	/*
	**	*path has the same length with full_path
	**	strlen(*path) is 0 since the memset called
	**	we use length of full_path to reprecent the *path's
	*/
	g_strlcpy(*path, full_path + root_len, strlen(full_path));
	if (strlen(*path) == 0) {
		IF_FREE(*path);
		*path = g_strdup("/");
	}

	return 0;
}

static void _mp_edit_cb_delete_empty_dir(const char *full_path, const char *root)
{
	MP_CHECK(full_path);
	char* path = NULL;
	while (full_path && g_strcmp0(full_path, root)) {
		if (mp_file_dir_is_empty(full_path)) {
			/*if not, delete the folder*/
			mp_file_recursive_rm(full_path);
			path = g_strrstr(full_path, "/");
			if (path != NULL) {
				*path = '\0';
			}
		} else {
			break;
		}
	}
	return;
}

inline static int
_move_track_spec_path(mp_media_info_h item_handle, char *dest_path)
{
	MP_CHECK_VAL(item_handle, -1);

	char *path = NULL;
	mp_media_info_get_file_path(item_handle, &path);

	char *filename = NULL;
	filename = g_strdup((char *)mp_file_file_get(path));
	char *dest = NULL;
	char *dest_root_path = dest_path;
	if (mp_util_is_in_personal_page((const char *)path)) {
		if (dest_root_path == NULL) {
			dest_root_path = MP_MUSIC_DIR;
		}

		/*if dest_root_path does not exist, create it*/
		bool mkdir_ret = mp_file_mkpath(dest_root_path);
		if (mkdir_ret == false) {
			DEBUG_TRACE("failed to make new directory");
		}

		char *unique_filename = NULL;
		while (mp_util_is_duplicated_name(dest_root_path, (const char *)filename)) {
			IF_FREE(unique_filename);
			mp_util_get_unique_name((char *)filename, &unique_filename);
			IF_FREE(filename);
			filename = g_strdup(unique_filename);
		}
		/*remove from personal page*/
		dest = g_strconcat(dest_root_path, "/", filename, NULL);
		if (dest == NULL) {
			IF_FREE(unique_filename);
			IF_FREE(filename);
			return -1;
		}
		mp_file_mv(path, dest);
		IF_FREE(unique_filename);
		IF_FREE(filename);
	} else {
		if (dest_root_path == NULL) {
			dest_root_path = MP_PERSONAL_PAGE_DIR;
		}

		/*if dest_root_path does not exist, create it*/
		bool mkdir_ret = mp_file_mkpath(dest_root_path);
		if (mkdir_ret == false) {
			DEBUG_TRACE("failed to make new directory");
		}

		char *unique_filename = NULL;
		while (mp_util_is_duplicated_name(dest_root_path, (const char *)filename)) {
			IF_FREE(unique_filename);
			mp_util_get_unique_name((char *)filename, &unique_filename);
			IF_FREE(filename);
			filename = g_strdup(unique_filename);
		}
		dest = g_strconcat(dest_root_path, "/", filename, NULL);
		/*add to personal page*/
		if (dest == NULL) {
			IF_FREE(unique_filename);
			IF_FREE(filename);
			return -1;
		}
		mp_file_mv(path, dest);
		IF_FREE(unique_filename);
		IF_FREE(filename);
	}
	mp_media_info_delete_from_db(path);
	mp_media_info_delete_from_db(dest);

	return 0;
}

inline static int
_move_folder(mp_media_info_h item_handle)
{
	MP_CHECK_VAL(item_handle, -1);

	char *path = NULL;
	mp_media_info_group_get_sub_info(item_handle, &path);
	//DEBUG_TRACE("------------>path is %s", path);

	char *folder_id = NULL;
	mp_media_info_group_get_folder_id(item_handle, &folder_id);

	char *root_path = NULL;
	_mp_edit_cb_get_root_path_by_full(path, &root_path);
	//DEBUG_TRACE("------------>root_path is %s", root_path);

	char *dest = NULL;
	if (mp_util_is_in_personal_page((const char *)path)) {
		/*remove from personal page*/
		/*1. check if the selected item is root path(/opt/storaget/PersonalStorage)*/
		if (!g_strcmp0(path, MP_PERSONAL_PAGE_DIR)) {
			/*move music related item to sounds*/
			int ret = 0;
			int count = 0;
			/*1. get track count*/
			ret = mp_media_info_list_count(MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, &count);
			if (ret) {
				DEBUG_TRACE("get track in folder failed");
			}

			if (count == 0) {
				ERROR_TRACE("empty folder");
				IF_FREE(folder_id);
				IF_FREE(path);
				IF_FREE(root_path);
				return -1;
			}

			/*2. get content from DB*/
			mp_media_list_h svc_handle = NULL;
			mp_media_info_list_create(&svc_handle, MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, 0, count);

			/*3. move item one by one*/
			int index = 0;
			for (index = 0; index < count; index++) {
				mp_media_info_h item = NULL;
				item = mp_media_info_list_nth_item(svc_handle, index);
				if (item == NULL) {
					continue;
				}

				_move_track_spec_path(item, NULL);
			}
		} else { /*2. other folder*/
			char *related_path = NULL;
			_mp_edit_cb_get_logical_path_by_full(path, &related_path);
			//DEBUG_TRACE("------------>related_path is %s", related_path);

			char **dir_levels = g_strsplit(related_path + 1, "/", 0);
			char *dir_name = g_strdup(dir_levels[0]);
			//DEBUG_TRACE("------------>dir_name is %s", dir_name);
			char *semi_path = NULL;
			semi_path = g_strjoinv("/", dir_levels + 1);
			//DEBUG_TRACE("------------>semi_path is %s", semi_path);
			/*create folder in destination*/
			/*1. check if duplicated Directory exists*/
			char *unique_filename = NULL;
			while (mp_util_is_duplicated_name(MP_MUSIC_DIR, (const char *)dir_name)) {
				IF_FREE(unique_filename);
				mp_util_get_unique_name(dir_name, &unique_filename);
				IF_FREE(dir_name);
				dir_name = g_strdup(unique_filename);
			}
			dest = g_strconcat(MP_MUSIC_DIR, "/", dir_name, "/", semi_path, NULL);
			IF_FREE(unique_filename);
			IF_FREE(dir_name);
			/*2. create new directory*/
			bool mkdir_ret = mp_file_mkpath(dest);
			if (mkdir_ret == false) {
				DEBUG_TRACE("failed to make new directory");
			}
			/*move music related item to new folder*/
			int ret = 0;
			int count = 0;
			/*1. get track count*/
			ret = mp_media_info_list_count(MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, &count);
			if (ret) {
				DEBUG_TRACE("get track in folder failed");
			}

			if (count == 0) {
				ERROR_TRACE("empty folder");
				IF_FREE(folder_id);
				IF_FREE(path);
				IF_FREE(root_path);
				return -1;
			}

			/*2. get content from DB*/
			mp_media_list_h svc_handle = NULL;
			mp_media_info_list_create(&svc_handle, MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, 0, count);

			/*3. move item one by one*/
			int index = 0;
			for (index = 0; index < count; index++) {
				mp_media_info_h item = NULL;
				item = mp_media_info_list_nth_item(svc_handle, index);
				if (item == NULL) {
					continue;
				}

				_move_track_spec_path(item, dest);
			}
			/*check if there is other item in the folder*/
			_mp_edit_cb_delete_empty_dir(path, root_path);
		}
	} else {
		/*add to personal page*/
		/*1. check if the selected item is root path(/opt/storaget/PersonalStorage)*/
		if (!g_strcmp0(path, MP_MUSIC_DIR)) {
			/*move music related item to sounds*/
			int ret = 0;
			int count = 0;
			/*1. get track count*/
			ret = mp_media_info_list_count(MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, &count);
			if (ret) {
				DEBUG_TRACE("get track in folder failed");
			}

			if (count == 0) {
				ERROR_TRACE("empty folder");
				IF_FREE(folder_id);
				IF_FREE(path);
				IF_FREE(root_path);
				return -1;
			}

			/*2. get content from DB*/
			mp_media_list_h svc_handle = NULL;
			mp_media_info_list_create(&svc_handle, MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, 0, count);

			/*3. move item one by one*/
			int index = 0;
			for (index = 0; index < count; index++) {
				mp_media_info_h item = NULL;
				item = mp_media_info_list_nth_item(svc_handle, index);
				if (item == NULL) {
					continue;
				}

				_move_track_spec_path(item, NULL);
			}
		} else { /*2. other folder*/
			char *related_path = NULL;
			_mp_edit_cb_get_logical_path_by_full(path, &related_path);
			//DEBUG_TRACE("------------>related_path is %s", related_path);

			char **dir_levels = g_strsplit(related_path + 1, "/", 0);
			char *dir_name = g_strdup(dir_levels[0]);
			//DEBUG_TRACE("------------>dir_name is %s", dir_name);
			char *semi_path = NULL;
			semi_path = g_strjoinv("/", dir_levels + 1);
			//DEBUG_TRACE("------------>semi_path is %s", semi_path);
			/*create folder in destination*/
			/*1. check if duplicated Directory exists*/
			char *unique_filename = NULL;
			while (mp_util_is_duplicated_name(MP_PERSONAL_PAGE_DIR, (const char *)dir_name)) {
				IF_FREE(unique_filename);
				mp_util_get_unique_name(dir_name, &unique_filename);
				IF_FREE(dir_name);
				dir_name = g_strdup(unique_filename);
			}
			dest = g_strconcat(MP_PERSONAL_PAGE_DIR, "/", dir_name, "/", semi_path, NULL);
			IF_FREE(unique_filename);
			IF_FREE(dir_name);
			/*2. create new directory*/
			bool mkdir_ret = mp_file_mkpath(dest);
			if (mkdir_ret == false) {
				DEBUG_TRACE("failed to make new directory");
			}
			/*move music related item to new folder*/
			int ret = 0;
			int count = 0;
			/*1. get track count*/
			ret = mp_media_info_list_count(MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, &count);
			if (ret) {
				DEBUG_TRACE("get track in folder failed");
			}

			if (count == 0) {
				ERROR_TRACE("empty folder");
				IF_FREE(folder_id);
				IF_FREE(path);
				IF_FREE(root_path);
				return -1;
			}

			/*2. get content from DB*/
			mp_media_list_h svc_handle = NULL;
			mp_media_info_list_create(&svc_handle, MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, 0, count);

			/*3. move item one by one*/
			int index = 0;
			for (index = 0; index < count; index++) {
				mp_media_info_h item = NULL;
				item = mp_media_info_list_nth_item(svc_handle, index);
				if (item == NULL) {
					continue;
				}

				_move_track_spec_path(item, dest);
			}
			/*check if there is other item in the folder*/
			_mp_edit_cb_delete_empty_dir(path, root_path);
		}
	}

	IF_FREE(path);
	IF_FREE(root_path);
	return 0;
}

static Evas_Object *
_mp_edit_cb_get_progressbar()
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = ad->popup[MP_POPUP_OPERATION_PROGRESS];
	MP_CHECK_NULL(popup);
	Evas_Object *layout = elm_object_content_get(popup);
	MP_CHECK_NULL(layout);
	Evas_Object *progressbar = elm_object_part_content_get(layout, "elm.swallow.content");
	return progressbar;
}

static Evas_Object *
_mp_edit_cb_get_popup_layout()
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = ad->popup[MP_POPUP_OPERATION_PROGRESS];
	MP_CHECK_NULL(popup);
	Evas_Object *layout = elm_object_content_get(popup);
	return layout;
}

static void
_mp_edit_move_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	if (g_personal_storage_thread) {
		ecore_thread_cancel(g_personal_storage_thread);
		g_personal_storage_thread = NULL;
	}
}

static void
_mp_edit_cb_move_thread(void *data, Ecore_Thread *thread)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_list_item_data_t *item = NULL;
	mp_media_info_h item_handle = NULL;

	//DEBUG_TRACE("g_list_type: %d, track_type: %d, g_group_type: %d", g_list_type, g_track_type, g_group_type);

	mp_media_info_connect();

	GList *node = g_list_last(g_selected_list);
	MP_CHECK_EXCEP(node);
	g_total_count = g_list_length(g_selected_list);
	int moved_count = 0;
	while (node) {
		if (ecore_thread_check(thread)) {	// pending cancellation
			WARN_TRACE("popup cancel clicked");
			goto mp_exception;
		}

		item = node->data;
		node = g_list_previous(node);
		if (!item) {
			WARN_TRACE("CHECK here...");
			ecore_thread_feedback(thread, (void *) - 1);
			goto mp_exception;
		}
		item_handle = (item->handle);
		if (!item_handle) {
			continue;
		}

		char *path = NULL;
		if (g_list_type == MP_LIST_TYPE_TRACK) {
			mp_media_info_get_file_path(item_handle, &path);
		} else if (g_list_type == MP_LIST_TYPE_GROUP) {
			mp_media_info_group_get_sub_info(item_handle, &path);
		} else if (g_list_type == MP_LIST_TYPE_ALBUM_DETAIL || g_list_type == MP_LIST_TYPE_ARTIST_DETAIL) {

			mp_media_info_get_file_path(item_handle, &path);
		} else {
			ERROR_TRACE("Wrong Type");
		}

		Eina_Bool folder = EINA_FALSE;
		if (path) {
			folder = mp_file_is_dir(path);
		}
		if (folder) {
			DEBUG_TRACE("folder");
			if (_move_folder(item_handle)) {
				g_error_count++;
				ecore_thread_feedback(thread, (void *) - 1);
			} else {
				moved_count++;
				ecore_thread_feedback(thread, (void *)moved_count);
			}
		} else { /*track*/
			DEBUG_TRACE("track --> path is %s", path);
			/*get related path information*/
			char *related_path = NULL;
			_mp_edit_cb_get_logical_path_by_full(path, &related_path);

			/*generate dest path*/
			char *dest_path = NULL;
			if (mp_util_is_in_personal_page((const char *)path)) {
				/*remove from personal page*/
				dest_path = g_strconcat(MP_MUSIC_DIR, related_path, NULL);
				MP_CHECK(dest_path);
			} else {
				/*remove from personal page*/
				dest_path = g_strconcat(MP_PERSONAL_PAGE_DIR, related_path, NULL);
				MP_CHECK(dest_path);
			}
			//DEBUG_TRACE("dest path is %s", dest_path);
			char *dest_dir = mp_file_dir_get(dest_path);
			//DEBUG_TRACE("dest dir is %s", dest_dir);

			/*for exception handle, if dest_dir is NULL, in _move_track_spec_path, it will use root path as dest*/
			if (_move_track_spec_path(item_handle, dest_dir)) {
				g_error_count++;
				ecore_thread_feedback(thread, (void *) - 1);
			} else {
				moved_count++;
				ecore_thread_feedback(thread, (void *)moved_count);
			}
			IF_FREE(dest_dir);
			IF_FREE(dest_path);
		}

	}
mp_exception:

	//To make progress popup visible if only one item deleted.
	sleep(1);
	mp_media_info_disconnect();
}

static void
_mp_edit_cb_move_notify_cb(void *data, Ecore_Thread *thread, void *msg_data)
{
	startfunc;
	int feedback = (int)msg_data;
	if (feedback == -1) {
		DEBUG_TRACE("----------->error happened in main thread");
	} else {
		/*1. set progress bar value*/
		Evas_Object *progressbar = _mp_edit_cb_get_progressbar();
		double progress_value = (double)feedback / (double)g_total_count;
		elm_progressbar_value_set(progressbar, progress_value);

		/*2. set text*/
		char *popup_information =  g_strdup_printf("%d / %d", feedback, g_total_count);
		Evas_Object *layout = _mp_edit_cb_get_popup_layout();
		elm_object_part_text_set(layout, "elm.text.right", popup_information);
		IF_FREE(popup_information);

		char *progress_text = g_strdup_printf("%d%%", (int)(progress_value * 100));
		elm_object_part_text_set(layout, "elm.text.left", progress_text);
		IF_FREE(progress_text);
	}
}

static void
_mp_edit_cb_move_thread_end_cb(void *data, Ecore_Thread *thread)
{
	WARN_TRACE("thread_end");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	g_personal_storage_thread = NULL;
	g_playlist_id = 0;
	g_playlist_track_count = 0;
	g_list_free(g_selected_list);

	_mp_edit_cb_check_playlist();

	mp_evas_object_del(ad->popup[MP_POPUP_OPERATION_PROGRESS]);

	mp_view_mgr_delete_view(GET_VIEW_MGR, MP_VIEW_EDIT);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_VIEW_TRANSITION_FINISHED);

	/*update top view*/
	MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
	MP_CHECK(top_view);
	mp_view_update(top_view);

	if (top_view->view_type != MP_VIEW_ALL) {
		mp_view_update(GET_ALL_VIEW);
	}

	if (top_view->view_type == MP_VIEW_FOLDER_DETAIL) {
		//update folder view
		MpView_t *folder_view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_FOLDER);
		MP_CHECK(folder_view);
		mp_view_update(folder_view);
	}
}

static void
_mp_edit_cb_move_cancel_cb(void *data, Ecore_Thread *thread)
{
	WARN_TRACE("thread_cancel");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	g_personal_storage_thread = NULL;
	g_list_free(g_selected_list);

	_mp_edit_cb_check_playlist();

	mp_evas_object_del(ad->popup[MP_POPUP_OPERATION_PROGRESS]);

	mp_view_mgr_delete_view(GET_VIEW_MGR, MP_VIEW_EDIT);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_VIEW_TRANSITION_FINISHED);

	/*update top view*/
	MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
	MP_CHECK(top_view);
	mp_view_update(top_view);

	if (top_view->view_type != MP_VIEW_ALL) {
		mp_view_update(GET_ALL_VIEW);
	}

	if (top_view->view_type == MP_VIEW_FOLDER_DETAIL) {
		//update folder view
		MpView_t *folder_view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_FOLDER);
		MP_CHECK(folder_view);
		mp_view_update(folder_view);
	}
}

void
mp_edit_cb_excute_move(void *data)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_selected_item_data_get((MpList_t *)list, &g_selected_list);
	MP_CHECK(g_selected_list);

	g_list_type = list->list_type;
	g_group_type = mp_list_get_group_type((MpList_t *)list);
	g_selected_count = g_list_length(g_selected_list);
	g_error_count = 0;
	g_edit_operation = MP_EDIT_MOVE;
	//g_track_type = mp_list_get_track_type((MpList_t *)list);
	//g_playlist_handle = mp_list_get_playlist_handle((MpList_t *)list);

	DEBUG_TRACE("g_list_type is %d", g_list_type);
	char *title = NULL;
	unsigned long long remained_size = 0;
	if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_ADD  || list->personal_page_storage == MP_LIST_PERSONAL_PAGE_NORMAL) {
		title = STR_MP_ADDIND;
		_mp_edit_cb_get_remain_space(MP_PERSONAL_PAGE_DIR, &remained_size);
	} else if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_REMOVE || list->personal_page_storage == MP_LIST_PERSONAL_PAGE_PRIVATE) {

		title = STR_MP_REMOVING;
		_mp_edit_cb_get_remain_space(MP_MUSIC_DIR, &remained_size);
	}

	/*recover list personal page type to prepare update list*/
	list->personal_page_type = MP_LIST_PERSONAL_PAGE_NONE;

	/*check selected file size*/
	mp_list_item_data_t *item = NULL;
	mp_media_info_h item_handle = NULL;

	GList *node = g_list_last(g_selected_list);

	unsigned long long selected_size = 0;
	unsigned long long file_size = 0;
	if (g_list_type == MP_LIST_TYPE_TRACK) {
		while (node) {
			item = node->data;
			node = g_list_previous(node);
			if (!item) {
				WARN_TRACE("CHECK here...");
				continue;
			}
			item_handle = (item->handle);
			if (!item_handle) {
				continue;
			}

			char *path = NULL;
			mp_media_info_get_file_path(item_handle, &path);
			_mp_edit_cb_get_item_size(path, &file_size);
			selected_size += file_size;
		}
	} else if (g_list_type == MP_LIST_TYPE_GROUP) {
		while (node) {
			item = node->data;
			node = g_list_previous(node);
			if (!item) {
				WARN_TRACE("CHECK here...");
				continue;
			}
			item_handle = (item->handle);
			if (!item_handle) {
				continue;
			}

			char *path = NULL;
			mp_media_info_group_get_sub_info(item_handle, &path);

			char *folder_id = NULL;
			mp_media_info_group_get_folder_id(item_handle, &folder_id);

			int ret = 0;
			int count = 0;
			/*1. get track count*/
			ret = mp_media_info_list_count(MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, &count);
			if (ret) {
				DEBUG_TRACE("get track in folder failed");
			}

			if (count == 0) {
				ERROR_TRACE("empty folder");
				IF_FREE(folder_id);
				continue;
			}

			/*2. get content from DB*/
			mp_media_list_h svc_handle = NULL;
			mp_media_info_list_create(&svc_handle, MP_TRACK_BY_FOLDER, folder_id, NULL, NULL, 0, 0, count);

			/*3. move item one by one*/
			int index = 0;
			for (index = 0; index < count; index++) {
				mp_media_info_h item = NULL;
				item = mp_media_info_list_nth_item(svc_handle, index);
				if (item == NULL) {
					continue;
				}

				char *file_path = NULL;
				mp_media_info_get_file_path(item, &file_path);

				_mp_edit_cb_get_item_size(file_path, &file_size);
				selected_size += file_size;
			}
			IF_FREE(folder_id);
		}
	}

	DEBUG_TRACE("selected size is %f", selected_size);
	DEBUG_TRACE("remained size is %f", remained_size);

	if (selected_size > remained_size) {
		Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, NULL, NULL, ad);
		MP_CHECK(popup);

		char *desc = g_strdup("not enough space");
		mp_popup_desc_set(popup, desc);
		SAFE_FREE(desc);

		mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_OK, MP_POPUP_YES);

		evas_object_show(popup);
		return;
	}

	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_OPERATION_PROGRESS, NULL, list,
	                                     _mp_edit_move_popup_response_cb, ad);
	evas_object_show(popup);
	Evas_Object *layout = _mp_edit_cb_get_popup_layout();
	mp_util_domain_translatable_part_text_set(layout, "elm.title", title);
	/*set text*/
	char *popup_information =  g_strdup_printf("%d / %d", 0, g_total_count);
	elm_object_part_text_set(layout, "elm.text.right", popup_information);
	IF_FREE(popup_information);

	char *progress_text = g_strdup_printf("%d%%", (int)0);
	elm_object_part_text_set(layout, "elm.text.left", progress_text);
	IF_FREE(progress_text);

	g_personal_storage_thread = ecore_thread_feedback_run(
	                                _mp_edit_cb_move_thread,
	                                _mp_edit_cb_move_notify_cb,
	                                _mp_edit_cb_move_thread_end_cb,
	                                _mp_edit_cb_move_cancel_cb,
	                                (const void *)g_selected_list,
	                                EINA_TRUE);

	if (!g_personal_storage_thread) {
		mp_popup_response(ad->popup[MP_POPUP_OPERATION_PROGRESS], MP_POPUP_NO);
	}
	ad->edit_in_progress = true;

}

#endif
#ifdef MP_FEATURE_CLOUD

static void
_mp_edit_delete_cloud_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	ad->popup_delete = NULL;
	mp_evas_object_del(obj);

	int response = (int)event_info;
	if (response == MP_POPUP_NO) {
		return;
	}

	mp_edit_cb_excute_delete(data);

	endfunc;
	return;
}

void
mp_edit_create_delete_cloud_confirm_popup(void *data)
{
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();

	MpList_t *list = data;
	MP_CHECK(list);

	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, data, _mp_edit_delete_cloud_popup_response_cb, ad);
	ad->popup_delete = popup;

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	elm_object_text_set(popup, STR_MP_THIS_WILL_BE_DELETE_FORM_SERVER);

	mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_DELETE, MP_POPUP_YES);

	evas_object_show(popup);

}
#endif

static void
_mp_edit_delete_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	ad->popup_delete = NULL;
	ad->del_cb_invoked = 0;
	mp_evas_object_del(obj);

	int response = (int)event_info;
	if (response == MP_POPUP_NO) {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_POPUP_CANCEL);
		return;
	}

#ifdef MP_FEATURE_CLOUD
	MpList_t *list = data;
	GList *sel_list = NULL;
	GList *node = NULL;
	bool cloud_data = 0;

	if (list->list_type == MP_LIST_TYPE_TRACK || list->list_type == MP_LIST_TYPE_ARTIST_DETAIL || list->list_type == MP_LIST_TYPE_ALBUM_DETAIL) {
		int track_type = mp_list_get_track_type(list);
		if (track_type != MP_TRACK_BY_PLAYLIST && track_type != MP_TRACK_BY_ADDED_TIME &&
		        track_type != MP_TRACK_BY_PLAYED_TIME && track_type != MP_TRACK_BY_FAVORITE &&
		        track_type != MP_TRACK_BY_PLAYED_COUNT) {
			mp_list_selected_item_data_get(list, &sel_list);
			MP_CHECK(sel_list);

			node = g_list_first(sel_list);
			while (node) {
				mp_list_item_data_t *item = sel_list->data;
				mp_media_info_h media = item->handle;
				mp_storage_type_e storage_type;
				mp_media_info_get_storage_type(media, &storage_type);
				if (storage_type == MP_STORAGE_CLOUD) {
					cloud_data = 1;
					break;
				}

				node = g_list_next(node);
			}
			g_list_free(sel_list);

			if (cloud_data) {
				mp_edit_create_delete_cloud_confirm_popup(data);
				return;
			}
		}
	}
#endif
	mp_edit_cb_excute_delete(data);

	endfunc;
	return;
}

void
mp_edit_create_delete_popup(void *data)
{
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();

	MpList_t *list = data;
	MP_CHECK(list);
	int selected_count = mp_list_get_checked_count(list);
	if (selected_count <= 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	char *btn_txt = NULL;
	char *title_txt = NULL;
	char *title = NULL;
	char *help_txt = NULL;

	mp_track_type_e type = mp_list_get_track_type(list);
	WARN_TRACE("track_type is %d", type);
	if (type > MP_TRACK_TYPE_PLAYLIST_MIN && type < MP_TRACK_TYPE_PLAYLIST_MAX) {
		//elm_object_text_set(popup, GET_STR("IDS_MUSIC_POP_REMOVE_Q"));
		btn_txt = STR_MP_REMOVE;
		title = STR_MP_REMOVE;
	} else {
		//elm_object_text_set(popup, GET_SYS_STR("IDS_COM_POP_DELETE_Q"));
		btn_txt = STR_MP_DELETE;
		title = STR_MP_DELETE;
	}

	title_txt = g_strconcat("<align=center>", GET_STR(title), "</align>", NULL);
	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, title_txt, data, _mp_edit_delete_popup_response_cb, ad);
	ad->popup_delete = popup;
	//making help_txt
	//1. get group type
	mp_group_type_e group_type = mp_list_get_group_type(list);
	WARN_TRACE("group_type is %d", group_type);

	switch (group_type) {
	case MP_GROUP_NONE:
		DEBUG_TRACE("MP_GROUP_NONE");
		if (selected_count == 1) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ONE_TRACK_DETELED), "</align>", NULL);
		} else if (selected_count == mp_list_get_editable_count(list, MP_LIST_EDIT_TYPE_NORMAL)) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ALL_TRACKS_DETELED), "</align>", NULL);
		} else {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_PLURAL_TRACKS_DETELED), "</align>", NULL);
		}
		break;
	case MP_GROUP_BY_ALBUM:					/**< Group by album*/
		DEBUG_TRACE("MP_GROUP_BY_ALBUM");
		if (selected_count == 1) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ONE_ALBUM_DETELED), "</align>", NULL);
		} else if (selected_count == mp_list_get_editable_count(list, MP_LIST_EDIT_TYPE_NORMAL)) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ALL_ALBUMS_DETELED), "</align>", NULL);
		} else {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_PLURAL_ALBUMS_DETELED), "</align>", NULL);
		}
		break;
	case MP_GROUP_BY_ARTIST:				/**< Group by artist*/
		DEBUG_TRACE("MP_GROUP_BY_ARTIST");
		if (selected_count == 1) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ONE_ARTIST_DETELED), "</align>", NULL);
		} else if (selected_count == mp_list_get_editable_count(list, MP_LIST_EDIT_TYPE_NORMAL)) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ALL_ARTISTS_DETELED), "</align>", NULL);
		} else {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_PLURAL_ARTISTS_DETELED), "</align>", NULL);

		}
		break;
	case MP_GROUP_BY_FOLDER:					/**< Group by folder*/
		DEBUG_TRACE("MP_GROUP_BY_FOLDER");
		if (selected_count == 1) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ONE_FOLDER_DETELED), "</align>", NULL);
		} else if (selected_count == mp_list_get_editable_count(list, MP_LIST_EDIT_TYPE_NORMAL)) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ALL_FOLDERS_DETELED), "</align>", NULL);
		} else {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_PLURAL_FOLDERS_DETELED), "</align>", NULL);
		}
		break;
	case MP_GROUP_BY_PLAYLIST:
		DEBUG_TRACE("MP_GROUP_BY_PLAYLIST");
		if (selected_count == 1) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ONE_PLAYLIST_DETELED), "</align>", NULL);
		} else if (selected_count == mp_list_get_editable_count(list, MP_LIST_EDIT_TYPE_NORMAL)) {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_ALL_PLAYLISTS_DETELED), "</align>", NULL);
		} else {
			help_txt = g_strconcat("<align=left>", GET_STR(STR_MP_PLURAL_PLAYLISTS_DETELED), "</align>", NULL);
		}
		break;
	default:
		DEBUG_TRACE("Other -1");
		help_txt = g_strconcat("<align=left>",  GET_SYS_STR("IDS_COM_POP_DELETE_Q"), "</align>", NULL);
		break;
	}

	mp_util_domain_translatable_text_set(popup, help_txt);
	IF_FREE(help_txt);
	mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_CANCEL, MP_POPUP_NO);
	mp_popup_button_set(popup, MP_POPUP_BTN_2, btn_txt, MP_POPUP_YES);

	evas_object_show(popup);

}

