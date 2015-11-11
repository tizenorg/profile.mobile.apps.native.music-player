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

#include "mp-add-track-view.h"
#include "mp-widget.h"
#include "mp-create-playlist-view.h"
#include "mp-select-track-view.h"
#include "mp-common.h"
#include "mp-util.h"

static void _mp_add_track_view_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAddTrackView_t *view = (MpAddTrackView_t *)data;
	Elm_Object_Item *it;
	MpAddTrackViewTab_e tab = MP_ADD_TRACK_VIEW_TAB_ALL;

	it = elm_toolbar_selected_item_get(obj);
	DEBUG_TRACE("selected toolbar item: 0x%x", it);
	mp_retm_if(it == NULL, "tab item is NULL");

	if (it == view->ctltab_album) {
		tab = MP_ADD_TRACK_VIEW_TAB_ALBUMS;
	} else if (it == view->ctltab_artist) {
		tab = MP_ADD_TRACK_VIEW_TAB_ARTIST;
	}
#ifdef MP_FEATURE_ADD_TO_INCLUDE_PLAYLIST_TAB
	else if (it == view->ctltab_plist) {
		tab = MP_ADD_TRACK_VIEW_TAB_PLAYLIST;
	}
#endif
	else if (it == view->ctltab_songs) {
		tab = MP_ADD_TRACK_VIEW_TAB_SONGS;
	} else if (it == view->ctltab_folders) {
		tab = MP_ADD_TRACK_VIEW_TAB_FOLDERS;
	} else {
		DEBUG_TRACE("selected item out of control");
		return;
	}

	if (view->content_tab == tab) {
		DEBUG_TRACE("same tab is selected");
		return;
	}

	view->content_tab = tab;
	mp_add_track_view_select_tab(view, tab);
}

static void
_mp_add_track_view_destory_cb(void *thiz)
{
	eventfunc;
	MpAddTrackView_t *view = thiz;
	MP_CHECK(view);

	mp_language_mgr_unregister_object_item(view->ctltab_songs);
#ifdef MP_FEATURE_ADD_TO_INCLUDE_PLAYLIST_TAB
	mp_language_mgr_unregister_object_item(view->ctltab_plist);
#endif
	mp_language_mgr_unregister_object_item(view->ctltab_album);
	mp_language_mgr_unregister_object_item(view->ctltab_artist);
	mp_language_mgr_unregister_object_item(view->ctltab_folders);

	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/

	free(view);
}

Eina_Bool  _mp_add_track_view_tracklist_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, EINA_TRUE);

	ad->del_cb_invoked = 0;
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	mp_view_mgr_pop_view(view_mgr, false);

	MpView_t *prev_view = mp_view_mgr_get_top_view(view_mgr);
	mp_view_update(prev_view);

	MpView_t *all_view = mp_view_mgr_get_view(view_mgr, MP_VIEW_ALL);
	mp_view_update_options(all_view);

	return EINA_TRUE;
}

void mp_add_track_view_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpAddTrackView_t *view = (MpAddTrackView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	MpTrackList_t *list = (MpTrackList_t *)view->content_to_show;
	MP_CHECK(list);

	mp_edit_cb_excute_add_to_playlist(list, view->playlist_id, NULL, true);
}

static int _mp_add_track_view_update_option(void *thiz)
{
	startfunc;
	MpAddTrackView_t *view = (MpAddTrackView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	/*mp_view_clear_options((MpView_t *)view);*/
	if (view->content_tab == MP_ADD_TRACK_VIEW_TAB_SONGS) {
		mp_util_create_selectioninfo_with_count((MpView_t *)view, mp_list_get_checked_count((MpList_t *)view->content_to_show));

		if (mp_list_get_checked_count((MpList_t *)view->content_to_show) == mp_list_get_editable_count((MpList_t *)view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
			elm_check_state_set(view->select_all_btn, EINA_TRUE);
		} else {
			elm_check_state_set(view->select_all_btn, EINA_FALSE);
		}

		unsigned int count = mp_list_get_editable_count((MpList_t *)view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL);
		if (count <= 0) {
			mp_view_mgr_pop_a_view((MpViewMgr_t *)GET_VIEW_MGR, (MpView_t *)view);

		}

		if (view->done_btn) {
			if (mp_list_get_checked_count((MpList_t *)view->content_to_show)) {
				elm_object_disabled_set(view->done_btn, EINA_FALSE);
			} else {
				elm_object_disabled_set(view->done_btn, EINA_TRUE);
			}
		}
	} else {
		Evas_Object *more_btn = NULL;
		more_btn = elm_object_item_part_content_get(view->navi_it, "toolbar");
		if (more_btn) {
			elm_object_item_part_content_unset(view->navi_it, "toolbar");
			mp_evas_object_del(more_btn);
		}
	}

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_add_track_view_tracklist_back_cb, view);

	endfunc;
	return 0;
}

static int
_mp_add_track_view_content_load(void *view)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_VAL(view, -1);
	MpAddTrackView_t *add_track_view = (MpAddTrackView_t *)view;

	Evas_Object *obj;

	obj = add_track_view->add_track_view_tabbar;

	add_track_view->ctltab_songs =
	    mp_util_toolbar_item_append(obj, NULL, (STR_MP_TRACKS), _mp_add_track_view_change_cb, add_track_view);
	/*mp_language_mgr_register_object_item(add_track_view->ctltab_songs, STR_MP_TRACKS);*/
#ifdef MP_FEATURE_ADD_TO_INCLUDE_PLAYLIST_TAB
	add_track_view->ctltab_plist =
	    mp_util_toolbar_item_append(obj, NULL, (STR_MP_PLAYLISTS), _mp_add_track_view_change_cb, add_track_view);
#endif
	/*mp_language_mgr_register_object_item(add_track_view->ctltab_plist,STR_MP_PLAYLISTS);*/

	add_track_view->ctltab_album =
	    mp_util_toolbar_item_append(obj, NULL, (STR_MP_ALBUMS), _mp_add_track_view_change_cb, add_track_view);
	/*mp_language_mgr_register_object_item(add_track_view->ctltab_album, STR_MP_ALBUMS);*/

	add_track_view->ctltab_artist =
	    mp_util_toolbar_item_append(obj, NULL, (STR_MP_ARTISTS), _mp_add_track_view_change_cb, add_track_view);
	/*mp_language_mgr_register_object_item(add_track_view->ctltab_artist, STR_MP_ARTISTS);*/

	/*add_track_view->ctltab_folders =
		mp_util_toolbar_item_append(obj, NULL, (STR_MP_FOLDERS), _mp_add_track_view_change_cb, add_track_view);*/

	elm_toolbar_item_selected_set(add_track_view->ctltab_songs, EINA_TRUE);

	evas_object_show(obj);
	endfunc;
	return 0;
}

static void
_mp_add_track_view_on_event(void *thiz, MpViewEvent_e event)
{
	MpAddTrackView_t * view = (MpAddTrackView_t *)thiz;
	switch (event) {
	case MP_ADD_TO_PLAYLIST_DONE:
		mp_add_track_view_destory(thiz);
		break;
	case MP_DB_UPDATED:
		mp_add_track_view_select_tab(view, view->content_tab);
		break;

	default:
		break;
	}
}

static int
_mp_add_track_view_init(Evas_Object *parent, MpAddTrackView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_ADD_TRACK);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = NULL;
	view->update_options = _mp_add_track_view_update_option;
	view->update_options_edit = _mp_add_track_view_update_option;
	view->view_destroy_cb = _mp_add_track_view_destory_cb;
	view->on_event = _mp_add_track_view_on_event;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;

	view->add_track_view_layout = view->layout;/*mp_common_load_edj(view->layout, MP_EDJ_NAME, "common_view_layout");*/
	MP_CHECK_VAL(view->add_track_view_layout, -1);

	view->add_track_view_tabbar = mp_widget_create_tabbar(view->add_track_view_layout);
	MP_CHECK_VAL(view->add_track_view_tabbar, -1);

	elm_object_part_content_set(view->add_track_view_layout, "tabbar", view->add_track_view_tabbar);
	edje_object_signal_emit(_EDJ(view->add_track_view_layout), "SHOW_TABBAR", "*");
	return ret;
}

MpAddTrackView_t *mp_add_track_view_create(Evas_Object *parent, int playlist_id)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpAddTrackView_t *view = calloc(1, sizeof(MpAddTrackView_t));
	MP_CHECK_NULL(view);
	view->first_start = TRUE;
	view->playlist_id = playlist_id;

	ret = _mp_add_track_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	_mp_add_track_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_add_track_view_create()");
	IF_FREE(view);
	return NULL;
}


int mp_add_track_view_destory(MpAddTrackView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	mp_view_mgr_pop_a_view(GET_VIEW_MGR, (MpView_t *)view);

	return 0;
}

int mp_add_track_view_select_tab(MpAddTrackView_t *view, MpAddTrackViewTab_e tab)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	Evas_Object *content = NULL;
	Evas_Object *save_btn = NULL;
	GList *checked_list = NULL;

	DEBUG_TRACE("selected view: %d", tab);

	if (tab == MP_ADD_TRACK_VIEW_TAB_SONGS) {
		checked_list = mp_list_get_checked_path_list(view->content_to_show);
	}
	content = elm_object_part_content_unset(view->add_track_view_layout, "list_content");
	evas_object_del(content);
	mp_evas_object_del(view->selection_info);

	if (tab == MP_ADD_TRACK_VIEW_TAB_FOLDERS) {
		view->content_to_show = (MpList_t *)mp_folder_list_create(view->layout);
		MP_CHECK_VAL(view->content_to_show, -1);
		mp_folder_list_set_data((MpFolderList_t *)view->content_to_show, MP_FOLDER_LIST_FUNC, MP_LIST_FUNC_ADD_TRACK, -1);
		mp_list_update(view->content_to_show);
		content = mp_list_get_layout(view->content_to_show);
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
		save_btn = elm_object_item_part_content_get(view->navi_it, "title_right_btn");
		elm_object_disabled_set(save_btn, EINA_TRUE);
	} else if (tab == MP_ADD_TRACK_VIEW_TAB_ALBUMS) {
		view->content_to_show = (MpList_t *)mp_album_list_create(view->add_track_view_layout);
		MP_CHECK_VAL(view->content_to_show, -1);
		mp_album_list_set_data((MpAlbumList_t *)view->content_to_show, MP_ALBUM_LIST_FUNC, MP_LIST_FUNC_ADD_TRACK, -1);
		mp_list_update(view->content_to_show);
		content = ((MpAlbumList_t *)view->content_to_show)->layout;
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
		save_btn = elm_object_item_part_content_get(view->navi_it, "title_right_btn");
		elm_object_disabled_set(save_btn, EINA_TRUE);
	} else if (tab == MP_ADD_TRACK_VIEW_TAB_ARTIST) {
		view->content_to_show = (MpList_t *)mp_artist_list_create(view->add_track_view_layout);
		MP_CHECK_VAL(view->content_to_show, -1);
		mp_artist_list_set_data((MpArtistList_t *)view->content_to_show, MP_ARTIST_LIST_TYPE, MP_GROUP_BY_ARTIST, MP_ARTIST_LIST_FUNC, MP_LIST_FUNC_ADD_TRACK, -1);
		mp_list_update(view->content_to_show);
		content = ((MpArtistList_t *)view->content_to_show)->layout;
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
		save_btn = elm_object_item_part_content_get(view->navi_it, "title_right_btn");
		elm_object_disabled_set(save_btn, EINA_TRUE);
	}
#ifdef MP_FEATURE_ADD_TO_INCLUDE_PLAYLIST_TAB
	else if (tab == MP_ADD_TRACK_VIEW_TAB_PLAYLIST) {
		view->content_to_show = (MpList_t *)mp_playlist_list_create(view->add_track_view_layout);
		MP_CHECK_VAL(view->content_to_show, -1);
		mp_playlist_list_set_data((MpPlaylistList_t *)view->content_to_show, MP_PLAYLIST_LIST_FUNC, MP_LIST_FUNC_ADD_TRACK, -1);
		content = ((MpPlaylistList_t *)view->content_to_show)->layout;
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
		save_btn = elm_object_item_part_content_get(view->navi_it, "title_right_btn");
		elm_object_disabled_set(save_btn, EINA_TRUE);
	}
#endif
	else if (tab == MP_ADD_TRACK_VIEW_TAB_SONGS) {
		view->content_to_show = (MpList_t *)mp_track_list_create(view->add_track_view_layout);
		MP_CHECK_VAL(view->content_to_show, -1);
		mp_track_list_set_data((MpTrackList_t *)view->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_ALL, MP_TRACK_LIST_PLAYLIT_ID, view->playlist_id, MP_TRACK_LIST_CHECKED_LIST, checked_list, -1);
		mp_list_update(view->content_to_show);
		mp_list_set_edit(view->content_to_show, TRUE);
		mp_list_view_set_select_all((MpListView_t *)view, TRUE);
		edje_object_signal_emit(_EDJ(view->add_track_view_layout), "SHOW_SELECT_ALL_PADDING", "*");
		content = ((MpTrackList_t *)view->content_to_show)->layout;
		save_btn = elm_object_item_part_content_get(view->navi_it, "title_right_btn");
		if (save_btn) {
			elm_object_disabled_set(save_btn, EINA_TRUE);
		}
	} else {
		DEBUG_TRACE("tab out of control");
		return -1;
	}

	mp_list_show_fast_scroll(view->content_to_show);

	view->content_tab = tab;
	elm_object_part_content_set(view->add_track_view_layout, "list_content", content);
	evas_object_show(content);

	mp_view_update_options((MpView_t *)view);
	/*update title*/
	int count = mp_list_get_checked_count(view->content_to_show);
	if (count > 0) {

		char *text =  g_strdup_printf(GET_STR(STR_MP_SELECT_ITEMS), count);
		mp_view_set_title((MpView_t *)view, text);
		IF_FREE(text);
	} else {
		mp_view_set_title((MpView_t *)view, STR_MP_TILTE_SELECT_ITEM);
	}
	return 0;
}


