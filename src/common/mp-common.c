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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <efl_extension.h>

#include "mp-common.h"
#include "mp-player-debug.h"
#include "mp-player-mgr.h"
#include "mp-media-info.h"
#include "mp-util.h"
#include "mp-menu.h"
#include "mp-search.h"
#include "mp-widget.h"
#include "mp-volume.h"
#include "mp-setting-view.h"
#include "mp-add-track-view.h"
#include "mp-playlist-detail-view.h"
#include "mp-album-detail-view.h"
#include "mp-artist-detail-view.h"
#include "mp-popup.h"
#include "mp-ctxpopup.h"
#include "mp-edit-callback.h"
#include "mp-player-view.h"
#include "mp-edit-view.h"
#include "mp-play.h"
#include "mp-track-list.h"
#include "mp-search-view.h"
#include "mp-all-view.h"
#include "mp-set-as-view.h"
#include "mp-playlist-list.h"
#include "mp-make-offline-view.h"
#include "mp-avrcp.h"
#include "mp-setting-ctrl.h"
#include "mp-ug-launch.h"
#include "mp-edit-playlist.h"
#include "mp-file-util.h"

#ifdef IDEBUILD
#include "idebuild.h"
#endif

#ifdef MP_3D_FEATURE
#endif

#define MP_POPUP_DETAIL_ITEM_W          568
#define MP_POPUP_DETAIL_ITEM_H          50
#define MP_STR_LEN_MAX                  4096

#ifndef MP_SOUND_PLAYER

void
mp_common_show_setting_view(void)
{
	MpViewMgr_t *view_mgr = GET_VIEW_MGR;
	MpView_t *setting_view = (MpView_t*)mp_setting_view_create(view_mgr->navi, MP_SETTING_VIEW_DEFAULT, NULL);
	mp_view_mgr_push_view(view_mgr, setting_view, NULL);
	mp_view_set_title(setting_view, STR_MP_SETTINGS);
	mp_view_update_options(setting_view);
}

void
mp_common_view_cancel_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	elm_naviframe_item_pop(view_mgr->navi);
	//mp_view_mgr_pop_view(view_mgr, false);
	//MpView_t *top_view = mp_view_mgr_get_top_view(GET_VIEW_MGR);
	//mp_view_mgr_pop_a_view(GET_VIEW_MGR, top_view);
}

void
mp_common_add_to_playlsit_view(void *list_view)
{
	MpListView_t *view = list_view;
#ifdef MP_FEATURE_PERSONAL_PAGE
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false, MP_EDIT_VIEW_PERSONAL_PAGE_NONE);
	MP_CHECK(edit_view);
#else
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false);
#endif
	if (edit_view == NULL) {
		ERROR_TRACE("Unable to create edit view");
		return;
	}
	edit_view->create_playlist = true;
	mp_view_mgr_push_view(GET_VIEW_MGR, (MpView_t *)edit_view, NULL);
	mp_view_update((MpView_t *)edit_view);
	mp_view_update_options((MpView_t *)edit_view);
	mp_view_set_title((MpView_t *)edit_view, STR_MP_TILTE_SELECT_ITEM);
	mp_list_view_set_select_all((MpListView_t*)edit_view, true);
	mp_list_view_set_done_btn((MpListView_t*)edit_view, true, MP_DONE_ADD_TO_TYPE);
	mp_list_view_set_cancel_btn((MpListView_t*)edit_view, true);
}

void
mp_common_set_list_to_reorder_view(void *list_view)
{
	MpListView_t *view = list_view;

#ifdef MP_FEATURE_PERSONAL_PAGE
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false, MP_EDIT_VIEW_PERSONAL_PAGE_NONE);
#else
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false);
#endif
	if (edit_view == NULL) {
		ERROR_TRACE("Unable to create edit view");
		return;
	}

	edit_view->list_mode = MP_EDIT_VIEW_REORDER;
	edit_view->content_to_show->reorderable = 1;
	mp_view_mgr_push_view(GET_VIEW_MGR, (MpView_t *)edit_view, NULL);
	mp_view_update((MpView_t *)edit_view);
	mp_view_update_options((MpView_t *)edit_view);
	mp_view_set_title((MpView_t *)edit_view, STR_MP_REORDER);
	mp_list_view_set_select_all((MpListView_t*)edit_view, false);
	mp_list_view_set_done_btn((MpListView_t*)edit_view, true, MP_DONE_REORDER_TYPE);
	mp_list_view_set_cancel_btn((MpListView_t*)edit_view, true);
}

void
mp_common_show_edit_view(void *list_view, mp_done_operator_type_t type)
{
	MpListView_t *view = list_view;
#ifdef MP_FEATURE_PERSONAL_PAGE
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false, MP_EDIT_VIEW_PERSONAL_PAGE_NONE);
#else
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false);
#endif
	mp_view_mgr_push_view(GET_VIEW_MGR, (MpView_t *)edit_view, NULL);
	mp_view_update((MpView_t *)edit_view);
	mp_view_update_options((MpView_t *)edit_view);

	mp_view_set_title((MpView_t *)edit_view, STR_MP_TILTE_SELECT_ITEM);
	mp_list_view_set_select_all((MpListView_t*)edit_view, true);
	mp_list_view_set_cancel_btn((MpListView_t*)edit_view, true);
	mp_list_view_set_done_btn((MpListView_t*)edit_view, true, type);
}



void mp_common_genlist_checkbox_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	mp_list_item_data_t *it_data = (mp_list_item_data_t *)data;
	MP_CHECK(it_data);
	Elm_Object_Item *gli = (Elm_Object_Item *)it_data->it;
	mp_list_item_selected_set(gli, EINA_FALSE);

	it_data->checked = !it_data->checked;

	elm_genlist_item_fields_update(gli, "elm.edit.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);

	return;
}

//ctx popup callback (popup should be center) and ctx popup need to be destoryed
void mp_common_ctxpopup_setting_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpView_t *view = data;
	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_show_setting_view();

}

void mp_common_ctxpopup_end_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpView_t *view = data;
	mp_evas_object_del(view->more_btn_ctxpopup);
	elm_exit();
}

#ifdef MP_FEATURE_CLOUD
static char *
_mp_common_cloud_popup_label_get(void *data, Evas_Object * obj, const char *part)
{
	int mode = (int)data;

	char *label = NULL;
	if (mode == MP_TRACK_LIST_VIEW_ALL) {
		label = STR_MP_ALL_CONTENTS;
	} else if (mode == MP_TRACK_LIST_VIEW_LOCAL) {
		label = STR_MP_CONTENTS_IN_PHONE;
	} else {
		label = STR_MP_CONTENTS_IN_CLOUD;
	}

	return g_strdup(GET_STR(label));
}

static Evas_Object *
_mp_common_cloud_popup_content_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *radio = elm_radio_add(obj);
	elm_radio_state_value_set(radio, (int)data);

	MpListView_t *view = evas_object_data_get(obj, "view");
	if (view) {
		elm_radio_group_add(radio, view->radio_btn);
		MpCloudView_e mode = mp_track_list_get_cloud_view((MpTrackList_t *)view->content_to_show);
		elm_radio_value_set(view->radio_btn, mode);
	}
	evas_object_show(radio);
	return radio;
}

static void
_mp_common_view_as_popup_radio_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpListView_t *view = data;
	MP_CHECK(view);
	MP_CHECK(view->radio_btn);
	MP_CHECK(view->content_to_show);

	MpCloudView_e request = elm_radio_value_get(view->radio_btn);
	MpCloudView_e current = view->content_to_show->cloud_view_type;
	EVENT_TRACE("cloud view change request [%d => %d]", current, request);

	if (request != current) {
		view->content_to_show->cloud_view_type =  request;
		mp_list_update(view->content_to_show);
		view->cloud_view = request;
	}
}

static void
_mp_common_cloud_radio_del_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpView_t *view = (MpView_t *)data;
	MP_CHECK(view);
	view->radio_btn = NULL;
}

static void
_mp_common_cloud_view_popup_item_sel(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpView_t *view = (MpView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->radio_btn);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	MpListDisplayMode_e mode = (int)elm_object_item_data_get(item);
	elm_radio_value_set(view->radio_btn, mode);
	evas_object_smart_callback_call(view->radio_btn, "changed", NULL);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_popup_destroy(ad);
}

void mp_common_cloud_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MpView_t *view = data;
	mp_evas_object_del(view->more_btn_ctxpopup);

	Evas_Object *popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_CLOUD_VIEW, view, ad);
	MP_CHECK(popup);

	mp_evas_object_del(view->radio_btn);
	view->radio_btn = elm_radio_add(popup);
	elm_radio_state_value_set(view->radio_btn, -1);
	evas_object_smart_callback_add(view->radio_btn, "changed", _mp_common_view_as_popup_radio_changed_cb, view);
	evas_object_event_callback_add(view->radio_btn, EVAS_CALLBACK_DEL, _mp_common_cloud_radio_del_cb, view);
	evas_object_hide(view->radio_btn);
	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);

	evas_object_data_set(genlist, "view", (void *)view);

	static Elm_Genlist_Item_Class itc;
	itc.item_style = "1text.1icon.3";
	itc.func.text_get = _mp_common_cloud_popup_label_get;
	itc.func.content_get = _mp_common_cloud_popup_content_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	int i = 0;
	while (i < MP_TRACK_LIST_VIEW_MAX) {
		elm_genlist_item_append(genlist, &itc, (void *)i, NULL, ELM_GENLIST_ITEM_NONE,
		                        _mp_common_cloud_view_popup_item_sel, view);

		++i;
	}

}

void mp_common_ctxpopup_make_offline_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpView_t *view = data;
	mp_evas_object_del(view->more_btn_ctxpopup);

	view = (MpView_t*)mp_make_offline_view_create(GET_NAVIFRAME);
	mp_view_mgr_push_view(GET_VIEW_MGR, view, NULL);
	mp_view_update(view);
	mp_view_set_title(view, STR_MP_MAKE_AVAILABLE_OFFLINE);
}
#endif

void mp_common_go_to_library_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpView_t *view = data;
	mp_evas_object_del(view->more_btn_ctxpopup);

	mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL);
}

void mp_common_ctxpopup_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpListView_t *view = data;
	if (view->more_btn_ctxpopup) {
		mp_evas_object_del(view->more_btn_ctxpopup);
		return;
	}

	mp_edit_create_add_to_playlist_popup(view->content_to_show);
}

//sweep button callback (genlist type should be ctx) and popup need to be destoryed in idler
void mp_common_sweep_share_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MP_CHECK(data);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	char *file_name = NULL;
	mp_media_info_h handle = data;
	mp_media_info_get_file_path(handle, &file_name);

	mp_ctxpopup_create(obj, MP_CTXPOPUP_PV_SHARE, file_name, ad);
	return;
}


//common button callback (genlist type should be center)
void
mp_common_button_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	mp_edit_create_add_to_playlist_popup(data);
}

void
mp_common_share_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpListView_t *view = (MpListView_t *)data;
	MP_CHECK(view);
	MpList_t *list = view->content_to_show;
	MP_CHECK(list);

	mp_evas_object_del(view->more_btn_ctxpopup);


#ifdef MP_FEATURE_PERSONAL_PAGE
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, true, MP_EDIT_VIEW_PERSONAL_PAGE_NONE);
#else
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, true);
#endif

	mp_view_mgr_push_view(GET_VIEW_MGR, (MpView_t *)edit_view, NULL);
	mp_view_update((MpView_t *)edit_view);
	mp_view_update_options((MpView_t *)edit_view);
	mp_view_set_title((MpView_t *)edit_view, STR_MP_SHARE);
	mp_list_view_set_select_all((MpListView_t*)edit_view, true);
}

void
mp_common_button_share_list_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpListView_t *view = (MpListView_t *)data;
	MP_CHECK(view);
	MpList_t *list = view->content_to_show;
	MP_CHECK(list);

	GList *sel_list = NULL;
	mp_list_selected_item_data_get(list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	mp_list_item_data_t *item_data = NULL;
	char *path = NULL;

	GList *node = g_list_first(sel_list);
	while (node) {
		if (node->data) {
			item_data = node->data;
			mp_media_info_get_file_path(item_data->handle, &path);
			DEBUG_TRACE("path:%s", path);
			node->data = path;
		} else {
			WARN_TRACE("path name is NULL");
		}
		node = g_list_next(node);
	}

	mp_genlist_popup_create(view->layout, MP_POPUP_LIST_SHARE, sel_list, ad);
	//evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _popup_del_free_resource_cb, sel_list);
	return;
}

void
mp_common_button_delete_list_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	mp_edit_create_delete_popup(data);
	return;
}

#endif

static void
_ctx_popup_dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MP_CHECK(data);
	mp_evas_object_del(data);
}

static void
_ctx_popup_naviframe_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MpView_t *view = data;
	MP_CHECK(view);
	MP_CHECK(view->more_btn_ctxpopup);

	Evas_Object *win;
	Evas_Coord w, h;
	int pos = -1;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->win_main);

	win = elm_object_top_widget_get(view->more_btn_ctxpopup);
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);

	switch (pos) {

	case 0:
	case 180:
		evas_object_move(view->more_btn_ctxpopup, (w / 2), h);
		break;
	case 90:
		evas_object_move(view->more_btn_ctxpopup, (h / 2), w);
		break;
	case 270:
		evas_object_move(view->more_btn_ctxpopup, (h / 2), w);
		break;
	}
	return;
}

static void
_ctx_popup_top_widget_rotation_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	_ctx_popup_naviframe_resize_cb(data, evas_object_evas_get(obj), obj, event_info);
}

static void
_ctx_popup_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	startfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->win_main);

	ad->del_cb_invoked = 1;

	Evas_Object *navi = GET_NAVIFRAME;
	if (navi) {
		evas_object_event_callback_del(navi, EVAS_CALLBACK_RESIZE, _ctx_popup_naviframe_resize_cb);
	}

	evas_object_smart_callback_del(elm_object_top_widget_get(eo), "rotation,changed", _ctx_popup_top_widget_rotation_changed_cb);

	MpView_t *view = data;
	MP_CHECK(view);
	view->more_btn_ctxpopup = NULL;
}

Evas_Object *
mp_common_create_more_ctxpopup(void *view)
{
	Evas_Object *popup = elm_ctxpopup_add(GET_WINDOW());
	elm_object_style_set(popup, "more/default");
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _ctx_popup_del_cb, view);
	evas_object_smart_callback_add(popup, "dismissed", _ctx_popup_dismissed_cb, popup);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_ctxpopup_back_cb, NULL);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_MORE, eext_ctxpopup_back_cb, NULL);

	elm_ctxpopup_auto_hide_disabled_set(popup, EINA_TRUE);

	evas_object_event_callback_add(elm_object_top_widget_get(popup), EVAS_CALLBACK_RESIZE, _ctx_popup_naviframe_resize_cb, view);
	evas_object_smart_callback_add(elm_object_top_widget_get(popup), "rotation,changed", _ctx_popup_top_widget_rotation_changed_cb, view);
	elm_ctxpopup_direction_priority_set(popup, ELM_CTXPOPUP_DIRECTION_UP, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN, ELM_CTXPOPUP_DIRECTION_UNKNOWN);

	return popup;
}

static Eina_Bool
_mp_timer_delay_cb(void *data)
{
	TIMER_TRACE();

	MpList_t *list = data;
	MP_CHECK_FALSE(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;

	mp_list_selected_item_data_get(list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return false;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}

	if (MP_POPUP_PV_SET_AS == list->popup_type) {
		mp_genlist_popup_create(list->genlist, MP_POPUP_PV_SET_AS, item_data->handle, ad);
	} else if (MP_POPUP_ADD_TO_PLST == list->popup_type) {
		mp_edit_create_add_to_playlist_popup(data);
	}
	/*
	else if (MP_POPUP_LIST_SHARE == list->popup_type)
	{
			mp_genlist_popup_create(list->genlist, MP_POPUP_LIST_SHARE, sel_list, ad);
	}
	*/

	if (list->pop_delay_timer) {
		ecore_timer_del(list->pop_delay_timer);
	}

	return ECORE_CALLBACK_DONE;
}

static void _mp_common_set_label_for_detail(Evas_Object *pBox, char *szString)
{

	Evas_Object *pLabel = NULL;
	if (!pBox) {
		return;
	}
	pLabel = elm_label_add(pBox);
	//elm_object_style_set(pLabel, "popup/default");
	elm_object_text_set(pLabel, szString);
	//mp_util_domain_translatable_text_set(pLabel, szString);
	//elm_label_ellipsis_set(pLabel, EINA_TRUE);
	elm_label_line_wrap_set(pLabel, ELM_WRAP_MIXED);
	//elm_label_wrap_width_set(pLabel, 700 * elm_config_scale_get());
	evas_object_size_hint_weight_set(pLabel, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pLabel, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_min_set(pLabel, 0, MP_POPUP_DETAIL_ITEM_H * elm_config_scale_get());

	elm_box_pack_end(pBox, pLabel);
	evas_object_show(pLabel);

}


static void _mp_common_list_track_more_detail(void *parent, void *data)
{
	MpList_t *list = data;
	Evas_Object *popup = parent;

	struct appdata *ad = mp_util_get_appdata();

	GList *sel_list = NULL;
	Evas_Object *pop_layout = NULL;
	Evas_Object *pBox = NULL;
	Evas_Object *pop_scroller = NULL;

	char szTmpStr[MP_STR_LEN_MAX] = {0,};
	mp_list_item_data_t *item_data = NULL;

	mp_list_selected_item_data_get(list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}

	pop_layout = elm_layout_add(popup);
	MP_CHECK(pop_layout);

	elm_layout_file_set(pop_layout, MP_EDJ_NAME, "popup_detail");

	pBox = elm_box_add(popup);
	elm_box_horizontal_set(pBox, EINA_FALSE);
	evas_object_size_hint_weight_set(pBox, EVAS_HINT_FILL, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(pBox, EVAS_HINT_FILL, EVAS_HINT_FILL);

	mp_media_info_h svc_item = item_data->handle;

	char *pathname = NULL, *title = NULL, *album = NULL, *artist = NULL, *thumbname = NULL, *date = NULL;
	char *author = NULL, *copyright = NULL, *format = NULL, *track = NULL;
	int duration = 0;

	{
		mp_media_info_get_file_path(svc_item, &pathname);
		mp_media_info_get_thumbnail_path(svc_item, &thumbname);
		mp_media_info_get_title(svc_item, &title);
		mp_media_info_get_album(svc_item, &album);
		mp_media_info_get_artist(svc_item, &artist);
		mp_media_info_get_recorded_date(svc_item, &date);
		mp_media_info_get_copyright(svc_item, &copyright);
		mp_media_info_get_composer(svc_item, &author);
		mp_media_info_get_duration(svc_item, &duration);
		mp_media_info_get_track_num(svc_item, &track);
		mp_media_info_get_format(svc_item, &format);

		DEBUG_TRACE("artist is %s", artist);

		char *info_format = "<color=#156C94FF><align=left>%s: %s </align>";

		snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_SYS_STR("IDS_COM_BODY_DETAILS_TITLE"), title);
		_mp_common_set_label_for_detail(pBox, szTmpStr);
		memset(szTmpStr, 0, MP_STR_LEN_MAX);

		if (artist && strlen(artist)) {
			snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_SYS_STR("IDS_MUSIC_BODY_ARTIST"), artist);
			_mp_common_set_label_for_detail(pBox, szTmpStr);
			memset(szTmpStr, 0, MP_STR_LEN_MAX);
		}

		if (album && strlen(album)) {
			snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_STR("IDS_MUSIC_BODY_ALBUM"), album);
			_mp_common_set_label_for_detail(pBox, szTmpStr);
			memset(szTmpStr, 0, MP_STR_LEN_MAX);
		}

		char duration_format[50] = { 0, };
		int dur_sec = duration / 1000;
		int sec = dur_sec % 60;
		int min = dur_sec / 60;

		snprintf(duration_format, sizeof(duration_format), "%02u:%02u", min, sec);

		snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_STR("IDS_MUSIC_BODY_TRACK_LENGTH"), duration_format);
		_mp_common_set_label_for_detail(pBox, szTmpStr);
		memset(szTmpStr, 0, MP_STR_LEN_MAX);

		if (date && strlen(date)) {
			snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_STR("IDS_MUSIC_BODY_RECORDED_DATE"), date);
			_mp_common_set_label_for_detail(pBox, szTmpStr);
			memset(szTmpStr, 0, MP_STR_LEN_MAX);
		}
		if (track && strlen(track)) {
			snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_STR("IDS_MUSIC_BODY_TRACK_NUMBER"), track);
			_mp_common_set_label_for_detail(pBox, szTmpStr);
			memset(szTmpStr, 0, MP_STR_LEN_MAX);
		}

		if (format && strlen(format)) {
			snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_STR("IDS_MUSIC_BODY_FORMAT"), format);
			_mp_common_set_label_for_detail(pBox, szTmpStr);
			memset(szTmpStr, 0, MP_STR_LEN_MAX);
		}

		snprintf(szTmpStr, MP_STR_LEN_MAX, info_format, GET_STR("IDS_MUSIC_BODY_MUSIC_LOCATION"), pathname);
		_mp_common_set_label_for_detail(pBox, szTmpStr);
		memset(szTmpStr, 0, MP_STR_LEN_MAX);

		pop_scroller = elm_scroller_add(pop_layout);
		if (pop_scroller == NULL) {
			IF_FREE(date);
			return;
		}

		elm_scroller_bounce_set(pop_scroller, EINA_TRUE, EINA_TRUE);
		elm_scroller_policy_set(pop_scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
		evas_object_show(pop_scroller);

		elm_object_content_set(pop_scroller, pBox);

		elm_object_part_content_set(pop_layout, "elm.swallow.layout", pop_scroller);

		elm_object_content_set(popup, pop_layout);
		IF_FREE(date);
		evas_object_show(pBox);
		evas_object_show(pop_layout);

	}

	if (sel_list) {
		g_list_free(sel_list);
		sel_list = NULL;
	}
}

void mp_common_track_delete_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_popup_destroy(ad);
	mp_edit_create_track_delete_popup(data);
	return;
}

void mp_common_create_search_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpView_t *parent = data;
	if (parent) {
		mp_evas_object_del(parent->more_btn_ctxpopup);
	}

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();

	MpSearchView_t *view = (MpSearchView_t *)mp_view_mgr_get_view(view_manager, MP_VIEW_SEARCH);

	if (view) {
		mp_view_mgr_pop_to_view(view_manager, MP_VIEW_SEARCH);
		mp_search_view_set_keyword(view, NULL);
	} else {
		view = mp_search_view_create(view_manager->navi, NULL);
		mp_view_mgr_push_view(view_manager, (MpView_t *)view, MP_SEARCH_VIEW_STYLE_EMPTY);
		mp_view_update_options((MpView_t *)view);
	}
}

void
mp_common_show_set_as_view(char* path)
{
	startfunc;
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	MP_CHECK(view_mgr);
	MpSetAsView_t *view = mp_set_as_view_create(view_mgr->navi, path);
	MP_CHECK(view);
	mp_view_mgr_push_view(view_mgr, (MpView_t *)view, NULL);
	mp_view_set_title((MpView_t *) view, STR_MP_SET_AS);
	mp_view_update_options((MpView_t *)view);

	endfunc;
}

void mp_common_list_set_as_cb(void *data, Evas_Object * obj, void *event_info)
{
	EVENT_TRACE();

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_popup_destroy(ad);

	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;

	mp_list_selected_item_data_get(list,  &sel_list);

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}
	char* path = NULL;
	mp_media_info_get_file_path(item_data->handle, &path);
	MP_CHECK(path);
	mp_common_show_set_as_view(path);

}

void mp_common_list_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_popup_destroy(ad);

	list->popup_type = MP_POPUP_ADD_TO_PLST;
	list->pop_delay_timer = ecore_timer_add(0.1, _mp_timer_delay_cb, data);
}

void mp_common_list_delete_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_popup_destroy(ad);

	mp_edit_create_delete_popup(data);
	return;
}

void mp_common_list_remove_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_popup_destroy(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	if (mp_list_get_checked_count(list) <= 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}
	mp_edit_cb_excute_delete(data);
	return;
}

static void _mp_common_list_set_same_id_favorite(void *data, mp_media_info_h media, bool favorite)
{
	MpList_t *list = data;
	MP_CHECK(list);
	MP_CHECK(media);

	GList *all_list = NULL;
	mp_list_item_data_t *item_data = NULL;
	char *media_id = NULL;
	char *tmp_media_id = NULL;

	mp_media_info_get_media_id(media, &media_id);
	MP_CHECK(media_id);
	mp_list_all_item_data_get(list, &all_list);
	MP_CHECK(all_list);
	GList *node = g_list_first(all_list);
	{
		while (node) {
			item_data = node->data;
			if (item_data && (!item_data->checked) && item_data->handle) {
				mp_media_info_get_media_id(item_data->handle, &tmp_media_id);
				if (g_strcmp0(tmp_media_id, media_id) == 0) {
					mp_media_info_set_favorite(item_data->handle, favorite);
				}
			}
			node = g_list_next(node);
		}
	}
	g_list_free(all_list);
	all_list = NULL;
}

void mp_common_list_add_to_favorite_cb(void *data, Evas_Object * obj, void *event_info)
{
	EVENT_TRACE();

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_popup_destroy(ad);

	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;

	mp_list_selected_item_data_get(list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		if (item_data && item_data->handle) {
			mp_media_info_set_favorite(item_data->handle, true);
			_mp_common_list_set_same_id_favorite(list, item_data->handle, true);
		}
		node = g_list_next(node);
	}

	char *fmt_str = GET_STR(STR_MP_POPUP_ADD_TO_FAVORITE);
	char *noti_str = g_strdup_printf(fmt_str, MP_PLAYLIST_MAX_ITEM_COUNT);
	mp_util_post_status_message(ad, noti_str);

	if (sel_list) {
		g_list_free(sel_list);
		sel_list = NULL;
	}
}

void mp_common_list_unfavorite_cb(void *data, Evas_Object * obj, void *event_info)
{
	EVENT_TRACE();
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);
	mp_popup_destroy(ad);

	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;

	mp_list_selected_item_data_get(list,  &sel_list);
	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		if (item_data && item_data->handle) {
			mp_media_info_set_favorite(item_data->handle, false);
			_mp_common_list_set_same_id_favorite(list, item_data->handle, false);
		}

		node = g_list_next(node);
	}

	mp_util_post_status_message(ad, GET_STR(STR_MP_POPUP_REMOVE_FROM_FAVORITE));

	if (sel_list) {
		g_list_free(sel_list);
		sel_list = NULL;
	}
}

void mp_common_list_more_info_cb(void *data, Evas_Object * obj, void *event_info)
{
	EVENT_TRACE();

	MpList_t *list = data;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_popup_destroy(ad);

	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, STR_MP_POPUP_MORE_INFO, NULL, NULL, ad);
	MP_CHECK(popup);

	mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_OK, MP_POPUP_YES);

	evas_object_show(popup);

	_mp_common_list_track_more_detail(popup, list);
}

void mp_common_list_update_albumart_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MpList_t *list = data;
	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;
	char *uri = NULL;
	char *media_id = NULL;

	mp_popup_destroy(ad);

	MP_CHECK(list);

	mp_list_selected_item_data_get(list,  &sel_list);
	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}
	GList *node = g_list_first(sel_list);
	MP_CHECK(node);
	item_data = node->data;

	mp_media_info_h svc_item = item_data->handle;

	mp_media_info_get_file_path(svc_item, &uri);
	mp_media_info_get_media_id(svc_item, &media_id);

	//mp_albumart_update(uri, media_id);

	if (sel_list) {
		g_list_free(sel_list);
		sel_list = NULL;
	}
}

/*
**	start_playback : if it is set true, either playing a file from start or resume playing
**	start_new_file : it indicates that after start player view, it will play the file from start
*/
void
mp_common_show_player_view(int launch_type, bool disable_effect, bool start_playback, bool start_new_file)
{
	startfunc;
	WARN_TRACE("launch type[%d] disable_effect[%d], start_playback[%d], start_new_file[%d]", launch_type, disable_effect, start_playback, start_new_file);
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();

	MpPlayerView_t *player_view = (MpPlayerView_t *)mp_view_mgr_get_view(view_manager, MP_VIEW_PLAYER);

#ifndef MP_SOUND_PLAYER
	if (player_view) {
		mp_view_mgr_pop_to_view(view_manager, MP_VIEW_PLAYER);
		mp_list_update((MpList_t *)player_view->queue_list);
	} else
#endif
	{
		player_view = mp_player_view_create(view_manager->navi, launch_type, start_new_file);
		if (player_view == NULL) {
			return ;
		}

		mp_view_mgr_push_view_with_effect(view_manager, (MpView_t *)player_view, NULL, disable_effect);
	}

	if (start_playback) {
		struct appdata *ad = mp_util_get_appdata();
		player_view->start_on_transition_finish = true;
		ad->player_state = PLAY_STATE_PLAYING;
	}
	mp_view_update((MpView_t *)player_view);
	mp_view_update_options((MpView_t *)player_view);

	int show = ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PLAYING);
	mp_player_view_set_play_image(player_view, show);

	mp_player_view_set_album_playing(player_view, true);
	endfunc;
}


#ifdef MP_DEBUG_MODE
static void
_mp_common_window_flush_pre(void *data, Evas * e, void *event_info)
{
	DEBUG_TRACE("");
	TA_E_L(0, "EVAS_CALLBACK_RENDER_FLUSH_PRE");
	evas_event_callback_del(e, EVAS_CALLBACK_RENDER_FLUSH_PRE, _mp_common_window_flush_pre);
}
#endif

#ifdef MP_DEBUG_MODE
//#define TEST_PLAYER_ONLY
#endif

#ifdef TEST_PLAYER_ONLY
#include <player.h>

#define RICH_AUDIO "/tmp/RA"
#define ALH "/tmp/ALH"

void _player_prepared_cb(void *user_data);
void _player_complete_cb(void *user_data);

static player_h g_tmp_player;

void _play_uri(char *path)
{
	player_create(&g_tmp_player);
#if 0 /*rich audio is used for change play speed. as the feature is removed, we block it for avoid potential issue*/
	if (mp_check_file_exist(RICH_AUDIO)) {
		player_set_rich_audio(g_tmp_player);
	}
#endif
	player_set_sound_type(g_tmp_player, SOUND_TYPE_MEDIA);

	if (mp_check_file_exist(ALH)) {
		player_set_audio_latency_mode(g_tmp_player, AUDIO_LATENCY_MODE_HIGH);
	}
	player_set_uri(g_tmp_player, path);

	player_set_completed_cb(g_tmp_player, _player_complete_cb, NULL);
	player_prepare_async(g_tmp_player, _player_prepared_cb, NULL);
}

Eina_Bool _prepared(void *data)
{
	startfunc;
	player_start(g_tmp_player);
	mp_util_sleep_lock_set(TRUE, FALSE);
	return false;
}

Eina_Bool _complete(void *data)
{
	startfunc;
	player_stop(g_tmp_player);
	player_unprepare(g_tmp_player);
	player_destroy(g_tmp_player);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	mp_util_sleep_lock_set(FALSE, FALSE);

	mp_plst_item *item = mp_playlist_mgr_get_next(ad->playlist_mgr, true, false);
	mp_playlist_mgr_set_current(ad->playlist_mgr, item);

	_play_uri(item->uri);

	return false;
}

void _player_prepared_cb(void *user_data)
{
	startfunc;
	ecore_idler_add(_prepared, NULL);
}

void _player_complete_cb(void *user_data)
{
	startfunc;
	ecore_idler_add(_complete, NULL);
}

static void
_test_player(void)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);

	_play_uri(item->uri);

}
#endif

void
mp_common_show_player_view_after_play()
{
	startfunc;
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpPlayerView_t *player_view = (MpPlayerView_t *)mp_view_mgr_get_view(view_manager, MP_VIEW_PLAYER);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	if (player_view) {
		mp_view_mgr_pop_to_view(view_manager, MP_VIEW_PLAYER);
	} else {
		ad->preload_player_view = (Evas_Object *)mp_player_view_create(view_manager->navi, 0, false);
		ad->create_view_on_play = true;
	}

	endfunc;
}

void mp_common_play_track_list_with_playlist_id(mp_list_item_data_t *item, Evas_Object *genlist, int playlist_id)
{
	startfunc;
	MP_CHECK(item);
	MP_CHECK(genlist);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	Elm_Object_Item *gli2 = NULL;

	mp_plst_item *plst_item = NULL;
	mp_plst_item *to_play = NULL;
	bool track_update = true;	//indicate if playing the same track
	bool playlist_update = true; //indicate if playing item is in the same play list
	bool start_new_file = true;	//only when same track and same playlist, we will set it false.

	if (!ad->playlist_mgr) {
		mp_common_create_playlist_mgr();
	}

	MP_CHECK(ad->playlist_mgr);
	/* before clear playlist_mgr, check if same playlist id */
	if (ad->playlist_mgr->playlist_id == playlist_id) {
		playlist_update = false;
	}

	/* check if the same track as current playing */
	char *to_play_uid = NULL;
	char *playing_uid = NULL;
	mp_plst_item *playing_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	if (playing_item != NULL) {
		playing_uid = playing_item->uid;
	}

	if (item->handle != NULL) {
		DEBUG_TRACE("item->handle is not NULL");
		mp_media_info_get_media_id(item->handle, &to_play_uid);

		if (g_strcmp0(to_play_uid, playing_uid) == 0) {
			track_update = false;
		}

		if (track_update == false  && playlist_update == false) {
			start_new_file = false;
		}
	}

	PROFILE_IN("mp_playlist_mgr_clear");
	mp_playlist_mgr_clear(ad->playlist_mgr);
	PROFILE_OUT("mp_playlist_mgr_clear");

	PROFILE_IN("playlist_item_append");
	gli2 = elm_genlist_first_item_get(genlist);
	while (gli2) {
		if (elm_genlist_item_select_mode_get(gli2)  != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			mp_list_item_data_t *item_data = elm_object_item_data_get(gli2);
			if (item_data && item_data->item_type == MP_LIST_ITEM_TYPE_NORMAL) {
				if (item_data->group_type == MP_GROUP_NONE || item_data->group_type == MP_GROUP_BY_ALLSHARE) {
					char *uri = NULL;
					char *uid = NULL;
					char *title = NULL;
					char *artist = NULL;

					mp_track_type track_type = MP_TRACK_URI;
					mp_media_info_get_media_id(item_data->handle, &uid);
					mp_media_info_get_file_path(item_data->handle, &uri);
					mp_media_info_get_title(item_data->handle, &title);
					mp_media_info_get_artist(item_data->handle, &artist);

					mp_storage_type_e storage;
					mp_media_info_get_storage_type(item_data->handle, &storage);
#ifdef MP_FEATURE_CLOUD
					if (storage == MP_STORAGE_CLOUD) {
						track_type = MP_TRACK_CLOUD;
					}
#endif
#ifdef MP_FEATURE_STORE
					if (storage == MP_STORAGE_MUSICHUB) {
						track_type = MP_TRACK_STORE;
					}
#endif

					plst_item = mp_playlist_mgr_item_append(ad->playlist_mgr, uri, uid, title, artist, track_type);
					if (playlist_id) {
						int member_id = 0;
						mp_media_info_get_playlist_member_id(item_data->handle, &member_id);
						//mp_debug("playlist memeber id = %d", member_id);
						mp_playlist_mgr_item_set_playlist_memeber_id(plst_item, member_id);
					}

					if (gli2 == item->it && plst_item) {
						to_play = plst_item;
					}
				}
			}
		}
		gli2 = elm_genlist_item_next_get(gli2);
	}
	PROFILE_OUT("playlist_item_append");
	if (playlist_id) {
		mp_playlist_mgr_set_playlist_id(ad->playlist_mgr, playlist_id);
	}

	if (to_play == NULL) {
		DEBUG_TRACE("to_play is NULL");
	} else {
		DEBUG_TRACE("to_play:%s", to_play->uri);
	}

	PROFILE_IN("mp_playlist_mgr_set_current");
	if (to_play == NULL) {
		mp_playlist_mgr_set_current(ad->playlist_mgr, mp_playlist_mgr_get_nth(ad->playlist_mgr, 0));
	} else {
		int shuffle = false;
		mp_setting_get_shuffle_state(&shuffle);
		if (shuffle) {	//Make selected item to the first item. to play all tracks in case of repeat none
			mp_playlist_mgr_set_shuffle_first_item(ad->playlist_mgr, to_play);
		}

		mp_playlist_mgr_set_current(ad->playlist_mgr, to_play);
	}
	PROFILE_OUT("mp_playlist_mgr_set_current");
	/*
	 ** here we need to distinguish if destroy the player
	 ** the only case we don't need to destroy player is the same playlist and same item is playing
	 ** in this case, we only show the player view and update progress bar
	 */
	ad->paused_by_user = FALSE;
	if (start_new_file) {
		PROFILE_IN("mp_play_destory");
		ad->freeze_indicator_icon = true;
		mp_play_fast_destory(ad);
		PROFILE_OUT("mp_play_destory");

		mp_play_new_file(ad, TRUE);
		mp_common_show_player_view_after_play();

#ifdef TEST_PLAYER_ONLY
		_test_player();
		return;
#endif
	} else {
		mp_player_mgr_play(ad);
		mp_common_show_player_view(MP_PLAYER_NORMAL, false, false, false);
	}

#ifdef MP_DEBUG_MODE
	TA_S_L(0, "EVAS_CALLBACK_RENDER_FLUSH_PRE");
	evas_event_callback_add(evas_object_evas_get(ad->win_main), EVAS_CALLBACK_RENDER_FLUSH_PRE,  _mp_common_window_flush_pre, NULL);
#endif
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MGR_ITEM_CHANGED);

	return;

}

void mp_common_play_track_list(mp_list_item_data_t *item, Evas_Object *genlist)
{
	mp_common_play_track_list_with_playlist_id(item, genlist, 0);
}

enum {
	MP_SEARCH_BY_MUSIC,
	MP_SEARCH_BY_INTERNET,
	MP_SEARCH_BY_MAX,
};

static char *
_popup_label_get(void *data, Evas_Object * obj, const char *part)
{
	int mode = (int)data;

	char *label = NULL;
	if (mode == MP_SEARCH_BY_INTERNET) {
		label = STR_MP_INTERNET;
	} else {
		label = STR_MP_MUSIC;
	}

	return g_strdup(GET_STR(label));
}

static void _search_by_internet(const char *keyword)
{
	startfunc;
	app_control_h app_control = NULL;
	app_control_create(&app_control);
	MP_CHECK(app_control);

	app_control_set_app_id(app_control, "com.samsung.browser");
	app_control_add_extra_data(app_control, "search", keyword);
	app_control_send_launch_request(app_control, NULL, NULL);

	app_control_destroy(app_control);
}

static void _searcy_by_music_app(const char *keyword)
{
	startfunc;
	MpView_t *search_view = (MpView_t *)mp_search_view_create(GET_NAVIFRAME, keyword);
	mp_view_mgr_push_view(GET_VIEW_MGR, search_view, MP_SEARCH_VIEW_STYLE_EMPTY);
	mp_view_update_options(search_view);
}

static void
_popup_item_sel(void *data, Evas_Object *obj, void *event_info)
{
	int type = (int)elm_object_item_data_get(event_info);
	EVENT_TRACE("Search by Selected. Search type: %d", type);

	if (type == MP_SEARCH_BY_INTERNET) {
		_search_by_internet(data);
	} else {
		_searcy_by_music_app(data);
	}


	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_popup_destroy(ad);
}

void
mp_common_search_by(const char *keyword)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_SEARCH, NULL, ad);
	MP_CHECK(popup);

	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);

	static Elm_Genlist_Item_Class itc;
	itc.item_style = "1text";
	itc.func.text_get = _popup_label_get;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	int i = 0;
	while (i < MP_SEARCH_BY_MAX) {
		elm_genlist_item_append(genlist, &itc, (void *)i, NULL, ELM_GENLIST_ITEM_NONE,
		                        _popup_item_sel, keyword);

		++i;
	}
}

MpView_t *mp_common_get_all_view()
{
	MpView_t *all_typed_view = GET_ALL_VIEW;
	MpView_t *all_view = NULL;

	all_view = all_typed_view;

	return all_view;
}

void mp_common_show_add_tracks_view(int playlist_id)
{
	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpAddTrackView_t *view = mp_add_track_view_create(view_manager->navi, playlist_id);
	if (view == NULL) {
		ERROR_TRACE("Unable to show add tracks");
		return;
	}
	mp_view_mgr_push_view(view_manager, (MpView_t *)view, NULL);

	mp_view_update_options((MpView_t *)view);
	mp_list_set_edit((MpList_t *)view->content_to_show, TRUE);
	mp_view_set_title((MpView_t *)view, STR_MP_TILTE_SELECT_ITEM);
	mp_add_track_view_select_tab(view, MP_ADD_TRACK_VIEW_TAB_SONGS);
	mp_list_view_set_cancel_btn((MpListView_t*)view, true);
	mp_list_view_set_done_btn((MpListView_t*)view, true, MP_DONE_ADD_TRACK_TYPE);

}

static void
_mp_common_playlist_item_change_callback(mp_plst_item *item, void *userdata)
{
	struct appdata *ad = userdata;
	MP_CHECK(ad);

	if (ad->current_track_info) {
		mp_util_free_track_info(ad->current_track_info);
		ad->current_track_info = NULL;
	}

	if (item) {
		mp_util_load_track_info(ad, item, &ad->current_track_info);
#ifdef MP_FEATURE_AVRCP_13
		mp_avrcp_noti_track(ad->current_track_info->title,
		                    ad->current_track_info->artist, ad->current_track_info->album,
		                    ad->current_track_info->date, ad->current_track_info->duration);
#endif
	}

}

void mp_common_create_playlist_mgr(void)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	if (ad->playlist_mgr) {
		// already created
		return;
	}

	DEBUG_TRACE("create playlist mgr");
	ad->playlist_mgr = mp_playlist_mgr_create();
	mp_playlist_mgr_set_item_change_callback(ad->playlist_mgr, _mp_common_playlist_item_change_callback, ad);
	int val = 0;
	mp_setting_get_shuffle_state(&val);
	mp_playlist_mgr_set_shuffle(ad->playlist_mgr, val);
	mp_setting_get_repeat_state(&val);
	mp_playlist_mgr_set_repeat(ad->playlist_mgr, val);
}

void mp_common_create_default_playlist()
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (mp_playlist_mgr_count(ad->playlist_mgr) > 0) {
		// already created
		return;
	}

	int count = 0;
	mp_media_list_h all = NULL;

	char *last_played_path = NULL;
	mp_setting_get_now_playing_path_from_file(&last_played_path);
	SECURE_DEBUG("last played path = %s", last_played_path);

	mp_common_create_playlist_mgr();
	mp_playlist_mgr_clear(ad->playlist_mgr);

	mp_playlist_mgr_lazy_append_with_file(ad->playlist_mgr, MP_NOWPLAYING_LIST_DATA, last_played_path, -1);

	if (mp_playlist_mgr_count(ad->playlist_mgr) == 0) {
		mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
		mp_media_info_list_create(&all, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, count);
		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, all, count, 0, last_played_path);
		mp_media_info_list_destroy(all);
	}
	IF_FREE(last_played_path);
}

void
mp_common_playlist_album_update(mp_media_info_h playlist_handle)
{
	startfunc;
	int ret = 0;
	mp_media_info_h media_info = NULL;
	mp_media_list_h svc_handle = NULL;
	int count = 0, playlist_id = 0;
	char *path = NULL;

	mp_media_info_group_get_playlist_id(playlist_handle, &playlist_id);
	mp_media_info_list_count(MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, playlist_id, &count);

	/* get music item data */
	ret = mp_media_info_list_create(&svc_handle, MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, playlist_id, 0, count);
	if (ret != 0) {
		DEBUG_TRACE("fail to get list item: %d", ret);
		ret = mp_media_info_list_destroy(svc_handle);
		svc_handle = NULL;
	}

	media_info = mp_media_info_list_nth_item(svc_handle, count - 1);
	mp_media_info_get_thumbnail_path(media_info, &path);

	if (!path || !g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) || !strcmp(BROKEN_ALBUMART_IMAGE_PATH, path)) {
		path = DEFAULT_THUMBNAIL;
	}

	mp_media_info_playlist_set_thumbnail_path(playlist_handle, path);
	mp_media_info_list_destroy(svc_handle);
}

void
mp_common_popup_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	Evas_Object *genlist = data;
	evas_object_data_set(genlist, "popup", NULL); // Set popup data as NULL when popup is deleted.
	elm_object_scroll_freeze_pop(genlist);      // Enable scrolling
}

#ifdef MP_SOUND_PLAYER
static char *
_mp_util_convert_url(char *uri)
{
	char *path = NULL;
	MP_CHECK_NULL(uri);
	if (strstr(uri,  MP_FILE_PREFIX)) {
		path = g_strdup(uri + strlen(MP_FILE_PREFIX));
	} else {
		path = g_strdup(uri);
	}
	return path;
}

static bool
_mp_common_set_current_playing_item(const char *path)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	mp_common_create_playlist_mgr();
	mp_playlist_mgr_clear(ad->playlist_mgr);
	mp_plst_item *item = mp_playlist_mgr_item_append(ad->playlist_mgr, path, NULL, NULL, NULL, MP_TRACK_URI);
	mp_playlist_mgr_set_current(ad->playlist_mgr, item);

	return true;
}

static bool
_mp_common_multiple_view_operation(app_control_h app_control)
{
	startfunc;
	bool res = false;
	char **value = NULL;
	char **thumbs = NULL;
	char **titles = NULL;
	char **artist = NULL;
	int i, length = 0, thumb_length = 0, title_length = 0, artist_length = 0;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	mp_common_create_playlist_mgr();
	mp_playlist_mgr_clear(ad->playlist_mgr);
	if (app_control_get_extra_data_array(app_control, APP_CONTROL_DATA_PATH, &value, &length) == APP_CONTROL_ERROR_NONE) {
		app_control_get_extra_data_array(app_control, "thumbnail/path", &thumbs, &thumb_length);
		if (thumbs && thumb_length != length) {
			WARN_TRACE("uri & thumbnail count is not same...");
			IF_FREE(thumbs);
		}

		app_control_get_extra_data_array(app_control, APP_CONTROL_DATA_TITLE, &titles, &title_length);
		if (titles && length != title_length) {
			WARN_TRACE("uri & title count is not same...");
			IF_FREE(titles);
		}
		app_control_get_extra_data_array(app_control, MP_SAMSUNG_LINK_ARTIST, &artist, &artist_length);
		if (artist && length != artist_length) {
			WARN_TRACE("uri & artist count is not same...");
			IF_FREE(artist);
		}
		for (i = 0; i < length; i++) {
			char *path = _mp_util_convert_url(value[i]);
			mp_plst_item *item = mp_playlist_mgr_item_append(ad->playlist_mgr, path, NULL, NULL, NULL, MP_TRACK_URI);
			if (thumbs) {
				item->thumbnail_path = g_strdup(thumbs[i]);
			}
			if (titles) {
				item->title = g_strdup(titles[i]);
			}
			if (artist) {
				item->artist = g_strdup(artist[i]);
			}
			IF_FREE(path);
		}
		mp_playlist_mgr_set_current(ad->playlist_mgr, mp_playlist_mgr_get_nth(ad->playlist_mgr, 0));
		res = true;
	}

	IF_FREE(value);
	IF_FREE(thumbs);
	IF_FREE(titles);
	IF_FREE(artist);

	return res;
}

char *_get_folder_path(const char *path)
{
	return mp_file_dir_get(path);
}

static Eina_List *
_mp_common_get_track_list_by_folder(const char *folder)
{
	MP_CHECK_NULL(folder);
	MP_CHECK_NULL(mp_file_path_dir_exists(folder));

	Eina_List *audio_list = NULL;

	Eina_List *ls = mp_file_ls(folder);
	MP_CHECK_NULL(ls);

	Eina_List *l = NULL;
	char *file = NULL;
	EINA_LIST_FOREACH(ls, l, file) {
		char *path = g_strdup_printf("%s/%s", folder, file);
		IF_FREE(file);
		if (path && !mp_file_is_dir(path)) {
			char *mime = mp_util_file_mime_type_get(path);
			DEBUG_TRACE("mime_type: %s", mime);
			if (mime && strstr(mime, "audio/")) {
				audio_list = eina_list_append(audio_list, (void *)path);
			} else {
				IF_FREE(path);
			}
			IF_FREE(mime);
		}
	}

	eina_list_free(ls);
	ls = NULL;

	return audio_list;
}

static bool
_mp_common_view_by_folder(const char *path, app_control_h app_control)
{
	MP_CHECK_FALSE(path);
	MP_CHECK_FALSE(app_control);
	int ret = false;
	char *value = NULL;
	char *folder = NULL;
	char *hidden_file_path = NULL;
	/* Handling dot(hidden) file path*/
	app_control_get_uri(app_control, &hidden_file_path);

	SECURE_DEBUG("The Hidden file path is  %s", hidden_file_path);

	//app_control_add_extra_data(service, "View By", "By Folder");
	//app_control_add_extra_data(service, "sort_type", "MYFILE_SORT_BY_NAME_A2Z");
	//app_control_add_extra_data(service, "sort_type", "MYFILE_SORT_BY_NAME_Z2A");
	//app_control_add_extra_data(service, "sort_type", "MYFILE_SORT_BY_DATE_O2R");
	//app_control_add_extra_data(service, "sort_type", "MYFILE_SORT_BY_SIZE_L2S");


	if (app_control_get_extra_data(app_control, "View By", &value) == APP_CONTROL_ERROR_NONE) {
		if (!g_strcmp0(value, "By Folder")) {
			folder = _get_folder_path(path);
			SECURE_DEBUG("View By folder %s", folder);
			ret = true;
		}
		IF_FREE(value);
	}

	if (ret) {
		if (app_control_get_extra_data(app_control, "sort_type", &value) == APP_CONTROL_ERROR_NONE) {
			struct appdata *ad = mp_util_get_appdata();
			mp_media_list_h list = NULL;
			mp_media_info_h media = NULL;
			mp_plst_item *item = NULL;
			int i = 0;
			mp_plst_item *cur = NULL;
			char *file_path = NULL;

			MP_CHECK_FALSE(ad);

			mp_common_create_playlist_mgr();
			mp_playlist_mgr_clear(ad->playlist_mgr);

			mp_media_info_sorted_track_list_create(&list, value);

			int count = mp_media_infor_list_get_count(list);
			if (count > 0) {
				count = 0;
				do {
					char *folder_path = NULL;
					media = mp_media_info_list_nth_item(list, i);
					if (!media) {
						break;
					}

					mp_media_info_get_file_path(media, &file_path);
					i ++;
					DEBUG_TRACE("file path:%s", file_path);
					folder_path = _get_folder_path(file_path);
					if (g_strcmp0(folder, folder_path)) {
						IF_FREE(folder_path);
						continue;
					}
					item = mp_playlist_mgr_item_append(ad->playlist_mgr, file_path, NULL, NULL, NULL, MP_TRACK_URI);
					++count;

					if (!cur && !g_strcmp0(path, file_path)) {
						ERROR_TRACE("setting current file");
						cur = item;
					}
					IF_FREE(folder_path);
				} while (media);
				mp_media_info_list_destroy(list);
				list = 0;
			}
			if (count == 0) {
				Eina_List *file_list = _mp_common_get_track_list_by_folder(folder);
				if (file_list) {
					Eina_List *l = NULL;
					EINA_LIST_FOREACH(file_list, l, file_path) {
						DEBUG_TRACE("file_path: %s", file_path);
						item = mp_playlist_mgr_item_append(ad->playlist_mgr, file_path, NULL, NULL, NULL, MP_TRACK_URI);
						if (!cur && !g_strcmp0(path, file_path)) {
							cur = item;
						}
						IF_FREE(file_path);
					}
					eina_list_free(file_list);
					file_list = NULL;
				}
			}

			if (cur) {
				mp_playlist_mgr_set_current(ad->playlist_mgr, cur);
			} else {
				DEBUG_TRACE("Setting hidden file");
				item = mp_playlist_mgr_item_append(ad->playlist_mgr, hidden_file_path, NULL, NULL, NULL, MP_TRACK_URI);
				cur = item;
				mp_playlist_mgr_set_current(ad->playlist_mgr, cur);
			}
		} else {
			WARN_TRACE("No sort type");
			ret = false;
		}

		IF_FREE(value);
	}

	IF_FREE(folder);

	return ret;
}

bool
mp_common_parse_view_operation(app_control_h app_control)
{
	char *uri = NULL;
	char *operation = NULL;
	bool res = false;

	MP_CHECK_FALSE(app_control);

	app_control_get_operation(app_control, &operation);
	DEBUG_TRACE("operation: %s", operation);

	if (!operation) {
		ERROR_TRACE("No operation");
		goto END;
	}

	if (!strcmp(APP_CONTROL_OPERATION_VIEW , operation)) {
		struct appdata *ad = mp_util_get_appdata();
		char *value = NULL;

		ad->samsung_link = false;

		app_control_get_extra_data(app_control, "LAUNCHAPP", &value);
		if (value && !g_strcmp0(value, "SamsungLink")) {
			ad->samsung_link = true;
		}
		IF_FREE(value);

		app_control_get_extra_data(app_control, "enableChangePlayer", &value);
		if (value && !g_strcmp0(value, "true")) {
			ad->disable_change_player = false;
		} else {
			ad->disable_change_player = true;
		}
		IF_FREE(value);

		if (_mp_common_multiple_view_operation(app_control)) {
			DEBUG_TRACE("Multiple view operation");
			res = true;
			goto END;
		}
		app_control_get_uri(app_control, &uri);
		if (uri) {
			char *path = _mp_util_convert_url(uri);

			if (!_mp_common_view_by_folder(path, app_control)) {
				_mp_common_set_current_playing_item(path);
			}
			IF_FREE(path);
			free(uri);
			res = true;
		} else {
			ERROR_TRACE("No URI.");
			goto END;
		}
	} else {
		WARN_TRACE("Operation is not APP_CONTROL_OPERATION_VIEW [%s]", operation);
		goto END;
	}

END:
	SAFE_FREE(operation);
	return res;
}
#endif

#ifndef MP_SOUND_PLAYER
bool
_mp_common_parse_open_shortcut(app_control_h app_control, MpTab_e *tab, char **shortcut_main_info)
{
	MP_CHECK_FALSE(app_control);
	MP_CHECK_FALSE(tab);
	MP_CHECK_FALSE(shortcut_main_info);

	char *shortcut_type = NULL;

	if (app_control_get_extra_data(app_control, MP_REQ_TYPE_SHORTCUT_TYPE, &shortcut_type) == APP_CONTROL_ERROR_NONE) {
		if (!g_strcmp0(shortcut_type, MP_SHORTCUT_PLAYLIST)) {
			*tab = MP_TAB_PLAYLISTS;
			app_control_get_extra_data(app_control, MP_REQ_TYPE_SHORTCUT_DESC, shortcut_main_info);
		} else if (!g_strcmp0(shortcut_type, MP_SHORTCUT_ALBUM)) {
			*tab = MP_TAB_ALBUMS;
			app_control_get_extra_data(app_control, MP_REQ_TYPE_SHORTCUT_DESC, shortcut_main_info);
		} else if (!g_strcmp0(shortcut_type, MP_SHORTCUT_ARTIST)) {
			*tab = MP_TAB_ARTISTS;
			app_control_get_extra_data(app_control, MP_REQ_TYPE_SHORTCUT_DESC, shortcut_main_info);
		}
	}
	mp_debug("load all_viwe type [%d][%s]", *tab, *shortcut_main_info);
	return true;
}

static bool
_mp_common_load_playlist_detail_view(char *shortcut_main_info)
{
	startfunc;

	int id = atoi(shortcut_main_info);
	mp_debug("playlist id = %d", id);
	char *name = NULL;

	if (mp_media_info_playlist_get_name_by_id(id, &name)) {
		return false;
	}

	mp_track_type_e type = MP_TRACK_BY_PLAYLIST;
	if (!g_strcmp0(STR_MP_MOST_PLAYED, name)) {
		type = MP_TRACK_BY_PLAYED_COUNT;
	} else if (!g_strcmp0((STR_MP_RECENTLY_ADDED), name)) {
		type = MP_TRACK_BY_ADDED_TIME;
	} else if (!g_strcmp0((STR_MP_RECENTLY_PLAYED), name)) {
		type = MP_TRACK_BY_PLAYED_TIME;
	} else if (!g_strcmp0((STR_MP_FAVOURITES), name)) {
		type = MP_TRACK_BY_FAVORITE;
	}

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpPlaylistDetailView_t *view_plst_detail = mp_playlist_detail_view_create(view_manager->navi, type, name, id);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_plst_detail, NULL);

	mp_view_update_options((MpView_t *)view_plst_detail);
	mp_view_set_title((MpView_t *)view_plst_detail, name);

	return true;
}

static bool
_mp_common_load_album_detail_view(char *shortcut_main_info)
{

	mp_media_list_h media_list = NULL;
	char *artist = NULL;
	char *thumbnail = NULL;

	int count = 0, index = 0;

	mp_media_info_group_list_count(MP_GROUP_BY_ALBUM, NULL, NULL, &count);
	mp_media_info_group_list_create(&media_list, MP_GROUP_BY_ALBUM, NULL, NULL, 0, count);
	MP_CHECK_FALSE(media_list);

	mp_media_info_h media = NULL;

	do {
		media = mp_media_info_group_list_nth_item(media_list, index);

		char *name = NULL;
		if (media) {
			mp_media_info_group_get_main_info(media, &name);
			if (g_strcmp0(shortcut_main_info, name)) {
				index++;
				continue;
			}
			mp_media_info_group_get_sub_info(media, &artist);
			mp_media_info_group_get_thumbnail_path(media, &thumbnail);
		}
		break;
	} while (media);
	MP_CHECK_FALSE(media);

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpAlbumDetailView_t *view_album_detail = mp_album_detail_view_create(view_manager->navi, shortcut_main_info, artist, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_album_detail, NULL);

	mp_view_update_options((MpView_t *)view_album_detail);
	mp_view_set_title((MpView_t *)view_album_detail, shortcut_main_info);

	mp_media_info_group_list_destroy(media_list);

	return true;
}


static bool
_mp_common_load_artist_detail_view(char *shortcut_main_info)
{

	mp_media_list_h media_list = NULL;
	char *artist = NULL;
	char *thumbnail = NULL;

	int count = 0, index = 0;

	mp_media_info_group_list_count(MP_GROUP_BY_ARTIST, NULL, NULL, &count);
	mp_media_info_group_list_create(&media_list, MP_GROUP_BY_ARTIST, NULL, NULL, 0, count);
	MP_CHECK_FALSE(media_list);

	mp_media_info_h media = NULL;

	do {
		media = mp_media_info_group_list_nth_item(media_list, index);

		char *name = NULL;
		if (media) {
			mp_media_info_group_get_main_info(media, &name);
			if (g_strcmp0(shortcut_main_info, name)) {
				index++;
				continue;
			}
			mp_media_info_group_get_sub_info(media, &artist);
			mp_media_info_group_get_thumbnail_path(media, &thumbnail);
		}
		break;
	} while (media);
	MP_CHECK_FALSE(media);

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MpArtistDetailView_t *view_artist_detail = mp_artist_detail_view_create(view_manager->navi, shortcut_main_info, thumbnail);
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_artist_detail, NULL);

	mp_view_update_options((MpView_t *)view_artist_detail);
	mp_view_set_title((MpView_t *)view_artist_detail, shortcut_main_info);

	mp_media_info_group_list_destroy(media_list);

	return true;
}

static void
_mp_common_load_view_by_shortcut(MpTab_e tab, char *shortcut_main_info)
{
	struct appdata *ad  = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_popup_destroy(ad);

	if (mp_view_mgr_pop_to_view(GET_VIEW_MGR, MP_VIEW_ALL))	{
		DEBUG_TRACE("Error: unable to pop to all view");
	}

	switch (tab) {
	case MP_TAB_PLAYLISTS:
		if (!_mp_common_load_playlist_detail_view(shortcut_main_info)) {
			mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_BODY_NO_PLAYLISTS"));
			elm_win_activate(ad->win_main);
		}
		break;
	case MP_TAB_ALBUMS:
		if (!_mp_common_load_album_detail_view(shortcut_main_info)) {
			mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_BODY_NO_ALBUMS"));
			elm_win_activate(ad->win_main);
		}
		break;
	case MP_TAB_ARTISTS:
		if (!_mp_common_load_artist_detail_view(shortcut_main_info)) {
			mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_BODY_NO_ARTISTS"));
			elm_win_activate(ad->win_main);
		}
		break;
	default:
		break;
	}
}

static void
_mp_common_transition_finish_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	evas_object_smart_callback_del(GET_VIEW_MGR->navi , "transition,finished", _mp_common_transition_finish_cb);
	struct appdata *ad  = mp_util_get_appdata();
	MP_CHECK(ad);

	evas_object_show(ad->win_main);
	elm_win_activate(ad->win_main);
	ad->app_is_foreground = true;
}

static void
_mp_common_create_all_view(MpViewMgr_t *view_manager)
{
	MpAllView_t *view = NULL;
	if (mp_view_mgr_count_view(view_manager) == 0) {
		PROFILE_IN("mp_all_view_create");
		view = mp_all_view_create(view_manager->navi, MP_TAB_SONGS);
		PROFILE_OUT("mp_all_view_create");

		PROFILE_IN("mp_view_update");
		mp_view_update((MpView_t*)view);
		//mp_all_view_select_tab((MpView_t*)view, tab);
		PROFILE_OUT("mp_view_update");

#ifndef MP_CREATE_FAKE_IMAGE
		PROFILE_IN("mp_view_mgr_push_view");
		mp_view_mgr_push_view(view_manager, (MpView_t *)view, NULL);
		PROFILE_OUT("mp_view_mgr_push_view");
#endif

		PROFILE_IN("mp_view_update_options");
		mp_view_update_options((MpView_t *)view);
		PROFILE_OUT("mp_view_update_options");

		PROFILE_IN("mp_view_set_title");
		mp_view_set_title((MpView_t *)view, STR_MP_MUSIC);
		PROFILE_OUT("mp_view_set_title");

	}
}


static void
_mp_common_create_main_view(MpViewMgr_t *view_manager)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	bool ready_current_track = (ad->current_track_info == NULL) ? true : false;

	if (ready_current_track) {
		PROFILE_IN("ready_recent_track_info");
		mp_plst_item *plst_item = NULL;

		if (mp_playlist_mgr_count(ad->playlist_mgr) > 0) {
			DEBUG_TRACE("playlist exist");
			plst_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
		} else {
			char *last_played_path = NULL;
			mp_setting_get_now_playing_path_from_file(&last_played_path);
			SECURE_DEBUG("last played path = %s", last_played_path);
			if (mp_check_file_exist(last_played_path)) {
				plst_item = mp_playlist_mgr_custom_item_new(last_played_path);
			}
			IF_FREE(last_played_path);
		}

		if (plst_item) {
			mp_util_load_track_info(ad, plst_item, &ad->current_track_info);
			mp_playlist_mgr_custom_item_free(plst_item);
			plst_item = NULL;
		}
		PROFILE_OUT("ready_recent_track_info");
	}

	_mp_common_create_all_view(view_manager);
}

#endif

void mp_common_create_initial_view(void *appdata, app_control_h app_control, int *launch_by_shortcut)
{
	struct appdata *ad  = appdata;
	MP_CHECK(ad);

#ifdef MP_SOUND_PLAYER
	ad->app_is_foreground = true;
	MpView_t *player_view = GET_PLAYER_VIEW;
	if (!player_view) {
		mp_common_show_player_view(MP_PLAYER_NORMAL, false, false, false);
	}
	mp_player_view_refresh((MpPlayerView_t *)player_view);
#else	//MP_SOUND_PLAYER

	MpTab_e tab = MP_TAB_SONGS;
	char *shortcut_main_info = NULL;

	_mp_common_parse_open_shortcut(app_control, &tab, &shortcut_main_info);
	if (shortcut_main_info) {
		if (mp_view_mgr_count_view(GET_VIEW_MGR) == 0) {
			_mp_common_create_main_view(GET_VIEW_MGR);
		}

		_mp_common_load_view_by_shortcut(tab, shortcut_main_info);
		if (launch_by_shortcut) {
			*launch_by_shortcut = true;
		}
		free(shortcut_main_info);

		evas_object_smart_callback_add(GET_VIEW_MGR->navi , "transition,finished", _mp_common_transition_finish_cb, NULL);

		DEBUG_TRACE("Create shortcut view done");
		return;
	}

	_mp_common_create_main_view(GET_VIEW_MGR);

#endif //MP_SOUND_PLAYER

}

/*used for long press playall*/
static void
_mp_common_selected_item_data_get(void *thiz, GList **selected)
{
	startfunc;
	MpList_t *list = thiz;
	GList *sel_list = NULL;

	if (!list->genlist) {
		goto END;
	}

	Elm_Object_Item *item = NULL;
	mp_list_item_data_t *gl_item = NULL;

	if (MP_LIST_OBJ_IS_GENGRID(list->genlist)) {
		item = elm_gengrid_first_item_get(list->genlist);
	} else {
		item = elm_genlist_first_item_get(list->genlist);
	}

	if (!item) {
		goto END;
	}

	while (item) {
		if (mp_list_item_select_mode_get(item) != ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
			gl_item = elm_object_item_data_get(item);
			if (gl_item && gl_item->checked) {
				sel_list = g_list_append(sel_list, gl_item);
			}
		}
		item = mp_list_item_next_get(item);
	}
END:
	if (selected) {
		*selected = sel_list;
	}
}

void mp_common_playall_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();

	MpList_t *list = data;
	MP_CHECK(list);

	int count = 0;
	char *type_str = NULL;
	int ret = 0;
	int playlist_id = -1;

	mp_popup_destroy(ad);

	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;
	mp_media_list_h svc_handle = NULL;

	_mp_common_selected_item_data_get((MpList_t*)list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}

	if (!ad->playlist_mgr) {
		mp_common_create_playlist_mgr();
	}

	mp_group_type_e group_type = mp_list_get_group_type((MpList_t*)list);

	DEBUG_TRACE("group_type: %d", group_type);
	if (group_type == MP_GROUP_BY_PLAYLIST) {
		/* get playlist name */
		ret = mp_media_info_group_get_playlist_id(item_data->handle, &playlist_id);
		mp_debug("get playlist name ret = %d", ret);
		/* create playlist */
		count = mp_playlist_list_set_playlist(ad->playlist_mgr, playlist_id);
		ad->paused_by_user = FALSE;
	} else if (group_type == MP_GROUP_BY_ALBUM) {
		/* get playlist name */
		ret = mp_media_info_group_get_main_info(item_data->handle, &type_str);

		mp_media_info_list_count(MP_TRACK_BY_ALBUM, type_str, NULL, NULL, 0, &count);
		mp_media_info_list_create(&svc_handle,
		                          MP_TRACK_BY_ALBUM, type_str, NULL, NULL, 0, 0, count);

		if (count) {
			mp_playlist_mgr_clear(ad->playlist_mgr);
			mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);
			ad->paused_by_user = FALSE;
		}
	} else if (group_type == MP_GROUP_BY_ARTIST) {
		/* get playlist name */
		ret = mp_media_info_group_get_main_info(item_data->handle, &type_str);
		mp_media_info_list_count(MP_TRACK_BY_ARTIST, type_str, NULL, NULL, 0, &count);
		mp_media_info_list_create(&svc_handle,
		                          MP_TRACK_BY_ARTIST, type_str, NULL, NULL, 0, 0, count);

		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);

		if (count) {
			mp_playlist_mgr_clear(ad->playlist_mgr);
			mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);
			ad->paused_by_user = FALSE;
		}
	} else if (group_type == MP_GROUP_BY_GENRE) {
		/* get playlist name */
		ret = mp_media_info_group_get_main_info(item_data->handle, &type_str);

		mp_media_info_list_count(MP_TRACK_BY_GENRE, type_str, NULL, NULL, 0, &count);
		mp_media_info_list_create(&svc_handle,
		                          MP_TRACK_BY_GENRE, type_str, NULL, NULL, 0, 0, count);

		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);

		if (count) {
			mp_playlist_mgr_clear(ad->playlist_mgr);
			mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);
			ad->paused_by_user = FALSE;
		}
	}
	if (count == 0) {
		mp_widget_text_popup(NULL, GET_STR(STR_MP_NO_TRACKS));
		return;
	}

	ret = mp_play_new_file(ad, TRUE);
	if (ret) {
		ERROR_TRACE("Error: mp_play_new_file..");
		if (ret == MP_PLAY_ERROR_NO_SONGS) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_UNABLE_TO_PLAY_ERROR_OCCURED));
		}
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
		}
#endif
	}

	if (svc_handle) {
		mp_media_info_list_destroy(svc_handle);
	}

	endfunc;
}

void mp_common_playlist_rename_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	MpPlaylistList_t *view = (MpPlaylistList_t *)data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_popup_destroy(ad);

	GList *sel_list = NULL;
	mp_list_item_data_t *item_data = NULL;

	_mp_common_selected_item_data_get((MpList_t*)list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}

	mp_group_type_e group_type = mp_list_get_group_type((MpList_t*)list);

	DEBUG_TRACE("group_type: %d", group_type);
	if (group_type == MP_GROUP_BY_PLAYLIST) {
		Mp_Playlist_Data *mp_playlist_data = mp_edit_playlist_create(MP_PLST_RENAME);
		MP_CHECK(mp_playlist_data);
		mp_playlist_data->playlist_handle = item_data->handle;
		mp_edit_playlist_content_create(mp_playlist_data);
	}
}

int mp_common_get_playlist_totaltime(mp_track_type_e track_type, int playlist_id, int count)
{
	mp_media_list_h media_list = NULL;
	mp_media_info_h item = NULL;
	int res = 0;
	int i = 0;
	int time = 0;
	int total = 0;

	res = mp_media_info_list_create(&media_list, track_type, NULL, NULL, NULL, playlist_id, 0, count);
	MP_CHECK_VAL((res == 0), 0);
	MP_CHECK_VAL(media_list, 0);

	for (i = 0; i < count; i++) {
		time = 0;
		item = mp_media_info_group_list_nth_item(media_list, i);
		if (NULL == item) {
			continue;
		}
		mp_media_info_get_duration(item, &time);
		total += time;
	}
	mp_media_info_list_destroy(media_list);
	media_list = NULL;

	return total;
}

#ifdef MP_FEATURE_ADD_TO_HOME
static char *_mp_media_info_get_live_auto_playlist_thumbnail_by_name(const char *name)
{
	MP_CHECK_VAL(name, NULL);

	char *thumb_path = NULL;

	if (!g_strcmp0(name, STR_MP_FAVOURITES)) {
		thumb_path = LIVE_THUMBNAIL_QUICK_LIST;
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_PLAYED)) {
		thumb_path = LIVE_THUMBNAIL_RECENTLY_PLAYED;
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_ADDED)) {
		thumb_path = LIVE_THUMBNAIL_RECENTLY_ADDED;
	} else if (!g_strcmp0(name, STR_MP_MOST_PLAYED)) {
		thumb_path = LIVE_THUMBNAIL_MOST_PLAYED;
	}

	return thumb_path;
}

static char *_mp_media_info_get_live_auto_playlist_icon_by_name(const char *name)
{
	MP_CHECK_VAL(name, NULL);

	char *icon_path = NULL;

	if (!g_strcmp0(name, STR_MP_FAVOURITES)) {
		icon_path = LIVE_ICON_QUICK_LIST;
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_PLAYED)) {
		icon_path = LIVE_ICON_RECENTLY_PLAYED;
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_ADDED)) {
		icon_path = LIVE_ICON_RECENTLY_ADDED;
	} else if (!g_strcmp0(name, STR_MP_MOST_PLAYED)) {
		icon_path = LIVE_ICON_MOST_PLAYED;
	}

	return icon_path;
}

void mp_common_add_to_home_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();

	MpList_t *list = data;
	MP_CHECK(list);

	GList *sel_list = NULL;
	char *name = NULL;
	char *thumbnail = NULL;
	int ret = 0;
	int p_id = -1;
	mp_list_item_data_t *item_data = NULL;

	mp_popup_destroy(ad);

	mp_list_selected_item_data_get((MpList_t *)list,  &sel_list);

	if (g_list_length(sel_list) == 0) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_NOTHING_SELECTED"));
		return;
	}

	GList *node = g_list_first(sel_list);
	while (node) {
		item_data = node->data;
		node = g_list_next(node);
	}

	ret = mp_media_info_group_get_main_info(item_data->handle, &name);
	mp_retm_if(ret != 0, "Fail to get value");
	mp_retm_if(name == NULL, "Fail to get value");
	mp_media_info_group_get_thumbnail_path(item_data->handle, &thumbnail);

	int type = 0;
	const char *extra1 = NULL;
	const char *extra2 = NULL;
	mp_group_type_e group_type = mp_list_get_group_type((MpList_t*)list);
	if (group_type == MP_GROUP_BY_PLAYLIST) {
		ret = mp_media_info_group_get_playlist_id(item_data->handle, &p_id);
		mp_retm_if(ret != 0, "Fail to get value");
		DEBUG_TRACE("p_id: %d", p_id);
		if (p_id < 0) {
			type = MP_ADD_TO_HOME_SHORTCUT_TYPE_SYS_PLAYLIST;
			extra1 = _mp_media_info_get_live_auto_playlist_thumbnail_by_name(name);
			extra2 = _mp_media_info_get_live_auto_playlist_icon_by_name(name);
		} else {
			type = MP_ADD_TO_HOME_SHORTCUT_TYPE_USER_PLAYLIST;
			name = (void *)p_id;
		}
		mp_menu_add_to_home(type, name, (void *)extra1, (void *)extra2);
	} else if (group_type == MP_GROUP_BY_ALBUM) {
		mp_menu_add_to_home(MP_ADD_TO_HOME_SHORTCUT_TYPE_ALBUM, name, thumbnail, NULL);
	}

	if (sel_list) {
		g_list_free(sel_list);
		sel_list = NULL;
	}
}
#endif

#ifdef MP_FEATURE_PERSONAL_PAGE
all_in_personal_e mp_common_is_all_in_personal_page(Evas_Object *genlist)
{
	MP_CHECK_VAL(genlist, MP_COMMON_ALL_ERROR);
	int count = elm_genlist_items_count(genlist);

	int in_count = 0;
	int out_count = 0;
	int invalid_count = 0;

	int index = 0;
	for (index = 0; index < count; index++) {
		Elm_Object_Item *item = NULL;
		item = elm_genlist_nth_item_get(genlist, index);
		if (item == NULL) {
			invalid_count++;
			continue;
		}

		mp_list_item_data_t *item_data = elm_object_item_data_get(item);
		if (item_data == NULL) {
			invalid_count++;
			continue;
		}

		if (item_data->handle == NULL) {
			invalid_count++;
			continue;
		}

		char *path = NULL;
		if (item_data->group_type == MP_GROUP_NONE) {
			mp_media_info_get_file_path(item_data->handle, &path);
		} else if (item_data->group_type == MP_GROUP_BY_FOLDER) {
			mp_media_info_group_get_sub_info(item_data->handle, &path);
		} else {
			ERROR_TRACE("Unsupported type");
		}

		DEBUG_TRACE("--------->		path is %s", path);
		if (path == NULL) {
			invalid_count++;
			continue;
		}

		if (mp_util_is_in_personal_page((const char *)path)) {
			in_count++;
		} else {
			out_count++;
		}
	}

	if (in_count == (count - invalid_count)) {
		return MP_COMMON_ALL_IN;
	} else if (out_count == (count - invalid_count)) {
		return MP_COMMON_ALL_OUT;
	} else {
		return MP_COMMON_PART;
	}
}

void mp_common_add_to_personal_page_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpListView_t *view = (MpListView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	MpView_t *edit_view = (MpView_t *)mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false, MP_EDIT_VIEW_PERSONAL_PAGE_ADD);
	if (edit_view == NULL) {
		return;
	}
	mp_view_mgr_push_view(GET_VIEW_MGR, edit_view, NULL);
	mp_view_update(edit_view);
	mp_view_update_options(edit_view);

	mp_view_set_title(edit_view, STR_MP_TILTE_SELECT_ITEM);
	mp_list_view_set_select_all((MpListView_t*)edit_view, true);
	mp_list_show_fast_scroll(((MpListView_t*)edit_view)->content_to_show);
}

void mp_common_remove_from_personal_page_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpListView_t *view = (MpListView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	MpView_t *edit_view = (MpView_t *)mp_edit_view_create(GET_NAVIFRAME, view->content_to_show, false, MP_EDIT_VIEW_PERSONAL_PAGE_REMOVE);

	if (edit_view == NULL) {
		return;
	}
	mp_view_mgr_push_view(GET_VIEW_MGR, edit_view, NULL);
	mp_view_update(edit_view);
	mp_view_update_options(edit_view);

	mp_view_set_title(edit_view, STR_MP_TILTE_SELECT_ITEM);
	mp_list_view_set_select_all((MpListView_t*)edit_view, true);
	mp_list_show_fast_scroll(((MpListView_t*)edit_view)->content_to_show);
}

all_in_personal_e mp_common_personal_status(void *thiz)
{
	MpList_t *list = (MpList_t *)thiz;
	return mp_common_is_all_in_personal_page(list->genlist);
}

void
mp_common_longpress_private_move_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_popup_destroy(ad);

	MpList_t *list = data;
	MP_CHECK(list);
	DEBUG_TRACE("list->personal_page_storage is %d", list->personal_page_storage);
	/*0. check if is remove from personal page*/
	if (list->personal_page_storage == MP_LIST_PERSONAL_PAGE_NORMAL) {
		mp_edit_cb_excute_move(list);
	} else {
		/*1. get personal don't ask again */
		bool no_ask_flag = false;
		mp_setting_get_personal_dont_ask_again(&no_ask_flag);
		DEBUG_TRACE("no_ask_flag is %d", no_ask_flag);
		if (no_ask_flag) {
			mp_edit_cb_excute_move(list);
		} else {
			mp_edit_view_notify_popup(list);
		}
	}
}

#endif

#ifndef MP_SOUND_PLAYER
void mp_common_force_close_delete()
{
	Ecore_Thread *delete_thread = (Ecore_Thread *)mp_edit_get_delete_thread();
	MP_CHECK(delete_thread);
	ecore_thread_cancel(delete_thread);
	delete_thread = NULL;
}
#endif

void mp_common_view_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	MP_CHECK(view_mgr);
	MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
	MP_CHECK(view);
	mp_view_update_options_edit(view);
}

bool mp_common_track_is_current(mp_media_info_h media, MpList_t *list)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);
	mp_track_info_t* current = ad->current_track_info;
	MP_CHECK_FALSE(current);

	char *uri = NULL;
	mp_media_info_get_file_path(media, &uri);
	mp_retv_if(!uri, NULL);
	bool match = false;
	if (current && !g_strcmp0(current->uri, uri) && list->edit_mode == 0) {
		match = true;
	}
	return match;
}


