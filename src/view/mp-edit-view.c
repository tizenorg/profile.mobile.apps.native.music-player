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

#include <media_content.h>

#include "mp-edit-view.h"
#include "mp-all-view.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-edit-callback.h"
#include "mp-track-list.h"
#include "mp-album-list.h"
#include "mp-artist-list.h"
#include "mp-album-detail-list.h"
#include "mp-artist-detail-list.h"
#include "mp-playlist-list.h"
#include "mp-folder-list.h"
#include "mp-create-playlist-view.h"
#include "mp-common.h"
#include "mp-setting-ctrl.h"
#include "mp-edit-playlist.h"

static void
_mp_edit_view_destory_cb(void *thiz)
{
	eventfunc;
	MpEditView_t *view = thiz;
	MP_CHECK(view);

	mp_ecore_timer_del(view->back_timer);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/

	free(view);
}


int _mp_edit_view_update(void *thiz)
{
	startfunc;
	MpEditView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	mp_list_update(view->content_to_show);
	if (view->list_mode == MP_EDIT_VIEW_REORDER) {
		view->reorder = false;
		mp_list_set_edit(view->content_to_show, EINA_FALSE);
		mp_list_set_reorder(view->content_to_show, EINA_TRUE);
	} else {
		mp_list_set_edit(view->content_to_show, EINA_TRUE);
	}

	unsigned int count = mp_list_get_editable_count((MpList_t *)view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL);
	if (count <= 0) {
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
	}

	return 0;
}

/***************	functions for track list update 	*******************/

static Eina_Bool _mp_edit_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;

	MpEditView_t *view = data;
	MP_CHECK_VAL(view, EINA_TRUE);

	struct appdata *ad = mp_util_get_appdata();
	ad->del_cb_invoked = 0;

/*
	if (view->content_to_show->reorderable) {
		DEBUG_TRACE("view->ref_list->reorderable = %d",view->ref_list->reorderable);
		int ret = 0;
		void *playlist_handle = mp_list_get_playlist_handle(view->content_to_show);
		DEBUG_TRACE("playlist_handle = %p", playlist_handle);
		ret = mp_media_info_playlist_update_db(playlist_handle);
		MP_CHECK_VAL(ret == 0, -1);
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_REORDER_DONE);
		mp_list_set_reorder((MpList_t *)view->content_to_show, FALSE);
	}
*/

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	mp_view_mgr_pop_view(view_mgr, false);

	MP_CHECK_FALSE(ad);
	mp_evas_object_del(ad->popup_delete);

	return EINA_TRUE;
}

void
mp_edit_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpEditView_t *view = data;
	MP_CHECK(view);
	mp_edit_create_add_to_playlist_popup(view->content_to_show);
}

void
mp_edit_view_delete_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpEditView_t *view = data;
	MP_CHECK(view);
	mp_edit_create_delete_popup(view->content_to_show);
}

void mp_edit_view_remove_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MP_CHECK(data);
	MpEditView_t *view = data;
	MP_CHECK(view);
	MP_CHECK(view->content_to_show);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	int list_count = _mp_list_get_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL);

	if(list_count == mp_list_get_checked_count(view->content_to_show)) {
		ad->is_sdcard_removed = 1;
		mp_lockscreenmini_destroy(ad);
		mp_minicontroller_destroy(ad);
	}
	if (mp_list_get_checked_count(view->content_to_show) <= 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}
	mp_edit_cb_excute_delete(view->content_to_show);
	return;
}


#ifdef MP_FEATURE_PERSONAL_PAGE
static void
_mp_edit_view_move_execute_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	bool flag_dont_ask = false;
	Evas_Object *layout = elm_object_content_get(ad->popup[MP_POPUP_CHECK_INFO_PERSONAL]);
	if (layout != NULL) {
		Evas_Object *checkbox = elm_object_part_content_get(layout, "elm.swallow.end");
		flag_dont_ask = elm_check_state_get(checkbox);
	}
	mp_evas_object_del(ad->popup[MP_POPUP_CHECK_INFO_PERSONAL]);

	if (flag_dont_ask)
		mp_setting_set_personal_dont_ask_again(flag_dont_ask);

	mp_edit_cb_excute_move(list);
}

static void _mp_edit_view_notify_cancel_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_POPUP_CANCEL);
	mp_evas_object_del(ad->popup[MP_POPUP_CHECK_INFO_PERSONAL]);
}

void
mp_edit_view_notify_popup(void *data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main, MP_POPUP_CHECK_INFO_PERSONAL, NULL, NULL, NULL, ad);
	evas_object_show(popup);

	Evas_Object *btn1 = mp_widget_create_button(popup, "popup", STR_MP_CANCEL, NULL,
		_mp_edit_view_notify_cancel_cb, NULL);
	Evas_Object *btn2 = mp_widget_create_button(popup, "popup", STR_MP_OK, NULL, _mp_edit_view_move_execute_cb, list);
	elm_object_part_content_set(popup, "button1", btn1);
	elm_object_part_content_set(popup, "button2", btn2);

	Evas_Object *layout = elm_object_content_get(popup);
	if (layout != NULL) {
		/*set lable*/
		Evas_Object *lable = elm_object_part_content_get(layout, "elm.swallow.content");
		if (lable != NULL) {
			int count = mp_list_get_checked_count(list);
			char *lable_text = NULL;
			if (count > 1)
				lable_text = g_strdup_printf(GET_STR(MP_PERSONAL_ITEMS_MOVE_TO), MP_MUSIC_DIR);
			else
				lable_text = g_strdup_printf(GET_STR(MP_PERSONAL_ITEM_MOVE_TO), MP_MUSIC_DIR);

			mp_util_domain_translatable_text_set(lable, lable_text);
			IF_FREE(lable_text);
		}
	}
}

/*static void
_mp_edit_view_move_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpEditView_t *view = data;
	MP_CHECK(view);
	//0. check if is remove from personal page
	if (view->person_page_sel == MP_EDIT_VIEW_PERSONAL_PAGE_ADD)
		mp_edit_cb_excute_move(view->content_to_show);
	else
	{
		//1. get personal don't ask again
		bool no_ask_flag = false;
		mp_setting_get_personal_dont_ask_again(&no_ask_flag);
		if (no_ask_flag)
			mp_edit_cb_excute_move(view->content_to_show);
		else
			mp_edit_view_notify_popup(view->content_to_show);
	}
}*/

#endif

static Eina_Bool
_mp_edit_view_reorder_back_cb(void *thiz)
{
	startfunc;
	MpEditView_t *view = thiz;
	MP_CHECK_FALSE(view);

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	elm_naviframe_item_pop(view_mgr->navi);

	view->back_timer = NULL;
	return EINA_FALSE;
}


void
mp_edit_view_list_item_reorder_update_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpEditView_t *view = (MpEditView_t *)data;
	MP_CHECK(view);

	MpTrackList_t *list = (MpTrackList_t *)view->content_to_show;
	MP_CHECK(list);

	int index = -1;
	int ret = 0;
	int member_id = 0;

	void *playlist_handle = mp_list_get_playlist_handle((MpList_t *)list);
	Elm_Object_Item *temp = elm_genlist_first_item_get(list->genlist);
	while (temp) {
		index = elm_genlist_item_index_get(temp);
		mp_list_item_data_t *item_data = elm_object_item_data_get(temp);
		MP_CHECK(item_data);

		ret = mp_media_info_get_playlist_member_id(item_data->handle, &member_id);
		MP_CHECK(ret == MEDIA_CONTENT_ERROR_NONE);

		ret = mp_media_info_playlist_set_play_order(playlist_handle, member_id, index);
		MP_CHECK(ret == MEDIA_CONTENT_ERROR_NONE);
		temp = elm_genlist_item_next_get(temp);
		/*DEBUG_TRACE("member_id: %d, index: %d", member_id, index);*/
	}

	ret = mp_media_info_playlist_update_db(playlist_handle);
	MP_CHECK(ret == MEDIA_CONTENT_ERROR_NONE);

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_REORDER_DONE);

	view->back_timer = ecore_timer_add(0.1, _mp_edit_view_reorder_back_cb, view);
}

static int _mp_edit_view_update_options(void *thiz)
{
	startfunc;
	MpEditView_t *view = (MpEditView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	MpList_t *list = view->content_to_show;
	MP_CHECK_VAL(list, -1);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, -1);

	mp_track_type_e track_type = mp_list_get_track_type(list);

	if (mp_list_get_checked_count((MpList_t *)view->content_to_show) == 0 && view->list_mode != MP_EDIT_VIEW_REORDER) {
		if (list->list_type == MP_LIST_TYPE_TRACK && MP_TRACK_BY_SQUARE != track_type) {
			MpTrackList_t *track_list = (MpTrackList_t *)list;
			if (track_list->load_timer == NULL) {
				mp_evas_object_del(ad->popup_delete);
			}
		} else {
			mp_evas_object_del(ad->popup_delete);
		}
	}
	mp_util_create_selectioninfo_with_count((MpView_t *)view, mp_list_get_checked_count((MpList_t *)view->content_to_show));
	if (mp_list_get_checked_count((MpList_t *)view->content_to_show) == mp_list_get_editable_count((MpList_t *)view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL)) {
		elm_check_state_set(view->select_all_btn, EINA_TRUE);
	} else {
		elm_check_state_set(view->select_all_btn, EINA_FALSE);
	}

	if (view->done_btn) {
		if (mp_list_get_checked_count((MpList_t *)view->content_to_show)) {
			elm_object_disabled_set(view->done_btn, EINA_FALSE);
		} else {
			elm_object_disabled_set(view->done_btn, EINA_TRUE);
		}
	}

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_edit_view_back_cb, view);

	unsigned int count = mp_list_get_editable_count(list, MP_LIST_EDIT_TYPE_NORMAL);

	if (count == 0) {
		mp_popup_destroy(ad);
		/*playlist detail view should not back to all view*/
		if (mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYLIST_DETAIL)) {
			mp_view_mgr_pop_a_view(GET_VIEW_MGR, thiz);
		} else {
			mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL);
		}
	}

	endfunc;
	return 0;
}

static MpList_t *
_create_group_list(MpEditView_t *view)
{
	startfunc;
	mp_group_type_e group_type = mp_list_get_group_type(view->ref_list);
	void *list = NULL;
	MP_CHECK_NULL(view);
	switch (group_type) {
	case MP_GROUP_BY_ALBUM:
	case MP_GROUP_BY_ARTIST_ALBUM:
		list = mp_album_list_create(view->layout);
		MP_CHECK_NULL(list);
		mp_album_list_copy_data((MpAlbumList_t *)view->ref_list, list);
		break;
	case MP_GROUP_BY_ARTIST:
		list = mp_artist_list_create(view->layout);
		MP_CHECK_NULL(list);
		mp_artist_list_copy_data((MpArtistList_t *)view->ref_list, list);
		mp_artist_list_set_data(list, MP_ARTIST_LIST_TYPE, MP_GROUP_BY_ARTIST, -1);
		break;
	case MP_GROUP_BY_FOLDER:
		list = mp_folder_list_create(view->layout);
		MP_CHECK_NULL(list);
		mp_folder_list_copy_data((MpFolderList_t *)view->ref_list, list);
		break;
	case MP_GROUP_BY_PLAYLIST:
		list = mp_playlist_list_create(view->layout);
		MP_CHECK_NULL(list);
		mp_playlist_list_copy_data((MpPlaylistList_t *)view->ref_list, list);
		break;
	case MP_GROUP_BY_GENRE:
		list = mp_genre_list_create(view->layout);
		MP_CHECK_NULL(list);
		mp_genre_list_copy_data((MpGenreList_t *)view->ref_list, list);
		break;
	default:
		ERROR_TRACE("Inavlid type: %d", group_type);
		break;
	}

	return list;

}

static int _mp_edit_view_content_load(void *thiz)
{
	startfunc;
	MpEditView_t *view = (MpEditView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	MpList_t *ref_list = view->ref_list;

	DEBUG_TRACE("------------------------>list type is %d", ref_list->list_type);
	switch (ref_list->list_type) {
	case MP_LIST_TYPE_TRACK:
	{
		view->content_to_show = (MpList_t *)mp_track_list_create(view->layout);

		if (view->content_to_show == NULL) {
			ERROR_TRACE("Unable to create track list");
			break;
		}
#ifdef MP_FEATURE_PERSONAL_PAGE
		MpTrackList_t *list = (MpTrackList_t *)view->content_to_show;
		list->personal_page_type = view->person_page_sel;
#endif
		if (view->ref_list->reorderable) {
			mp_list_set_reorder((MpList_t *)view->content_to_show, TRUE);
		}
		mp_track_list_copy_data((MpTrackList_t *)ref_list, (MpTrackList_t *)view->content_to_show);
		edje_object_signal_emit(_EDJ(view->layout), "SHOW_SELECT_ALL_PADDING", "*");
		break;
	}
	case MP_LIST_TYPE_GROUP:
		view->content_to_show = _create_group_list(view);
		MP_CHECK_VAL(view->content_to_show, -1);
		edje_object_signal_emit(_EDJ(view->layout), "SHOW_SELECT_ALL_PADDING", "*");
#ifdef MP_FEATURE_PERSONAL_PAGE
		view->content_to_show->personal_page_type = view->person_page_sel;
#endif

		break;
	case MP_LIST_TYPE_PLAYLIST:
		view->content_to_show = (MpList_t *)mp_playlist_list_create(view->layout);
		if (view->content_to_show == NULL) {
			ERROR_TRACE("Unable to create music playlist");
			break;
		}
		mp_playlist_list_copy_data((MpPlaylistList_t *)ref_list, (MpPlaylistList_t *)view->content_to_show);
		break;
	case MP_LIST_TYPE_ALBUM_DETAIL:
		view->content_to_show = (MpList_t *)mp_album_detail_list_create(view->layout);
		if (view->content_to_show == NULL) {
			ERROR_TRACE("Unable to create album deatil list");
			break;
		}
		mp_album_detail_list_copy_data((MpAlbumDetailList_t *)ref_list, (MpAlbumDetailList_t *)view->content_to_show);
#ifdef MP_FEATURE_PERSONAL_PAGE
		view->content_to_show->personal_page_type = view->person_page_sel;
#endif
		break;
	case MP_LIST_TYPE_ARTIST_DETAIL:
		view->content_to_show = (MpList_t *)mp_artist_detail_list_create(view->layout);
		if (view->content_to_show == NULL) {
			ERROR_TRACE("Unable to create artist deatil list");
			break;
		}
		mp_artist_detail_list_copy_data((MpArtistDetailList_t *)ref_list, (MpArtistDetailList_t *)view->content_to_show);
#ifdef MP_FEATURE_PERSONAL_PAGE
		view->content_to_show->personal_page_type = view->person_page_sel;
#endif
		break;
	case MP_LIST_TYPE_ALL:
	{
		MpView_t *all_view = mp_common_get_all_view();

		if (all_view == NULL) {
			ERROR_TRACE("all view is NULL");
			break;
		}

		MpTab_e tab = ((MpAllView_t *)all_view)->tab_status;

		if (tab == MP_TAB_SONGS) {
			MpTrackList_t *list = mp_track_list_create(view->layout);
			
			if (list == NULL) {
				ERROR_TRACE("Cannot create track list");
				break;
			}
			
			list->cloud_view_type = ref_list->cloud_view_type;
#ifdef MP_FEATURE_PERSONAL_PAGE
			list->personal_page_type = view->person_page_sel;
#endif
			view->content_to_show = (MpList_t *)list;
		} else if (tab == MP_TAB_PLAYLISTS) {
			MpPlaylistList_t *list = mp_playlist_list_create(view->layout);
			if (list == NULL) {
				ERROR_TRACE("Cannot create Playlist list");
				break;
			}
			list->group_type = MP_GROUP_BY_PLAYLIST;
			view->content_to_show = (MpList_t *)list;
		} else if (tab == MP_TAB_ALBUMS) {
			MpAlbumList_t *list = mp_album_list_create(view->layout);
			if (list == NULL) {
				ERROR_TRACE("Cannot create Album list");
				break;
			}
			list->group_type = MP_GROUP_BY_ALBUM;
			list->display_mode = ref_list->display_mode;
			view->content_to_show = (MpList_t *)list;
		} else if (tab == MP_TAB_ARTISTS) {
			MpArtistList_t *list = mp_artist_list_create(view->layout);
			if (list == NULL) {
				ERROR_TRACE("Cannot create Artist list");
				break;
			}
			list->group_type = MP_GROUP_BY_ARTIST;
			list->display_mode = ref_list->display_mode;
			view->content_to_show = (MpList_t *)list;
		} else
			ERROR_TRACE("Never should be here");

		break;
	}
	default:
		ERROR_TRACE("Inavlid type: %d", ref_list->list_type);
		break;
	}
	MP_CHECK_VAL(view->content_to_show, -1);

	if (view->share)
		mp_list_set_edit_type(view->content_to_show, MP_LIST_EDIT_TYPE_SHARE);

	elm_object_part_content_set(view->layout, "list_content", view->content_to_show->layout);

	return 0;
}

static void
_mp_edit_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpEditView_t *view = thiz;
	switch (event) {
	case MP_DELETE_DONE:
		mp_view_mgr_pop_a_view(GET_VIEW_MGR, thiz);
		break;
	case MP_ADD_TO_PLAYLIST_DONE:
		mp_view_mgr_pop_a_view(GET_VIEW_MGR, thiz);
		break;
#ifdef MP_FEATURE_LANDSCAPE
	case MP_VIEW_ROTATE:
		mp_list_rotate(view->content_to_show);
		break;
#endif
	case MP_POPUP_CANCEL:
		mp_view_update_options((MpView_t *)view);
		break;
	case MP_LANG_CHANGED:
	{
		int count = 0;
		if (view->content_to_show)
			count = mp_list_get_checked_count((MpList_t *)view->content_to_show);
		mp_util_create_selectioninfo_with_count((MpView_t *)view, count);
		break;
	}
	case MP_PERSONAL_PAGE_OFF:
		if (view->person_page_sel != MP_EDIT_VIEW_PERSONAL_PAGE_NONE)
			mp_view_mgr_pop_a_view(GET_VIEW_MGR, thiz);
		break;

	case MP_REORDER_DISABLE:
		view->reorder = false;
		if (view->done_btn)
			elm_object_disabled_set(view->done_btn, EINA_TRUE);
		break;
#ifndef MP_SOUND_PLAYER
	case MP_UPDATE_PLAYING_LIST:
		if (view->list_mode == MP_EDIT_VIEW_REORDER) {
			mp_list_realized_item_part_update((MpList_t *)view->content_to_show, "elm.text.main.left.top", ELM_GENLIST_ITEM_FIELD_TEXT);
			mp_list_realized_item_part_update((MpList_t *)view->content_to_show, "elm.text.sub.left.bottom", ELM_GENLIST_ITEM_FIELD_TEXT);
			mp_list_realized_item_part_update((MpList_t *)view->content_to_show, "elm.icon.left", ELM_GENLIST_ITEM_FIELD_CONTENT);
		}
	break;
#endif
	case MP_REORDER_ENABLE:
		view->reorder = TRUE;
		if (view->done_btn)
			elm_object_disabled_set(view->done_btn, EINA_FALSE);
		break;
	default:
		break;
	}
}

static int
_mp_edit_view_init(Evas_Object *parent, MpEditView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_EDIT);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_edit_view_update;
	view->update_options = _mp_edit_view_update_options;
	view->update_options_edit = _mp_edit_view_update_options;
	view->view_destroy_cb = _mp_edit_view_destory_cb;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;
	view->on_event = _mp_edit_view_on_event;

	return ret;
}

#ifdef MP_FEATURE_PERSONAL_PAGE
MpEditView_t *mp_edit_view_create(Evas_Object *parent, MpList_t *list, bool share, MpEditViewPersonalPageType person_page_sel)
#else
MpEditView_t *mp_edit_view_create(Evas_Object *parent, MpList_t *list, bool share)
#endif
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(list);

	MpEditView_t *view = calloc(1, sizeof(MpEditView_t));
	MP_CHECK_NULL(view);

	view->ref_list = list;
	view->share = share;
#ifdef MP_FEATURE_PERSONAL_PAGE
	view->person_page_sel = person_page_sel;
#endif

	ret = _mp_edit_view_init(parent, view);
	if (ret) goto Error;

	_mp_edit_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_edit_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_edit_view_destory(MpEditView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	return 0;
}

