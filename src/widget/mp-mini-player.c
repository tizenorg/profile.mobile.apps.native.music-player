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


#ifdef MP_FEATURE_APP_IN_APP

#include <Elementary.h>
#include <Ecore.h>

#include "mp-mini-player.h"
#include "mp-player-debug.h"
#include "mp-common.h"
#include "mp-widget.h"
#include "mp-app.h"
#include "mp-player-control.h"
#include "mp-util.h"
#include "mp-mw-lyric-view.h"

#define CTR_EDJ_SIG_SRC "ctrl_edj"

#define BASE_W				720
#define BASE_H				1280

#define MINI_PLAYER_W		362
#define MINI_PLAYER_H		177
#define MINI_PLAYER_X		(BASE_W-MINI_PLAYER_W-12)
#define MINI_PLAYER_Y		85

#define MINI_ALBUM_ART_SIZE0_W	90
#define MINI_ALBUM_ART_SIZE0_H	90

#define MINI_ALBUM_ART_SIZE1_W	348
#define MINI_ALBUM_ART_SIZE1_H	188

#define MINI_ALBUM_ART_SIZE2_W	706
#define MINI_ALBUM_ART_SIZE2_H	546


#define ALBUMART_PART_NAME		"albumart"
#define THUMB_NAIL_PART_NAME		"thumb_nail"

static Evas_Coord_Rectangle nLastRect = {0,};
static bool MINI_CALLED = false;

static bool _mp_mini_player_view_show_lyric(void *data, bool show_lyric);


static Eina_Bool
_mp_mini_player_delay_play_timer_cb(void *data)
{
	TIMER_TRACE();
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_FALSE(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "set_play", "*");

	mw_view->play_delay_timer = NULL;

	return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_mp_mini_player_switch_timer_cb(void *data)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_FALSE(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	Evas_Coord_Rectangle nCurRect = {0,};
	evas_object_geometry_get(mw_view->win_mini, &nCurRect.x, &nCurRect.y, &nCurRect.w, &nCurRect.h);

	if (nLastRect.w != nCurRect.w || nLastRect.h != nCurRect.h) {
		mp_mini_player_mode_set(mw_view, 3);
		nLastRect.w = nCurRect.w;
		nLastRect.h = nCurRect.h;
	}

	return ECORE_CALLBACK_RENEW;
}

static void
_mp_mini_player_resize_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MP_CHECK(emission);

	Evas_Coord_Rectangle nCurRect = {0,};
	evas_object_geometry_get(mw_view->win_mini, &nCurRect.x, &nCurRect.y, &nCurRect.w, &nCurRect.h);
	int x = 0;
	int y = 0;

	if (!g_strcmp0(emission, "sig_size_bt_btn_down")) {
//              ecore_x_pointer_last_xy_get(&x, &y);
//              ecore_x_mouse_up_send(mw_view->xwin , x, y, 1);
//              ecore_x_pointer_ungrab();
		mp_mini_player_window_drag_resize(mw_view, x, y, 1);
		edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "bg_edit_show", "*");
	}

	if (mw_view->switch_timer) {
		MP_TIMER_THAW(mw_view->switch_timer);
	} else {
		mw_view->switch_timer = ecore_timer_add(0.5, _mp_mini_player_switch_timer_cb, mw_view);
	}
	nLastRect.w = nCurRect.w;
	nLastRect.h = nCurRect.h;
}


static void
_mp_mini_player_close_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (mw_view->switch_timer) {
		ecore_timer_del(mw_view->switch_timer);
		mw_view->switch_timer = NULL;
	}
	evas_object_hide(mw_view->win_mini);
	MINI_CALLED = false;
}

static void
_mp_mini_player_full_screen_click_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	mp_mini_player_hide(mw_view);
}


static void
_mp_mini_player_contrl_btn_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MP_CHECK(emission);

	if (!g_strcmp0(emission, "sig_rew_btn_down")) {
		mp_play_control_rew(true, false, true);
	} else if (!g_strcmp0(emission, "sig_rew_btn_up")) {
		mp_play_control_rew(false, false, true);
		mp_mini_player_refresh(mw_view);
	} else if (!g_strcmp0(emission, "sig_ff_btn_down")) {
		mp_play_control_ff(true, false, true);
	} else if (!g_strcmp0(emission, "sig_ff_btn_up")) {
		mp_play_control_ff(false, false, true);
		mp_mini_player_refresh(mw_view);
	} else if (!g_strcmp0(emission, "sig_play_pause_btn_clicked")) {
		if (ad->player_state == PLAY_STATE_PLAYING) {
			mp_play_control_play_pause(ad, false);
			edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "set_pause", "*");
		} else if (ad->player_state == PLAY_STATE_PAUSED) {
			mp_play_control_play_pause(ad, true);
			edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "set_play", "*");
		}

	}
}

static void
_mp_mini_player_eventbox_flick_click_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_FALSE(mw_view);
	MP_CHECK_FALSE(mw_view->mini_player_view_layout);

	struct appdata *ad = mp_util_get_appdata();;
	MP_CHECK(ad);

	_mp_mini_player_view_show_lyric(mw_view, !ad->b_show_lyric);
}


static void
_mp_mini_player_eventbox_flick_left_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	Evas_Object *layout = NULL;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_play_control_ff(false, false, true);
	mp_mini_player_refresh(mw_view);
}

static void
_mp_mini_player_eventbox_flick_right_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_play_control_rew(false, false true);
	mp_mini_player_refresh(mw_view);
}

static void
_mp_mini_player_title_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	int x, y;
//        ecore_x_pointer_last_xy_get(&x, &y);
//        ecore_x_mouse_up_send(mw_view->xwin, x, y, 1);
//        ecore_x_pointer_ungrab();
	mp_mini_player_window_drag_start(mw_view, x, y, 1);
}

Evas_Object*
_mp_mini_player_layout_add(Evas_Object *parent, void *data, int sizenum)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	int w = 0;
	int h = 0;
	Evas_Object *albumart = NULL;

	double scale = elm_config_scale_get();

	if (WINDOW_SIZE_0 == sizenum) {
		mw_view->mini_player_view_layout = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, "mini-app-0");
		mw_view->mini_player_current_size = WINDOW_SIZE_0;
	} else if (WINDOW_SIZE_1 == sizenum) {
		mw_view->mini_player_view_layout = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, "mini-app-1");
		mw_view->mini_player_current_size = WINDOW_SIZE_1;
	} else if (WINDOW_SIZE_2 == sizenum) {
		mw_view->mini_player_view_layout = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, "mini-app-2");
		MP_CHECK_NULL(mw_view->mini_player_view_layout);
		w = MINI_ALBUM_ART_SIZE0_W * scale;
		h = MINI_ALBUM_ART_SIZE0_W * scale;
		mw_view->mini_player_current_size = WINDOW_SIZE_2;
		albumart = elm_image_add(mw_view->mini_player_view_layout);
		evas_object_size_hint_align_set(albumart, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(albumart, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(mw_view->mini_player_view_layout, THUMB_NAIL_PART_NAME, albumart);
	} else if (WINDOW_SIZE_3 == sizenum) {
		mw_view->mini_player_current_size = WINDOW_SIZE_3;
		mw_view->mini_player_view_layout = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, "mini-app-3");
		MP_CHECK_NULL(mw_view->mini_player_view_layout);
		albumart = elm_image_add(mw_view->mini_player_view_layout);
		evas_object_size_hint_align_set(albumart, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(albumart, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(mw_view->mini_player_view_layout, ALBUMART_PART_NAME, albumart);

	} else if (WINDOW_SIZE_4 == sizenum) {
		mw_view->mini_player_current_size = WINDOW_SIZE_4;
		mw_view->mini_player_view_layout = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, "mini-app-4");
		MP_CHECK_NULL(mw_view->mini_player_view_layout);
		albumart = elm_image_add(mw_view->mini_player_view_layout);
		evas_object_size_hint_align_set(albumart, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(albumart, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(mw_view->mini_player_view_layout, ALBUMART_PART_NAME, albumart);
	}

	MP_CHECK_NULL(mw_view->mini_player_view_layout);
	evas_object_size_hint_weight_set(mw_view->mini_player_view_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(mw_view->mini_player_view_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	Evas_Object *edj = _EDJ(mw_view->mini_player_view_layout);
	edje_object_signal_callback_add(edj, "sig_exit_btn_clicked", "*", _mp_mini_player_close_btn_clicked_cb, mw_view);
	edje_object_signal_callback_add(edj, "sig_full_screen_up", "*", _mp_mini_player_full_screen_click_cb, mw_view);
	edje_object_signal_callback_add(edj, "sig_*_btn_down", "*", _mp_mini_player_contrl_btn_clicked_cb, mw_view);
	edje_object_signal_callback_add(edj, "sig_*_btn_up", "*", _mp_mini_player_contrl_btn_clicked_cb, mw_view);
	edje_object_signal_callback_add(edj, "sig_play_pause_btn_clicked", "*", _mp_mini_player_contrl_btn_clicked_cb, mw_view);
	edje_object_signal_callback_add(edj, "sig_size_bt_btn_down", "*", _mp_mini_player_resize_btn_clicked_cb, mw_view);

	mw_view->title = elm_label_add(mw_view->mini_player_view_layout);
	evas_object_size_hint_weight_set(mw_view->title, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(mw_view->title, EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_text_set(mw_view->title, "");

	elm_object_part_content_set(mw_view->mini_player_view_layout, "title_touch", mw_view->title);
	evas_object_show(mw_view->title);
	evas_object_event_callback_add(mw_view->title, EVAS_CALLBACK_MOUSE_MOVE, _mp_mini_player_title_move_cb, mw_view);

	mw_view->event_box = mp_smart_event_box_add(mw_view->mini_player_view_layout);
	MP_CHECK(mw_view->event_box);
	evas_object_size_hint_weight_set(mw_view->event_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(mw_view->event_box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(mw_view->event_box, "mouse.clicked", _mp_mini_player_eventbox_flick_click_cb, mw_view);
	evas_object_smart_callback_add(mw_view->event_box, "mouse.flick.left", _mp_mini_player_eventbox_flick_left_cb, mw_view);
	evas_object_smart_callback_add(mw_view->event_box, "mouse.flick.right", _mp_mini_player_eventbox_flick_right_cb, mw_view);
	evas_object_show(mw_view->event_box);
	elm_object_part_content_set(mw_view->mini_player_view_layout, "event_box", mw_view->event_box);

	return mw_view->mini_player_view_layout;
}

static bool _mp_mini_player_view_show_lyric(void *data, bool show_lyric)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_FALSE(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	DEBUG_TRACE("show_lyric=%d", show_lyric);
	ad->b_show_lyric = show_lyric;

	if (!show_lyric) {
		mp_mw_lyric_view_hide(mw_view);
	} else {
		mp_mw_lyric_view_show(mw_view);
	}

	return true;
}


void
mp_mini_player_layout_update(void *data, int num)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_NULL(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (mw_view->mini_player_view_layout) {
		evas_object_hide(mw_view->mini_player_view_layout);
		evas_object_del(mw_view->mini_player_view_layout);
		elm_win_resize_object_del(mw_view->win_mini, mw_view->mini_player_view_layout);
	}
	_mp_mini_player_layout_add(mw_view->win_mini, mw_view, num);
	elm_win_resize_object_add(mw_view->win_mini, mw_view->mini_player_view_layout);

	mp_mini_player_refresh(mw_view);
}


void mp_mini_player_destory(void *data)
{
	startfunc;
	MpMwView_t *mw_view = calloc(1, sizeof(MpMwView_t));
	MP_CHECK_NULL(mw_view);

	if (mw_view->switch_timer) {
		ecore_timer_del(mw_view->switch_timer);
		mw_view->switch_timer = NULL;
	}

	if (mw_view->win_mini) {
		evas_object_del(mw_view->win_mini);
	}

//        if (mw_view->xwin)
//        {
//                evas_object_del(mw_view->xwin);
//        }

	if (mw_view->bg) {
		evas_object_del(mw_view->bg);
	}

	if (mw_view->title) {
		evas_object_del(mw_view->title);
	}

	if (mw_view->event_box) {
		evas_object_del(mw_view->event_box);
	}

	if (mw_view->mini_lyric_view) {
		evas_object_del(mw_view->mini_lyric_view);
	}
	MINI_CALLED = false;
	IF_FREE(mw_view);

}

void
mp_mini_player_show(struct appdata *ad, int num)
{
	startfunc;

	if (MINI_CALLED) {
		WARN_TRACE("multi window has running");
		return;
	}

	MpMwView_t *mw_view = calloc(1, sizeof(MpMwView_t));
	MP_CHECK_NULL(mw_view);

	mw_view->win_mini = mp_create_win("music_multi_window");
//        mw_view->xwin = elm_win_xwindow_get(mw_view->win_mini);

	evas_object_hide(mw_view->win_mini);
	mw_view->bg = evas_object_rectangle_add(evas_object_evas_get(mw_view->win_mini));
	evas_object_size_hint_weight_set(mw_view->bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(mw_view->win_mini, mw_view->bg);
	evas_object_show(mw_view->bg);

	_mp_mini_player_layout_add(mw_view->win_mini, mw_view, num);
	elm_win_resize_object_add(mw_view->win_mini, mw_view->mini_player_view_layout);

	mw_view->mini_player_current_size = -1;
	mp_mini_player_mode_set(mw_view, 0);
	mp_mini_player_refresh(mw_view);

	evas_object_show(mw_view->win_mini);
	MINI_CALLED = true;
}

void
mp_mini_player_hide(void *data)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_ecore_timer_del(mw_view->play_delay_timer);
	mp_evas_object_del(mw_view->mini_player_view_layout);
	mw_view->mini_player_view_layout = NULL;

	mp_mini_player_mode_set(mw_view, 1);

#ifdef MP_FEATURE_LANDSCAPE
	app_device_orientation_e mode;
	mode = app_get_device_orientation();
	mp_app_rotate(mode, ad);
#endif

	elm_win_lower(mw_view->win_mini);
	elm_win_raise(ad->win_main);
	evas_object_show(ad->win_main);

	mp_mini_player_destory(mw_view);
}

void
mp_mini_player_mode_set(void *data, int is_set)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	double scale = elm_config_scale_get();
	int x = 0;
	int y = 0;

	if (0 == is_set) {
		int w = MINI_PLAYER_W * scale;
		int h = MINI_PLAYER_H * scale;

		elm_win_floating_mode_set(mw_view->win_mini, EINA_TRUE);

		evas_object_resize(mw_view->win_mini, w, h);
		evas_object_move(mw_view->win_mini, MINI_PLAYER_X * scale, MINI_PLAYER_Y * scale);
		mw_view->mini_player_mode = true;
	} else if (1 == is_set) {
		elm_win_floating_mode_set(mw_view->win_mini, EINA_FALSE);
		mw_view->mini_player_mode = false;
	} else {
		elm_win_floating_mode_set(mw_view->win_mini, EINA_TRUE);
		MP_TIMER_FREEZE(mw_view->switch_timer);
		edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "bg_edit_hide", "*");

		Evas_Coord_Rectangle nCurRect = {0,};
		evas_object_geometry_get(mw_view->win_mini, &nCurRect.x, &nCurRect.y, &nCurRect.w, &nCurRect.h);
		//evas_object_resize(mw_view->win_mini, nCurRect.w, nCurRect.h);
		int x = 0;

		if (nCurRect.h < 365) {
			if (nCurRect.w < 540) {
				mp_mini_player_layout_update(mw_view, WINDOW_SIZE_0);
				x = nCurRect.w;
			} else {
				mp_mini_player_layout_update(mw_view, WINDOW_SIZE_2);
				x = nCurRect.w;
			}
		} else if (nCurRect.w < 540) {
			x = nCurRect.w;
			mp_mini_player_layout_update(mw_view, WINDOW_SIZE_3);
		} else if (nCurRect.w < 720) {
			x = nCurRect.w;
			mp_mini_player_layout_update(mw_view, WINDOW_SIZE_4);
		}
		if (nCurRect.h < 177) {
			nCurRect.h = 177;
		}

		if (x < 362) {
			x = 362;
		}

		evas_object_resize(mw_view->win_mini, x, nCurRect.h);
		mw_view->mini_player_mode = true;

	}

}

void
mp_mini_player_refresh(void *data)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK(mw_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->current_track_info);

	MP_CHECK(!ad->is_lcd_off);

	mp_track_info_t *current_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(current_item);

	/* albumart */
	char *albumart_path = NULL;
	if (ad->current_track_info && mp_util_is_image_valid(ad->evas, ad->current_track_info->thumbnail_path)) {
		albumart_path = ad->current_track_info->thumbnail_path;
	} else {
		char default_thumbnail[1024] = {0};
		char *shared_path = app_get_shared_resource_path();
		snprintf(default_thumbnail, 1024, "%s%s/%s", shared_path, "shared_images", DEFAULT_THUMBNAIL);
		free(shared_path);
		albumart_path = g_strdup(default_thumbnail);
	}

	DEBUG_TRACE("albumart = %s", albumart_path);

	Evas_Object *thumb_nail = elm_object_part_content_get(mw_view->mini_player_view_layout, THUMB_NAIL_PART_NAME);
	Evas_Object *albumart_bg = elm_object_part_content_get(mw_view->mini_player_view_layout, ALBUMART_PART_NAME);

	if (albumart_bg) {
		elm_image_file_set(albumart_bg, albumart_path, NULL);
		evas_object_show(albumart_bg);
	}

	if (thumb_nail) {
		elm_image_file_set(thumb_nail, albumart_path, NULL);
		evas_object_show(thumb_nail);
	}

	/* title */
	//elm_object_part_text_set(mw_view->mini_player_view_layout, "text.title", ad->current_track_info->title);
	char *markup_title = elm_entry_utf8_to_markup(ad->current_track_info->title);
	char *colored_title = g_strdup_printf("<align=left><font_size=40><color=#FFFFFFFF>%s</color></font_size></align>", markup_title);
	Evas_Object *title_label = mp_widget_slide_title_create(mw_view->mini_player_view_layout, "slide_short", colored_title);
	elm_label_slide_go(title_label);
	elm_object_part_content_set(mw_view->mini_player_view_layout, "text.title", title_label);
	IF_FREE(markup_title);
	IF_FREE(colored_title);

	/* artist */
	elm_object_part_text_set(mw_view->mini_player_view_layout, "text.artist", ad->current_track_info->artist);

	if (-1 == mw_view->mini_player_current_size) {
		mw_view->mini_player_current_size = WINDOW_SIZE_0;
		if (ad->player_state == PLAY_STATE_PLAYING) {
			edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "set_play", "*");
		} else if (ad->player_state == PLAY_STATE_PAUSED) {
			edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "set_pause", "*");
		}
	} else {
		edje_object_signal_emit(_EDJ(mw_view->mini_player_view_layout), "set_pause", "*");
		mw_view->play_delay_timer = ecore_timer_add(0.5, _mp_mini_player_delay_play_timer_cb, mw_view);
	}

	/* rotation */
	app_device_orientation_e mode;
	mode = app_get_device_orientation();
	mp_mini_player_rotation_cb(mode, mw_view);

	evas_object_show(mw_view->mini_player_view_layout);
	endfunc;
}

void
mp_mini_player_window_drag_resize(void *data, int start_x, int start_y, unsigned int button)
{
	startfunc;

	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK(mw_view);
//	MP_CHECK(mw_view->xwin);

//	ecore_x_netwm_moveresize_request_send(mw_view->xwin, start_x, start_y, ECORE_X_NETWM_DIRECTION_SIZE_BR, 1);
}


void
mp_mini_player_window_drag_start(void *data, int start_x, int start_y, unsigned int button)
{
	startfunc;

	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK(mw_view);
//	MP_CHECK(mw_view->xwin);

//	ecore_x_netwm_moveresize_request_send(mw_view->xwin, start_x, start_y, ECORE_X_NETWM_DIRECTION_MOVE, button);
}

int
mp_mini_player_rotation_cb(app_device_orientation_e mode, void *data)
{
	startfunc;
	MpMwView_t *mw_view = (MpMwView_t *)data;
	MP_CHECK_VAL(mw_view, 0);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, 0);
	MP_CHECK_VAL(mw_view->mini_player_view_layout, 0);

	int angle;
	DEBUG_TRACE("Enum Rotation  is %d", mode);
	DEBUG_TRACE("Rotation b is %d", elm_win_rotation_get(mw_view->win_mini));

	switch (mode) {
	case APP_DEVICE_ORIENTATION_270:
		angle = -90;
		break;

	case APP_DEVICE_ORIENTATION_90:
		angle = 90;
		break;

	case APP_DEVICE_ORIENTATION_180:
		angle = 180;
		break;

	case APP_DEVICE_ORIENTATION_0:
	default:
		angle = 0;
		break;
	}

	DEBUG_TRACE("Angle  Rotation  is %d", angle);
	elm_win_rotation_with_resize_set(mw_view->win_mini, angle);

	return 0;
}

#endif /* MP_FEATURE_APP_IN_APP */

