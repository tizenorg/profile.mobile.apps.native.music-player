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

#include "mp-genre-detail-view.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-util.h"

#define MP_MAX_TEXT_PRE_FORMAT_LEN 256
#define MP_MAX_ARTIST_NAME_WIDTH 320
#define MP_LABEL_SLIDE_DURATION 5
#define MP_GENRE_INDEX_ICON_SIZE (202 * elm_config_scale_get())

static void _mp_genre_detail_view_tracklist_edit_cb(void *data, Evas_Object *obj, void *event_info);

static void
_mp_genre_detail_view_destory_cb(void *thiz)
{
	startfunc;
	MpGenreDetailView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource.. */
	mp_genre_detail_view_destory(view);

	free(view);
}

int _mp_genre_detail_view_update(void *thiz)
{
	startfunc;
	MpGenreDetailView_t *view = thiz;
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

static void _mp_genre_detail_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpGenreDetailView_t *view = (MpGenreDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_add_to_playlsit_view((MpListView_t *)view);

}

static void _mp_genre_detail_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpGenreDetailView_t *view = (MpGenreDetailView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	if (mp_list_get_editable_count(view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL)) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _mp_genre_detail_view_add_to_playlist_cb, view);
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
								STR_MP_DELETE,
								MP_PLAYER_MORE_BTN_DELETE_IMAGE,
								_mp_genre_detail_view_tracklist_edit_cb,
								view);
	}

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

/***************	functions for track list update 	*******************/
static void _mp_genre_detail_view_tracklist_edit_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpGenreDetailView_t *view = (MpGenreDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_show_edit_view((MpListView_t *)view, MP_DONE_DELETE_TYPE);

}

Eina_Bool _mp_genre_detail_view_tracklist_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpGenreDetailView_t *view = (MpGenreDetailView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	MpList_t *track_list = view->content_to_show;
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

static int _mp_genre_detail_view_update_options(void *thiz)
{
	startfunc;
	MpGenreDetailView_t *view = (MpGenreDetailView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);

	Evas_Object *btn = NULL;

	btn = mp_widget_create_toolbar_btn(view->genre_detail_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_genre_detail_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_genre_detail_view_tracklist_back_cb, view);
	/*view->toolbar_options[MP_OPTION_MORE] = btn;*/

	/* update the first controlba item */
	/*mp_view_manager_update_first_controlbar_item(layout_data);*/
	endfunc;
	return 0;
}

static void _mp_genre_detail_view_content_load(void *thiz)
{
	startfunc;
	MP_CHECK(thiz);
	MpGenreDetailView_t *view = (MpGenreDetailView_t *)thiz;
	MP_CHECK(view);
	MP_CHECK(view->layout);
	MP_CHECK(view->genre_detail_view_layout);
	elm_object_part_content_set(view->layout, "list_content", view->genre_detail_view_layout);
	/* genre index */

	view->content_to_show = (MpList_t *)mp_track_list_create(view->layout);
	MP_CHECK(view->content_to_show);
	mp_track_list_set_data((MpTrackList_t *)view->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_BY_GENRE, MP_TRACK_LIST_TYPE_STR, view->name, -1);
	mp_list_update(view->content_to_show);
	elm_object_part_content_set(view->genre_detail_view_layout, "list-content", view->content_to_show->layout);
}

static void
_mp_genre_detaill_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpGenreDetailView_t *view = thiz;
	switch (event) {
	case MP_DELETE_DONE:
		mp_list_update(view->content_to_show);
		if (!mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
			mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL);
		}
	break;
#ifndef MP_SOUND_PLAYER
	case MP_UPDATE_PLAYING_LIST:
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.main.left.top", ELM_GENLIST_ITEM_FIELD_TEXT);
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.sub.left.bottom", ELM_GENLIST_ITEM_FIELD_TEXT);
		break;
#endif
	case MP_VIEW_EVENT_ALBUMART_CHANGED:
	{
		mp_media_list_h media_list = NULL;
		mp_media_info_h media = NULL;
		mp_media_info_list_create(&media_list, MP_TRACK_BY_GENRE, view->name, NULL, NULL, 0, 0, 1);
		media = mp_media_info_list_nth_item(media_list, 0);

		char *path = NULL;
		mp_media_info_get_thumbnail_path(media, &path);

		IF_FREE(view->thumbnail);
		view->thumbnail = g_strdup(path);
		Evas_Object *icon = mp_util_create_thumb_icon(view->genre_index, view->thumbnail, MP_GENRE_INDEX_ICON_SIZE, MP_GENRE_INDEX_ICON_SIZE);
		elm_object_part_content_set(view->genre_index, "elm.icon", icon);

		mp_media_info_list_destroy(media_list);

	}
		break;

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
_mp_genre_detail_view_init(Evas_Object *parent, MpGenreDetailView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_GENRE_DETAIL);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_genre_detail_view_update;
	view->update_options = _mp_genre_detail_view_update_options;
	/*view->update_options_edit = _mp_genre_detail_view_update_options_edit;*/
	view->view_destroy_cb = _mp_genre_detail_view_destory_cb;
	view->content_set = _mp_genre_detail_view_content_load;
	view->on_event = _mp_genre_detaill_view_on_event;

	view->genre_detail_view_layout = mp_common_load_edj(parent, MP_EDJ_NAME, "genre_view_layout");

	MP_CHECK_VAL(view->genre_detail_view_layout, -1);
	return ret;
}

MpGenreDetailView_t *mp_genre_detail_view_create(Evas_Object *parent, char *genre, char *artist, char *thumbnail)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpGenreDetailView_t *view = calloc(1, sizeof(MpGenreDetailView_t));
	MP_CHECK_NULL(view);

	ret = _mp_genre_detail_view_init(parent, view);
	if (ret) goto Error;

	IF_G_FREE(view->name);
	IF_G_FREE(view->artist);
	IF_G_FREE(view->thumbnail);
	view->name = g_strdup(genre);
	view->artist = g_strdup(artist);
	view->thumbnail = g_strdup(thumbnail);
	_mp_genre_detail_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_genre_detail_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_genre_detail_view_destory(MpGenreDetailView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	IF_G_FREE(view->name);
	IF_G_FREE(view->artist);
	IF_G_FREE(view->thumbnail);
	return 0;
}


