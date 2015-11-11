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


#include "mp-make-offline-view.h"
#include "mp-track-list.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-edit-callback.h"

#ifdef MP_FEATURE_CLOUD

static void
_mp_make_offline_view_destory_cb(void *thiz)
{
	eventfunc;
	MpMakeOfflineView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/

	free(view);
}

static Eina_Bool _mp_make_offline_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	mp_view_mgr_pop_view(view_mgr, false);
	return EINA_TRUE;
}

static void _mp_make_offline_view_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpMakeOfflineView_t *view = (MpMakeOfflineView_t *)data;
	MP_CHECK(view);

	MpTrackList_t *list = (MpTrackList_t *)view->content_to_show;
	MP_CHECK(list);

	mp_edit_cb_excute_make_offline_available(list);
}

static int _mp_make_offline_view_update_option(void *thiz)
{
	startfunc;
	MpMakeOfflineView_t *view = (MpMakeOfflineView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);
	Evas_Object *toolbar = mp_widget_create_naviframe_toolbar(view->navi_it);
	Elm_Object_Item *toolbar_item = NULL;

	toolbar_item = mp_widget_create_toolbar_item_btn(toolbar,
	               MP_TOOLBAR_BTN_DEFAULT, STR_MP_DONE, _mp_make_offline_view_done_cb, view);
	view->toolbar_options[MP_OPTION_LEFT] = toolbar_item;
	if (mp_list_get_checked_count(view->content_to_show) == 0) {
		elm_object_item_disabled_set(view->toolbar_options[MP_OPTION_LEFT], EINA_TRUE);
	}

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_make_offline_view_back_cb, view);

	endfunc;
	return 0;
}

static int
_mp_make_offline_view_content_load(void *view)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_VAL(view, -1);
	/*MpMakeOfflineView_t *make_offline_view = (MpMakeOfflineView_t *)view;*/

	endfunc;
	return 0;
}

static void
_mp_make_offline_view_on_event(void *thiz, MpViewEvent_e event)
{
	switch (event) {
	case MP_ADD_TO_PLAYLIST_DONE:
		DEBUG_TRACE("Edit done");
		break;

	default:
		break;
	}
}

static int _mp_make_offline_view_update(void *thiz)
{
	MpMakeOfflineView_t *view = thiz;
	mp_list_update(view->content_to_show);
	mp_list_set_edit(view->content_to_show, true);
	return 0;
}

static int
_mp_make_offline_view_init(Evas_Object *parent, MpMakeOfflineView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_MAKE_OFFLINE);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_make_offline_view_update;
	view->update_options = _mp_make_offline_view_update_option;
	view->update_options_edit = _mp_make_offline_view_update_option;
	view->view_destroy_cb = _mp_make_offline_view_destory_cb;
	view->on_event = _mp_make_offline_view_on_event;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;

	MpTrackList_t *track_list = mp_track_list_create(view->layout);
	view->content_to_show = (MpList_t *)track_list;
	MP_CHECK_VAL(view->content_to_show, -1);
	mp_track_list_set_data(track_list, MP_TRACK_LIST_CLOUD_TYPE, MP_TRACK_LIST_VIEW_CLOUD, -1);

	elm_object_part_content_set(view->layout, "list_content", mp_list_get_layout(view->content_to_show));

	return ret;
}

MpMakeOfflineView_t *mp_make_offline_view_create(Evas_Object *parent)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpMakeOfflineView_t *view = calloc(1, sizeof(MpMakeOfflineView_t));
	MP_CHECK_NULL(view);

	ret = _mp_make_offline_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	_mp_make_offline_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_make_offline_view_create()");
	IF_FREE(view);
	return NULL;
}

#endif

