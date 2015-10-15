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

#include "mp-folder-view.h"
#include "mp-folder-list.h"
#include "mp-widget.h"
#include "mp-create-playlist-view.h"
#include "mp-common.h"
#include "mp-util.h"

static void _mp_folder_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_folder_view_edit_cb(void *data, Evas_Object *obj, void *event_info);

static void
_mp_folder_view_destory_cb(void *thiz)
{
	eventfunc;
	MpFolderView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource.. */

	free(view);
}

int _mp_folder_view_update(void *thiz)
{
	startfunc;
	MpFolderView_t *view = thiz;

	MP_CHECK_VAL(view, -1);
	int edit_flag = view->content_to_show->edit_mode;
	view->content_set(view);
	if (edit_flag) {
		mp_list_set_edit(view->content_to_show, true);
		mp_view_update_options_edit((MpView_t *)view);
		mp_list_view_set_select_all((MpListView_t *)view, true);
	} else {
		mp_view_update_options((MpView_t *)view);
		mp_list_view_set_select_all((MpListView_t *)view, false);
	}
	return 0;
}

static void _mp_folder_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	MpFolderView_t *view = (MpFolderView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);


	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL) > 0) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _mp_folder_view_add_to_playlist_cb, view);
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
								STR_MP_DELETE,
								MP_PLAYER_MORE_BTN_DELETE_IMAGE,
								_mp_folder_view_edit_cb,
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

#ifdef MP_FEATURE_PERSONAL_PAGE
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
#endif

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_GO_TO_LIBRARY, MP_PLAYER_MORE_BTN_GO_TO_LIB, mp_common_go_to_library_cb, view);

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

/***************	functions for album list update 	*******************/
static void _mp_folder_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpFolderView_t *view = (MpFolderView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_add_to_playlsit_view((MpListView_t *)view);

}

static void _mp_folder_view_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpFolderView_t *view = (MpFolderView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_show_edit_view((MpListView_t *)view, MP_DONE_DELETE_TYPE);

}


static Eina_Bool _mp_folder_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpFolderView_t *view = (MpFolderView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	MpFolderList_t *folder_list = (MpFolderList_t *)view->content_to_show;
	MP_CHECK_VAL(folder_list, EINA_TRUE);
	if (folder_list->edit_mode == 1) {
		mp_list_set_edit((MpList_t *)folder_list, FALSE);
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
		mp_view_update_options((MpView_t *)view);
		mp_evas_object_del(view->selection_info);
		return EINA_FALSE;
	} else {
		DEBUG_TRACE("");
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		mp_view_mgr_pop_view(view_mgr, false);

		/* MpView_t *prev_view = mp_view_mgr_get_top_view(view_mgr);
		mp_view_update(prev_view); */
	}
	return EINA_TRUE;
}

static int _mp_folder_view_update_options(void *thiz)
{
	startfunc;
	MpFolderView_t *view = (MpFolderView_t *)thiz;
	MP_CHECK_VAL(view, -1);
	mp_view_clear_options((MpView_t *)view);

	Evas_Object *btn = NULL;
	btn = mp_widget_create_toolbar_btn(view->folder_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_folder_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_folder_view_back_cb, view);

	/* update the first controlba item */
	/* mp_view_manager_update_first_controlbar_item(layout_data); */
	endfunc;
	return 0;
}

static int _mp_folder_view_update_options_edit(void *thiz)
{
	startfunc;
	MpFolderView_t *view = (MpFolderView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);

	Evas_Object *toolbar = mp_widget_create_naviframe_toolbar(view->navi_it);

	mp_widget_create_toolbar_item_btn(toolbar,
		MP_TOOLBAR_BTN_LEFT, STR_MP_ADD_TO, mp_common_button_add_to_playlist_cb, view->navi_it);

	mp_widget_create_toolbar_item_btn(toolbar,
		MP_TOOLBAR_BTN_RIGHT, STR_MP_DELETE, mp_common_button_delete_list_cb, view->navi_it);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_folder_view_back_cb, view);

	/* update the first controlba item */
	/* mp_view_manager_update_first_controlbar_item(layout_data); */
	endfunc;
	return 0;
}

static void
_mp_folder_view_content_load(void *thiz)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(thiz);
	MpFolderView_t *view = (MpFolderView_t *)thiz;
	MP_CHECK(view);

	view->content_to_show = (MpList_t *)mp_folder_list_create(view->layout);
	MP_CHECK(view->content_to_show);
	mp_folder_list_set_data((MpFolderList_t *)view->content_to_show, MP_FOLDER_LIST_TYPE, MP_GROUP_BY_FOLDER, -1);
	mp_list_update(view->content_to_show);
	MP_CHECK(view->folder_view_layout);
	elm_object_part_content_set(view->folder_view_layout, "list_content", mp_list_get_layout(view->content_to_show));
	endfunc;
}

static void
_mp_folder_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpFolderView_t *view = thiz;
	switch (event) {
	case MP_DELETE_DONE:
		mp_list_update(view->content_to_show);
		break;
	default:
		break;
	}
}

static int
_mp_folder_view_init(Evas_Object *parent, MpFolderView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_FOLDER);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_folder_view_update;
	view->update_options = _mp_folder_view_update_options;
	view->update_options_edit = _mp_folder_view_update_options_edit;
	view->view_destroy_cb = _mp_folder_view_destory_cb;
	view->content_set = _mp_folder_view_content_load;
	view->on_event = _mp_folder_view_on_event;

	view->folder_view_layout = view->layout;
	MP_CHECK_VAL(view->folder_view_layout, -1);

	return ret;
}

MpFolderView_t *mp_folder_view_create(Evas_Object *parent)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpFolderView_t *view = calloc(1, sizeof(MpFolderView_t));
	MP_CHECK_NULL(view);

	ret = _mp_folder_view_init(parent, view);
	if (ret) goto Error;

	view->content_set(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_folder_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_folder_view_destory(MpFolderView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	return 0;
}
