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

#include "mp-album-detail-view.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-util.h"

#define MP_MAX_TEXT_PRE_FORMAT_LEN 256
#define MP_MAX_ARTIST_NAME_WIDTH 320
#define MP_LABEL_SLIDE_DURATION 5
#define MP_ALBUM_INDEX_ICON_SIZE (202 * elm_config_scale_get())

#define MP_ALBUM_TITLE_TEXT_STYLE \
		"DEFAULT='font=tizen;style=Bold font_size=45 wrap=mixed '\
		newline='br' \
		b='+ font=tizen style=Bold'"
#define MP_ALBUM_TITLE_TEXT_WIDTH       446
#define MP_ALBUM_TITLE_TEXT_WIDTH_LD    1006
#define MP_ALBUM_TITLE_TEXT_HEIGHT      54

static void _mp_album_detail_view_tracklist_edit_cb(void *data, Evas_Object * obj, void *event_info);

static void _mp_album_detail_view_cb(void *data, Evas_Object * obj, void *event_info)
{
	elm_naviframe_item_pop(GET_NAVIFRAME);
}

static void
_mp_album_detail_view_destory_cb(void *thiz)
{
	startfunc;
	MpAlbumDetailView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/
	mp_album_detail_view_destory(view);

	free(view);
}

int _mp_album_detail_view_update(void *thiz)
{
	startfunc;
	MpAlbumDetailView_t *view = thiz;
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

#ifdef MP_FEATURE_ADD_TO_HOME
static void
_mp_album_detail_view_add_to_home_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	mp_menu_add_to_home(MP_ADD_TO_HOME_SHORTCUT_TYPE_ALBUM, view->name, view->thumbnail, NULL);
}
#endif

static void _mp_album_detail_view_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_add_to_playlsit_view((MpListView_t *)view);

}

static void _mp_album_detail_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *)data;
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
				STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _mp_album_detail_view_add_to_playlist_cb, view);
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
								STR_MP_DELETE,
								MP_PLAYER_MORE_BTN_DELETE_IMAGE,
								_mp_album_detail_view_tracklist_edit_cb,
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
static void _mp_album_detail_view_tracklist_edit_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_show_edit_view((MpListView_t *)view, MP_DONE_DELETE_TYPE);
}

Eina_Bool _mp_album_detail_view_tracklist_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	MpAlbumDetailList_t *track_list = (MpAlbumDetailList_t *)view->content_to_show;
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

static int _mp_album_detail_view_update_options(void *thiz)
{
	startfunc;
	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);

	Evas_Object *btn = NULL;

	btn = mp_widget_create_toolbar_btn(view->album_detail_view_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_album_detail_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);

	Evas_Object *back_button = elm_button_add(view->album_detail_view_layout);
	elm_object_style_set(back_button, "naviframe/end_btn/default");
	elm_object_item_part_content_set(view->navi_it, "prev_btn", back_button);
	evas_object_smart_callback_add(back_button, "clicked", _mp_album_detail_view_cb, view);
	/* view->toolbar_options[MP_OPTION_MORE] = btn;*/

	/* update the first controlba item */
	/*mp_view_manager_update_first_controlbar_item(layout_data);*/
	endfunc;
	return 0;
}

/*
static char *
_mp_album_detail_view_album_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	MP_CHECK_NULL(data);
	char *name = NULL;
	int ret = 0;
	mp_layout_data_t *layout_data = (mp_layout_data_t *) data;
	mp_media_info_h svc_item = mp_media_info_list_nth_item(layout_data->svc_handle, 0);
	MP_CHECK_NULL(svc_item);

	if (!g_strcmp0(part, "elm.text.1")) {
		ret = mp_media_info_get_album(svc_item, &name);
		if (!name || !strlen(name))
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		return strdup(name);

	} else if (!g_strcmp0(part, "elm.text.3")) {
		return g_strdup_printf("%d %s", layout_data->item_count, GET_STR("IDS_MUSIC_HEADER_SONGS"));
	}

	DEBUG_TRACE("Unusing part: %s", part);
	return NULL;
}
*/

/*static char *_mp_album_detail_view_get_year(void *thiz)
{
	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *)thiz;
	MP_CHECK_NULL(view);

	int ret = 0;

	mp_media_list_h svc_handle = NULL;
	ret = mp_media_info_list_create(&svc_handle, MP_TRACK_BY_ALBUM, view->name, NULL, NULL, -1, 0, 1);
	MP_CHECK_NULL(ret == 0);

	mp_media_info_h item = NULL;
	char *year = NULL;
	char *get_year = NULL;
	item = mp_media_info_list_nth_item(svc_handle, 0);
	if (item)
	{
		ret = mp_media_info_get_year(item, &year);
	}
	DEBUG_TRACE("year=%s", year);
	get_year = year ? g_strdup(year) : g_strdup("1990");

	mp_media_info_list_destroy(svc_handle);

	return get_year;
}*/

static void _mp_album_detail_view_content_load(void *thiz)
{
	startfunc;
	MpAlbumDetailView_t *view = (MpAlbumDetailView_t *)thiz;
	MP_CHECK(view);
	MP_CHECK(view->layout);
	MP_CHECK(view->album_detail_view_layout);

	view->content_to_show = (MpList_t *)mp_album_detail_list_create(view->layout);
	MP_CHECK(view->content_to_show);
	mp_album_detail_list_set_data((MpAlbumDetailList_t *)view->content_to_show, MP_ALBUM_DETAIL_LIST_TYPE, MP_TRACK_BY_ALBUM, MP_ALBUM_DETAIL_TYPE_STR, view->name, MP_ALBUM_DETAIL_ARTIST, view->artist, MP_ALBUM_DETAIL_THUMBNAIL, view->thumbnail, -1);
	mp_list_update(view->content_to_show);
	elm_object_part_content_set(view->album_detail_view_layout, "list_content", view->content_to_show->layout);

}

static void
_mp_album_detaill_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpAlbumDetailView_t *view = thiz;
	switch (event) {
	case MP_DELETE_DONE:
		mp_list_update(view->content_to_show);
		if (!mp_list_get_editable_count(view->content_to_show, mp_list_get_edit_type(view->content_to_show))) {
/*			mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL); */
			elm_object_item_del(view->navi_it);
		}
		break;
	case MP_POPUP_DELETE_DONE:
		mp_album_detail_list_popup_delete_genlist_item(view->content_to_show);
		mp_album_detail_list_update_genlist(view->content_to_show);
		break;
#ifndef MP_SOUND_PLAYER
	case MP_UPDATE_PLAYING_LIST:
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.main.left", ELM_GENLIST_ITEM_FIELD_TEXT);
		mp_list_realized_item_part_update(view->content_to_show, "elm.text.sub.right", ELM_GENLIST_ITEM_FIELD_TEXT);
		break;
#endif
	case MP_VIEW_EVENT_ALBUMART_CHANGED:
	{
		mp_media_list_h media_list = NULL;
		mp_media_info_h media = NULL;
		mp_media_info_list_create(&media_list, MP_TRACK_BY_ALBUM, view->name, NULL, NULL, 0, 0, 1);
		media = mp_media_info_list_nth_item(media_list, 0);

		char *path = NULL;
		mp_media_info_get_thumbnail_path(media, &path);

		IF_FREE(view->thumbnail);
		view->thumbnail = g_strdup(path);

		MpAlbumDetailList_t *list = (MpAlbumDetailList_t *)view->content_to_show;
		MP_CHECK(list);
		mp_album_detail_list_set_data(list, MP_ALBUM_DETAIL_THUMBNAIL, view->thumbnail, -1);
		if (list->shuffle_it)
			elm_genlist_item_fields_update(list->shuffle_it, "elm.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);

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
_mp_album_detail_view_init(Evas_Object *parent, MpAlbumDetailView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_ALBUM_DETAIL);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_album_detail_view_update;
	view->update_options = _mp_album_detail_view_update_options;
	/*view->update_options_edit = _mp_album_detail_view_update_options_edit;*/
	view->view_destroy_cb = _mp_album_detail_view_destory_cb;
	view->content_set = _mp_album_detail_view_content_load;
	view->on_event = _mp_album_detaill_view_on_event;
	view->album_detail_view_layout = view->layout;

	MP_CHECK_VAL(view->album_detail_view_layout, -1);
	return ret;
}

MpAlbumDetailView_t *mp_album_detail_view_create(Evas_Object *parent, char *album, char *artist, char *thumbnail)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpAlbumDetailView_t *view = calloc(1, sizeof(MpAlbumDetailView_t));
	MP_CHECK_NULL(view);

	ret = _mp_album_detail_view_init(parent, view);
	if (ret) goto Error;

	IF_G_FREE(view->name);
	IF_G_FREE(view->artist);
	IF_G_FREE(view->thumbnail);
	view->name = g_strdup(album);
	view->artist = g_strdup(artist);
	view->thumbnail = g_strdup(thumbnail);
	_mp_album_detail_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_album_detail_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_album_detail_view_destory(MpAlbumDetailView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	IF_G_FREE(view->name);
	IF_G_FREE(view->artist);
	IF_G_FREE(view->thumbnail);
	return 0;
}



