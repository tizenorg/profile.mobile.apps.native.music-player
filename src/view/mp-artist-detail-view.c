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

#include "mp-artist-detail-view.h"
#include "mp-artist-detail-list.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-util.h"

#define MP_ARTIST_INDEX_ICON_SIZE (202 * elm_config_scale_get())

static void _mp_artist_detail_view_tracklist_edit_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_artist_detail_view_content_load(void *thiz);

static void _mp_artist_detail_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *ev_obj = elm_naviframe_item_pop(GET_NAVIFRAME);
}

static void
_mp_artist_detail_view_destory_cb(void *thiz)
{
	eventfunc;
	MpArtistDetailView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/
	mp_artist_detail_view_destory(view);

	free(view);
}

int _mp_artist_detail_view_update(void *thiz)
{
	startfunc;
	MpArtistDetailView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	int edit_flag = view->content_to_show->edit_mode;
	_mp_artist_detail_view_content_load(view);
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

static void _mp_artist_detail_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpArtistDetailView_t *view = (MpArtistDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_add_to_playlsit_view((MpListView_t *)view);

}

static void _mp_artist_detail_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpArtistDetailView_t *view = (MpArtistDetailView_t *)data;
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
				STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _mp_artist_detail_view_add_to_playlist_cb, view);
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_DELETE, MP_PLAYER_MORE_BTN_DELETE_IMAGE, _mp_artist_detail_view_tracklist_edit_cb, view);
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

	/*search*/
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_SEARCH, NULL, mp_common_create_search_view_cb, view);

	/*mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, mp_common_ctxpopup_setting_cb, view);*/
#ifndef MP_FEATURE_NO_END
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);
}

/***************	functions for track list update 	*******************/
static void _mp_artist_detail_view_tracklist_edit_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpArtistDetailView_t *view = (MpArtistDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	mp_common_show_edit_view((MpListView_t *)view, MP_DONE_DELETE_TYPE);
}

/*static Eina_Bool _mp_artist_detail_view_tracklist_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpArtistDetailView_t *view = (MpArtistDetailView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	MpArtistDetailList_t *artist_detail_list = (MpArtistDetailList_t *)view->content_to_show;
	MP_CHECK_VAL(artist_detail_list, EINA_TRUE);
	if (artist_detail_list->edit_mode == 1)
	{
		mp_list_set_edit((MpList_t *)artist_detail_list, FALSE);
		mp_list_view_set_select_all((MpListView_t *)view, FALSE);
		mp_view_update_options((MpView_t *)view);
		mp_evas_object_del(view->selection_info);
		return EINA_FALSE;
	}
	else
	{
		DEBUG_TRACE("");
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		mp_view_mgr_pop_view(view_mgr, false);
	}
	return EINA_TRUE;
}*/

static int _mp_artist_detail_view_update_options(void *thiz)
{
	startfunc;
	MpArtistDetailView_t *view = (MpArtistDetailView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);

	Evas_Object *btn = NULL;
	btn = mp_widget_create_toolbar_btn(view->artist_detail_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_artist_detail_view_normal_more_btn_cb, view);

	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);

	Evas_Object *back_button = elm_button_add(view->artist_detail_view_layout);
	elm_object_style_set(back_button, "naviframe/end_btn/default");
	elm_object_item_part_content_set(view->navi_it, "prev_btn", back_button);
	evas_object_smart_callback_add(back_button, "clicked", _mp_artist_detail_view_cb, view);

	/* update the first controlba item */
	/* mp_view_manager_update_first_controlbar_item(layout_data);*/
	endfunc;
	return 0;
}

static void _mp_artist_detail_view_content_load(void *thiz)
{
	startfunc;
	MpArtistDetailView_t *view = (MpArtistDetailView_t *)thiz;
	MP_CHECK(view);
	MP_CHECK(view->artist_detail_view_layout);
	/*elm_object_part_content_set(view->layout, "list_content", view->artist_detail_view_layout);*/

	view->content_to_show = (MpList_t *)mp_artist_detail_list_create(view->layout);
	MP_CHECK(view->content_to_show);
	mp_artist_detail_list_set_data((MpArtistDetailList_t *)view->content_to_show, MP_ARTIST_DETAIL_LIST_TYPE, MP_TRACK_BY_ARTIST_ALBUM, MP_ARTIST_DETAIL_LIST_TYPE_STR, view->name, -1);
	mp_list_update(view->content_to_show);
	elm_object_part_content_set(view->artist_detail_view_layout, "list_content", view->content_to_show->layout);

	/* artist index */
	/*remove index part in artist detail view according to UI 3.6*/
/*	mp_evas_object_del(view->artist_index);

	if (mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
		view->artist_index = _mp_artist_detail_view_append_artist_index(view);
		elm_object_part_content_set(view->artist_detail_view_layout, "tabbar", view->artist_index);
	}*/
}

static void
_mp_artist_detaill_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpArtistDetailView_t *view = thiz;
	switch (event) {
	case MP_DELETE_DONE:
		mp_list_update(view->content_to_show);
		if (!mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
/*			mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL);*/
			elm_object_item_del(view->navi_it);
		}
		/*mp_evas_object_del(view->artist_index);
		if (mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
			view->artist_index = _mp_artist_detail_view_append_artist_index(view);
			elm_object_part_content_set(view->artist_detail_view_layout, "tabbar", view->artist_index);
		}*/
		break;
	case MP_POPUP_DELETE_DONE:
		mp_artist_detail_list_update_genlist(view->content_to_show);
		break;
	case MP_VIEW_EVENT_ALBUMART_CHANGED:
		mp_list_realized_item_part_update(view->content_to_show, "elm.icon", ELM_GENLIST_ITEM_FIELD_CONTENT);
		break;

#ifndef MP_SOUND_PLAYER
	case MP_UPDATE_PLAYING_LIST:
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.main.left", ELM_GENLIST_ITEM_FIELD_TEXT);
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.sub.right", ELM_GENLIST_ITEM_FIELD_TEXT);
		break;
#endif
	case MP_UPDATE_FAVORITE_LIST:
	{
		mp_list_update(view->content_to_show);
		break;
	}

	default:
		break;
	}
}

static int
_mp_artist_detail_view_init(Evas_Object *parent, MpArtistDetailView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_ARTIST_DETAIL);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_artist_detail_view_update;
	view->update_options = _mp_artist_detail_view_update_options;
	/*view->update_options_edit = _mp_artist_detail_view_update_options_edit;*/
	view->view_destroy_cb = _mp_artist_detail_view_destory_cb;
	view->on_event = _mp_artist_detaill_view_on_event;

	view->artist_detail_view_layout = view->layout; /*mp_common_load_edj(parent, MP_EDJ_NAME, "common_view_layout");*/
	MP_CHECK_VAL(view->artist_detail_view_layout, -1);
	return ret;
}

MpArtistDetailView_t *mp_artist_detail_view_create(Evas_Object *parent, char *name, char *thumbnail)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpArtistDetailView_t *view = calloc(1, sizeof(MpArtistDetailView_t));
	MP_CHECK_NULL(view);

	ret = _mp_artist_detail_view_init(parent, view);
	if (ret) goto Error;

	IF_G_FREE(view->name);
	IF_G_FREE(view->thumbnail);
	view->name = g_strdup(name);
	view->thumbnail = g_strdup(thumbnail);
	_mp_artist_detail_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_artist_detail_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_artist_detail_view_destory(MpArtistDetailView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	IF_G_FREE(view->name);
	IF_G_FREE(view->thumbnail);
	return 0;
}




