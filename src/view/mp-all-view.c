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

#include "mp-all-view.h"
#include "mp-widget.h"
#include "mp-create-playlist-view.h"
#include "mp-common.h"
#include "mp-popup.h"
#include "mp-util.h"
#include "mp-player-view.h"
#include "mp-play.h"
#include "mp-smart-event-box.h"
#include "mp-edit-view.h"
#include "mp-player-mgr.h"
#include "mp-edit-playlist.h"
#include "ms-key-ctrl.h"

static void _mp_all_view_playlist_list_create_playlist_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_all_view_edit_cb(void *data, Evas_Object *obj, void *event_info);
#ifdef MP_FEATURE_SHORTCUT
#ifndef MP_DEBUG_MODE
static void _mp_all_view_update_favourite(MpAllView_t *view);
#endif
static void _mp_all_view_update_shortcut(MpAllView_t *view);
#endif
static void _mp_all_view_content_load(void *thiz, MpTab_e tab);

#define TAB_COUNT 4
#define MP_INIT_VALUE -2
static char *tab_str[TAB_COUNT] = {STR_MP_PLAYLISTS, STR_MP_TRACKS, STR_MP_ALBUMS, STR_MP_ARTISTS};
static int tab_index[TAB_COUNT] = {0};
static int tab_index_exist[TAB_COUNT] = {0};

static void _mp_all_view_tabs_sequence_get();

static int _mp_all_view_tab_index_get(MpAllView_t *view)
{
	MP_CHECK_VAL(view, 0);
	int index = 0;
	if (view->tab_status < TAB_COUNT) {
		index = tab_index_exist[view->tab_status];
	}

	return index;

}


static void
_mp_all_view_destory_cb(void *thiz)
{
	eventfunc;
	MpAllView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/

	mp_ecore_timer_del(view->bringin_timer);
	mp_ecore_idler_del(view->show_last_idler);

	free(view);
}

int _mp_all_view_update(void *thiz)
{
	startfunc;
	MpAllView_t *view = thiz;

	mp_list_update(view->content_to_show);
	if (_mp_all_view_tab_index_get(view) == MP_TAB_SONGS && mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
		mp_list_show_fast_scroll(view->content_to_show);
	}

	if (_mp_all_view_tab_index_get(view) == MP_TAB_SONGS && mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show)) == 0) {
		mp_list_hide_fast_scroll(view->content_to_show);
	}
	return 0;
}

#ifdef MP_LIST_THUMBNAIL
static void _mp_all_view_view_as_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAllView_t *all_view = (MpAllView_t *)data;
	MP_CHECK(all_view);
	mp_evas_object_del(all_view->more_btn_ctxpopup);

	/*   remove popup   */
	/*

		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);

		if (mp_list_is_display_mode_changable(all_view->content_to_show)) {
			Evas_Object *popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_CHANGE_LIST_DISPLAY_MODE, all_view, ad);
			MP_CHECK(popup);

			mp_evas_object_del(all_view->radio_main);
			all_view->radio_main = elm_radio_add(popup);
			elm_radio_state_value_set(all_view->radio_main, -1);
			evas_object_smart_callback_add(all_view->radio_main, "changed", _mp_all_view_view_as_popup_radio_changed_cb, all_view);
			evas_object_event_callback_add(all_view->radio_main, EVAS_CALLBACK_DEL, _mp_all_view_radio_main_del_cb, all_view);
			evas_object_hide(all_view->radio_main);

			Evas_Object *genlist = evas_object_data_get(popup, "genlist");
			MP_CHECK(genlist);

			evas_object_data_set(genlist, "all_view", (void *)all_view);

			static Elm_Genlist_Item_Class itc;
			itc.item_style = "1text.1icon.3";
			itc.func.text_get = _mp_all_view_view_as_popup_label_get;
			itc.func.content_get = _mp_all_view_view_as_popup_content_get;
			itc.func.state_get = NULL;
			itc.func.del = NULL;

			int i = 0;
			while (i < MP_LIST_DISPLAY_MODE_MAX) {
				elm_genlist_item_append(genlist, &itc, (void *)i, NULL, ELM_GENLIST_ITEM_NONE,
						_mp_all_view_view_as_popup_item_sel, all_view);

				++i;
			}
		}
			*/
	MpListDisplayMode_e current = mp_list_get_display_mode(all_view->content_to_show);
	mp_debug("List display change request [%d => %d]", current);

	if (MP_LIST_DISPLAY_MODE_NORMAL == current) {
		current = MP_LIST_DISPLAY_MODE_THUMBNAIL;
	} else if (MP_LIST_DISPLAY_MODE_THUMBNAIL == current) {
		current = MP_LIST_DISPLAY_MODE_NORMAL;
	}

	all_view->display_mode[all_view->tab_status] = current;

	if (all_view->content_to_show) {
		mp_list_change_display_mode(all_view->content_to_show, current);
	}
}
#endif

static void _mp_all_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAllView_t *view = (MpAllView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_add_to_playlsit_view((MpListView_t *)view);

}

static void _mp_all_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAllView_t *view = (MpAllView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	int index  = _mp_all_view_tab_index_get(view);
	int count = 0;
	int playlistcount = mp_media_playlist_get_playlist_count_from_db();
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);

	/*create playlist */
	if (index == MP_TAB_PLAYLISTS) {
		if (playlistcount < 100) {
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_CREATE_PLAYLIST, MP_PLAYER_MORE_BTN_CREATE_PLAYLIST_IMAGE,
			                             _mp_all_view_playlist_list_create_playlist_cb, view);
			mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);
			evas_object_show(view->more_btn_ctxpopup);
		}
	}

#ifdef MP_FEATURE_SHARE
	/* share via */
	if (index == MP_TAB_SONGS) {
		if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_SHARE))
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_SHARE, MP_PLAYER_MORE_BTN_SHARE, mp_common_share_cb, view);
	}
#endif

	/*edit button*/
	if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL)) {
		if (index != MP_TAB_PLAYLISTS) {
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _mp_all_view_add_to_playlist_cb, view);
		}
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_DELETE, MP_PLAYER_MORE_BTN_DELETE_IMAGE, _mp_all_view_edit_cb, view);
		/*search*/
		if (count > 0) {
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_SEARCH, NULL, mp_common_create_search_view_cb, view);
		}

		/*view as*/
#ifdef MP_LIST_THUMBNAIL
		if (mp_list_is_display_mode_changable(view->content_to_show)) {
			MpListDisplayMode_e current = mp_list_get_display_mode(view->content_to_show);
			if (MP_LIST_DISPLAY_MODE_NORMAL == current)
				mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				                             STR_MP_ALL_THUMBNAIL_VIEW, MP_PLAYER_MORE_BTN_MORE_THUMBNAIL_VIEW_IMAGE, _mp_all_view_view_as_select_cb, view);
			else if (MP_LIST_DISPLAY_MODE_THUMBNAIL == current)
				mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				                             STR_MP_ALL_LIST_VIEW, MP_PLAYER_MORE_BTN_MORE_LIST_VIEW_IMAGE, _mp_all_view_view_as_select_cb, view);

		}
#endif

#ifdef MP_FEATURE_PERSONAL_PAGE
		if (index == MP_TAB_SONGS) {
			if (mp_util_is_personal_page_on()) {
				all_in_personal_e status = mp_common_is_all_in_personal_page(((MpList_t *)view->content_to_show)->genlist);
				/*add*/
				if (status != MP_COMMON_ALL_IN && status != MP_COMMON_ALL_ERROR)
					mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
					                             STR_MP_ADD_TO_PERSONAL_PAGE, MP_PLAYER_MORE_BTN_ADD_TO_PERSONAL_PAGE, mp_common_add_to_personal_page_cb, view);

				/*remove*/
				if (status != MP_COMMON_ALL_OUT && status != MP_COMMON_ALL_ERROR)
					mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
					                             STR_MP_REMOVE_FROM_PERSONAL_PAGE, MP_PLAYER_MORE_BTN_REMOVE_FROM_PERSONAL_PAGE, mp_common_remove_from_personal_page_cb, view);
			}
		}
#endif


		/*settings*/
		/*mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
					STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, mp_common_ctxpopup_setting_cb, view);*/
#ifndef MP_FEATURE_NO_END
		/*End*/
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
		mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);
		evas_object_show(view->more_btn_ctxpopup);
	}
}


/***************	functions for track list update 	*******************/
static void _mp_all_view_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAllView_t *view = (MpAllView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_show_edit_view((MpListView_t *)view, MP_DONE_DELETE_TYPE);
}

Eina_Bool _mp_all_view_pop_cb(void *data, Elm_Object_Item *it)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);
	elm_win_lower(ad->win_main);
	return FALSE;
}

static void _mp_all_view_update_toolbar_option(void *thiz)
{
	startfunc;
	MpAllView_t *view = (MpAllView_t *)thiz;
	MP_CHECK(view);

	Evas_Object *btn = NULL;
	btn = mp_widget_create_toolbar_btn(view->all_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_all_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);
	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_all_view_pop_cb, view);

	endfunc;
	return;
}

/***************	functions for playlist list update 	*******************/
static void _mp_all_view_playlist_list_create_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpAllView_t *view = (MpAllView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	Mp_Playlist_Data *mp_playlist_data = mp_edit_playlist_create(MP_PLST_CREATE_TO_ADD_TRACK);
	mp_edit_playlist_content_create(mp_playlist_data);
}

int _mp_all_view_update_options(void *thiz)
{
	startfunc;
#ifdef MP_CREATE_FAKE_IMAGE
	return 0;
#endif
	MpAllView_t *view = (MpAllView_t *)thiz;
	MP_CHECK_VAL(view->navi_it, -1);

	PROFILE_IN("mp_view_clear_options");
	mp_view_clear_options((MpView_t *)view);
	PROFILE_OUT("mp_view_clear_options");
	/*add search btn*/

	mp_view_set_title_visible((MpView_t *)view, true);

	/*PROFILE_IN("elm_naviframe_item_title_enabled_set");
	elm_naviframe_item_title_enabled_set(view->navi_it, EINA_TRUE, EINA_FALSE);
	PROFILE_OUT("elm_naviframe_item_title_enabled_set");*/

	PROFILE_IN("_mp_all_view_update_toolbar_option");
	_mp_all_view_update_toolbar_option(thiz);
	PROFILE_OUT("_mp_all_view_update_toolbar_option");

	/*edje_object_signal_emit(_EDJ(view->all_view_layout), "show,tabbar", "*");*/
	return 0;
}

static int
_mp_all_view_playlist_update(void *thiz)
{
	MpAllView_t *view = (MpAllView_t *)thiz;
	MP_CHECK_VAL(view, -1);
	if (_mp_all_view_tab_index_get(view) == MP_TAB_PLAYLISTS) {
		mp_view_update((MpView_t *)view);
		mp_view_update_options((MpView_t *)view);
	}
	return 0;
}

static void _all_view_tab_change_cb(void *data, Evas_Object * obj, void *event_info)
{
	MpAllView_t *view = data;
	Evas_Object *content = NULL;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/*the normal case*/
	if (!view->reorder_flag) {
		Elm_Object_Item *it = NULL;
		Elm_Object_Item *it2 = NULL;

		it = elm_toolbar_selected_item_get(obj);
		mp_retm_if(it == NULL, "tab item is NULL");

		it2 = elm_toolbar_first_item_get(obj);

		int i = 0;
		for (i = 0; i < MP_TAB_MAX; i++) {
			if (it == it2) {
				break;
			}
			it2 = elm_toolbar_item_next_get(it2);
			if (!it2) {
				break;
			}
		}

		if (view->tab_status == i) {
			return;
		}

		view->tab_status = i;
	}


	MpList_t *list = NULL;
	DEBUG_TRACE("view->tab_status %d", view->tab_status);
	/*record the tab status for reorder*/
	int index = _mp_all_view_tab_index_get(view);

	view->history = _mp_all_view_tab_index_get(view);

	ad->del_cb_invoked = 0;

	switch (index) {
	case MP_TAB_PLAYLISTS:
		list = (void *)mp_playlist_list_create(view->all_view_layout);
		if (view->tab_status < MP_TAB_MAX) {
			mp_playlist_list_set_data((MpPlaylistList_t *)list, MP_PLAYLIST_LIST_DISPLAY_MODE, view->display_mode[view->tab_status], -1);
		}
		mp_list_update(list);
		break;
	case MP_TAB_ALBUMS:
		list = (void *)mp_album_list_create(view->all_view_layout);
		mp_album_list_set_data((MpAlbumList_t *)list, MP_ALBUM_LIST_DISPLAY_MODE, view->display_mode[view->tab_status], -1);
		mp_list_update(list);
		if (view->display_mode[view->tab_status] == MP_LIST_DISPLAY_MODE_NORMAL) {
			mp_list_show_fast_scroll(list);
		}
		break;
	case MP_TAB_ARTISTS:
		list = (void *)mp_artist_list_create(view->all_view_layout);
		mp_artist_list_set_data((MpArtistList_t *)list, MP_ARTIST_LIST_DISPLAY_MODE, view->display_mode[view->tab_status], -1);
		mp_list_update(list);
		if (view->display_mode[view->tab_status] == MP_LIST_DISPLAY_MODE_NORMAL) {
			mp_list_show_fast_scroll(list);
		}
		break;
		/*case MP_TAB_FOLDERS:
			list = (void *)mp_folder_list_create(view->all_view_layout);
			mp_folder_list_set_data((MpFolderList_t *)list, MP_FOLDER_LIST_TYPE, MP_GROUP_BY_FOLDER, -1);
			mp_list_update(list);
			if (view->display_mode[view->tab_status] == MP_LIST_DISPLAY_MODE_NORMAL)
				mp_list_show_fast_scroll(list);
			break;*/
	case MP_TAB_SONGS:
	default:
		PROFILE_IN("mp_track_list_create");
		list = (void *)mp_track_list_create(view->all_view_layout);
		PROFILE_OUT("mp_track_list_create");
		PROFILE_IN("mp_list_update");
		mp_list_update(list);
		PROFILE_OUT("mp_list_update");
		PROFILE_IN("mp_list_show_fast_scroll");
		mp_list_show_fast_scroll(list);
		PROFILE_OUT("mp_list_show_fast_scroll");
		break;
	}

	view->content_to_show = list;
	content = mp_list_get_layout(list);
	if (content != NULL) {
		elm_object_part_content_set(view->all_view_layout, "list-content", content);
	}

	/*reset the flag*/
	view->reorder_flag = FALSE;
}

static Eina_Bool _item_bring_in_first(void *data)
{
	MpAllView_t *view = data;
	MP_CHECK_FALSE(view);

	Elm_Object_Item *it = NULL;
	it = elm_toolbar_first_item_get(view->all_view_tabbar);

	elm_toolbar_item_bring_in(it, ELM_TOOLBAR_ITEM_SCROLLTO_FIRST);

	view->bringin_timer = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Eina_Bool _item_show_last(void *data)
{
	MpAllView_t *view = data;
	MP_CHECK_FALSE(view);

	Elm_Object_Item *it = NULL;
	it = elm_toolbar_last_item_get(view->all_view_tabbar);

	elm_toolbar_item_show(it, ELM_TOOLBAR_ITEM_SCROLLTO_IN);

	view->show_last_idler = NULL;
	view->bringin_timer = ecore_timer_add(0.3, _item_bring_in_first, view);
	return ECORE_CALLBACK_CANCEL;
}

static void _mp_all_view_tabs_sequence_get()
{
	char *get_str = NULL;
	ms_key_get_tabs_str(&get_str);
	int value = atoi(get_str);
	int j = 0;
	for (j = TAB_COUNT - 1; j >= 0 ; j--) {
		tab_index[j] = value % 10;
		value = value / 10;
	}

}

static Evas_Object *_all_view_create_tabbar(Evas_Object *parent, MpAllView_t *view)
{
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(view);

	Evas_Object *obj = mp_widget_create_tabbar(parent);

	int i = 0;

	memset(tab_index_exist, 0, sizeof(tab_index_exist));

	/*append the exists tab*/
	for (i = 0; i < TAB_COUNT; i++) {
		mp_util_toolbar_item_append(obj, NULL, tab_str[i], _all_view_tab_change_cb, view);
		tab_index_exist[i] = i;
	}

	elm_toolbar_item_selected_set(mp_util_toolbar_nth_item(obj, view->tab_status), EINA_TRUE);
	DEBUG_TRACE("select tab is %d", view->tab_status);

	view->show_last_idler = ecore_idler_add(_item_show_last, view);
	evas_object_show(obj);

	return obj;
}


static void
_mp_all_view_content_load(void *thiz, MpTab_e tab)
{
	startfunc;
	MP_CHECK(thiz);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpAllView_t *view = (MpAllView_t *)thiz;
	view->reorder_flag = TRUE;
	view->tab_status = 0;
	/*we set the removed tab -1,so should set other value*/
	view->history = MP_INIT_VALUE;

	PROFILE_IN("_all_view_create_tabbar");
	Evas_Object *tabbar = _all_view_create_tabbar(view->all_view_layout, view);
	PROFILE_OUT("_all_view_create_tabbar");

	elm_object_part_content_set(view->all_view_layout, "tabbar", tabbar);
	/*elm_naviframe_item_style_set(view->navi_it, "tabbar/notitle");
	elm_object_item_part_content_set(view->navi_it, "tabbar", tabbar);*/
	view->all_view_tabbar = tabbar;
	endfunc;
}

static void _mp_all_view_popup_delete_update_genlist(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpList_t *list = thiz;
	MP_CHECK(list->genlist);

	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);

	if (count <= 0) {
		mp_list_update(thiz);
		mp_list_hide_fast_scroll((MpList_t *)list);
	} else {
		elm_genlist_realized_items_update(list->genlist);
	}
}

static void _mp_all_view_tabs_refresh(void *thiz)
{
	startfunc;
	MP_CHECK(thiz);
	MpAllView_t *view = (MpAllView_t *)thiz;

	Evas_Object *content = elm_object_part_content_get(view->all_view_layout, "tabbar");
	if (content) {
		mp_evas_object_del(content);
		view->all_view_tabbar = NULL;
	}
	_mp_all_view_tabs_sequence_get();

	view->reorder_flag = TRUE;
	Evas_Object *tabbar = _all_view_create_tabbar(view->all_view_layout, view);
	PROFILE_OUT("_all_view_create_tabbar");

	elm_object_part_content_set(view->all_view_layout, "tabbar", tabbar);
	view->all_view_tabbar = tabbar;
	endfunc;
}

static void
_mp_all_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpAllView_t *view = thiz;

	MpList_t *list = view->content_to_show;
	int index = _mp_all_view_tab_index_get(view);

	switch (event) {
	case MP_SETTING_PLAYLIST_CHANGED:
		if (index == MP_TAB_PLAYLISTS) {
			mp_view_update(thiz);
		}
		break;
		/*case MP_PLAY_TIME_COUNT_UPDATED:*/
	case MP_ADD_TO_PLAYLIST_DONE:
#ifdef MP_FEATURE_SHORTCUT
		/*_mp_all_view_update_shortcut(view);*/
#endif
		if (index == MP_TAB_PLAYLISTS) {
			mp_view_update(thiz);
		}
		break;
	case MP_LANG_CHANGED: {
		MpListDisplayMode_e current = mp_list_get_display_mode(list);
		if (index == MP_TAB_ARTISTS && MP_LIST_DISPLAY_MODE_NORMAL == current) {
			mp_list_realized_item_part_update(list, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
		}
	}
	break;
	case MP_POPUP_DELETE_DONE:
		if (index == MP_TAB_PLAYLISTS) {
			mp_view_update(thiz);
		} else {
			_mp_all_view_popup_delete_update_genlist(list);
		}
		break;
	case MP_DELETE_DONE:
		mp_list_update(list);
		if (index != MP_TAB_PLAYLISTS) {
			if (list->genlist) {
				int count = 0;
				count = elm_genlist_items_count(list->genlist);
				if (count <= 0) {
					mp_list_hide_fast_scroll((MpList_t *)list);
				}
			} else {
				mp_list_hide_fast_scroll((MpList_t *)list);
			}
		}
		break;
	case MP_PLAYLIST_RENAMED:
		mp_list_update(list);
		break;
	case MP_PLAYLIST_MGR_ITEM_CHANGED:
		mp_view_update_nowplaying((MpView_t *)view, true);
		break;
	case MP_PLAYLIST_CREATED:
		_mp_all_view_playlist_update(view);
		break;
	case MP_SIP_STATE_CHANGED:
		/*
			if (view->navi_it) {
				const char *signal = mp_util_get_sip_state() ? "elm,state,toolbar,instant_close" : "elm,state,toolbar,instant_open";
				elm_object_item_signal_emit(view->navi_it, signal, "");

				bool title_visible = (mp_util_get_sip_state() && mp_util_is_landscape()) ? false : true;
				mp_view_set_title_visible(thiz, title_visible);
			}
			*/
		break;
#ifndef MP_SOUND_PLAYER
	case MP_UPDATE_PLAYING_LIST:
		mp_list_realized_item_part_update(list, "elm.text.main.left.top", ELM_GENLIST_ITEM_FIELD_TEXT);
		mp_list_realized_item_part_update(list, "elm.text.sub.left.bottom", ELM_GENLIST_ITEM_FIELD_TEXT);
		break;
#endif
#ifdef MP_FEATURE_LANDSCAPE
	case MP_VIEW_ROTATE:
		mp_list_rotate(list);
		break;
#endif
	case MP_PLAYLIST_IMAGE_UPDATED:
		mp_list_update(list);
		break;
#ifdef MP_FEATURE_PERSONAL_PAGE
	case MP_PERSONAL_PAGE_ON:
		view->personal_page_status = true;
		mp_list_update(list);
		break;
	case MP_PERSONAL_PAGE_OFF:
		view->personal_page_status = false;
		mp_list_update(list);
		break;
#endif
	case MP_VIEW_EVENT_ALBUMART_CHANGED:
		if (index == MP_TAB_SONGS) {
			mp_list_realized_item_part_update(list, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
		} else {
			mp_list_update(list);
		}
		break;
	case MP_UPDATE_FAVORITE_LIST: {
		if (index == MP_TAB_PLAYLISTS) {
			mp_list_update(list);
		}
		break;
	}
	case MP_TABS_ITEM_CHANGED:
	case MP_TABS_REORDER_DONE: {
		_mp_all_view_tabs_refresh(view);
		break;
	}
	case MP_PLAYLISTS_REORDER_DONE: {
		_mp_all_view_tabs_sequence_get();
		if (tab_index[view->tab_status] - 1 == MP_TAB_PLAYLISTS) {
			mp_view_update(thiz);
		}
		break;
	}
	case MP_START_PLAYBACK:
	case MP_RESUME_PLAYBACK:
	case MP_PAUSE_PLAYBACK:
	case MP_PLAYING_TRACK_CHANGED:
	case MP_STOP_PLAYBACK: {
		if (index == MP_TAB_SONGS) {
			mp_list_realized_item_part_update(list, "elm.icon.left", ELM_GENLIST_ITEM_FIELD_CONTENT);
		}
		break;
	}
	default:
		break;
	}
}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_all_view_rotate_cb(void *thiz, int randscape)
{
	DEBUG_TRACE("allview rotated");
	MpAllView_t *view = thiz;
	MP_CHECK(view);

	MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);

	if (mp_util_get_sip_state() && (int)top_view == (int)view) {
		_mp_all_view_on_event(view, MP_SIP_STATE_CHANGED);
	}

	_mp_all_view_on_event(view, MP_VIEW_ROTATE);
}
#endif

#ifdef MP_FEATURE_SHORTCUT
#ifndef MP_DEBUG_MODE
static void
_mp_all_view_update_favourite(MpAllView_t *view)
{
	startfunc;
	/*mp_all_list_update_favourite((MpAllList_t *)view->content_to_show);*/
}
#endif
static void
_mp_all_view_update_shortcut(MpAllView_t *view)
{
	startfunc;

	/*mp_all_list_update_shortcut((MpAllList_t *)view->content_to_show);*/
}


/*#define SHORCUT_W 720*elm_config_scale_get()*/
#define LARGE_BOX_SIZE 480*elm_config_scale_get()
#define SMALL_BOX_SIZE 240*elm_config_scale_get()
#define LANDSCAPE_BOX_SIZE 377*elm_config_scale_get()
#define BOX_COUNT 3

#endif


static int
_mp_all_view_init(Evas_Object *parent, MpAllView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_ALL);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_all_view_update;
	view->update_options = _mp_all_view_update_options;
	view->view_destroy_cb = _mp_all_view_destory_cb;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_all_view_rotate_cb;
#endif
	view->on_event = _mp_all_view_on_event;

	view->all_view_layout = mp_common_load_edj(view->layout, MP_EDJ_NAME, "common_view_layout");
	MP_CHECK_VAL(view->all_view_layout, -1);

	elm_object_part_content_set(view->layout, "list_content", view->all_view_layout);

	return ret;
}

MpAllView_t *mp_all_view_create(Evas_Object *parent, MpTab_e init_tab)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpAllView_t *view = calloc(1, sizeof(MpAllView_t));
	MP_CHECK_NULL(view);

	PROFILE_IN("_mp_all_view_init");
	ret = _mp_all_view_init(parent, view);
	PROFILE_OUT("_mp_all_view_init");
	if (ret) {
		goto Error;
	}

#ifdef MP_FEATURE_PERSONAL_PAGE
	view->personal_page_status = mp_util_is_personal_page_on();
#endif
	PROFILE_IN("_mp_all_view_content_load");
	_mp_all_view_content_load(view, init_tab);
	PROFILE_OUT("_mp_all_view_content_load");
	return view;

Error:
	ERROR_TRACE("Error: mp_all_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_all_view_destory(MpAllView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	return 0;
}

int mp_all_view_select_tab(MpAllView_t *view, MpTab_e tab)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	/*mp_all_list_select_tab((MpAllList_t *)view->content_to_show, tab);*/
	elm_toolbar_item_selected_set(mp_util_toolbar_nth_item(view->all_view_tabbar, tab), EINA_TRUE);
	mp_view_update_options((MpView_t *)view);
	return 0;
}


