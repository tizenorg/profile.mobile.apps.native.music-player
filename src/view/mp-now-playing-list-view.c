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

#include "mp-now-playing-list-view.h"
#include "mp-now-playing-list.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-common.h"
#include "mp-popup.h"

static void _mp_now_playing_view_open_playlist_btn_edit_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpNowPlayingListiew_t *view = (MpNowPlayingListiew_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	/*mp_common_show_edit_view((MpNowPlayingListiew_t *)view);*/
	mp_list_set_edit(view->content_to_show, true);
	mp_view_update_options_edit((MpView_t *)view);
}


static void
_mp_now_playing_list_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	MpNowPlayingListiew_t *view = (MpNowPlayingListiew_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);
	/*
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
					GET_SYS_STR("IDS_COM_SK_REFRESH"), NULL, NULL, view);
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
					GET_SYS_STR("IDS_COM_BODY_SETTINGS"), NULL, NULL, view);
	*/

#ifdef MP_FEATURE_SHARE
	/* share via */
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_SHARE, MP_PLAYER_MORE_BTN_SHARE, mp_common_share_cb, view);
#endif
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_EDIT, MP_PLAYER_MORE_BTN_EDIT, _mp_now_playing_view_open_playlist_btn_edit_cb, view);

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, mp_common_ctxpopup_setting_cb, view);
#ifndef MP_FEATURE_NO_END
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);
}

static void
_mp_now_playing_list_view_remove_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
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

	MpNowPlayingListiew_t *view = data;
	MP_CHECK(view);

	mp_now_playing_list_remove_selected_item((MpNowPlayingList_t *)view->content_to_show);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MODIFIED);

	mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count(view->content_to_show));
	mp_view_update_options_edit((MpView_t *)view);
	if (mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show)) == 0) {
		mp_list_update((MpList_t *)view->content_to_show);
		mp_view_mgr_pop_a_view(GET_VIEW_MGR, GET_PLAYER_VIEW);
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_UNSET_NOW_PLAYING);
	}
}

static void
_mp_now_playing_list_view_remove_popup_show(MpNowPlayingListiew_t *view)
{
	DEBUG_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = view->content_to_show;
	MP_CHECK(list);

	if (mp_list_get_checked_count(list) <= 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, view, _mp_now_playing_list_view_remove_popup_response_cb, ad);
	ad->popup_delete = popup;

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, GET_STR("IDS_MUSIC_POP_REMOVE_Q"));

	mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_REMOVE, MP_POPUP_YES);

	evas_object_show(popup);

}

static void
_mp_now_playing_list_view_remove_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	MpNowPlayingListiew_t *view = (MpNowPlayingListiew_t *)data;
	MP_CHECK(view);

	_mp_now_playing_list_view_remove_popup_show(view);
}

static Eina_Bool
_mp_now_playing_list_view_back_cb(void *data, Elm_Object_Item *it)
{
	DEBUG_TRACE("");
	MpNowPlayingListiew_t *view = (MpNowPlayingListiew_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	/*MpNowPlayingListiew_t *now_playing_list = (MpNowPlayingListiew_t *)view->content_to_show;
	MP_CHECK(now_playing_list);
	if (now_playing_list->edit_mode == 1)*/
	{
		/*		mp_list_set_edit((MpList_t *)now_playing_list, FALSE);
				view->update_options(view);*/
	}
	/*	else*/
	{
		MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
		mp_view_mgr_pop_view(view_manager, false);
	}
	return EINA_TRUE;

}

static int _mp_now_playing_list_view_update_options(void *thiz)
{
	startfunc;
	MpNowPlayingListiew_t *view = (MpNowPlayingListiew_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);

	Evas_Object *btn = NULL;

	btn = mp_widget_create_toolbar_btn(view->now_playing_list_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_now_playing_list_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);
	/*view->toolbar_options[MP_OPTION_MORE] = btn;*/

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_now_playing_list_view_back_cb, view);

	endfunc;
	return 0;
}

static int
_mp_now_playing_list_view_update_options_edit(void *thiz)
{
	startfunc;
	MpNowPlayingListiew_t *view = (MpNowPlayingListiew_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);
	mp_view_unset_nowplaying((MpView_t *)view);

	Evas_Object *toolbar = mp_widget_create_naviframe_toolbar(view->navi_it);
	Elm_Object_Item *toolbar_item = NULL;

	toolbar_item = mp_widget_create_toolbar_item_btn(toolbar,
	               MP_TOOLBAR_BTN_LEFT, STR_MP_ADD_TO, mp_common_button_add_to_playlist_cb, view->content_to_show);
	view->toolbar_options[MP_OPTION_LEFT] = toolbar_item;

	toolbar_item = mp_widget_create_toolbar_item_btn(toolbar,
	               MP_TOOLBAR_BTN_RIGHT, STR_MP_REMOVE, _mp_now_playing_list_view_remove_btn_cb, view);
	view->toolbar_options[MP_OPTION_RIGHT] = toolbar_item;

	Eina_Bool disabled = (mp_list_get_checked_count(view->content_to_show) == 0) ? EINA_TRUE : EINA_FALSE;
	elm_object_item_disabled_set(view->toolbar_options[MP_OPTION_LEFT], disabled);
	elm_object_item_disabled_set(view->toolbar_options[MP_OPTION_RIGHT], disabled);
	if (disabled) {
		mp_evas_object_del(view->selection_info);
	}

	if (view->select_all_btn) {
		elm_object_disabled_set(view->select_all_btn, !((Eina_Bool)mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))));
	}

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_now_playing_list_view_back_cb, view);

	endfunc;
	return 0;
}

static void
_mp_now_playing_list_view_content_set(void *thiz)
{
	startfunc;
	MpNowPlayingListiew_t *view = (MpNowPlayingListiew_t *)thiz;
	MP_CHECK(view);

	elm_object_part_content_set(view->layout, "list_content", view->now_playing_list_layout);

	view->content_to_show = (MpList_t *)mp_now_playing_list_create(view->now_playing_list_layout);
	MP_CHECK(view->content_to_show);
	elm_object_part_content_set(view->now_playing_list_layout, "list_content", view->content_to_show->layout);

	mp_list_update(view->content_to_show);
}

static int
_mp_now_playing_list_view_update(void *thiz)
{
	startfunc;
	MpNowPlayingListiew_t *view = thiz;

	if (view->content_to_show) {
		mp_list_update((MpList_t *)view->content_to_show);
	}

	if (mp_list_get_edit(view->content_to_show)) {
		mp_list_set_edit(view->content_to_show, true);
	}

	endfunc;
	return 0;
}


static void
_mp_now_playing_list_view_destory_cb(void *thiz)
{
	startfunc;
	MpNowPlayingListiew_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/

	free(view);
}


static int
_mp_now_playing_list_view_init(Evas_Object *parent, MpNowPlayingListiew_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_NOW_PLAYING_LIST);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_now_playing_list_view_update;
	view->update_options = _mp_now_playing_list_view_update_options;
	view->update_options_edit = _mp_now_playing_list_view_update_options_edit;
	view->view_destroy_cb = _mp_now_playing_list_view_destory_cb;

	view->now_playing_list_layout = view->layout;

	MP_CHECK_VAL(view->now_playing_list_layout, -1);
	return ret;
}


MpNowPlayingListiew_t *
mp_now_playing_list_view_create(Evas_Object *parent)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpNowPlayingListiew_t *view = calloc(1, sizeof(MpNowPlayingListiew_t));
	MP_CHECK_NULL(view);

	ret = _mp_now_playing_list_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	_mp_now_playing_list_view_content_set(view);

	return view;

Error:
	ERROR_TRACE("Error: mp_now_playing_list_view_create()");
	IF_FREE(view);
	return NULL;
}

int
mp_now_playihng_list_view_destory(MpNowPlayingListiew_t *view)
{
	MP_CHECK_VAL(view, -1);

	return 0;
}

