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

#include "mp-playlist-detail-view.h"
#include "mp-create-playlist-view.h"
#include "mp-add-track-view.h"
#include "mp-common.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-popup.h"
#include "mp-ug-launch.h"
#include "mp-edit-playlist.h"
#include <efl_extension.h>

static void _mp_playlist_detail_view_tracklist_add_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_playlist_detail_view_tracklist_reorder_cb(void *data, Evas_Object *obj, void *event_info);

static void _mp_playlist_detail_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	elm_naviframe_item_pop(GET_NAVIFRAME);
}

static void
_mp_playlist_detail_view_destory_cb(void *thiz)
{
	eventfunc;
	MpPlaylistDetailView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/
	mp_playlist_detail_view_destory(view);

	free(view);
}

int _mp_playlist_detail_view_update(void *thiz)
{
	startfunc;
	MpPlaylistDetailView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	view->content_set(view);
	mp_view_update_options((MpView_t *)view);

	return 0;
}

static void
_mp_playlist_detail_rename_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	Mp_Playlist_Data *mp_playlist_data = mp_edit_playlist_create(MP_PLST_RENAME);
	MP_CHECK(mp_playlist_data);
	MpTrackList_t *track_list = (MpTrackList_t *)view->content_to_show;
	if (track_list != NULL) {
		mp_playlist_data->playlist_handle = track_list->playlist_handle;
	}
	mp_edit_playlist_content_create(mp_playlist_data);
}


#ifdef MP_FEATURE_ADD_TO_HOME
static char *_mp_media_info_get_live_auto_playlist_icon_by_name(const char *name)
{
	MP_CHECK_VAL(name, NULL);

	char icon_path[1024] = {0};
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

	free(shared_path);

	return icon_path;
}

static char *_mp_media_info_get_live_auto_playlist_thumbnail_by_name(const char *name)
{
	MP_CHECK_VAL(name, NULL);

	char thumb_path[1024] = {0};
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

static void
_mp_playlist_detail_add_to_home_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	int type = 0;
	void *name = NULL;
	const char *extra1 = NULL;
	const char *extra2 = NULL;

	if (view->p_id < 0) {
		type = MP_ADD_TO_HOME_SHORTCUT_TYPE_SYS_PLAYLIST;
		name = (void *)view->name;
		extra1 = _mp_media_info_get_live_auto_playlist_thumbnail_by_name(view->name);
		extra2 = _mp_media_info_get_live_auto_playlist_icon_by_name(view->name);
	} else {
		type = MP_ADD_TO_HOME_SHORTCUT_TYPE_USER_PLAYLIST;
		name = (void *)view->p_id;
	}
	mp_menu_add_to_home(type, name, (void *)extra1, (void *)extra2);
}
#endif

#ifdef MP_FEATURE_EDIT_PLAYLIST_IMAGE
static void
_mp_playlist_list_take_picture_button_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_popup_destroy(ad);

	int ret = mp_ug_camera_take_picture(mp_list_get_playlist_handle(view->content_to_show));
	if (ret != 0) {
		ERROR_TRACE("taken picture failed");
	}
}

static void
_mp_playlist_list_change_image_button_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_popup_destroy(ad);

	int ret = mp_ug_gallery_get_picture(mp_list_get_playlist_handle(view->content_to_show));
	if (ret != 0) {
		ERROR_TRACE("taken picture failed");
	}

}

static void
_mp_playlist_detail_edit_image_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)data;
	MP_CHECK(view);
	Evas_Object *popup = NULL;

	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	popup = mp_genlist_popup_create(obj, MP_POPUP_EDIT_IMAGE, ad, ad);
	MP_CHECK(popup);

	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);

	mp_genlist_popup_item_append(popup, GET_STR(STR_MP_TAKE_PICTURE), NULL, NULL, NULL,
	                             _mp_playlist_list_take_picture_button_cb, data);
	mp_genlist_popup_item_append(popup, GET_STR(STR_MP_CHANGE_IMAGE), NULL, NULL, NULL,
	                             _mp_playlist_list_change_image_button_cb, data);

	evas_object_show(popup);

	return;
}
#endif

static void _mp_playlist_detail_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);


	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

#ifdef MP_FEATURE_SHARE
	if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_SHARE)) {
		/*share*/
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_SHARE, MP_PLAYER_MORE_BTN_SHARE, mp_common_share_cb, view);
	}
#endif

	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	if (count > 0) {
		if ((view->list_type == MP_TRACK_BY_PLAYLIST) || !strcmp((STR_MP_FAVOURITES), view->name) ||
		        !strcmp(GET_STR(STR_MP_FAVOURITES), view->name)) {
			/*add tracks*/
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_ADD_TRACKS, MP_PLAYER_MORE_BTN_ADD,
			                             _mp_playlist_detail_view_tracklist_add_cb,
			                             view);
		}

#ifdef MP_FEATURE_CLOUD
		/*cloud */
		int is_on = false;
		mp_cloud_is_on(&is_on);
		if (is_on) {
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_VIEW, MP_PLAYER_MORE_BTN_VIEW, mp_common_cloud_view_cb, view);

			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_MAKE_AVAILABLE_OFFLINE, MP_PLAYER_MORE_BTN_MAKE_AVAILABLE_OFFICE, mp_common_ctxpopup_make_offline_cb, view);
		}
#endif

		if (view->list_type == MP_TRACK_BY_PLAYLIST) {
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_RENAME, MP_PLAYER_MORE_BTN_RENAME_IMAGE, _mp_playlist_detail_rename_cb, view);
		}

		if ((mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL) > 1) &&
		        view->list_type == MP_TRACK_BY_PLAYLIST) {
			/*reorder*/
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_REORDER, MP_PLAYER_MORE_BTN_SET_REORDER,
			                             _mp_playlist_detail_view_tracklist_reorder_cb,
			                             view);
		}

#ifdef MP_FEATURE_EDIT_PLAYLIST_IMAGE
		/*edit image*/
		if (view->p_id >= 0) {
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_EDIT_IMAGE, MP_PLAYER_MORE_BTN_EDIT_IMAGE, _mp_playlist_detail_edit_image_cb, view);
		}
#endif

		if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL)) {
			/*remove*/
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_REMOVE, MP_PLAYER_MORE_BTN_EDIT,
			                             mp_playlist_detail_view_tracklist_edit_cb,							view);
		}

		if (count) {
			/*search*/
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_SEARCH, NULL, mp_common_create_search_view_cb, view);
		}

		/*mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
					STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, mp_common_ctxpopup_setting_cb, view);*/
#ifndef MP_FEATURE_NO_END
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
		/*
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
						GET_SYS_STR(STR_MP_END), MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
		*/

		mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

		evas_object_show(view->more_btn_ctxpopup);
	} else {
		if (view->list_type == MP_TRACK_BY_PLAYLIST) {
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_RENAME, MP_PLAYER_MORE_BTN_RENAME_IMAGE, _mp_playlist_detail_rename_cb, view);
			mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

			evas_object_show(view->more_btn_ctxpopup);
		}
	}
}

/***************	functions for track list update 	*******************/
void mp_playlist_detail_view_tracklist_edit_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *) data;
	MP_CHECK(view);

	/*
	if (view->list_type == MP_TRACK_BY_PLAYLIST) {
		mp_list_set_reorder((MpList_t *)view->content_to_show, TRUE);

	}
	*/
	view->content_to_show->reorderable = 0;

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_show_edit_view((MpListView_t *)view, MP_DONE_REMOVED_TYPE);
}

static void _mp_playlist_detail_view_tracklist_reorder_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	mp_common_set_list_to_reorder_view((MpListView_t *)view);
}

static void _mp_playlist_detail_view_tracklist_add_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpPlaylistDetailView_t *parent_view = (MpPlaylistDetailView_t *)data;
	MP_CHECK(parent_view);
	mp_common_show_add_tracks_view(parent_view->p_id);
}

/*static Eina_Bool _mp_playlist_detail_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	MpTrackList_t *track_list = (MpTrackList_t *)view->content_to_show;
	MP_CHECK_VAL(track_list, EINA_TRUE);

	ERROR_TRACE("track_list->reorderable = %d",track_list->reorderable);

	if (track_list->reorderable == 1) {
		mp_list_set_reorder((MpList_t *)track_list, FALSE);
	}

	if (track_list->edit_mode == 1) {
		mp_list_set_edit((MpList_t *)track_list, FALSE);
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
		mp_view_update_options((MpView_t *)view);
		mp_evas_object_del(view->selection_info);
		return EINA_FALSE;
	} else {
		DEBUG_TRACE("");
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		mp_view_mgr_pop_view(view_mgr, false);
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_FAVORITE_LIST);
	}
	return EINA_TRUE;
}*/

static Eina_Bool _mp_playlist_detail_view_pop_cb(void *data, Elm_Object_Item *it)
{
	startfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)data;
	MP_CHECK_VAL(view, EINA_TRUE);

	mp_view_mgr_pop_view(GET_VIEW_MGR, true);
	endfunc;
	return EINA_TRUE;
}

static int _mp_playlist_detail_view_update_options(void *thiz)
{
	startfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	Evas_Object *btn = NULL;

	btn = mp_widget_create_toolbar_btn(view->playlist_detail_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_playlist_detail_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);
	/*view->toolbar_options[MP_OPTION_MORE] = btn;

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_playlist_detail_view_back_cb, view);*/

	Evas_Object *back_button = elm_button_add(view->playlist_detail_view_layout);
	elm_object_style_set(back_button, "naviframe/end_btn/default");
	elm_object_item_part_content_set(view->navi_it, "prev_btn", back_button);
	evas_object_smart_callback_add(back_button, "clicked", _mp_playlist_detail_view_cb, view);
	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_playlist_detail_view_pop_cb, view);

	/* update the first controlbar item */
	/* mp_view_manager_update_first_controlbar_item(layout_data);*/
	endfunc;
	return 0;
}

static void _mp_playlist_detail_view_content_load(void *thiz)
{
	startfunc;
	Evas_Object *content = NULL;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)thiz;
	MP_CHECK(view);

	view->content_to_show = (MpList_t *)mp_track_list_create(view->layout);

	if (view->list_type == MP_TRACK_BY_PLAYLIST) {
		DEBUG_TRACE("playlist id: %d", view->p_id);
		mp_track_list_set_data((MpTrackList_t *)view->content_to_show, MP_TRACK_LIST_TYPE, view->list_type, MP_TRACK_LIST_PLAYLIT_ID, view->p_id, -1);
	} else {
		mp_track_list_set_data((MpTrackList_t *)view->content_to_show, MP_TRACK_LIST_TYPE, view->list_type, -1);
	}
	/*mp_track_list_set_data((MpTrackList_t *)view->content_to_show, MP_TRACK_LIST_INDEX_TYPE, MP_TRACK_LIST_INDEX_ALBUM_ART_LIST, -1);*/

	mp_list_update(view->content_to_show);
	content = mp_list_get_layout(view->content_to_show);
	if (content != NULL) {
		elm_object_part_content_set(view->playlist_detail_view_layout, "list_content", content);
	}
}

static void
_mp_playlist_detail_view_on_event_cb(void *thiz, MpViewEvent_e event)
{
	MpPlaylistDetailView_t *view = thiz;
	MP_CHECK(view);

	mp_debug("event = %d", event);
	switch (event) {
	case MP_ADD_TO_PLAYLIST_DONE: {
		int count = mp_list_get_editable_count((MpList_t *)view->content_to_show, mp_list_get_edit_type(view->content_to_show));
		if (count != 0) {
			mp_track_list_update_albumart_index((MpTrackList_t *)view->content_to_show);
		}
		break;
	}
	case MP_DELETE_DONE:
	case MP_PLAYLIST_REORDER_DONE:
		mp_list_update(view->content_to_show);
		break;
	case MP_POPUP_DELETE_DONE:
		mp_track_list_popup_delete_genlist_item(view->content_to_show);
		mp_track_list_update_genlist(view->content_to_show);
		break;
	case MP_UPDATE_FAVORITE_LIST: {
		mp_list_update(view->content_to_show);
		break;
	}
#ifdef MP_FEATURE_LANDSCAPE
	case MP_VIEW_ROTATE: {
		Evas_Object *content = NULL;
		content = elm_object_part_content_unset(view->playlist_detail_view_layout, "list-content");
		evas_object_del(content);
		/*the selected track can not be removed when screen rotated*/
		mp_track_list_update_genlist(view->content_to_show);
		/*mp_list_update(view->content_to_show);*/
		int count = mp_list_get_editable_count((MpList_t *)view->content_to_show, mp_list_get_edit_type(view->content_to_show));
		if (count != 0) {
			mp_track_list_update_albumart_index((MpTrackList_t *)view->content_to_show);
		}

		mp_view_set_nowplaying(thiz);
		break;
	}
#endif
#ifndef MP_SOUND_PLAYER
	case MP_UPDATE_PLAYING_LIST:
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.main.left.top", ELM_GENLIST_ITEM_FIELD_TEXT);
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.sub.left.bottom", ELM_GENLIST_ITEM_FIELD_TEXT);
		break;
#endif
	case MP_PLAY_TIME_COUNT_UPDATED: {
		if (view->p_id == MP_SYS_PLST_RECENTELY_PLAYED ||
		        view->p_id == MP_SYS_PLST_MOST_PLAYED) {
			Evas_Object *content = NULL;
			content = elm_object_part_content_unset(view->playlist_detail_view_layout, "list-content");
			evas_object_del(content);
			mp_list_update(view->content_to_show);
			int count = mp_list_get_editable_count((MpList_t *)view->content_to_show, mp_list_get_edit_type(view->content_to_show));
			if (count != 0) {
				mp_track_list_update_albumart_index((MpTrackList_t *)view->content_to_show);
			}
		}
		break;
	}
	case MP_VIEW_EVENT_ALBUMART_CHANGED:
		mp_list_realized_item_part_update(view->content_to_show, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
		break;
	case MP_VIEW_TRANSITION_FINISHED:
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_CREATED);
//		eext_object_event_callback_add(GET_VIEW_MGR->navi, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
		break;
	case MP_START_PLAYBACK:
	case MP_RESUME_PLAYBACK:
	case MP_PAUSE_PLAYBACK:
	case MP_PLAYING_TRACK_CHANGED:
	case MP_STOP_PLAYBACK: {
		mp_list_realized_item_part_update(view->content_to_show, "elm.icon.left", ELM_GENLIST_ITEM_FIELD_CONTENT);
		break;
	}
	default:
		break;
	}

	endfunc;
}

#ifdef MP_FEATURE_LANDSCAPE
static void _mp_playlist_detail_view_rotate_cb(void *thiz, int randscape)
{
	startfunc;

	DEBUG_TRACE("playlist_detail_view rotated");
	MpPlaylistDetailView_t *view = thiz;
	MP_CHECK(view);

	_mp_playlist_detail_view_on_event_cb(view, MP_VIEW_ROTATE);
	endfunc;
}
#endif

static void _mp_playlist_detail_view_resume(void *thiz)
{
	startfunc;
	MpPlaylistDetailView_t *view = (MpPlaylistDetailView_t *)thiz;
	mp_view_freeze_nowplaying((MpView_t *)thiz, 0);

	/*check is no content*/
	MpTrackList_t *track_list = (MpTrackList_t *)view->content_to_show;
	if (track_list->no_content != NULL) {
		mp_list_update((MpList_t *)track_list);
	}
}

static int
_mp_playlist_detail_view_init(Evas_Object *parent, MpPlaylistDetailView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_PLAYLIST_DETAIL);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_playlist_detail_view_update;
	view->update_options = _mp_playlist_detail_view_update_options;
	/*view->update_options_edit = _mp_playlist_detail_view_update_options_edit;*/
	view->view_destroy_cb = _mp_playlist_detail_view_destory_cb;
	view->content_set = _mp_playlist_detail_view_content_load;
	view->on_event = _mp_playlist_detail_view_on_event_cb;
	view->view_resume = _mp_playlist_detail_view_resume;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_playlist_detail_view_rotate_cb;
#endif

	/*Todo: we need to add a new layout style*/
	view->playlist_detail_view_layout = view->layout;
	MP_CHECK_VAL(view->playlist_detail_view_layout, -1);
	return ret;
}

MpPlaylistDetailView_t *mp_playlist_detail_view_create(Evas_Object *parent, mp_track_type_e list_type, char *name, int p_id)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpPlaylistDetailView_t *view = calloc(1, sizeof(MpPlaylistDetailView_t));
	MP_CHECK_NULL(view);

	ret = _mp_playlist_detail_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	view->list_type = list_type;
	view->name = g_strdup(name);
	view->p_id = p_id;
	view->content_set(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_playlist_detail_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_playlist_detail_view_destory(MpPlaylistDetailView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	IF_G_FREE(view->name);

	return 0;
}



