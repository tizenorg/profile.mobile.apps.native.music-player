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

#include "mp-folder-detail-view.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-util.h"

#define MP_MAX_TEXT_PRE_FORMAT_LEN 256
#define MP_MAX_ARTIST_NAME_WIDTH 320
#define MP_LABEL_SLIDE_DURATION 5

static void _mp_folder_detail_view_tracklist_edit_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_folder_detail_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info);

static void
_mp_folder_detail_view_destory_cb(void *thiz)
{
	eventfunc;
	MpFolderDetailView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource.. */

	free(view);
}

int _mp_folder_detail_view_update(void *thiz)
{
	startfunc;
	MpFolderDetailView_t *view = thiz;

	MP_CHECK_VAL(view, -1);
	int edit_flag = view->content_to_show->edit_mode;
	view->content_set(view);

	int count = mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL);

	if (!count) {
		mp_view_mgr_pop_a_view(GET_VIEW_MGR, thiz);
	}

	if (edit_flag) {
		mp_list_set_edit((MpList_t *)view->content_to_show, true);
		mp_view_update_options_edit((MpView_t *)view);
		mp_list_view_set_select_all((MpListView_t *)view, true);
	} else {
		mp_view_update_options((MpView_t *)view);
		mp_list_view_set_select_all((MpListView_t *)view, false);
	}
	return 0;
}

static void _mp_folder_detail_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	MpFolderDetailView_t *view = (MpFolderDetailView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

#ifdef MP_FEATURE_SHARE
	if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_SHARE)) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_SHARE, MP_PLAYER_MORE_BTN_SHARE, mp_common_share_cb, view);
	}
#endif
	if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL)) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _mp_folder_detail_view_add_to_playlist_cb, view);
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_DELETE, MP_PLAYER_MORE_BTN_DELETE_IMAGE, _mp_folder_detail_view_tracklist_edit_cb, view);
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

#ifdef MP_FEATURE_PERSONAL_PAGE
	if (mp_util_is_personal_page_on()) {
		all_in_personal_e status = mp_common_personal_status(view->content_to_show);
		/*add*/
		if (status != MP_COMMON_ALL_IN && status != MP_COMMON_ALL_ERROR)
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_ADD_TO_PERSONAL_PAGE, MP_PLAYER_MORE_BTN_ADD_TO_PERSONAL_PAGE, mp_common_add_to_personal_page_cb, view);

		/*remove*/
		if (status != MP_COMMON_ALL_OUT && status != MP_COMMON_ALL_ERROR)
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			                             STR_MP_REMOVE_FROM_PERSONAL_PAGE, MP_PLAYER_MORE_BTN_REMOVE_FROM_PERSONAL_PAGE, mp_common_remove_from_personal_page_cb, view);
	}
#endif

	/*search*/
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_SEARCH, NULL, mp_common_create_search_view_cb, view);

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, mp_common_ctxpopup_setting_cb, view);
#ifndef MP_FEATURE_NO_END
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);
}

static void _mp_folder_detail_view_edit_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	MpFolderDetailView_t *view = (MpFolderDetailView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	/* Todo: supports multi-language */
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_PLAYLIST, mp_common_ctxpopup_add_to_playlist_cb, view);

	if (mp_view_get_nowplaying_show_flag((MpView_t *)view)) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             STR_MP_SHARE, MP_PLAYER_MORE_BTN_SHARE, mp_common_button_share_list_cb, view);

		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		                             "IDS_COM_SK_DELETE", NULL, mp_common_button_delete_list_cb, view->content_to_show);
	}

	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);
}

/***************	functions for track list update 	*******************/
static void _mp_folder_detail_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpFolderDetailView_t *view = (MpFolderDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_add_to_playlsit_view((MpListView_t *)view);

}

static void _mp_folder_detail_view_tracklist_edit_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpFolderDetailView_t *view = (MpFolderDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_show_edit_view((MpListView_t *)view, MP_DONE_DELETE_TYPE);
}

static Eina_Bool _mp_folder_detail_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpFolderDetailView_t *view = (MpFolderDetailView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	MpTrackList_t *track_list = (MpTrackList_t *)view->content_to_show;
	MP_CHECK_VAL(track_list, EINA_TRUE);
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

	}
	return EINA_TRUE;
}

static int _mp_folder_detail_view_update_options(void *thiz)
{
	startfunc;
	MpFolderDetailView_t *view = (MpFolderDetailView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);

	Evas_Object *btn = NULL;

	btn = mp_widget_create_toolbar_btn(view->folder_detail_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_folder_detail_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);
	/*view->toolbar_options[MP_OPTION_MORE] = btn;*/

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_folder_detail_view_back_cb, view);

	/* update the first controlba item */
	/*mp_view_manager_update_first_controlbar_item(layout_data);*/
	endfunc;
	return 0;
}

static int _mp_folder_detail_view_update_options_edit(void *thiz)
{
	startfunc;
	MpFolderDetailView_t *view = (MpFolderDetailView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	Evas_Object *toolbar = mp_widget_create_naviframe_toolbar(view->navi_it);
	Elm_Object_Item *toolbar_item = NULL;

	mp_view_clear_options((MpView_t *)view);

	Evas_Object *btn = mp_widget_create_toolbar_btn(view->folder_detail_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_folder_detail_view_edit_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);
	/*view->toolbar_options[MP_OPTION_MORE] = btn;*/

	toolbar_item = mp_widget_create_toolbar_item_btn(toolbar,
	               MP_TOOLBAR_BTN_LEFT, STR_MP_SHARE, mp_common_button_share_list_cb, view->navi_it);
	view->toolbar_options[MP_OPTION_LEFT] = toolbar_item;

	toolbar_item = mp_widget_create_toolbar_item_btn(toolbar,
	               MP_TOOLBAR_BTN_RIGHT, "IDS_COM_SK_DELETE", mp_common_button_delete_list_cb, view->navi_it);
	view->toolbar_options[MP_OPTION_RIGHT] = toolbar_item;

	if (mp_list_get_checked_count(view->content_to_show) == 0) {
		elm_object_item_disabled_set(view->toolbar_options[MP_OPTION_LEFT], EINA_TRUE);
		elm_object_item_disabled_set(view->toolbar_options[MP_OPTION_RIGHT], EINA_TRUE);
		/* elm_object_disabled_set(view->toolbar_options[MP_OPTION_MORE], EINA_TRUE); */
	}

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_folder_detail_view_back_cb, view);

	/* update the first controlba item */
	/*mp_view_manager_update_first_controlbar_item(layout_data);*/
	endfunc;
	return 0;
}
static void _mp_folder_detail_view_content_load(void *thiz)
{
	startfunc;
	MpFolderDetailView_t *view = (MpFolderDetailView_t *)thiz;
	MP_CHECK(view);
	/* title item genlist */
	/* elm_object_part_content_set(view->layout, "list_content", view->folder_detail_view_layout); */

	view->content_to_show = (MpList_t *)mp_track_list_create(view->layout);
	if (view->content_to_show == NULL) {
		DEBUG_TRACE("Uable to create content_to_show layout");
		return;
	}
	mp_track_list_set_data((MpTrackList_t *)view->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_BY_FOLDER, MP_TRACK_LIST_TYPE_STR, view->name, -1);
	view->content_to_show->update(view->content_to_show);
	/*show fast scroll*/
	mp_list_show_fast_scroll(view->content_to_show);
	elm_object_part_content_set(view->folder_detail_view_layout, "list_content", view->content_to_show->layout);
}

/*static void _mp_folder_detail_view_show_path(void *thiz)
{
	MpFolderDetailView_t *view = (MpFolderDetailView_t *)thiz;
	MP_CHECK(view);

	char *path = NULL;
	mp_media_info_get_folder_path_by_folder_id(view->name, &path);
	MP_CHECK(path);
	mp_debug("path = %s", path);

	char *current = path + strlen(path);
	int count = 0;
	while (current != path) {
		if (current[0] == '/')
			count++;

		if (count < 3)
			current--;
		else
			break;
	}

	char *display_path = NULL;
	if (current != path)
		display_path = g_strdup_printf("...%s", current);
	else
		display_path = g_strdup(path);

	elm_object_part_text_set(view->layout, "elm.text.info", display_path);
	elm_object_signal_emit(view->layout, "SHOW_INFO_TEXT_BAR", "");

	SAFE_FREE(path);
	SAFE_FREE(display_path);
}*/

static void
_mp_folder_detail_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpFolderDetailView_t *view = thiz;
	switch (event) {
	case MP_DELETE_DONE:
		mp_list_update(view->content_to_show);
		if (!mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
			mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL);
		}
		break;
	case MP_POPUP_DELETE_DONE:
		mp_track_list_popup_delete_genlist_item(view->content_to_show);
		mp_track_list_update_genlist(view->content_to_show);
		break;
#ifndef MP_SOUND_PLAYER
	case MP_UPDATE_PLAYING_LIST:
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.main.left.top", ELM_GENLIST_ITEM_FIELD_TEXT);
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.sub.left.bottom", ELM_GENLIST_ITEM_FIELD_TEXT);
		break;
#endif
	case MP_VIEW_EVENT_ALBUMART_CHANGED:
		mp_list_realized_item_part_update(view->content_to_show, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
		break;

	case MP_UPDATE_FAVORITE_LIST: {
		mp_list_update(view->content_to_show);
		break;
	}
	case MP_START_PLAYBACK:
	case MP_RESUME_PLAYBACK:
	case MP_PAUSE_PLAYBACK:
	case MP_PLAYING_TRACK_CHANGED:
	case MP_STOP_PLAYBACK:
		mp_list_realized_item_part_update(view->content_to_show, "elm.icon.left", ELM_GENLIST_ITEM_FIELD_CONTENT);
		break;
	default:
		break;
	}
}

static int
_mp_folder_detail_view_init(Evas_Object *parent, MpFolderDetailView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_FOLDER_DETAIL);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_folder_detail_view_update;
	view->update_options = _mp_folder_detail_view_update_options;
	view->update_options_edit = _mp_folder_detail_view_update_options_edit;
	view->view_destroy_cb = _mp_folder_detail_view_destory_cb;
	view->content_set = _mp_folder_detail_view_content_load;
	view->on_event = _mp_folder_detail_view_on_event;

	view->folder_detail_view_layout = view->layout;

	MP_CHECK_VAL(view->folder_detail_view_layout, -1);
	return ret;
}

MpFolderDetailView_t *mp_folder_detail_view_create(Evas_Object *parent, char *name)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpFolderDetailView_t *view = calloc(1, sizeof(MpFolderDetailView_t));
	MP_CHECK_NULL(view);

	ret = _mp_folder_detail_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	view->name = g_strdup(name);
	_mp_folder_detail_view_content_load(view);

	/*_mp_folder_detail_view_show_path(view);*/

	return view;

Error:
	ERROR_TRACE("Error: mp_folder_detail_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_folder_detail_view_destory(MpFolderDetailView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	return 0;
}


