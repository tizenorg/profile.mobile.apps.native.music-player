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

#include "mp-view.h"
#include "music.h"
#include "mp-common.h"
#include "mp-widget.h"
#include "mp-now-playing.h"
#include "mp-player-view.h"
#include "mp-util.h"
#include "mp-player-control.h"
#include "mp-player-mgr.h"
#include "mp-list-view.h"

#define CHECK_VIEW(view, val)	\
	do {\
		MP_CHECK_VAL(view, val);\
		mp_retvm_if (view->view_magic != VIEW_MAGIC, val,\
		             "Error: param is not view object!!!magic: %d", view->view_magic);\
	} while (0);


#ifndef MP_SOUND_PLAYER
static void _mp_view_show_now_playing(void *thiz, int show);
#endif

static int _mp_view_set_title(void *thiz, char *text_id)
{
	startfunc;
#ifdef MP_CREATE_FAKE_IMAGE
	return 0;
#endif
	MpView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	mp_util_item_domain_translatable_part_text_set(view->navi_it, "elm.text.title", (const char *)text_id);

	if (!view->disable_title_icon && view->layout) {
		Evas_Object *title_icon = mp_widget_create_title_icon(view->layout, IMAGE_EDJ_NAME, MP_ICON_APP_MUSIC);
		elm_object_item_part_content_set(view->navi_it, "icon", title_icon);
	}

	return 0;
}

static int _mp_view_set_sub_title(void *thiz, char *title)
{
	startfunc;
	MpView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	/*Evas_Object *label = _get_sub_title_label(view->layout, title);
	elm_object_style_set(label, "music/naviframe_subtitle");*/
	elm_object_item_part_text_set(view->navi_it, "subtitle", title);

	return 0;
}

static int _mp_view_title_slide_go(void *thiz)
{
	startfunc;
	MpView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	Elm_Object_Item *nf_it = view->navi_it;
	Evas_Object *label = elm_object_item_part_content_get(nf_it, "elm.swallow.title");
	if (label) {
		elm_label_slide_go(label);
	}

	label = elm_object_item_part_content_get(nf_it, "elm.swallow.subtitle");
	if (label) {
		elm_label_slide_go(label);
	}

	return 0;
}

#ifndef MP_SOUND_PLAYER
static void _mp_view_now_playing_cb(void *data)
{
	startfunc;
	MpViewMgr_t *view_mgr = GET_VIEW_MGR;
	MpView_t *player_view = mp_view_mgr_get_view(view_mgr, MP_VIEW_PLAYER);
	if (player_view) {
		mp_view_mgr_pop_to_view(view_mgr, MP_VIEW_PLAYER);
	} else {
		mp_common_show_player_view(MP_PLAYER_NORMAL, false, false, true);
	}
}

static void _mp_view_now_playing_play_pause_cb(void *data)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (ad->player_state == PLAY_STATE_PLAYING) {
		mp_play_control_play_pause(ad, false);
	} else {
		mp_play_control_play_pause(ad, true);
	}
}

static int _mp_view_update_nowplaying(void *thiz, bool with_title)
{
	startfunc;
	char *thumbpath = NULL;
	MpView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, -1);

	mp_track_info_t *info = ad->current_track_info;
	MP_CHECK_VAL(info, -1);

	if (mp_util_is_image_valid(ad->evas, info->thumbnail_path)) {
		thumbpath = info->thumbnail_path;
	} else {
		char default_thumbnail[1024] = {0};
		char *shared_path = app_get_shared_resource_path();
		snprintf(default_thumbnail, 1024, "%s%s/%s", shared_path, "shared_images", DEFAULT_THUMBNAIL);
		free(shared_path);
		thumbpath = g_strdup(default_thumbnail);
	}

	if (!view->nowplaying_bar) {
		mp_view_set_nowplaying(thiz);
	}

	mp_now_playing_update(view->nowplaying_bar, info->title, info->artist, thumbpath, with_title);

	return 0;
}

static int _mp_view_freeze_nowplaying(void *thiz, int freeze)
{
	startfunc;
	MpView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	if (count <= 0) {
		DEBUG_TRACE("no songs");
		_mp_view_show_now_playing(thiz, FALSE);
		mp_now_playing_freeze_timer(view->nowplaying_bar);
		mp_evas_object_del(view->nowplaying_bar);
		return 0;
	}

	if (view->nowplaying_bar) {
		if (freeze) {
			mp_now_playing_freeze_timer(view->nowplaying_bar);
		} else {
			mp_now_playing_thaw_timer(view->nowplaying_bar);
		}
	}

	return 0;
}

static void _mp_view_show_now_playing(void *thiz, int show)
{
	MpView_t *view = thiz;
	MP_CHECK(view);
	MP_CHECK(view->layout);
	MP_CHECK(view->nowplaying_bar);
	if (show) {
#ifdef MP_FEATURE_LANDSCAPE
		bool landscape = mp_util_is_landscape();
		if (landscape) {
			edje_object_signal_emit(_EDJ(view->layout), "LANDSCAPE_SHOW_NOW_PLAING", "music_app");
		} else
#endif
			edje_object_signal_emit(_EDJ(view->layout), "SHOW_NOW_PLAING", "music_app");
	} else {
		edje_object_signal_emit(_EDJ(view->layout), "HIDE_NOW_PLAING", "music_app");
	}
	evas_object_data_set(view->nowplaying_bar, "NOW_PLAYING_SHOW_FLAG", (void *)show);
}

static int _mp_view_set_nowplaying(void *thiz)
{
	startfunc;
	MpView_t *view = thiz;
#ifndef MP_SOUND_PLAYER
	bool  val = false;
	mp_list_view_is_list_view((MpListView_t *)view, &val);

	MpList_t *list = ((MpListView_t *)view)->content_to_show;
	if (val && mp_list_get_edit(list) && mp_list_get_reorder(list)) {
		return 0;
	}

	struct appdata *ad = mp_util_get_appdata();
	if (!ad->current_track_info) {
		return 0;
	}

#endif
#ifdef MP_FEATURE_LANDSCAPE
	if (mp_view_is_rotate_available(view)) {
		view->rotate_flag = TRUE;
		if (!view->nowplaying_bar || mp_util_is_landscape() != mp_now_playing_is_landscape(view->nowplaying_bar)) {
			view->nowplaying_bar = mp_now_playing_create(view->layout, _mp_view_now_playing_play_pause_cb, _mp_view_now_playing_cb, view);
			elm_object_part_content_set(view->layout, "now_playing", view->nowplaying_bar);
		}
		mp_view_update_nowplaying(view, true);
	} else
#endif
		if (!view->nowplaying_bar) {
			view->rotate_flag = FALSE;
			view->nowplaying_bar = mp_now_playing_create(view->layout, _mp_view_now_playing_play_pause_cb, _mp_view_now_playing_cb, view);
			elm_object_part_content_set(view->layout, "now_playing", view->nowplaying_bar);
			mp_view_update_nowplaying(view, true);
		} else {
			mp_view_update_nowplaying(view, true);
		}
	_mp_view_show_now_playing(thiz, TRUE);

	if (view == mp_view_mgr_get_top_view(GET_VIEW_MGR)) {
		mp_view_freeze_nowplaying(view, 0);
	}

	return 0;
}

static int _mp_view_unset_nowplaying(void *thiz)
{
	startfunc;
	MpView_t *view = thiz;

	_mp_view_show_now_playing(thiz, FALSE);
	mp_view_freeze_nowplaying(view, 1);
	mp_evas_object_del(view->nowplaying_bar);
	return 0;
}

static int _mp_view_start_playback(void *thiz)
{
	startfunc;
	MpView_t *view = thiz;

	if (view == mp_view_mgr_get_top_view(GET_VIEW_MGR)) {
		mp_view_freeze_nowplaying(view, 0);
	}

	return 0;
}

static int _mp_view_pause_playback(void *thiz)
{
	startfunc;
	MpView_t *view = thiz;
	mp_view_freeze_nowplaying(view, 1);

	return 0;
}

static int _mp_view_stop_playback(void *thiz)
{
	startfunc;
	MpView_t *view = thiz;
	mp_view_freeze_nowplaying(view, 1);

	return 0;
}

static int _mp_view_clear_options(void *thiz)
{
	startfunc;
	MpView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	int i = 0;
	for (i = 0; i < MP_OPTION_MAX; i++) {
		if (view->toolbar_options[i]) {
			elm_object_item_del(view->toolbar_options[i]);
			view->toolbar_options[i] = NULL;
		}
	}

	Evas_Object *btn = NULL;
	btn = elm_object_item_part_content_unset(view->navi_it, "toolbar_button1");
	mp_evas_object_del(btn);
	btn = elm_object_item_part_content_unset(view->navi_it, "toolbar_button2");
	mp_evas_object_del(btn);
	btn = elm_object_item_part_content_unset(view->navi_it, "toolbar_more_btn");
	mp_evas_object_del(btn);

	Evas_Object *toolbar = elm_object_item_part_content_unset(view->navi_it, "toolbar");
	mp_evas_object_del(toolbar);

	endfunc
	return 0;
}
#endif

static void
_mp_view_layout_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	MpView_t *view = data;
	MP_CHECK(view);
	MP_CHECK(view->view_destroy_cb);
	view->view_destroy_cb(view);
}

static void _mp_view_view_lcd_off(void *thiz)
{
	startfunc;

#ifndef MP_SOUND_PLAYER
	MpView_t *view = thiz;
	mp_view_freeze_nowplaying(view, 1);
#endif
}

static void _mp_view_view_lcd_on(void *thiz)
{
	startfunc;

#ifndef MP_SOUND_PLAYER
	MpView_t *view = thiz;
	if (mp_view_mgr_get_top_view(GET_VIEW_MGR) == view) {
		mp_view_freeze_nowplaying(view, 0);
	}
	mp_view_update_nowplaying(view, true);
#endif

}

static void _mp_view_view_pause(void *thiz)
{
	startfunc;
#ifndef MP_SOUND_PLAYER
	MpView_t *view = thiz;
	mp_view_freeze_nowplaying(view, 1);
	mp_evas_object_del(view->more_btn_ctxpopup);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_longpress_popup_destroy(ad);
#endif
}

static void _mp_view_view_resume(void *thiz)
{
	startfunc;
#ifndef MP_SOUND_PLAYER
	MpView_t *view = thiz;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	if (mp_playlist_mgr_get_current(ad->playlist_mgr) != NULL) {
		mp_view_set_nowplaying(view);
	}

	mp_view_freeze_nowplaying(view, 0);
#endif
}

static void
_mp_view_on_event(void *thiz, MpViewEvent_e event)
{
	startfunc;
#ifdef MP_FEATURE_LANDSCAPE
	MpView_t *view = (MpView_t *)thiz;
#endif
	DEBUG_TRACE("event; %d", event);
	switch (event) {
#ifdef MP_FEATURE_LANDSCAPE
	case MP_VIEW_ROTATE:
		if (view->view_type != MP_VIEW_EDIT) {
			mp_view_update_options(thiz);
		}
		break;
#endif
	default:
		break;
	}

	endfunc;
}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_view_rotate_cb(void *thiz, int randscape)
{
	startfunc;

	DEBUG_TRACE("_mp_view_rotate_cb rotated");
	MpView_t *view = thiz;
	MP_CHECK(view);

	_mp_view_on_event(view, MP_VIEW_ROTATE);

	endfunc;
}
#endif

int mp_view_init(Evas_Object *parent, MpView_t *view, MpViewType_e view_type, ...)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	view->view_magic = VIEW_MAGIC;

	view->view_type = view_type;
	view->rotate_flag = FALSE;

	view->set_title = _mp_view_set_title;
	view->set_subtitle = _mp_view_set_sub_title;
	view->title_slide_go = _mp_view_title_slide_go;
#ifndef MP_SOUND_PLAYER
	view->set_nowplaying = _mp_view_set_nowplaying;
	view->unset_nowplaying = _mp_view_unset_nowplaying;
	view->update_nowplaying = _mp_view_update_nowplaying;
	view->freeze_nowplaying = _mp_view_freeze_nowplaying;
	view->start_playback = _mp_view_start_playback;
	view->pause_playback = _mp_view_pause_playback;
	view->stop_playback = _mp_view_stop_playback;
	view->clear_options = _mp_view_clear_options;
#endif
	view->view_lcd_off = _mp_view_view_lcd_off;
	view->view_lcd_on = _mp_view_view_lcd_on;
	view->view_pause = _mp_view_view_pause;
	view->view_resume = _mp_view_view_resume;

#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_view_rotate_cb;
#endif
	view->on_event = _mp_view_on_event;

	view->layout = mp_common_load_edj(parent, MP_EDJ_NAME, "main_layout");

#ifdef MP_FEATURE_MULTIWINDOW
	if (!view->disable_scroller) {
		view->scroller = elm_scroller_add(parent);
		evas_object_size_hint_weight_set(view->scroller, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(view->scroller, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_object_content_set(view->scroller, view->layout);
		elm_scroller_bounce_set(view->scroller, EINA_FALSE, EINA_FALSE);
		evas_object_show(view->scroller);
	}
#endif
	MP_CHECK_VAL(view->layout, -1);

	evas_object_data_set(view->layout, "view_data", view);

	evas_object_event_callback_add(view->layout, EVAS_CALLBACK_FREE, _mp_view_layout_del_cb,
	                               view);

	if (view_type == MP_VIEW_PLAYER
	        || view_type == MP_VIEW_SEARCH
	        || view_type == MP_VIEW_EDIT
	        || view_type == MP_VIEW_ADD_TRACK
	        || view_type == MP_VIEW_SELECT_TRACK
	        || view_type == MP_VIEW_MAKE_OFFLINE) {
		view->disable_title_icon = true;
	}

	return 0;
}

int mp_view_fini(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	view->view_magic = 0;

	return 0;
}

EXPORT_API int mp_view_update(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->update, -1);
	view->update(view);
#ifndef MP_SOUND_PLAYER
	bool val = false;
	mp_list_view_is_list_view((MpListView_t *)view, &val);

	if (val && mp_list_get_edit(((MpListView_t *)view)->content_to_show)) {
		mp_view_update_options_edit(view);
	} else
#endif
		mp_view_update_options(view);

	return 0;
}

EXPORT_API int mp_view_update_options(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->update_options, -1);

	int ret = view->update_options(view);
	endfunc;
	return ret;
}

int mp_view_update_options_edit(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->update_options_edit, -1);

	mp_view_unset_nowplaying(view);

	int ret = view->update_options_edit(view);

	return ret;
}

int mp_view_clear_options(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->clear_options, -1);
	return view->clear_options(view);
}

Evas_Object *mp_view_get_base_obj(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, NULL);

	if (view->scroller) {
		return view->scroller;
	} else {
		return view->layout;
	}

}

EXPORT_API int mp_view_set_title(MpView_t *view, char *title)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->set_title, -1);
	return view->set_title(view, title);
}

int mp_view_set_sub_title(MpView_t *view, char *title)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->set_subtitle, -1);
	return view->set_subtitle(view, title);
}

int mp_view_set_title_visible(MpView_t *view, int visible)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->navi_it, -1);
	elm_naviframe_item_title_enabled_set(view->navi_it, (Eina_Bool)visible, false);
	return 0;
}

int mp_view_title_slide_go(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->navi_it, -1);
	return view->title_slide_go(view);

}

int mp_view_set_nowplaying(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	if (view->set_nowplaying) {
		view->set_nowplaying(view);
	}
	return 0;
}

int mp_view_unset_nowplaying(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	if (view->unset_nowplaying) {
		view->unset_nowplaying(view);
	}
	return 0;
}

int mp_view_update_nowplaying(MpView_t *view, bool with_title)
{
	startfunc;
	CHECK_VIEW(view, -1);
	if (view->update_nowplaying) {
		view->update_nowplaying(view, with_title);
	}
	return 0;
}

int mp_view_freeze_nowplaying(MpView_t *view, int freeze)
{
	startfunc;
	CHECK_VIEW(view, -1);
	if (view->freeze_nowplaying) {
		view->freeze_nowplaying(view, freeze);
	}
	return 0;
}

int mp_view_get_nowplaying_show_flag(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, 0);
	MP_CHECK_VAL(view->update_nowplaying, 0);
	return ((int)evas_object_data_get(view->nowplaying_bar, "NOW_PLAYING_SHOW_FLAG"));
}

int mp_view_start_playback(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->start_playback, -1);
	return view->start_playback(view);
}

int mp_view_pause_playback(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->pause_playback, -1);
	return view->pause_playback(view);
}

int mp_view_stop_playback(MpView_t *view)
{
	startfunc;
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->stop_playback, -1);
	return view->stop_playback(view);
}

int mp_view_view_lcd_off(MpView_t *view)
{
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->view_lcd_off, -1);
	view->view_lcd_off(view);
	return 0;
}

int mp_view_view_lcd_on(MpView_t *view)
{
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(view->view_lcd_on, -1);
	view->view_lcd_on(view);
	mp_view_update_nowplaying(view, true);
	return 0;
}

int mp_view_view_pause(MpView_t *view)
{
	CHECK_VIEW(view, -1);
	view->paused = true;
	MP_CHECK_VAL(view->view_pause, -1);
	view->view_pause(view);
	return 0;
}

int mp_view_view_resume(MpView_t *view)
{
	CHECK_VIEW(view, -1);
	view->paused = false;
	MP_CHECK_VAL(view->view_resume, -1);
	view->view_resume(view);
	return 0;
}

EXPORT_API int mp_view_on_event(MpView_t *view, MpViewEvent_e event)
{
	CHECK_VIEW(view, -1);

	if (view->on_event) {
		view->on_event(view, event);
	}
	return 0;
}
#ifdef MP_FEATURE_LANDSCAPE
int mp_view_is_rotate_available(MpView_t *view)
{
	CHECK_VIEW(view, 0);
	if (view->is_rotate_available) {
		return view->is_rotate_available(view);
	}

	return 1;
}

int mp_view_rotate(MpView_t *view)
{
	CHECK_VIEW(view, -1);
	int landscape = (int)mp_util_is_landscape();
	if (view->rotate) {
		view->rotate(view, landscape);
	}
#ifndef MP_SOUND_PLAYER
	if (view->nowplaying_bar) {
		if (mp_now_playing_is_landscape(view->nowplaying_bar) != landscape) {
			_mp_view_set_nowplaying(view);
		}
	}
#endif
	return 0;
}

int mp_view_is_now_push_transit(MpView_t *view, bool *now_transit)
{
	CHECK_VIEW(view, -1);
	MP_CHECK_VAL(now_transit, -1);

	*now_transit = view->push_transition;

	return 0;
}

#endif
