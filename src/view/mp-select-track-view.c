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

#include "mp-select-track-view.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-util.h"
#include "mp-add-track-view.h"

#define MP_MAX_TEXT_PRE_FORMAT_LEN 256
#define MP_MAX_ARTIST_NAME_WIDTH 320
#define MP_LABEL_SLIDE_DURATION 5


static void
_mp_select_track_view_destory_cb(void *thiz)
{
	eventfunc;
	MpSelectTrackView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/

	free(view);
}

int _mp_select_track_view_update(void *thiz)
{
	startfunc;
	MpSelectTrackView_t *view = thiz;

	mp_list_update(view->content_to_show);
	mp_list_set_edit(view->content_to_show, TRUE);

	return 0;
}

/***************	functions for track list update 	*******************/

static Eina_Bool _mp_select_track_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	mp_view_mgr_pop_view(view_mgr, false);

	return EINA_TRUE;
}

void
mp_select_track_view_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpAddTrackView_t *view = (MpAddTrackView_t *)mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ADD_TRACK);
	MP_CHECK(view);

	mp_edit_cb_excute_add_to_playlist(data, view->playlist_id, NULL, true);
}

static int _mp_select_track_view_update_options(void *thiz)
{
	startfunc;
	MpSelectTrackView_t *view = (MpSelectTrackView_t *)thiz;
	MP_CHECK_VAL(view, -1);
	mp_util_create_selectioninfo_with_count((MpView_t *)view, mp_list_get_checked_count((MpList_t *)view->content_to_show));

	unsigned int count = mp_list_get_editable_count((MpList_t *)view->content_to_show, MP_LIST_EDIT_TYPE_NORMAL);
	if (count <= 0) {
		mp_view_mgr_pop_a_view((MpViewMgr_t *)GET_VIEW_MGR, (MpView_t *)view);
	}

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_select_track_view_back_cb, view);

	if (mp_list_get_checked_count((MpList_t *)view->content_to_show) == count) {
		elm_check_state_set(view->select_all_btn, EINA_TRUE);
	} else {
		elm_check_state_set(view->select_all_btn, EINA_FALSE);
	}
	if (view->done_btn) {
		if (mp_list_get_checked_count((MpList_t *)view->content_to_show)) {
			elm_object_disabled_set(view->done_btn, EINA_FALSE);
		} else {
			elm_object_disabled_set(view->done_btn, EINA_TRUE);
		}
	}



	/* update the first controlba item */
	/*mp_view_manager_update_first_controlbar_item(layout_data);*/
	endfunc;
	return 0;
}

static void _mp_select_track_view_content_load(void *thiz)
{
	startfunc;
	MpSelectTrackView_t *view = (MpSelectTrackView_t *)thiz;
	MP_CHECK(view);
	MP_CHECK(view->layout);

	view->content_to_show = (MpList_t *)mp_track_list_create(view->layout);
	MP_CHECK(view->content_to_show);
	/*Todo: move the followed outside*/
	/* mp_track_list_set_data(view->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_BY_ALBUM, MP_TRACK_LIST_TYPE_STR, view->name, -1);
	view->content_to_show->update(view->content_to_show);*/
	MP_CHECK(view->content_to_show->layout);
	elm_object_part_content_set(view->layout, "list_content", view->content_to_show->layout);
	edje_object_signal_emit(_EDJ(view->layout), "SHOW_SELECT_ALL_PADDING", "*");
}

static void
_mp_select_track_view_on_event(void *thiz, MpViewEvent_e event)
{
	switch (event) {
	case MP_ADD_TO_PLAYLIST_DONE:
		mp_view_mgr_pop_a_view(GET_VIEW_MGR, thiz);
		break;

	default:
		break;
	}
}

static int
_mp_select_track_view_init(Evas_Object *parent, MpSelectTrackView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_SELECT_TRACK);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_select_track_view_update;
	view->update_options = _mp_select_track_view_update_options;
	view->update_options_edit = _mp_select_track_view_update_options;
	view->view_destroy_cb = _mp_select_track_view_destory_cb;
	view->content_set = _mp_select_track_view_content_load;
	view->on_event = _mp_select_track_view_on_event;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;

	return ret;
}

MpSelectTrackView_t *mp_select_track_view_create(Evas_Object *parent)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpSelectTrackView_t *view = calloc(1, sizeof(MpSelectTrackView_t));
	MP_CHECK_NULL(view);

	ret = _mp_select_track_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	_mp_select_track_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_select_track_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_select_track_view_destory(MpSelectTrackView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	return 0;
}


