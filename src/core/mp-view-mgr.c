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

#include "mp-view-mgr.h"
#include "music.h"
#include "mp-widget.h"
#include "mp-list-view.h"
#include "mp-player-mgr.h"
#include "mp-util.h"
#include "mp-common.h"
#include "mp-now-playing.h"
#include "mp-minicontroller.h"
#include "mp-lockscreenmini.h"
#include <efl_extension.h>

static MpViewMgr_t *g_view_mgr;
#define MP_PRINT_VIEW_STACK
/* #define MP_EVENT_BLOCKER */

#ifdef MP_PRINT_VIEW_STACK
static void _print_view_stack()
{
	Eina_List *list = elm_naviframe_items_get(g_view_mgr->navi);
	MP_CHECK(list);

	Eina_List *l = NULL;
	int i = 0;
	Elm_Object_Item *data = NULL;
	EINA_LIST_FOREACH(list, l, data) {
		MpView_t *view = elm_object_item_data_get(data);
		if (!view) {
			continue;
		}
		WARN_TRACE("view[0x%x], depth[%d], type[%d]", view, i, view->view_type);
		i++;
	}

	eina_list_free(list);
	list = NULL;
}
#endif

#ifdef MP_EVENT_BLOCKER

static Evas_Object *g_rect;
static Ecore_Timer *g_blocker_timer;

static Eina_Bool
_bloker_hide_timer(void *data)
{
	TIMER_TRACE();
	mp_evas_object_del(g_rect);
	g_blocker_timer = NULL;
	return EINA_FALSE;
}

static void
_mp_view_mgr_hide_event_blocker(void)
{
	mp_evas_object_del(g_rect);
	mp_ecore_timer_del(g_blocker_timer);
}

static void
_mp_view_mgr_show_event_blocker(void)
{
	int x, y, w, h;
	struct appdata *ad = mp_util_get_appdata();

	mp_ecore_timer_del(g_blocker_timer);

	evas_object_geometry_get(ad->conformant, &x, &y, &w, &h);

	if (!g_rect) {
		g_rect = evas_object_rectangle_add(evas_object_evas_get(ad->win_main));
	}

	evas_object_repeat_events_set(g_rect, 0);
	evas_object_color_set(g_rect, 0, 0, 0, 0);
	evas_object_resize(g_rect, w, h);
	evas_object_move(g_rect, x, y);
	evas_object_show(g_rect);

	g_blocker_timer = ecore_timer_add(0.5, _bloker_hide_timer, NULL);
}
#endif
void
_mp_view_mg_transition_finish_cb(void *data, Evas_Object * obj, void *event_info)
{
#ifdef MP_EVENT_BLOCKER
	_mp_view_mgr_hide_event_blocker();
#endif
	mp_view_mgr_post_event(data, MP_VIEW_TRANSITION_FINISHED);
}

static void
_mp_view_mgr_win_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *navi = data;
	Evas_Coord w, h, win_w, win_h;
	evas_object_geometry_get(obj, NULL, NULL, &win_w, &win_h);
	evas_object_geometry_get(navi, NULL, NULL, &w, &h);

	DEBUG_TRACE("window size: [%d]x[%d]", win_w, win_h);
	if (w != win_w) {
		DEBUG_TRACE("Resize naviframe");
		evas_object_resize(navi, win_w, h);
	}

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_WIN_RESIZED);
}

static void
_layout_del_cb(void *data , Evas *e, Evas_Object *obj, void *event_info)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	evas_object_event_callback_del(ad->win_main, EVAS_CALLBACK_RESIZE, _mp_view_mgr_win_resize_cb);
}

MpViewMgr_t *mp_view_mgr_create(Evas_Object *parent)
{
	startfunc;
	MP_CHECK_NULL(parent);
	MpViewMgr_t *view_mgr = calloc(1, sizeof(MpViewMgr_t));
	MP_CHECK_NULL(view_mgr);

	view_mgr->navi = mp_widget_navigation_new(parent);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	evas_object_event_callback_add(ad->win_main, EVAS_CALLBACK_RESIZE, _mp_view_mgr_win_resize_cb, view_mgr->navi);

	eext_object_event_callback_add(view_mgr->navi, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(view_mgr->navi, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	evas_object_event_callback_add(view_mgr->navi, EVAS_CALLBACK_DEL, _layout_del_cb, NULL);

	evas_object_smart_callback_add(view_mgr->navi , "transition,finished", _mp_view_mg_transition_finish_cb, view_mgr);
	/* evas_object_smart_callback_add(view_mgr->navi, "title,clicked", _title_clicked_cb, view_mgr); */

	g_view_mgr = view_mgr;
	return view_mgr;
}

int mp_view_mgr_destory(MpViewMgr_t *view_mgr)
{
	startfunc;
	MP_CHECK_VAL(view_mgr, -1);
	free(view_mgr);
	g_view_mgr = NULL;
	return 0;
}

EXPORT_API MpViewMgr_t *mp_view_mgr_get_view_manager()
{
	return g_view_mgr;
}

MpView_t *mp_view_mgr_get_top_view(MpViewMgr_t *view_mgr)
{
	MP_CHECK_NULL(view_mgr);
	Elm_Object_Item *navi_it = elm_naviframe_top_item_get(view_mgr->navi);
	MP_CHECK_NULL(navi_it);
	return elm_object_item_data_get(navi_it);
}

EXPORT_API MpView_t *mp_view_mgr_get_view(MpViewMgr_t *view_mgr, MpViewType_e type)
{
	MpView_t *view = NULL;
	MP_CHECK_NULL(view_mgr);
	Eina_List *list = elm_naviframe_items_get(view_mgr->navi);

	Eina_List *l = NULL;
	Elm_Object_Item *data = NULL;
	EINA_LIST_FOREACH(list, l, data) {
		view = elm_object_item_data_get(data);
		if (!view) {
			continue;
		}
		if (view->view_type == type) {
			break;
		}
		view = NULL;
	}
	eina_list_free(list);

	return view;
}

EXPORT_API MpView_t *mp_view_mgr_get_view_prev(MpViewMgr_t *view_mgr, MpViewType_e type)
{
	MpView_t *view = NULL;
	MP_CHECK_NULL(view_mgr);
	Eina_List *list = elm_naviframe_items_get(view_mgr->navi);

	Eina_List *l = NULL;
	Elm_Object_Item *data = NULL;
	MpView_t *prev_view = NULL;
	EINA_LIST_FOREACH(list, l, data) {
		view = elm_object_item_data_get(data);
		if (!view) {
			return NULL;
		}
		if (view->view_type == type) {
			break;
		}
		prev_view = view;
		view = NULL;
	}
	eina_list_free(list);

	return prev_view;
}

EXPORT_API int mp_view_mgr_push_view_with_effect(MpViewMgr_t *view_mgr, MpView_t *view, const char *item_style, bool disable_effect)
{
	startfunc;
	MP_CHECK_VAL(view_mgr, -1);
	MP_CHECK_VAL(view, -1);
	Elm_Object_Item *navi_it = NULL;

	Elm_Object_Item *last_item = NULL;
	last_item = elm_naviframe_top_item_get(view_mgr->navi);

#ifdef MP_EVENT_BLOCKER
	if (list && eina_list_count(list)) {
		_mp_view_mgr_show_event_blocker();
	}
#endif


	elm_naviframe_prev_btn_auto_pushed_set(view_mgr->navi, EINA_FALSE);
	/* if (item_style == NULL)
		item_style = MP_NAVI_ITEM_STYLE_TOPLINE; */

	bool request_transition_effect = false;
	Elm_Object_Item *after = elm_naviframe_top_item_get(view_mgr->navi);
	if (disable_effect && after) {
		navi_it = elm_naviframe_item_insert_after(view_mgr->navi, after, NULL, NULL, NULL, mp_view_get_base_obj(view), item_style);
	} else {
		navi_it = elm_naviframe_item_push(view_mgr->navi, NULL, NULL, NULL, mp_view_get_base_obj(view), item_style);
		if (navi_it != elm_naviframe_bottom_item_get(view_mgr->navi)) {
			request_transition_effect = true;
		}
	}

	view->navi_it = navi_it;
	elm_object_item_data_set(navi_it, view);

	if (request_transition_effect) {
		mp_view_mgr_post_event(view_mgr, MP_VIEW_TRANSITION_REQUESTED);
	}

	mp_view_view_resume(view);

	if (!request_transition_effect && last_item) {
		mp_view_view_pause(elm_object_item_data_get(last_item));
	}

	_print_view_stack();

#ifndef MP_SOUND_PLAYER
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, -1);
	if (ad->current_track_info) {
		mp_view_set_nowplaying(view);
		mp_view_update_nowplaying(view, true);
	}
#endif
	return 0;
}

EXPORT_API int mp_view_mgr_push_view(MpViewMgr_t *view_mgr, MpView_t *view, const char *item_style)
{
	return mp_view_mgr_push_view_with_effect(view_mgr, view, item_style, false);
}

static Eina_Bool _create_main_view_cb(void *data)
{
	startfunc;
	mp_common_create_initial_view(data, NULL, NULL);
	return FALSE;
}

/*this function is called in backkey callback. it should not be called in other case.
	use  elm_naviframe_item_pop() instead.. */
int mp_view_mgr_pop_view(MpViewMgr_t *view_mgr, bool pop_view)
{
	startfunc;
	MP_CHECK_VAL(view_mgr, -1);
	Eina_List *list = elm_naviframe_items_get(view_mgr->navi);

#ifdef MP_EVENT_BLOCKER
	if (eina_list_count(list) > 1) {
		_mp_view_mgr_show_event_blocker();
	}
#endif

	list = eina_list_last(list);
	list = eina_list_prev(list);
	if (!list) {
		DEBUG_TRACE("There is no previous view..");
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK_VAL(ad, -1);
		elm_win_lower(ad->win_main);
		ecore_idler_add(_create_main_view_cb, ad);
		goto END;
	}

	mp_view_view_resume(elm_object_item_data_get(list->data));
END:
	eina_list_free(list);
	return 0;
}

int mp_view_mgr_pop_a_view(MpViewMgr_t *view_mgr, MpView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view_mgr, -1);
	MP_CHECK_VAL(view, -1);

	if (mp_view_mgr_get_top_view(view_mgr) == view) {
		elm_naviframe_item_pop(view_mgr->navi);
	} else {
		elm_object_item_del(view->navi_it);
	}

	return 0;
}

int mp_view_mgr_pop_to_view(MpViewMgr_t *view_mgr, MpViewType_e type)
{
	MpView_t *view = NULL;
	MP_CHECK_VAL(view_mgr, -1);

	MpView_t *pop_to = mp_view_mgr_get_view(view_mgr, type);
	MP_CHECK_VAL(pop_to, -1);

	if (pop_to == mp_view_mgr_get_top_view(view_mgr)) {
		return 0;
	}
#ifdef MP_EVENT_BLOCKER
	_mp_view_mgr_show_event_blocker();
#endif
	Eina_List *list = elm_naviframe_items_get(view_mgr->navi);
	Eina_List *l = NULL;
	Elm_Object_Item *data = NULL;
	EINA_LIST_FOREACH(list, l, data) {
		view = elm_object_item_data_get(data);
		if (view == pop_to) {
			break;
		}
	}

	if (l) {
		mp_view_view_resume(elm_object_item_data_get(l->data));
	}

	eina_list_free(list);
	MP_CHECK_VAL(view, -1);

	elm_naviframe_item_pop_to(view->navi_it);

	return 0;
}

int mp_view_mgr_delete_view(MpViewMgr_t *view_mgr, MpViewType_e type)
{
	MP_CHECK_VAL(view_mgr, -1);

	MpView_t *view = mp_view_mgr_get_view(view_mgr, type);
	MP_CHECK_VAL(view, -1);

	bool need_to_resume = false;
	MpView_t *top_view = mp_view_mgr_get_top_view(view_mgr);
	if (top_view == view) {
		need_to_resume = true;
	}

	elm_object_item_del(view->navi_it);
	view = NULL;
	top_view = NULL;

	if (need_to_resume) {
		top_view = mp_view_mgr_get_top_view(view_mgr);
		mp_view_view_resume(top_view);
	}

	return 0;
}

int mp_view_mgr_count_view(MpViewMgr_t *view_mgr)
{
	MP_CHECK_VAL(view_mgr, 0);
	Eina_List *list = elm_naviframe_items_get(view_mgr->navi);
	int count = eina_list_count(list);
	eina_list_free(list);
	return count;
}

static void _view_foreach_cb(void *data, void *user_data)
{
	MpView_t *view = data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpViewEvent_e event = (int)user_data;

	if (ad->is_lcd_off) {
		switch (event) {
		case MP_UPDATE_NOW_PLAYING:
		case MP_START_PLAYBACK:
		case MP_RESUME_PLAYBACK:
		case MP_PAUSE_PLAYBACK:
		case MP_STOP_PLAYBACK:
		case MP_UPDATE_PLAYING_LIST:
		case MP_UPDATE:
			DEBUG_TRACE("Lcd off event exit:%d", event);
			return;
		default:
			break;
		}
	}

	switch (event) {
	case MP_PLAYING_TRACK_CHANGED:
		mp_view_update_nowplaying(view, true);
		mp_view_on_event(view, event);
		break;
	case MP_UPDATE_NOW_PLAYING:
		mp_view_update_nowplaying(view, true);
		break;
	case MP_UNSET_NOW_PLAYING:
		if ((view->view_type == MP_VIEW_DETAIL) || (view->view_type == MP_VIEW_PLAYER)) { /*if detail view, it need to do view pop*/
			mp_view_on_event(view, event);
		} else {
			mp_view_unset_nowplaying(view);
		}
		break;
	case MP_START_PLAYBACK:
		mp_view_update_nowplaying(view, true);
		mp_view_start_playback(view);
		mp_view_on_event(view, event);
		break;
	case MP_RESUME_PLAYBACK:
		mp_view_update_nowplaying(view, false);
		mp_view_start_playback(view);
		mp_view_on_event(view, event);
		break;
	case MP_PAUSE_PLAYBACK:
		mp_view_update_nowplaying(view, false);
		mp_view_pause_playback(view);
		mp_view_on_event(view, event);
		break;
	case MP_STOP_PLAYBACK:
		mp_view_update_nowplaying(view, true);
		mp_view_stop_playback(view);
		mp_view_on_event(view, event);
		break;
	case MP_DOUBLE_TAP:
#ifndef MP_SOUND_PLAYER
		mp_list_view_double_tap((MpListView_t *)view);
#endif
		break;
	case MP_LCD_OFF:
		mp_view_view_lcd_off(view);
		break;
	case MP_LCD_ON:
		mp_view_view_lcd_on(view);
		break;
	case MP_UPDATE:
		mp_view_update(view);
		break;
#ifdef MP_FEATURE_LANDSCAPE
	case MP_VIEW_ROTATE:
		mp_view_rotate(view);
		break;
#endif
	case MP_DB_UPDATED:
		if ((view->view_type == MP_VIEW_ADD_TRACK) || (view->view_type == MP_VIEW_SET_AS)) {
			mp_view_on_event(view, event);
		} else {
			mp_view_update(view);
		}
		break;
	case MP_DELETE_DONE:
		if (view->nowplaying_bar && ad && ad->playlist_mgr) {
			if (mp_playlist_mgr_count(ad->playlist_mgr) == 0) {
				mp_view_unset_nowplaying(view);
				if (ad && ad->current_track_info) {
					mp_util_free_track_info(ad->current_track_info);
					ad->current_track_info = NULL;
				}
				if (ad->b_minicontroller_show) {
					mp_minicontroller_hide(ad);
				}
#ifdef MP_FEATURE_LOCKSCREEN
				if (ad->b_lockmini_show) {
					mp_lockscreenmini_hide(ad);
				}
#endif
			}
		}
		mp_view_on_event(view, event);
		break;
	case MP_POPUP_DELETE_DONE: {
		/*check if to unset now-playing*/
		int count = 0;
		mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);

		if (count == 0) {
			mp_view_unset_nowplaying(view);
			if (ad && ad->current_track_info) {
				mp_util_free_track_info(ad->current_track_info);
				ad->current_track_info = NULL;
			}
			if (ad->b_minicontroller_show) {
				mp_minicontroller_hide(ad);
			}
#ifdef MP_FEATURE_LOCKSCREEN
			if (ad->b_lockmini_show) {
				mp_lockscreenmini_hide(ad);
			}
#endif
		}
		mp_view_on_event(view, event);
		break;
	}
#ifndef MP_SOUND_PLAYER
	case MP_WIN_RESIZED: {
		if (view->nowplaying_bar) {
			mp_now_playing_set_layout(view->nowplaying_bar);
		}
		mp_view_on_event(view, event);
		break;
	}
#endif
	default:
		mp_view_on_event(view, event);
		break;
	}

}

void mp_view_mgr_post_event(MpViewMgr_t *view_mgr, MpViewEvent_e event)
{
	MP_CHECK(view_mgr);

	Eina_List *list = elm_naviframe_items_get(view_mgr->navi);
	Eina_List *l = NULL;
	Elm_Object_Item *data = NULL;
	MP_CHECK(list);

	l = eina_list_last(list);

	if (event == MP_SIP_STATE_CHANGED) {
		/* top view only */
		_view_foreach_cb(elm_object_item_data_get(l->data), (void *)event);
	} else if (event == MP_VIEW_TRANSITION_REQUESTED) {
		/* top view only */
		MpView_t *view = elm_object_item_data_get(l->data);
		if (view) {
			view->push_transition = true;
			_view_foreach_cb(view, (void *)event);
		}
	} else if (event == MP_VIEW_TRANSITION_FINISHED) {
		/* top view only */
		MpView_t *view = elm_object_item_data_get(l->data);
		bool pushed = false;
		if (view) {
			pushed = view->push_transition;
			view->push_transition = false;
			_view_foreach_cb(view, (void *)event);
		}

		l = l->prev;
		if (pushed && l) {
			MpView_t *second_view = elm_object_item_data_get(l->data);
			mp_view_view_pause(second_view);
		}
	} else {
		EINA_LIST_FOREACH(list, l, data) {
			if (data) {
				_view_foreach_cb(elm_object_item_data_get(data), (void *)event);
			}
		}
	}

	eina_list_free(list);
}


