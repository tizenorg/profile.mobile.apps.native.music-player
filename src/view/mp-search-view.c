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

#include "mp-search-view.h"
#include "mp-search-list.h"
#include "mp-widget.h"
#include "music.h"
#include "mp-search.h"
#include "mp-util.h"

#define MP_SEARCHBAR_W 400*elm_config_scale_get()

static void
_mp_search_view_destory_cb(void *thiz)
{
	eventfunc;
	MpSearchView_t *view = thiz;
	MP_CHECK(view);
	mp_list_view_fini((MpListView_t *)view);

	/* TODO: release resource..*/
	mp_search_view_destory(view);
	mp_ecore_timer_del(view->search_timer);

	free(view);
}

int _mp_search_view_update(void *thiz)
{
	startfunc;
	MpSearchView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	view->content_set(view);
	return 0;
}

static void _mp_search_view_update_option_clear(void *thiz)
{
	startfunc;
	MpSearchView_t *view = (MpSearchView_t *)thiz;
	MP_CHECK(view);

	/* destroy back button */
	Evas_Object *btn = NULL;
	btn = elm_object_item_part_content_unset(view->navi_it,
	        "toolbar_more_btn");
	mp_evas_object_del(btn);
	endfunc
}

/***************	functions for track list update	*******************/
static Eina_Bool _mp_search_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, EINA_TRUE);

	ad->del_cb_invoked = 0;
	MpSearchView_t *view = (MpSearchView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	{
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		mp_view_mgr_pop_view(view_mgr, false);

	}
	return EINA_TRUE;
}

static int _mp_search_view_update_options(void *thiz)
{
	startfunc;
	MpSearchView_t *view = (MpSearchView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	_mp_search_view_update_option_clear(view);

	elm_naviframe_item_title_enabled_set(view->navi_it,
	                                     (Eina_Bool)EINA_FALSE, false);
	elm_naviframe_item_pop_cb_set(view->navi_it,
	                              _mp_search_view_back_cb, view);

	/* update the first controlba item */
	/* mp_view_manager_update_first_controlbar_item(layout_data); */
	endfunc;
	return 0;
}

static Eina_Bool
_mp_search_view_update_list_timer_cb(void *data)
{
	eventfunc;
	MpSearchView_t *view = (MpSearchView_t *) data;
	MP_CHECK_FALSE(view);

	if (view->transition) {
		WARN_TRACE("transition");
		return EINA_TRUE;
	}

	view->content_set(view);

	DEBUG_TRACE("view->needle: %s", view->needle);
	/* if (!view->needle || !strlen(view->needle))
	mp_search_show_imf_pannel(view->search_bar); */

	view->search_timer = NULL;
	return EINA_FALSE;
}


static void
_mp_search_view_keyword_changed_cb(void *data,
                                   Evas_Object *obj, void *event_info)
{
	MpSearchView_t *view = (MpSearchView_t *) data;
	MP_CHECK(view);
	char *search_str = NULL;

	search_str = mp_search_text_get(view->search_bar);

	EVENT_TRACE("search_str: %s", search_str);
	if (search_str) {
		int length = strlen(search_str);
		if (length > 0) {
			elm_object_signal_emit(view->search_bar,
			                       "image,enable,1", "*");
		} else {
			elm_object_signal_emit(view->search_bar,
			                       "image,disable,1", "*");
		}
	}
	if (search_str) {
		if (view->needle) {
			free(view->needle);
		}
		view->needle = search_str;
		/* signal = "hide.screen"; */
	} else {
		if (view->needle) {
			free(view->needle);
		}
		/* signal = "show.screen"; */
	}
	/*when create search view, we use this first_called flag to load genlsit
	,then change key word, refresh the genlist*/
	if (view->first_called) {
		view->first_called = FALSE;
		return;
	} else {
		view->needle_change = TRUE;
	}
	mp_ecore_timer_del(view->search_timer);
	view->search_timer = ecore_timer_add(0.1,
	                                     _mp_search_view_update_list_timer_cb, view);
}


static void
_mp_search_view_create_search_bar(void *thiz)
{
	startfunc;
	MpSearchView_t *view = (MpSearchView_t *)thiz;
	MP_CHECK(view);
	MP_CHECK(view->search_base_layout);

	view->search_bar = mp_search_create_new(view->search_base_layout,
	                                        _mp_search_view_keyword_changed_cb, view, NULL, NULL,
	                                        NULL, view, NULL, view);
	MP_CHECK(view->search_base_layout);
	evas_object_show(mp_search_entry_get(view->search_bar));
	/*elm_object_focus_set(mp_search_entry_get(view->search_bar), TRUE);

	elm_object_signal_callback_add(view->search_view_layout,
	SIGNAL_MOUSE_CLICK, "elm.rect.screen", _mp_search_view_screen_
	clicked_cb, view); */
	endfunc;
}

static void _mp_search_view_content_load(void *thiz)
{
	startfunc;
	Evas_Object *layout = NULL;
	MpSearchView_t *view = (MpSearchView_t *)thiz;
	MP_CHECK(view);

	/* when keyword change, we hide items,
	do not need create genlist again */
	if (view->needle_change) {
		MpSearchList_t* list = (MpSearchList_t *)view->content_to_show;
		mp_search_list_set_data(list,
		                        MP_SEARCH_LIST_FILTER_STR, view->needle, -1);
		list->refresh(list);
	} else {
		Evas_Object *content = elm_object_part_content_unset(
		                           view->search_view_layout, "elm.swallow.content");
		evas_object_del(content);

		view->content_to_show = (MpList_t *)mp_search_list_create(
		                            view->layout);
		mp_search_list_set_data((MpSearchList_t *)view->content_to_show
		                        , MP_SEARCH_LIST_FILTER_STR, view->needle, -1);

		mp_list_update(view->content_to_show);
		layout = mp_list_get_layout(view->content_to_show);
		if (layout != NULL) {
			elm_object_part_content_set(view->search_view_layout,
			                            "elm.swallow.content", layout);
		}
	}
}

static void
_mp_search_view_on_event_cb(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpSearchView_t *view = thiz;
	MP_CHECK(view);

	switch (event) {
	case MP_VIEW_TRANSITION_FINISHED:
		view->transition = false;
		if ((int)mp_view_mgr_get_top_view(GET_VIEW_MGR) == (int)view) {
			elm_object_focus_allow_set(mp_search_entry_get(view->
			                           search_bar), EINA_TRUE);
			elm_object_focus_set(mp_search_entry_get(view->
			                     search_bar), EINA_TRUE);
		}
		break;
	case MP_SIP_STATE_CHANGED: {
		if ((int)mp_view_mgr_get_top_view(GET_VIEW_MGR) ==
		        (int)view) {
			struct appdata *ad = mp_util_get_appdata();
			MP_CHECK(ad);
			if (ad->sip_state) {
				elm_object_focus_allow_set(
				    mp_search_entry_get(view->search_bar),
				    EINA_TRUE);
			} else {
				elm_object_focus_allow_set(
				    mp_search_entry_get(view->search_bar),
				    EINA_FALSE);
			}
		}
	}
	break;
	default:
		break;
	}
}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_search_view_rotate_cb(void *thiz, int randscape)
{
	DEBUG_TRACE("Serach view rotated");
	MpSearchView_t *view = thiz;
	MP_CHECK(view);

	if (mp_util_get_sip_state() && (int)mp_view_mgr_get_top_view
	        (GET_VIEW_MGR) == (int)view) {
		_mp_search_view_on_event_cb(view, MP_SIP_STATE_CHANGED);
	}
}
#endif

static void
_mp_search_view_back_button_clicked(void *data, Evas_Object *o,
                                    const char *emission, const char *source)
{
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	MP_CHECK(view_mgr);
	elm_naviframe_item_pop(view_mgr->navi);
}

static int
_mp_search_view_init(Evas_Object *parent, MpSearchView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_list_view_init(parent, (MpListView_t *)view, MP_VIEW_SEARCH);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_search_view_update;
	view->update_options = _mp_search_view_update_options;
	view->update_options_edit = NULL;
	view->view_destroy_cb = _mp_search_view_destory_cb;
	view->content_set = _mp_search_view_content_load;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;
	view->start_playback = NULL;
	view->pause_playback = NULL;
	view->stop_playback = NULL;
	view->on_event = _mp_search_view_on_event_cb;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_search_view_rotate_cb;
#endif

	view->transition = true;
	Elm_Theme *th = elm_theme_new();
	elm_theme_extension_add(th, THEME_NAME);

	view->search_view_layout  = elm_layout_add(view->layout);
	MP_CHECK_VAL(view->search_view_layout, -1);

	double scale = elm_config_scale_get();
	if ((scale - 1.8) < 0.0001) {
		elm_layout_theme_set(view->search_view_layout,
		                     "layout", "application", "search_view_layout_wvga");
	} else if ((scale - 2.6) < 0.0001) {
		elm_layout_theme_set(view->search_view_layout,
		                     "layout", "application", "search_view_layout_hd");
	} else {
		elm_layout_theme_set(view->search_view_layout,
		                     "layout", "application", "search_view_layout_qhd");
	}
	elm_object_part_content_set(view->layout,
	                            "list_content", view->search_view_layout);

	/* search bar Base Layout */
	Elm_Theme *th1 = elm_theme_new();
	elm_theme_extension_add(th1, THEME_NAME);

	view->search_base_layout  = elm_layout_add(view->search_view_layout);
	elm_layout_theme_set(view->search_base_layout,
	                     "layout", "application", "searches");
	elm_object_part_content_set(view->search_view_layout,
	                            "search_bar", view->search_base_layout);

	_mp_search_view_create_search_bar(view);
	elm_object_part_content_set(view->search_base_layout,
	                            "searching", view->search_bar);

	edje_object_signal_callback_add(_EDJ(view->search_base_layout),
	                                "elm,action,click", "back_button",
	                                _mp_search_view_back_button_clicked, view);
	return ret;
}

MpSearchView_t *mp_search_view_create(Evas_Object *parent, const char *keyword)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpSearchView_t *view = calloc(1, sizeof(MpSearchView_t));
	MP_CHECK_NULL(view);

	ret = _mp_search_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	view->needle = g_strdup(keyword);

	view->first_called = TRUE;
	mp_search_text_set(view->search_bar, keyword);

	view->content_set(view);

	return view;

Error:
	ERROR_TRACE("Error: mp_search_view_create()");
	IF_FREE(view);
	return NULL;
}

void
mp_search_view_set_keyword(MpSearchView_t *view, const char *keyword)
{
	MP_CHECK(view);
	IF_FREE(view->needle);
	view->needle = g_strdup(keyword);
	mp_search_text_set(view->search_bar, keyword);
}

int mp_search_view_destory(MpSearchView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	IF_G_FREE(view->needle);
	return 0;
}


