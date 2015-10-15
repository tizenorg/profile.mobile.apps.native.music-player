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

#include <glib.h>
#include "mp-player-view.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-player-mgr.h"
#include "mp-playlist-mgr.h"
#include "mp-player-volume-widget.h"
#include "mp-setting-view.h"
#include "mp-ctxpopup.h"
#include "mp-popup.h"
#include "mp-volume.h"
#include "mp-now-playing-list.h"
#include "mp-player-control.h"
#include "mp-play.h"
#include "mp-menu.h"
#include "mp-common.h"
#include "mp-detail-view.h"
#include "mp-smart-event-box.h"
#include "mp-create-playlist-view.h"
#include "mp-lockscreenmini.h"
#include "mp-edit-playlist.h"
#include "mp-edit-callback.h"
#include "mp-setting-ctrl.h"
#include "mp-edit-view.h"
#include "mp-db-client.h"
#include "mp-playlist-detail-view.h"
#include <runtime_info.h>
#ifndef MP_SOUND_PLAYER
#include "mp-now-playing-list-view.h"
#include "ms-effect-view.h"
#include "mp-resource.h"

#endif

#ifdef MP_FEATURE_AVRCP_13
#include "mp-avrcp.h"
#endif

#ifdef MP_FEATURE_APP_IN_APP
#include "mp-mini-player.h"
#endif
#include "mp-minicontroller.h"
#ifdef MP_FEATURE_ALBUM_COVER_BG
#include "mp-collect-color.h"
#endif
#define CHECK_VIEW(view)	\
do {\
	MpPlayerView_t *player_view = view;\
	if (player_view && player_view->player_view_magic != PLAYER_VIEW_MAGIC) {\
		ERROR_TRACE("Error: param is not player_view object!!!magic: %d", player_view->player_view_magic);\
		mp_assert(0); } \
} while (0);

#define NAVIFRAME_PLAYER_VIEW NULL /*"music/player_view"*/
#define PLAYER_VIEW_REW_SOURCE "control_previous"
#define PLAYER_VIEW_FF_SOURCE "control_next"
#define PLAYER_VIEW_DETAULT_ALBUM_IMAGE DEFAULT_PLAYER_THUMBNAIL
#define PLAYER_VIEW_ALBUM_SIZE 720 * elm_config_scale_get()
#define PLAYER_VIEW_QUEUE_BG_SIZE 68 * elm_config_scale_get()
#define PLAYER_VIEW_VOLUME_WIDGET_HIDE_TIME 3.0
#define PLAYER_VIEW_LONG_PRESS_INTERVAL 1.0
#define PLAYER_VIEW_FF_REW_INTERVAL 0.3
#define PLAYER_VIEW_TRANSIT_INTERVAL 0.3
#define PLAYER_VIEW_ZOOM_FROM 0.6
#define PLAYER_VIEW_ZOOM_TO   1.0

#define PLAYER_VIEW_LONG_PRESS_TIME_INCREASE	1.0	/*sec*/
#define ROTATE_TIMEOUT 0.2

#define MP_PORTRAIT_H			635
#define MP_MULTI_WIDGET_W		116
#define MP_MULTI_WIDGET_H		509
#define MP_MULTI_WIDGET_PADDING_W	12
#define MP_MULTI_WIDGET_START_H		20
#define MP_MULTI_WIDGET_END_H		400
#define MP_MULTI_WIDGET_SCALE		0.5
#define MP_MULTI_WINDOW_NO_PROGRESS_HEIGHT  421
#define MP_MULTI_WINDOW_NO_OPTION_HEIGHT    505
#define MP_MULTI_WINDOW_NO_ALBUMART_WIDTH   560

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER
#define SUGGEST_TRACK_MIN_COUNT 2
#define SUGGEST_TRACK_MAX_COUNT 20
#define SUGGEST_TRACK_RANDOM_COUNT 5
#define SUGGEST_TRACK_THUMBNAIL_SIZE 175
#define SUGGEST_GENGRID_ITEM_WIDTH 298
#define SUGGEST_GENGRID_ITEM_HEIGHT 376
#define SUGGEST_ALBUM_THUMBNAIL_SIZE 380*elm_config_scale_get()
static Evas_Object *_mp_player_view_gengrid_content_get(void *data, Evas_Object *obj, const char *part);
static char *_mp_player_view_gengrid_text_get(void *data, Evas_Object *obj, const char *part);
static void _mp_player_view_gengrid_item_del_cb(void *data, Evas_Object *obj);
static void _mp_player_view_genrid_create(MpPlayerView_t *thiz);
static void _mp_player_view_gengrid_item_select_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_player_view_suggestion_album_load(void *thiz);
static void _mp_player_view_current_track_info_set(MpPlayerView_t *view);

static void _mp_player_view_suggestion_album_append(MpPlayerView_t *view, char *cur_path);
static void _mp_player_genre_method_get_data(MpPlayerView_t *view, char *path);
static void _mp_player_mood_method_get_data(MpPlayerView_t *view, char *path);
static void _mp_player_random_method_get_data(MpPlayerView_t *view, char *cur_path);

#endif
#endif


static bool _mp_player_view_init_progress_bar(void *data);
static void _mp_player_view_volume_popup_hide(void *data);
static bool _mp_player_view_show_lyric(void *data);
#ifdef MP_FEATURE_MUSIC_VIEW
static void _mp_player_view_show_wave_view_set_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_player_view_hide_wave_view_set_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_player_wave_view_destory(void *data);
#endif
static Eina_Bool _mp_player_view_update_progressbar_cb(void *data);
static void _mp_player_view_show_detail_view(MpPlayerView_t *view);

void _mp_player_view_set_focused_UI(void *this);

#ifndef MP_SOUND_PLAYER
static void _mp_player_view_set_queue_list_btn_icon(void *data);
#endif

static Evas_Object *_mp_player_view_create_album_image(Evas_Object *obj, const char *path, int w, int h);
static void _mp_player_view_transit_by_item(void *data, int move_direction);

#ifndef MP_SOUND_PLAYER
/* static void _mp_player_view_create_queue_list_btn(void *data);*/
static void _mp_player_view_update_control_queue_list_btn(void *data);

static void _mp_player_view_destroy_queue_list_transit_del_cb(void *data, Elm_Transit *transit)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->queue_list);

	view->queue_status = false;
	Evas_Object *queue_title_list = elm_object_part_content_get(view->player_view_layout, "queue_list_area");
	MP_CHECK(queue_title_list);
	Evas_Object *layout = ((MpList_t *)view->queue_list)->layout;
	MP_CHECK(layout);

	elm_object_part_content_unset(queue_title_list, "queue_list_content");
	elm_object_part_content_unset(view->player_view_layout, "queue_list_area");

	evas_object_del(layout);
	evas_object_del(queue_title_list);

	view->queue_list = NULL;
	view->trans_queue_list = NULL;

	_mp_player_view_update_control_queue_list_btn(view);
}
static void _mp_player_view_destroy_queue_list(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->queue_list);

	Evas_Object *queue_title_list = elm_object_part_content_get(view->player_view_layout, "queue_list_area");
	MP_CHECK(queue_title_list);
	int p_y, y, w, h;
	evas_object_geometry_get(view->player_view_layout, NULL, &p_y, NULL, NULL);
	edje_object_part_geometry_get(_EDJ(view->player_view_layout), "queue_list_area", NULL, &y, &w, &h);
	y += p_y;

	edje_object_signal_emit(_EDJ(view->player_view_layout), "HIDE_QUEUE_LIST", "music_app");
	Elm_Transit *trans;
	trans = elm_transit_add();
	elm_transit_object_add(trans, queue_title_list);

#if 0 /*update effect from zoom to bottom-top*/
	elm_transit_effect_zoom_add(trans, PLAYER_VIEW_ZOOM_TO, PLAYER_VIEW_ZOOM_FROM);
#else
	elm_transit_effect_translation_add(trans, 0, y, 0, y + h);
#endif
	elm_transit_tween_mode_set(trans, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
	elm_transit_duration_set(trans, PLAYER_VIEW_TRANSIT_INTERVAL);
	elm_transit_smooth_set(trans, EINA_FALSE);
	elm_transit_go(trans);
	elm_transit_del_cb_set(trans, _mp_player_view_destroy_queue_list_transit_del_cb, data);

	view->trans_queue_list = trans;
}

static Eina_Bool
_mp_player_view_queue_list_update_idler_cb(void *data)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(view);
	view->queue_list_update_idler = NULL;

	MP_CHECK_FALSE(view->queue_list);
	mp_list_update((MpList_t *)view->queue_list);
	mp_now_playing_list_current_item_show((MpNowPlayingList_t *)view->queue_list);

	return ECORE_CALLBACK_DONE;
}

static void _mp_player_view_create_queue_list_transit_del_cb(void *data, Elm_Transit *transit)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	view->trans_queue_list = NULL;


	mp_ecore_idler_del(view->queue_list_update_idler);
	view->queue_list_update_idler = ecore_idler_add(_mp_player_view_queue_list_update_idler_cb, view);
}

static Eina_Bool _mp_player_view_queue_list_show_title_idler(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(view);
	MP_CHECK_FALSE(view->player_view_layout);

	Evas_Object *queue_title_list = elm_object_part_content_get(view->player_view_layout, "queue_list_area");
	MP_CHECK_FALSE(queue_title_list);
	edje_object_signal_emit(_EDJ(queue_title_list), "SHOW_QUEUE_TITLE", "music_app");

	view->queue_title_idler = NULL;
	return ECORE_CALLBACK_DONE;
}

static void _mp_player_view_create_queue_list_transit(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	CHECK_VIEW(view);

	mp_ecore_idler_del(view->queue_list_update_idler);

	Evas_Object *queue_title_list = elm_object_part_content_get(view->player_view_layout, "queue_list_area");
	MP_CHECK(queue_title_list);
	/*hide temporarily to avoid title blink when zooming, show title in idler*/
	edje_object_signal_emit(_EDJ(queue_title_list), "HIDE_QUEUE_TITLE", "music_app");

	int p_y, y, w, h;
	evas_object_geometry_get(view->player_view_layout, NULL, &p_y, NULL, NULL);
	edje_object_part_geometry_get(_EDJ(view->player_view_layout), "queue_list_area", NULL, &y, &w, &h);
	y += p_y;

	Elm_Transit *trans;
	trans = elm_transit_add();
	elm_transit_object_add(trans, queue_title_list);
#if 0 /*update effect from zoom to bottom-top*/
	elm_transit_effect_zoom_add(trans, PLAYER_VIEW_ZOOM_FROM, PLAYER_VIEW_ZOOM_TO);
#else
	elm_transit_effect_translation_add(trans, 0, y + h, 0, y);
#endif
	elm_transit_objects_final_state_keep_set(trans, EINA_TRUE);
	elm_transit_tween_mode_set(trans, ELM_TRANSIT_TWEEN_MODE_ACCELERATE);
	elm_transit_duration_set(trans, PLAYER_VIEW_TRANSIT_INTERVAL);
	elm_transit_smooth_set(trans, EINA_TRUE);
	elm_transit_go(trans);
	elm_transit_del_cb_set(trans, _mp_player_view_create_queue_list_transit_del_cb, data);

	view->trans_queue_list = trans;
	view->queue_title_idler = ecore_idler_add(_mp_player_view_queue_list_show_title_idler, view);
}

static void _mp_player_view_refresh_current_count_info(Evas_Object *layout)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);
	int track_count = 0;
	int current_index = 0;
	char *str_count = NULL;
	char *str_current = NULL;

	track_count = mp_playlist_mgr_count(ad->playlist_mgr);
	current_index = mp_playlist_mgr_get_normal_index(ad->playlist_mgr);
	if (track_count == 1) {
		str_count = g_strdup(GET_STR(STR_MP_1_SONG));
	} else {
		str_count = g_strdup_printf(GET_STR(STR_MP_PD_SONGS), track_count);
	}

	str_current = g_strdup_printf("%d / %d", (current_index+1), track_count);

	elm_object_part_text_set(layout, "track_count", str_current);
	elm_object_part_text_set(layout, "track_current", str_count);

	IF_FREE(str_count);
	IF_FREE(str_current);
	return ;
}

static void _mp_player_view_refresh_queue_list(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_layout);
	Evas_Object *queue_title_list = NULL;
	if (!view->queue_list) {
		queue_title_list = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "queue_title_list");
		MP_CHECK(queue_title_list);
		MpNowPlayingList_t *list = mp_now_playing_list_create(view->player_view_layout);
		MP_CHECK(list);
		mp_now_playing_list_set_data(list, MP_NOW_PLAYING_LIST_ATTR_HIGHLIGHT_CURRENT, true, -1);
		view->queue_list = (void *)list;
		elm_object_part_content_set(queue_title_list, "queue_list_content", ((MpList_t *)view->queue_list)->layout);
		elm_object_part_content_set(view->player_view_layout, "queue_list_area", queue_title_list);
		evas_object_show(((MpList_t *)view->queue_list)->layout);
		mp_list_update((MpList_t *)view->queue_list);
		mp_now_playing_list_current_item_show((MpNowPlayingList_t *)view->queue_list);
	} else {
		queue_title_list = elm_object_part_content_get(view->player_view_layout, "queue_list_area");
		MP_CHECK(queue_title_list);
	}
	view->queue_status = true;
	_mp_player_view_set_queue_list_btn_icon(view);
	_mp_player_view_refresh_current_count_info(queue_title_list);
	edje_object_signal_emit(_EDJ(view->player_view_layout), "SHOW_QUEUE_LIST", "music_app");
	endfunc;
}

static void _mp_player_view_create_queue_list(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	CHECK_VIEW(view);
	MP_CHECK(view->player_view_layout);

	Evas_Object *queue_title_list = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "queue_title_list");
	MP_CHECK(queue_title_list);

	if (!view->queue_list) {
		MpNowPlayingList_t *list = mp_now_playing_list_create(view->player_view_layout);
		MP_CHECK(list);
		mp_now_playing_list_set_data(list, MP_NOW_PLAYING_LIST_ATTR_HIGHLIGHT_CURRENT, true, -1);
		view->queue_list = (void *)list;
	}
	view->queue_status = true;
	elm_object_part_content_set(queue_title_list, "queue_list_content", ((MpList_t *)view->queue_list)->layout);
	elm_object_part_content_set(view->player_view_layout, "queue_list_area", queue_title_list);
	_mp_player_view_refresh_current_count_info(queue_title_list);
	evas_object_show(((MpList_t *)view->queue_list)->layout);
	/*mp_list_update((MpList_t *)view->queue_list);*/
	edje_object_signal_emit(_EDJ(view->player_view_layout), "SHOW_QUEUE_LIST", "music_app");
	/*mp_now_playing_list_current_item_show((MpNowPlayingList_t *)view->queue_list);*/

	_mp_player_view_create_queue_list_transit(view);

	endfunc;
}

static void _mp_player_view_set_queue_list_btn_icon(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	const char *path = NULL;
	const char *group = NULL;

	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->option_button[OPTION_QUEUE]);/*view->queue_button*/
	MP_CHECK(view->queue_list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *image = elm_object_part_content_get(view->option_button[OPTION_QUEUE], "icon");/*view->queue_button*/
	elm_object_style_set(view->option_button[OPTION_QUEUE], "music/control_queue_thumbnail");
	MP_CHECK(image);
	elm_image_fill_outside_set(image, EINA_TRUE);
	elm_image_file_get(image, &path, &group);

	if (ad->current_track_info && mp_util_is_image_valid(ad->evas,
			ad->current_track_info->thumbnail_path) && strcmp(ad->current_track_info->thumbnail_path,
					BROKEN_ALBUMART_IMAGE_PATH)) {
		if (!g_strcmp0(path, ad->current_track_info->thumbnail_path)) {
			return;
		}
		elm_image_file_set(image, ad->current_track_info->thumbnail_path, NULL);
	} else {
	if (!g_strcmp0(path, PLAYER_VIEW_DETAULT_ALBUM_IMAGE)) {
		return;
	}
		elm_image_file_set(image, PLAYER_VIEW_DETAULT_ALBUM_IMAGE, NULL);
	}

	endfunc;
}
static void _mp_player_view_update_control_queue_list_btn(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->navi_it);

	struct appdata *ad = mp_util_get_appdata();
	Evas_Object *image = elm_object_content_get(view->option_button[OPTION_QUEUE]);

	if (view->queue_list) {
		DEBUG_TRACE("queue list create");
		elm_object_style_set(view->option_button[OPTION_QUEUE], "music/control_queue_thumbnail");
		if (ad->current_track_info && mp_util_is_image_valid(ad->evas,
				ad->current_track_info->thumbnail_path) && strcmp(ad->current_track_info->thumbnail_path,
						BROKEN_ALBUMART_IMAGE_PATH)) {
			elm_image_file_set(image, ad->current_track_info->thumbnail_path, NULL);
		} else {
			elm_image_file_set(image, PLAYER_VIEW_DETAULT_ALBUM_IMAGE, NULL);
		}
		elm_image_fill_outside_set(image, EINA_TRUE);
	} else {
		DEBUG_TRACE("no queue list");
		elm_object_style_set(view->option_button[OPTION_QUEUE], "music/control_queue");
		elm_image_file_set(image, IMAGE_EDJ_NAME, MP_ICON_PLAY_LIST_PATH);
	}
	endfunc;
}

static void _mp_player_view_control_queue_list_btn_clicked(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(!view->trans_queue_list);

	if (!view->queue_list)
		_mp_player_view_create_queue_list(view);
	else
		_mp_player_view_destroy_queue_list(view);
	_mp_player_view_update_control_queue_list_btn(view);

	endfunc;
}

Evas_Object *_mp_player_view_create_control_queue_icon_btn(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data)
{
	Evas_Object *ic;
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "music/control_queue");

	ic = elm_icon_add(parent);
	elm_image_file_set(ic, file, group);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_content_set(btn, ic);

	evas_object_smart_callback_add(btn, "clicked", func, data);

	evas_object_show(btn);
	return btn;
}

static void _mp_player_view_create_control_queue_btn(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->layout);
	view->option_button[OPTION_QUEUE] = _mp_player_view_create_control_queue_icon_btn(view->player_view_option_layout,
			IMAGE_EDJ_NAME,
			MP_ICON_PLAY_LIST_PATH,
			_mp_player_view_control_queue_list_btn_clicked,
			view);
	MP_CHECK(view->option_button[OPTION_QUEUE]);

	evas_object_show(view->option_button[OPTION_QUEUE]);
	elm_object_part_content_set(view->player_view_option_layout, "options_queue", view->option_button[OPTION_QUEUE]);

	mp_util_domain_translatable_part_text_set(view->player_view_option_layout, "queue_text", STR_PLAYER_VIEW_QUEUE);
	endfunc;
}

#endif
static void
_mp_player_view_popup_bt_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MP_CHECK(data);
	MpPlayerView_t *view = data;

	Evas_Object *image = elm_object_part_content_get(view->snd_button, "icon");
	elm_image_file_set(image, IMAGE_EDJ_NAME, MP_ICON_BT_HEADSET_PATH);

	mp_popup_destroy(mp_util_get_appdata());
	endfunc;
}

static void
_mp_player_view_popup_mirroring_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MP_CHECK(data);
	MpPlayerView_t *view = data;

	Evas_Object *image = elm_object_part_content_get(view->snd_button, "icon");
	elm_image_file_set(image, IMAGE_EDJ_NAME, MP_ICON_MIRRORING);

	mp_popup_destroy(mp_util_get_appdata());
	endfunc;
}

static void
_mp_player_view_popup_HDMI_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MP_CHECK(data);
	MpPlayerView_t *view = data;

	Evas_Object *image = elm_object_part_content_get(view->snd_button, "icon");
	elm_image_file_set(image, IMAGE_EDJ_NAME, MP_ICON_HDMI);

	mp_popup_destroy(mp_util_get_appdata());
	endfunc;
}

static void
_mp_player_view_popup_USB_audio_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MP_CHECK(data);
	MpPlayerView_t *view = data;

	Evas_Object *image = elm_object_part_content_get(view->snd_button, "icon");
	elm_image_file_set(image, IMAGE_EDJ_NAME, MP_ICON_USB_AUDIOE);

	mp_popup_destroy(mp_util_get_appdata());
	endfunc;
}

static void
_mp_player_view_popup_headphone_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MP_CHECK(data);
	MpPlayerView_t *view = data;


	Evas_Object *image = elm_object_part_content_get(view->snd_button, "icon");
	elm_image_file_set(image, IMAGE_EDJ_NAME, MP_ICON_HEADSET_PATH);

	mp_popup_destroy(mp_util_get_appdata());
	endfunc;
}

static void
_mp_player_view_popup_speaker_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MP_CHECK(data);
	MpPlayerView_t *view = data;

	Evas_Object *image = elm_object_part_content_get(view->snd_button, "icon");
	elm_image_file_set(image, IMAGE_EDJ_NAME, MP_ICON_SPEAKER_PATH);

	mp_popup_destroy(mp_util_get_appdata());
	endfunc;
}

static void _mp_player_view_append_snd_path_device(MpPlayerView_t *view, char *bt_name, int bt_connected, int earjack, Evas_Object *popup)
{
	MP_CHECK(view);
	MP_CHECK(popup);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (view->available_route & MP_SND_PATH_BT)
		mp_genlist_popup_item_append(popup, bt_name, NULL, ad->radio_group, (void *)MP_SND_PATH_BT, _mp_player_view_popup_bt_cb, view);

	if (view->available_route & MP_SND_PATH_HDMI)
		mp_genlist_popup_item_append(popup, GET_STR(STR_MP_HDMI), NULL, ad->radio_group, (void *)MP_SND_PATH_HDMI, _mp_player_view_popup_HDMI_cb, view);

	if (view->available_route & MP_SND_PATH_MIRRORING)
		mp_genlist_popup_item_append(popup, GET_STR(STR_MP_MIRRORING), NULL, ad->radio_group, (void *)MP_SND_PATH_MIRRORING, _mp_player_view_popup_mirroring_cb, view);

	if (view->available_route & MP_SND_PATH_USB_AUDIO)
		mp_genlist_popup_item_append(popup, GET_STR(STR_MP_USB_AUDIO), NULL, ad->radio_group, (void *)MP_SND_PATH_USB_AUDIO, _mp_player_view_popup_USB_audio_cb, view);

	if (view->available_route & MP_SND_PATH_EARPHONE)
		mp_genlist_popup_item_append(popup, GET_STR(STR_MP_HEADPHONES), NULL, ad->radio_group, (void *)MP_SND_PATH_EARPHONE, _mp_player_view_popup_headphone_cb, view);
	else
		mp_genlist_popup_item_append(popup, GET_STR(STR_MP_SPEAKER), NULL, ad->radio_group, (void *)MP_SND_PATH_SPEAKER, _mp_player_view_popup_speaker_cb, view);

	mp_util_get_sound_path(&(ad->snd_path));

	SECURE_DEBUG("bt_name=%s,bt_connected=%d", bt_name, bt_connected);
	DEBUG_TRACE("snd_path=%d,earjack=%d", ad->snd_path, earjack);
}

static void
_mp_player_view_radio_group_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->radio_group);

	elm_radio_value_set(ad->radio_group, ad->snd_path);
	endfunc;
}

static void
_mp_player_view_radio_group_del_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	ad->radio_group = NULL;
}

#ifdef MP_FEATURE_APP_IN_APP
#ifndef MP_SOUND_PLAYER
static void _mp_player_view_minimize_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);
	struct appdata *ad = mp_util_get_appdata();
	elm_win_lower(ad->win_main);


#ifdef MP_FEATURE_APP_IN_APP
	DEBUG_TRACE("mp_mini_player_show");
	mp_mini_player_show(ad, 0);
	#endif

	endfunc
}
#endif
#endif

static void _mp_player_view_update_snd_button_state(void *data)
{
	MpPlayerView_t *view = data;
	if (!view) return;

	MP_CHECK(view->snd_button);
	if (mp_util_mirroring_is_connected()) {
		elm_object_disabled_set(view->snd_button, EINA_TRUE);
	} else {
		elm_object_disabled_set(view->snd_button, EINA_FALSE);
	}
}

/*Replaced for _prod dependency*/
void mp_player_view_set_snd_path_sensitivity(void *data)
{
	startfunc;
	MpPlayerView_t *view = data;
	MP_CHECK(view); CHECK_VIEW(view);

	if (view->snd_button) {
		elm_object_item_part_content_unset(view->inner_navi_it, "title_right_btn");
		mp_evas_object_del(view->snd_button);
	}

	view->available_route = 0;
	view->available_route_count  = 1; /*speaker or headphone */
	WARN_TRACE("Enter sound_manager_foreach_available_route");

	sound_device_h device;
	sound_device_type_e type;
	sound_device_list_h g_device_list = NULL;
	sound_device_mask_e g_device_mask = SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
	WARN_TRACE("Enter sound_manager_get_active_device");
	int ret;
	if ((ret = sound_manager_get_current_device_list(g_device_mask, &g_device_list)))
		ERROR_TRACE("sound_manager_get_active_device()... [0x%x]", ret);

	while (!(ret = sound_manager_get_next_device(g_device_list, &device))) {
		ERROR_TRACE("success to get next device\n");
		if ((ret = sound_manager_get_device_type (device, &type)))
			ERROR_TRACE("failed to get device type, ret[0x%x]\n", ret);
		switch (type) {
		case SOUND_DEVICE_BUILTIN_SPEAKER:
			DEBUG_TRACE("SOUND_DEVICE_BUILTIN_SPEAKER");
			view->available_route |= MP_SND_PATH_SPEAKER;
			break;
		case SOUND_DEVICE_AUDIO_JACK:
			DEBUG_TRACE("SOUND_DEVICE_AUDIO_JACK");
			view->available_route |= MP_SND_PATH_EARPHONE;
			break;
		case SOUND_DEVICE_BLUETOOTH:
			DEBUG_TRACE("SOUND_DEVICE_BLUETOOTH");
			view->available_route |= MP_SND_PATH_BT;
			view->available_route_count++;
			break;
		case SOUND_DEVICE_HDMI:
			DEBUG_TRACE("SOUND_DEVICE_HDMI");
			view->available_route |= MP_SND_PATH_HDMI;
			view->available_route_count++;
			break;
		case SOUND_DEVICE_MIRRORING:
			DEBUG_TRACE("SOUND_DEVICE_MIRRORING");
			view->available_route |= MP_SND_PATH_MIRRORING;
			view->available_route_count++;
			break;
		case SOUND_DEVICE_USB_AUDIO:
			DEBUG_TRACE("SOUND_DEVICE_USB_AUDIO");
			view->available_route |= MP_SND_PATH_USB_AUDIO;
			view->available_route_count++;
			break;
		default:
			break;
		}
	}
	WARN_TRACE("Leave sound_manager_foreach_available_route");

	if (view->available_route_count < 2)
		return;

	mp_snd_path snd_path;
	mp_util_get_sound_path(&snd_path);
	DEBUG_TRACE("snd_path=%d", snd_path);
/*
	const char *icon = NULL;
	if (snd_path == MP_SND_PATH_BT)
		icon = MP_ICON_BT_HEADSET_PATH;
	else if (snd_path == MP_SND_PATH_EARPHONE)
		icon = MP_ICON_HEADSET_PATH;
	else if (snd_path == MP_SND_PATH_HDMI)
		icon = MP_ICON_HDMI;
	else if (snd_path == MP_SND_PATH_MIRRORING)
		icon = MP_ICON_MIRRORING;
	else if (snd_path == MP_SND_PATH_USB_AUDIO)
		icon = MP_ICON_USB_AUDIOE;
	else
		icon = MP_ICON_SPEAKER_PATH;
*/
	MP_CHECK(view->snd_button);
	evas_object_show(view->snd_button);
	elm_object_item_part_content_set(view->inner_navi_it, "title_right_btn", view->snd_button);
	_mp_player_view_update_snd_button_state(view);

	/*if (ad->snd_path_changable && !view->dmr_button) {	// if DMR btn exist show snd btn at more
		view->snd_button = mp_widget_create_title_icon_btn_second(view->layout, IMAGE_EDJ_NAME, MP_ICON_HEADSET_PATH,
								_mp_player_view_set_snd_path_cb, view);
		mp_screen_reader_set_obj_info(view->snd_button, "Audio device");
		elm_object_item_part_content_set(view->navi_it, "title_left_btn", view->snd_button);
	}*/
}
/*Replaced for _prod dependency end*/
#ifdef MP_FEATURE_SHARE
static void _ctxpopup_share_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);

	mp_genlist_popup_create(view->layout, MP_POPUP_PV_SHARE, item->uri, ad);
}
#endif

#ifndef MP_SOUND_PLAYER
static char *
_mp_player_view_get_fid_by_handle(mp_media_info_h record)
{
	MP_CHECK_NULL(record);

	char *fid = NULL;

	mp_media_info_get_media_id(record, &fid);

	return fid;
}

static void
_mp_player_view_add_playlist_create_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	evas_object_del(obj);
	int response = (int)event_info;
	MP_CHECK(response);

	Mp_Playlist_Data *mp_playlist_data = mp_edit_playlist_create(MP_PLST_CREATE);
	MP_CHECK(mp_playlist_data);
	mp_edit_playlist_set_media_id(mp_playlist_data,  _mp_player_view_get_fid_by_handle(view->add_to_plst_handle));
	mp_edit_playlist_content_create(mp_playlist_data);

	endfunc;
}

static void
_mp_player_view_track_delete_cb(void *data, Evas_Object * obj, void *event_info)
{
	EVENT_TRACE();
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);
		int ret = 0;
		int playlist_id = -1;

		MP_CHECK(data);

		mp_media_info_h item_handler = data;

		Popup_genlist_item *gli_data = elm_object_item_data_get(event_info);
		mp_media_info_h plst = gli_data->item_data;

		int item_count = 0;
}

static void
_mp_player_view_add_playlist_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	EVENT_TRACE();
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	int ret = 0;
	int playlist_id = -1;

	MP_CHECK(data);

	mp_media_info_h item_handler = data;

	Popup_genlist_item *gli_data = elm_object_item_data_get(event_info);
	mp_media_info_h plst = gli_data->item_data;

	char *playlist_name = NULL;
	int item_count = 0;
	ret = mp_media_info_group_get_main_info(plst, &playlist_name);
	ret = mp_media_info_group_get_playlist_id(plst, &playlist_id);

	mp_media_info_list_count(MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, playlist_id, &item_count);

#ifdef MP_PLAYLIST_MAX_ITEM_COUNT
	if (item_count > MP_PLAYLIST_MAX_ITEM_COUNT) {
		char *fmt_str = GET_STR("IDS_MUSIC_POP_UNABLE_TO_ADD_MORE_THAN_PD_MUSIC_FILE");
		char *noti_str = g_strdup_printf(fmt_str, MP_PLAYLIST_MAX_ITEM_COUNT);
		mp_util_post_status_message(ad, noti_str);
		IF_FREE(noti_str);
		goto END;
	}
#endif

	mp_retm_if (ret != 0, "Fail to get value");

	bool result = false;

	char *fid = _mp_player_view_get_fid_by_handle(item_handler);
	result = mp_util_add_to_playlist_by_key(playlist_id, fid);

	if (result) {
		mp_debug("sucess add to play list");

		MpView_t *view = NULL;
		view = (MpView_t *)mp_playlist_detail_view_create(GET_NAVIFRAME,
			MP_TRACK_BY_PLAYLIST, playlist_name, playlist_id);
		mp_view_mgr_push_view(GET_VIEW_MGR, view, NULL);
		mp_view_update_options(view);
		mp_view_set_title(view, playlist_name);
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_ADD_TO_PLAYLIST_DONE);

	} else {
		mp_debug("fail add to play list");
#ifdef MP_PLAYLIST_MAX_ITEM_COUNT
		char *fmt_str = GET_STR("IDS_MUSIC_POP_UNABLE_TO_ADD_MORE_THAN_PD_MUSIC_FILE");
		char *noti_str = g_strdup_printf(fmt_str, MP_PLAYLIST_MAX_ITEM_COUNT);
		mp_util_post_status_message(ad, noti_str);
		IF_FREE(noti_str);
#else
		mp_util_post_status_message(ad, GET_STR("IDS_MUSIC_POP_UNABLE_TO_ADD"));
#endif
	}

#ifdef MP_PLAYLIST_MAX_ITEM_COUNT
      END:
#endif

	mp_popup_destroy(ad);
	return;
}

static void _mp_player_view_popup_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	mp_media_list_h list = data;
	mp_media_info_group_list_destroy(list);
}

static void _ctxpopup_delete_track_cb(void *data, Evas_Object *obj, void *event_info)
{
	EVENT_TRACE();
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);

	DEBUG_TRACE("uid of selected song is %c", *item->uid);

	if (view->add_to_plst_handle) {
			mp_media_info_destroy(view->add_to_plst_handle);
			view->add_to_plst_handle = NULL;
		}
	DEBUG_TRACE("post if");

	mp_media_info_create(&view->add_to_plst_handle, item->uid);


		mp_media_info_h handle = view->add_to_plst_handle;

		Evas_Object *popup = NULL;

		popup = mp_genlist_popup_create(obj, MP_POPUP_DELETE_TRACK, ad, ad);
		MP_CHECK(popup);
		DEBUG_TRACE("popup created");
		int ret = 0;
		int i = 0, count = -1, err = -1;

		mp_popup_response_callback_set(popup, _mp_player_view_track_delete_cb, view);
}

static void _ctxpopup_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	EVENT_TRACE();

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);

	if (view->add_to_plst_handle) {
		mp_media_info_destroy(view->add_to_plst_handle);
		view->add_to_plst_handle = NULL;
	}
	mp_media_info_create(&view->add_to_plst_handle, item->uid);


	mp_media_info_h handle = view->add_to_plst_handle;

	Evas_Object *popup = NULL;

	popup = mp_genlist_popup_create(obj, MP_POPUP_ADD_TO_PLST, ad, ad);
	MP_CHECK(popup);

	int ret = 0;
	int i = 0, count = -1, err = -1;

	/*mp_genlist_popup_item_append(popup, GET_STR("IDS_MUSIC_OPT_CREATE_PLAYLIST"), NULL, NULL, NULL, _mp_player_view_add_playlist_create_select_cb, handle);*/
	mp_popup_response_callback_set(popup, _mp_player_view_add_playlist_create_select_cb, view);

	err = mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, NULL, NULL, &count);

	if ((err != 0) || (count < 0)) {
		ERROR_TRACE("Error in mp_media_info_group_list_count (%d)\n", err);
		return;
	}

	if (count) {
		mp_media_list_h playlists = NULL;	/*must be free*/

		ret = mp_media_info_group_list_create(&playlists, MP_GROUP_BY_PLAYLIST, NULL, NULL, 0, count);
		mp_retm_if (ret != 0, "Fail to get playlist");
		evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_player_view_popup_del_cb, playlists);

		for (i = 0; i < count; i++) {
			/* it should be released in a proper place */
			mp_media_info_h plst = NULL;
			char *name = NULL;
			plst = mp_media_info_group_list_nth_item(playlists, i);
			mp_retm_if (!plst, "Fail to get item");

			ret = mp_media_info_group_get_main_info(plst, &name);
			mp_retm_if (ret != 0, "Fail to get value");

			mp_genlist_popup_item_append(popup, name, NULL, NULL, plst,
						_mp_player_view_add_playlist_select_cb,
						handle);
		}

	} else {
		Elm_Object_Item *it = mp_genlist_popup_item_append(popup, GET_STR(STR_MP_NO_PLAYLISTS), NULL, NULL, NULL, NULL, ad);
		elm_genlist_item_select_mode_set(it, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}

	evas_object_show(popup);

	return;
}
#endif

#ifndef MP_SOUND_PLAYER
/*static void _ctxpopup_group_play_cb(void *data, Evas_Object * obj, void *event_info)
{
	EVENT_TRACE("");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);


	mp_track_info_t *track_info = ad->current_track_info;
	MP_CHECK(track_info);

	app_control_h service = NULL;
	app_control_create (&service);
	app_control_set_app_id(service, "com.samsung.group-cast");
	app_control_add_extra_data(service, APP_CONTROL_DATA_SELECTED, track_info->uri);*/
	//app_control_set_mime(service, "audio/*");
	/*app_control_send_launch_request(service, NULL, NULL);
	app_control_destroy(service);
}*/

#ifdef MP_FEATURE_ALBUMART_UPDATE
static void _ctxpopup_update_albumart_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_track_info_t *track_info = ad->current_track_info;
	MP_CHECK(track_info);

	mp_albumart_update(track_info->uri, track_info->media_id);
}
#endif
#endif

static void _ctxpopup_detail_cb(void *data, Evas_Object *obj, void *event_info)
{
	EVENT_TRACE();
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	_mp_player_view_volume_popup_hide(view);
	_mp_player_view_show_detail_view(view);
}


#ifndef MP_SOUND_PLAYER
static void _ctxpopup_setting_cb(void *data, Evas_Object *obj, void *event_info)
{
	EVENT_TRACE();
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	_mp_player_view_volume_popup_hide(view);
	 mp_music_viewas_pop_cb();
}

static void _mp_player_view_queue_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	MpPlayerView_t *view = (MpPlayerView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

#ifdef MP_FEATURE_PERSONAL_PAGE
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, (MpList_t *)(view->queue_list), false,
			MP_EDIT_VIEW_PERSONAL_PAGE_NONE);
	if (edit_view == NULL)
		return;
#else
	MpEditView_t *edit_view = mp_edit_view_create(GET_NAVIFRAME, (MpList_t *)(view->queue_list), false);
#endif
	MP_CHECK(edit_view);
	edit_view->list_mode = MP_EDIT_VIEW_EDIT;
	edit_view->content_to_show->reorderable = 0;
	edit_view->create_playlist = true;
	mp_view_mgr_push_view(GET_VIEW_MGR, (MpView_t *)edit_view, NULL);
	mp_view_update((MpView_t *)edit_view);
	mp_view_update_options((MpView_t *)edit_view);
	mp_view_set_title((MpView_t *)edit_view, STR_MP_TILTE_SELECT_ITEM);
	mp_list_view_set_select_all((MpListView_t *)edit_view, true);
	mp_list_view_set_done_btn((MpListView_t *)edit_view, true, MP_DONE_ADD_TO_TYPE);
	mp_list_view_set_cancel_btn((MpListView_t *)edit_view, true);
}

/*static void _mp_player_view_queue_save_as_to_playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	MpPlayerView_t *view = (MpPlayerView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	Mp_Playlist_Data *mp_playlist_data = mp_edit_playlist_create(MP_PLST_SAVE_AS);
	MP_CHECK(mp_playlist_data);
	mp_edit_playlist_set_edit_list(mp_playlist_data, (MpList_t *)(view->queue_list));
	mp_edit_playlist_content_create(mp_playlist_data);
	mp_edit_playlist_add_to_selected_mode(mp_playlist_data, false);
}*/

/*static void
_append_group_play_option(MpPlayerView_t *view, mp_track_info_t *track_info, bool playable)
{
	MP_CHECK(track_info);

	if (playable && track_info->track_type == MP_TRACK_URI)
	{
		char *mime = mp_util_file_mime_type_get(track_info->uri);
		DEBUG_TRACE("mime: %s", mime);
		if (!g_strcmp0(mime, "audio/x-wav")||!g_strcmp0(mime, "audio/mpeg"))
			mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_PLAY_VIA_GROUP_PLAY,  MP_PLAYER_MORE_BTN_GROUP_PLAY_IMAGE, _ctxpopup_group_play_cb, view);
		IF_FREE(mime);
	}
}*/

static void
_mp_player_view_add_cover_view_options(MpPlayerView_t *view, mp_track_info_t *track_info, bool playable)
{
	/*enable set as only it's local file*/
	/*_append_set_as_option(view, track_info, playable);
	_append_group_play_option(view, track_info, playable);*/

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _ctxpopup_add_to_playlist_cb, view);

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			STR_MP_DELETE, MP_PLAYER_MORE_BTN_DELETE_IMAGE, mp_common_track_delete_cb, view);

#ifdef MP_FEATURE_ALBUMART_UPDATE
	if (mp_check_file_exist(track_info->uri) && mp_util_file_playable(track_info->uri))
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			STR_MP_UPDATE_ALBUM_ART, MP_PLAYER_MORE_BTN_UPDATE_ALBUMART, _ctxpopup_update_albumart_cb, view);
#endif

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_DETAILS, MP_PLAYER_MORE_BTN_DETAIL_IMAGE, _ctxpopup_detail_cb, view);

		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_SHOW_LYRICS, MP_PLAYER_MORE_BTN_SETTING, _ctxpopup_setting_cb, view);

#ifndef MP_FEATURE_NO_END
	/*End*/
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
}

static void
_mp_player_view_add_radio_view_options(MpPlayerView_t *view)
{

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_DETAILS, MP_PLAYER_MORE_BTN_CONN_INFO, _ctxpopup_detail_cb, view);

	/*mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, _ctxpopup_setting_cb, view);*/

}

static void
_mp_player_view_add_artist_view_options(MpPlayerView_t *view)
{
	/*mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, _ctxpopup_setting_cb, view);*/
}

static void
_mp_player_view_add_queue_list_options(MpPlayerView_t *view, bool playable)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_track_info_t *track_info = ad->current_track_info;
	MP_CHECK(track_info);

	if (mp_list_get_editable_count((MpList_t *)view->queue_list, MP_LIST_EDIT_TYPE_NORMAL)) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_ADD_TO_PLAYLIST, MP_PLAYER_MORE_BTN_ADD_TO_PLAYLSIT_IMAGE, _mp_player_view_queue_add_to_playlist_cb, view);
	}

	/*_append_group_play_option(view, track_info, playable);*/
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			STR_MP_DELETE, MP_PLAYER_MORE_BTN_DELETE_IMAGE, mp_common_track_delete_cb, view);

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_DETAILS, MP_PLAYER_MORE_BTN_CONN_INFO, _ctxpopup_detail_cb, view);

	/*mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, _ctxpopup_setting_cb, view);*/

#ifndef MP_FEATURE_NO_END
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
}
#endif

void _mp_player_view_ctxpopup_end_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MP_CHECK(data);
	MpView_t *view = data;
	MP_CHECK(view);
	mp_evas_object_del(view->more_btn_ctxpopup);
	elm_exit();
}

static void _mp_player_view_more_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_track_info_t *track_info = ad->current_track_info;
	MP_CHECK(track_info);



#ifndef MP_SOUND_PLAYER
	/*music player more option*/
	bool playable = mp_util_file_playable(track_info->uri);
	if (view->queue_list) {
		_mp_player_view_add_queue_list_options(view, playable);
	} else {
		if (view->launch_type == MP_PLAYER_NORMAL)
			_mp_player_view_add_cover_view_options(view, track_info, playable);
		else if (view->launch_type ==  MP_PLAYER_RADIO_PLAY)
			_mp_player_view_add_radio_view_options(view);
		else if (view->launch_type ==  MP_PLAYER_ARTIST_PLAY)
			_mp_player_view_add_artist_view_options(view);
	}
#else
	/*sound player more option*/
	/*
	if (mp_player_mgr_get_player_type() == MP_PLAYER_TYPE_ASF ||track_info->track_type == MP_TRACK_ALLSHARE) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
			STR_MP_CONNECTION_INFO, MP_PLAYER_MORE_BTN_CONN_INFO, _mp_player_view_connection_info_cb, view);
	}*/
	int count = 0;
	/*if (playable && (track_info->track_type == MP_TRACK_URI) && file_in_db) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_SET_AS, MP_PLAYER_MORE_BTN_SET_AS, _ctxpopup_set_as_cb, view);
		count++;
	}*/

	if (!ad->samsung_link && (!mp_util_is_streaming(track_info->uri))) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_DETAILS, MP_PLAYER_MORE_BTN_CONN_INFO, _ctxpopup_detail_cb, view);
				count++;
	}
#ifndef MP_FEATURE_NO_END
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, _mp_player_view_ctxpopup_end_cb, view);
			count++;
#endif

	if (count == 0) {
		mp_evas_object_del(view->more_btn_ctxpopup);
		return;
	}
#endif
	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);
}


static Evas_Object *_mp_player_view_create_toolbar_more_btn(Evas_Object *parent, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) return NULL;
	elm_object_style_set(btn, "naviframe/more/default");
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

static Eina_Bool _mp_player_view_back_button_clicked_cb(void *data, Elm_Object_Item *it)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	ad->del_cb_invoked = 0;

	mp_evas_object_del(view->volume_popup);
	CHECK_VIEW(view);
#ifdef MP_SOUND_PLAYER
	MP_CHECK_FALSE(view);

	if (ad->caller_win_id || ad->player_state == PLAY_STATE_NONE)
		elm_exit();
	else
		elm_win_lower(ad->win_main);

	endfunc;
	return EINA_FALSE;
#else
	MP_CHECK_VAL(view, EINA_TRUE);
	mp_view_mgr_pop_view(GET_VIEW_MGR, false);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_FAVORITE_LIST);
	endfunc;
	return EINA_TRUE;
#endif
}

void mp_player_view_set_title(void *thiz)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)thiz;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->navi_it);
	MP_CHECK(view->inner_navi_it);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* set title and sub title */
	char *title = NULL;
	char *artist = NULL;

	elm_naviframe_item_title_enabled_set(view->navi_it, EINA_FALSE, EINA_FALSE);

	if (ad->is_lcd_off) {
		title = g_strdup("");
		artist = g_strdup("");
	} else {
		title = g_strdup_printf("%s", (ad->current_track_info && ad->current_track_info->title ? ad->current_track_info->title : ""));
		artist = g_strdup_printf("%s", (ad->current_track_info && ad->current_track_info->artist ? ad->current_track_info->artist : GET_STR("IDS_COM_BODY_UNKNOWN")));
	}


	char *newtitle  =  elm_entry_utf8_to_markup(title);
	char *newartist =  elm_entry_utf8_to_markup(artist);

	elm_object_part_text_set(view->player_view_layout, "player_view_title_main", newtitle);
	elm_object_part_text_set(view->player_view_layout, "player_view_title_subtitle", newartist);

	IF_G_FREE(newtitle);
	IF_G_FREE(newartist);
	IF_G_FREE(title);
	IF_G_FREE(artist);
}

static void mp_player_view_set_title_and_buttons(void *thiz)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)thiz;
	MP_CHECK(view); CHECK_VIEW(view);

	/* set title and sub title */
	mp_player_view_set_title(view);

	if (view->launch_type == MP_PLAYER_NORMAL) {
		/* Add Title Sound Path Button */
		mp_player_view_set_snd_path_sensitivity(view);

		/* Add Title Queue list Button */
#ifndef MP_SOUND_PLAYER
		/*_mp_player_view_create_queue_list_btn(view);*/
#endif

	/*int i = 0;
	for (i = 0; i < MP_OPTION_MORE; i++)
		mp_evas_object_del(view->toolbar_options[i]);*/
		Evas_Object *btn = NULL;
		btn = elm_object_item_part_content_unset(view->navi_it, "toolbar_button1");
		mp_evas_object_del(btn);
		btn = elm_object_item_part_content_unset(view->navi_it, "toolbar_button2");
		mp_evas_object_del(btn);
	}

	/* Add Title More Button */
	bool playable = true;

	if (playable) {
		Evas_Object *btn = _mp_player_view_create_toolbar_more_btn(view->layout, _mp_player_view_more_button_clicked_cb, view);
		elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);
	/*view->toolbar_options[MP_OPTION_MORE] = btn;*/
	}

	/* Add back button */
	/*_mp_player_view_create_back_button(view->layout, view->navi_it, _mp_player_view_back_button_clicked_cb, view);
	view->toolbar_options[MP_OPTION_BACK] = btn;*/
	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_player_view_back_button_clicked_cb, view);


	endfunc;
}

static void
_mp_player_view_destory_cb(void *thiz)
{
	eventfunc;
	MpPlayerView_t *view = thiz;
	MP_CHECK(view); CHECK_VIEW(view);

	mp_evas_object_del(view->more_btn_ctxpopup);
#ifdef MP_FEATURE_MUSIC_VIEW
	_mp_player_wave_view_destory(view);
#endif

	mp_player_mgr_unset_seek_done_cb();

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	elm_object_signal_emit(ad->conformant, "elm,state,virtualkeypad,enable", "");
	MP_CHECK(ad->win_main);

#ifdef MP_FEATURE_ALBUM_COVER_BG
	SAFE_FREE(view->cur_path);
#endif
#ifdef MP_FEATURE_SUGGEST_FOR_YOU
	if (view->suggest_list) {
		g_list_free(view->suggest_list);
	}
	if (view->gic) {
		elm_gengrid_item_class_free(view->gic);
		view->gic = NULL;
	}
#endif
	if (view->start_on_transition_finish) {
		DEBUG_TRACE("view destoryed before transition effect");
		ad->paused_by_user = FALSE;
		mp_play_new_file(ad, TRUE);
	}

	mp_popup_destroy(ad);
	mp_view_fini((MpView_t *)view);

	mp_ecore_timer_del(view->progressbar_timer);
	mp_ecore_timer_del(view->volume_popup_timer);
	/*mp_ecore_timer_del(view->asf_volume_popup_timer);*/
	mp_ecore_timer_del(view->stop_update_timer);
	mp_ecore_idler_del(view->queue_title_idler);
	mp_ecore_idler_del(view->queue_list_update_idler);

	if (view->trans_queue_list) {
		elm_transit_del(view->trans_queue_list);
		view->trans_queue_list = NULL;
	}
	if (view->transit_done_timer) {
		mp_play_item_play_current_item(ad);
		mp_ecore_timer_del(view->transit_done_timer);
	}
	if (view->set_as_handle) {
		mp_media_info_destroy(view->set_as_handle);
		view->set_as_handle = NULL;
	}
	if (view->add_to_plst_handle) {
		mp_media_info_destroy(view->add_to_plst_handle);
		view->add_to_plst_handle = NULL;
	}

#ifdef MP_FEATURE_SPLIT_WINDOW
	mp_evas_object_del(view->popup_win);
#endif

	/*mp_lyric_view_destroy(view);*/
	view->player_view_magic = 0;
	free(view);

	mp_volume_key_event_callback_del();
	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, false);
}

static int _mp_player_view_update_options(void *thiz)
{
	startfunc;

	MpPlayerView_t *playing_view = (MpPlayerView_t *)thiz;
	MP_CHECK_FALSE(playing_view);

	struct appdata *ad = mp_util_get_appdata();
	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	if (item == NULL) {
		if (ad->current_track_info) {
			mp_util_free_track_info(ad->current_track_info);
			ad->current_track_info = NULL;
		}
		mp_view_mgr_pop_a_view(GET_VIEW_MGR, GET_PLAYER_VIEW);
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_UNSET_NOW_PLAYING);
		if (ad->b_minicontroller_show)
			mp_minicontroller_hide(ad);
#ifdef MP_FEATURE_LOCKSCREEN
		if (ad->b_lockmini_show) {
			mp_lockscreenmini_hide(ad);
		}
#endif
	}
	endfunc;
	return TRUE;
}

#ifdef MP_IMAGE_ROTATE_FEATURE
static void _mp_player_view_animation_set(MpPlayerView_t *playing_view, bool playing)
{
	MP_CHECK(playing_view);
	DEBUG_TRACE("view paused: %d, state: %d", playing_view->paused, playing);

	playing_view->imgId = 0;
	if (NULL == playing_view->rotation_image) {
		playing_view->rotation_image = elm_image_add(playing_view->player_view_layout);
		elm_image_file_set(playing_view->rotation_image, IMAGE_EDJ_NAME, rotateimg[0]);
		elm_object_part_content_set(playing_view->player_view_layout, "album_turn_table", playing_view->rotation_image);
	}
}
#endif

void mp_player_view_set_album_playing(void *thiz, bool playing)
{
	startfunc;
	MpPlayerView_t *playing_view = (MpPlayerView_t *)thiz;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
	MP_CHECK(playing_view->player_view_layout);
	if (playing_view->transition_state) {
		DEBUG_TRACE("It is in transition status, do nothing");
		return;
	}
	DEBUG_TRACE("playing=%d", playing);
	if (playing) {
		/*edje_object_signal_emit(_EDJ(playing_view->player_view_layout), "ALBUM_PLAING", "music_app");*/
		edje_object_signal_emit(_EDJ(playing_view->player_view_layout), "ALBUM_STOP", "music_app");
	} else {
		edje_object_signal_emit(_EDJ(playing_view->player_view_layout), "ALBUM_STOP", "music_app");
	}
	/*p_player_view_animation_set(playing_view, playing);*/
	endfunc;
}

#ifdef MP_FEATURE_ALBUM_COVER_BG
static void _mp_player_view_set_bg_color(void *thiz, Evas_Object *album_image)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)thiz;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(album_image);

	int *rgb = NULL;
	const char *album_path = NULL;
	evas_object_image_file_get(album_image, &album_path, NULL);
	if (g_strcmp0(album_path, view->cur_path)) {
		DEBUG_TRACE("album_path=%s", album_path);
		int *color = mp_collect_color_set_image(album_path, 1);
		MP_CHECK(color);
		view->cur_color = *color;
		SAFE_FREE(view->cur_path);
		view->cur_path = g_strdup(album_path);
		rgb = mp_collect_color_get_RGB(color, 1);
		SAFE_FREE(color);
	} else {
		rgb = mp_collect_color_get_RGB(&view->cur_color, 1);
	}
	DEBUG_TRACE("r=%d, g=%d, b=%d", rgb[0], rgb[1], rgb[2]);
	Evas_Object *bg = evas_object_rectangle_add(evas_object_evas_get(view->player_view_layout));
	evas_object_color_set(bg, rgb[0], rgb[1], rgb[2], 255);
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER
	if (mp_util_is_landscape()) {
			elm_object_part_content_set(view->player_view_layout, "base_bg", bg);
		} else
	#endif
		{
			elm_object_item_part_content_set(view->inner_navi_it, "base_bg", bg);
		}
#else
	elm_object_item_part_content_set(view->inner_navi_it, "base_bg", bg);
#endif
	evas_object_show(bg);
	SAFE_FREE(rgb);
}
#endif

static void _mp_player_view_set_album_image(void *thiz)
{
	startfunc;
	MpPlayerView_t *playing_view = (MpPlayerView_t *)thiz;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
	MP_CHECK(playing_view->player_view_layout);
	int h = PLAYER_VIEW_ALBUM_SIZE, w = PLAYER_VIEW_ALBUM_SIZE;

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER

	if (mp_util_is_landscape()) {
		_mp_player_view_current_track_info_set(playing_view);
		h = SUGGEST_ALBUM_THUMBNAIL_SIZE;
		w = SUGGEST_ALBUM_THUMBNAIL_SIZE;
	}
#endif
#endif

	Evas_Object *album_image = elm_object_part_content_unset(playing_view->player_view_layout, "album_image_temp");

	if (!album_image) {
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);
		MP_CHECK(ad->current_track_info);
		album_image =
			_mp_player_view_create_album_image(playing_view->player_view_layout,
				ad->current_track_info->thumbnail_path, h, w);
	}

#ifdef MP_FEATURE_ALBUM_COVER_BG
	_mp_player_view_set_bg_color(thiz, album_image);
#endif
	elm_object_part_content_set(playing_view->player_view_layout, "album_image", album_image);

	/* hide temp album area */
	edje_object_signal_emit(elm_layout_edje_get(playing_view->player_view_layout), "set_temp_invsible", "album_area_temp");
	endfunc;
}

static void _mp_player_view_set_content_info_icon(void *thiz)
{
	startfunc;
	MpPlayerView_t *playing_view = (MpPlayerView_t *)thiz;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
	MP_CHECK(playing_view->player_view_layout);

	Evas_Object *uha_icon = elm_object_part_content_unset(playing_view->player_view_layout, "uhq_icon_area");
	mp_evas_object_del(uha_icon);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->current_track_info);

	bool uhqa = mp_util_media_is_uhqa(ad->current_track_info->media_id);
	if (uhqa) {
		uha_icon = elm_icon_add(playing_view->player_view_layout);
		elm_image_file_set(uha_icon, IMAGE_EDJ_NAME, MP_PLAY_UHA_ICON);
		elm_image_resizable_set(uha_icon, EINA_TRUE, EINA_TRUE);
		elm_object_part_content_set(playing_view->player_view_layout, "uhq_icon_area", uha_icon);
	}
}

/* volume popup */
static void
_mp_player_view_volume_popup_hide(void *data)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	mp_ecore_timer_del(view->volume_popup_timer);
	mp_evas_object_del(view->volume_popup);
#ifdef MP_FEATURE_SPLIT_WINDOW
	if (view->popup_win)
		elm_win_lower(view->popup_win);
		mp_evas_object_del(view->popup_win);
#endif
}

static Eina_Bool
_mp_player_view_volume_widget_timer_cb(void *data)
{
	TIMER_TRACE();

	startfunc;
	MpPlayerView_t *view = data;
	MP_CHECK_FALSE(view);
	CHECK_VIEW(view);

	_mp_player_view_volume_popup_hide(view);

	return ECORE_CALLBACK_DONE;
}

static inline void
_mp_player_view_volume_widget_hide_timer_start(MpPlayerView_t *view)
{
	startfunc;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_ecore_timer_del(view->volume_popup_timer);
	view->volume_popup_timer = ecore_timer_add(PLAYER_VIEW_VOLUME_WIDGET_HIDE_TIME, _mp_player_view_volume_widget_timer_cb, view);
}

static void
__mp_player_view_volume_popup_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MpPlayerView_t *view = data;
	MP_CHECK(view); CHECK_VIEW(view);

	view->volume_popup = NULL;
	mp_ecore_timer_del(view->volume_popup_timer);
	mp_volume_add_change_cb(NULL, NULL);
	view->volume_popup_now_dragging = false;
}

static void _mp_player_view_volume_update(void *data)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	int volume = mp_player_mgr_volume_get_current();
	DEBUG_TRACE("volume:%d", volume);

	if (volume > 0) {
		edje_object_signal_emit(_EDJ(view->player_view_option_layout), "options_volume_visible", "options_volume");
		edje_object_signal_emit(_EDJ(view->player_view_option_layout), "options_volume_mute_invisible", "options_volume_mute");
	} else {
		edje_object_signal_emit(_EDJ(view->player_view_option_layout), "options_volume_invisible", "options_volume");
		edje_object_signal_emit(_EDJ(view->player_view_option_layout), "options_volume_mute_visible", "options_volume_mute");
	}
	mp_util_domain_translatable_part_text_set(view->player_view_option_layout, "volume_text", STR_PLAYER_VIEW_VOLUME);
}

static void
__mp_player_view_volume_widget_event_cb(void *data, Evas_Object *obj, volume_widget_event_e event)
{
	startfunc;
	MpPlayerView_t *view = data;
	MP_CHECK(view); CHECK_VIEW(view);

	if (event == VOLUME_WIDGET_EVENT_DRAG_START) {
		view->volume_popup_now_dragging = true;
		mp_ecore_timer_del(view->volume_popup_timer);
	} else if (event == VOLUME_WIDGET_EVENT_DRAG_STOP) {
		view->volume_popup_now_dragging = false;
		_mp_player_view_volume_widget_hide_timer_start(view);
	}

	_mp_player_view_volume_update(view);
}

static void
_mp_player_view_volume_change_cb(int volume, void *user_data)
{
	MpPlayerView_t *view = user_data;
	MP_CHECK(view); CHECK_VIEW(view);

	if (!view->volume_popup || view->volume_popup_now_dragging)
		return;

	mp_debug("volume = %d", volume);
	mp_player_volume_widget_set_val(view->volume_popup, volume);
	_mp_player_view_volume_update(view);

	if (volume != 0)
		view->unmute_vol = 0;

	if (view->volume_popup_timer) {
		/* re-start timer for voice control */
		mp_ecore_timer_del(view->volume_popup_timer);
		_mp_player_view_volume_widget_hide_timer_start(view);
	}
}

static void
_mp_player_view_volume_route_change(void *user_data)
{
	MpPlayerView_t *view = user_data;
	MP_CHECK(view); CHECK_VIEW(view);

	int volume = mp_player_mgr_volume_get_current();

	_mp_player_view_volume_change_cb(volume, user_data);
}


#ifdef MP_FEATURE_SPLIT_WINDOW
static void _mp_player_view_get_multi_win_position(void *data)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);

	MP_CHECK(view->inner_naviframe);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	int w = 0, h = 0, x = 0, y = 0;
	evas_object_geometry_get(view->inner_naviframe, &x, &y, &w, &h);
	elm_win_screen_position_get(ad->win_main, &x, &y);

	view->nPosY = y;
	view->nWidth = w;
	view->nHeight = h;
	DEBUG_TRACE("nPosY=%d, nWidth=%d, nHeight=%d", view->nPosY, view->nWidth, view->nHeight);
}

static int _mp_player_view_get_multi_move_pos_y(void *data)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_VAL(view, -1);

	int y = 0;
	DEBUG_TRACE("nPosY=%d, nHeight=%d", view->nPosY, view->nHeight);
	if (view->nPosY >= (MP_PORTRAIT_H-MP_MULTI_WIDGET_H)) {
		y = MP_MULTI_WIDGET_END_H;
	} else if (view->nPosY > 0 && view->nPosY < (MP_PORTRAIT_H-MP_MULTI_WIDGET_H)) {
		y = MP_MULTI_WIDGET_END_H-(view->nHeight-MP_MULTI_WIDGET_H)*MP_MULTI_WIDGET_SCALE;
	} else {
		if (view->nHeight <= MP_MULTI_WIDGET_H) {
			y = MP_MULTI_WIDGET_START_H;
		} else {
			y = MP_MULTI_WIDGET_START_H+(view->nHeight-MP_MULTI_WIDGET_H)*MP_MULTI_WIDGET_SCALE;
		}
	}
	return y;
}

static Evas_Object *_mp_player_view_create_new_win(const char *winName, Evas_Object *pParent, int x, int y, int w, int h)
{
	MP_CHECK_NULL(pParent);

	Evas_Object *pWin = NULL;
	pWin = elm_win_add(pParent, winName, ELM_WIN_UTILITY);
	Ecore_Evas *ee = ecore_evas_ecore_evas_get(evas_object_evas_get(pWin));
	ecore_evas_name_class_set(ee, "APP_POPUP", "APP_POPUP");
	elm_win_alpha_set(pWin, EINA_TRUE);
	evas_object_resize(pWin, MP_MULTI_WIDGET_W, MP_PORTRAIT_H);
	DEBUG_TRACE("Move window to (%d,%d)", x, y);
	evas_object_move(pWin, x, y);

	if (elm_win_wm_rotation_supported_get(pWin)) {
		const int rots[4] = { APP_DEVICE_ORIENTATION_0,
				APP_DEVICE_ORIENTATION_90,
				APP_DEVICE_ORIENTATION_180,
				APP_DEVICE_ORIENTATION_270 };
		elm_win_wm_rotation_available_rotations_set(pWin, rots, 4);
	}
	/* pass '-1' value to this API then it will unset preferred rotation angle */
	elm_win_wm_rotation_preferred_rotation_set(pWin, -1);
	ea_screen_reader_window_property_set(pWin, EINA_FALSE);
	evas_object_show(pWin);

	return pWin;
}

static void _mp_player_view_volume_popup_focus_in_cb(void *data, Evas *e, void *event_info)
{
	startfunc;

	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_WINDOW_FOCUS, true);
}

static void _mp_player_view_volume_popup_focus_out_cb(void *data, Evas *e, void *event_info)
{
	startfunc;

	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_WINDOW_FOCUS, false);
}

#endif


static void
_mp_player_view_volume_popup_show(MpPlayerView_t *view)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_layout);

	view->volume_popup_now_dragging = false;

#ifdef MP_FEATURE_SPLIT_WINDOW
	if (!mp_util_is_landscape()) {
		_mp_player_view_get_multi_win_position(view);
		Evas_Object *pNewWin = NULL;
		int pos_y = _mp_player_view_get_multi_move_pos_y(view);
		pNewWin = _mp_player_view_create_new_win("MUSIC_MULTI_VIEW_VOLUME_POPUP", ad->win_main,
			MP_MULTI_WIDGET_PADDING_W,
			pos_y,
			MP_MULTI_WIDGET_W,
			MP_PORTRAIT_H);
			view->popup_win = pNewWin;
		view->volume_popup = mp_player_volume_widget_add(pNewWin);
		evas_event_callback_add(evas_object_evas_get(view->volume_popup), EVAS_CALLBACK_CANVAS_FOCUS_IN, _mp_player_view_volume_popup_focus_in_cb, NULL);
		evas_event_callback_add(evas_object_evas_get(view->volume_popup), EVAS_CALLBACK_CANVAS_FOCUS_OUT, _mp_player_view_volume_popup_focus_out_cb, NULL);
	} else
#endif
	{
		view->volume_popup = mp_player_volume_widget_add(view->player_view_layout);
		elm_object_part_content_set(view->player_view_layout, "volume_popup", view->volume_popup);

	}
	evas_object_event_callback_add(view->volume_popup, EVAS_CALLBACK_DEL, __mp_player_view_volume_popup_del_cb, view);
	mp_player_volume_widget_event_callback_add(view->volume_popup, __mp_player_view_volume_widget_event_cb, view);

	mp_volume_add_change_cb(_mp_player_view_volume_change_cb, view);

	endfunc;
}

void
mp_player_view_volume_popup_control(void *data, bool force_show)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	if (view->volume_popup) {
		if (force_show)
			_mp_player_view_volume_widget_hide_timer_start(view);	/* restart timer */
		else
			_mp_player_view_volume_popup_hide(view);
	} else {
		_mp_player_view_volume_popup_show(view);
		_mp_player_view_volume_widget_hide_timer_start(view);
	}
}

static void
_mp_player_view_volume_hw_key_cb(void *user_data, mp_volume_key_e key, bool released)
{
	MpPlayerView_t *view = (MpPlayerView_t *)user_data;
	MP_CHECK(view); CHECK_VIEW(view);

	mp_ecore_timer_del(view->volume_popup_timer);

	if (released) {
		_mp_player_view_volume_widget_hide_timer_start(view);
		return;
	}
	if (!view->volume_popup)
		_mp_player_view_volume_popup_show(view);

	if (view->volume_popup && !released) {
		if (key == MP_VOLUME_KEY_DOWN) {
			view->unmute_vol = 0;
			mp_player_mgr_volume_down();
		} else if (key == MP_VOLUME_KEY_UP) {
			view->unmute_vol = 0;
			mp_player_mgr_volume_up();
		} else if (key == MP_VOLUME_KEY_MUTE) {
			if (view->unmute_vol == 0) {
				/*mute*/
				view->unmute_vol = mp_player_mgr_volume_get_current();
				mp_player_mgr_volume_set(0);
			} else {
				/*unmute*/
				mp_player_mgr_volume_set(view->unmute_vol);
				view->unmute_vol = 0;
			}
		}
	}
}

static void _mp_player_view_set_shuffle_image(void *data, int shuffle_state)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	/*MP_CHECK(view->player_view_option_layout);*/
	MP_CHECK(view->player_view_control_layout);
	if (shuffle_state) {
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_shuffle_on_visible", "control_shuffle_on");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_shuffle_off_invisible", "control_shuffle_off");
	} else {
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_shuffle_on_invisible", "control_shuffle_on");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_shuffle_off_visible", "control_shuffle_off");
	}
	mp_util_domain_translatable_part_text_set(view->player_view_control_layout, "shuffle_text", STR_PLAYER_VIEW_SHUFFLE);
}
/*add favourite begin*/
static void _mp_player_view_set_favourite_image(void *data, int favourite_state)
{
	DEBUG_TRACE("favourite_state=%d", favourite_state);
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

/*favourite button not exist in sound player*/
	Evas_Object *layout = view->player_view_layout;
#ifdef MP_FEATURE_SUGGEST_FOR_YOU
	bool landscape = mp_util_is_landscape();
	if (landscape) {
		layout = view->player_view_layout;
	}
#endif
	MP_CHECK(layout);

	if (favourite_state) {
		edje_object_signal_emit(_EDJ(layout), "options_favourite_on_visible", "options_favourite_on");
		edje_object_signal_emit(_EDJ(layout), "options_favourite_off_invisible", "options_favourite_off");
	} else {
		edje_object_signal_emit(_EDJ(layout), "options_favourite_on_invisible", "options_favourite_on");
		edje_object_signal_emit(_EDJ(layout), "options_favourite_off_visible", "options_favourite_off");
	}
}
/*add favourite end*/


static void _mp_player_view_set_rep_image(void *data, int repeat_state)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_control_layout);

	if (repeat_state == MP_PLST_REPEAT_NONE) {
		evas_object_show(view->control_button[CONTROL_REP_A]);
		evas_object_hide(view->control_button[CONTROL_REP_1]);
		evas_object_hide(view->control_button[CONTROL_REP_ALL]);

		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_A_visible", "control_rep_A");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_1_invisible", "control_rep_1");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_all_invisible", "control_rep_all");
	} else if (repeat_state == MP_PLST_REPEAT_ONE) {
		evas_object_hide(view->control_button[CONTROL_REP_A]);
		evas_object_show(view->control_button[CONTROL_REP_1]);
		evas_object_hide(view->control_button[CONTROL_REP_ALL]);
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_A_invisible", "control_rep_A");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_1_visible", "control_rep_1");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_all_invisible", "control_rep_all");
	} else if (repeat_state == MP_PLST_REPEAT_ALL) {
		evas_object_hide(view->control_button[CONTROL_REP_A]);
		evas_object_hide(view->control_button[CONTROL_REP_1]);
		evas_object_show(view->control_button[CONTROL_REP_ALL]);
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_A_invisible", "control_rep_A");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_1_invisible", "control_rep_1");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_rep_all_visible", "control_rep_all");
	} else {
		ERROR_TRACE("Error when set repeat\n");
	}
	mp_util_domain_translatable_part_text_set(view->player_view_control_layout, "repeat_text", STR_PLAYER_VIEW_REPEAT);
}

void mp_player_view_set_play_image(void *data, bool playing)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_control_layout);

	if (playing || view->start_on_transition_finish) {
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_play_visible", "control_play");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_pause_invisible", "control_pause");

	} else {
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_play_invisible", "control_play");
		edje_object_signal_emit(_EDJ(view->player_view_control_layout), "control_pause_visible", "control_pause");
	}
}

static void
_mp_player_view_show_detail_view(MpPlayerView_t *view)
{
	MpViewMgr_t *view_mgr = GET_VIEW_MGR;
	MP_CHECK(view_mgr);
	MpView_t *detail_view = (MpView_t *)mp_detail_view_create(view_mgr->navi);
	mp_view_mgr_push_view_with_effect(view_mgr, detail_view, NULL, true);

	mp_view_update(detail_view);
}

static void
_mp_player_view_option_btn_click_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	startfunc;
	return;
}

static void
_mp_player_view_progress_val_set(void *data, double position)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->progress_box);
	MP_CHECK(view->progress_bar);
	edje_object_part_drag_value_set(_EDJ(view->progress_bar), "progressbar_playing", position, 0.0);
	return;
}

void mp_player_view_update_progressbar(void *data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(!ad->is_lcd_off);

	MpPlayerView_t *playing_view = data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
	MP_CHECK(playing_view->progress_box);
	MP_CHECK(playing_view->progress_bar);

	/*INFO_TRACE("\npos = %f / length = %f\n", ad->music_pos, ad->music_length);*/

	char play_time[16] = { 0, };
	char total_time[16] = { 0, };

	double duration = ad->music_length;

	if (duration > 0.) {
		if (duration > 3600.) {
			snprintf(total_time, sizeof(total_time), "%" MUSIC_TIME_FORMAT,
				 MUSIC_TIME_ARGS(duration));
			snprintf(play_time, sizeof(play_time), "%" MUSIC_TIME_FORMAT, MUSIC_TIME_ARGS(ad->music_pos));
		} else {
			snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
				PLAY_TIME_ARGS(duration));
			snprintf(play_time, sizeof(play_time), "%" PLAY_TIME_FORMAT, PLAY_TIME_ARGS(ad->music_pos));
		}
	} else {
		if (ad->current_track_info)
			snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
					 PLAY_TIME_ARGS(ad->current_track_info->duration/1000.));
		snprintf(play_time, sizeof(play_time), "%" PLAY_TIME_FORMAT, PLAY_TIME_ARGS(ad->music_pos));
	}

	double played_ratio = 0.;
	if (duration > 0. && ad->music_pos > 0.)
		played_ratio = ad->music_pos / duration;
	if (played_ratio == 0) {
		DEBUG_TRACE("ad->music_pos=%f, duration=%f", ad->music_pos, duration);
	}
	_mp_player_view_progress_val_set(playing_view, played_ratio);

	edje_object_part_text_set(_EDJ(playing_view->progress_box), "progress_text_total", total_time);
	edje_object_part_text_set(_EDJ(playing_view->progress_box), "progress_text_playing", play_time);
}

void
mp_player_view_progress_timer_thaw(void *data)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);

	if (ad->player_state == PLAY_STATE_PLAYING) {
		if (playing_view->progressbar_timer)
			MP_TIMER_THAW(playing_view->progressbar_timer);
		else
			playing_view->progressbar_timer = ecore_timer_add(0.1, _mp_player_view_update_progressbar_cb, playing_view);

	} else if (ad->player_state == PLAY_STATE_PAUSED) {
		mp_player_view_update_progressbar(playing_view);
	}
}

void
mp_player_view_progress_timer_freeze(void *data)
{
	startfunc;
	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);

	MP_TIMER_FREEZE(playing_view->progressbar_timer);
}

static void
_mp_player_view_progressbar_down_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	if (data == NULL && obj == NULL && event_info == NULL)
		return;

	evas_object_data_set(obj, "pressed", (void *)1);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	DEBUG_TRACE("ad->player_state=%d", ad->player_state);

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);

	Evas_Event_Mouse_Down *ev = event_info;
	Evas_Object *progressbar = obj;
	int duration = 0, w = 0, current = 0, x = 0;
	double ratio = 0.0;

	playing_view->progressbar_dragging = true;

	mp_player_view_progress_timer_freeze(playing_view);
	evas_object_geometry_get(progressbar, &x, NULL, &w, NULL);
	current = ev->canvas.x - x;

	if (current < 0)
		current = 0;
	else if (current > w)
		current = w;

	ratio = (double)current / w;

	duration = mp_player_mgr_get_duration();

	if (duration <= 0) {
		mp_track_info_t *track_info = ad->current_track_info;
		if (track_info)
			duration = track_info->duration;
	}
	ad->music_length = duration / 1000.;


	ad->music_pos = ratio * ad->music_length;

	if (playing_view->update_flag == true)/* only collect position data when rotation does not start*/
		playing_view->update_pos = ad->music_pos;

	mp_player_view_update_progressbar(playing_view);
	endfunc;
}

static Eina_Bool
_mp_player_view_update_progressbar_cb(void *data)
{
	TIMER_TRACE();
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(playing_view);

	if (playing_view->progressbar_dragging)
		return ECORE_CALLBACK_RENEW;

	if (ad->is_lcd_off || mp_player_mgr_get_state() != PLAYER_STATE_PLAYING) {
		playing_view->progressbar_timer = NULL;
		return ECORE_CALLBACK_CANCEL;
	}

	static double prev_length = 0.;
	double get_pos = 0.;

	if (ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED) {
		if (ad->music_length <= 0) {
			mp_track_info_t *track_info = ad->current_track_info;
			ad->music_length = track_info->duration / 1000.0;
		}

		/*even if duration is 0, we need to update position for some streaming case
		if ((ad->music_length) > 0)*/
		{
			get_pos = mp_player_mgr_get_position() / 1000.0;
		}

		if (prev_length != ad->music_length || get_pos != ad->music_pos) {
			prev_length = ad->music_length;
			ad->music_pos = get_pos;

			mp_player_view_update_progressbar(playing_view);
		}
	}

	if (playing_view->progressbar_timer)
		ecore_timer_interval_set(playing_view->progressbar_timer, 0.5);

	if (playing_view->lyric && ad->b_show_lyric) {
		mp_lyric_sync_update(playing_view->lyric);
	}

	return ECORE_CALLBACK_RENEW;
}

void _mp_player_view_add_progress_timer(void *data)
{
	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);

	mp_ecore_timer_del(playing_view->progressbar_timer);

	playing_view->progressbar_timer = ecore_timer_add(0.1, _mp_player_view_update_progressbar_cb, playing_view);
	return;
}

static bool
_mp_player_view_init_progress_bar(void *data)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(playing_view);

	int pos = 0, duration = 0;
	if (ad->start_pos > 0) {
		pos = ad->start_pos;
		DEBUG_TRACE("ad->start_pos %d", ad->start_pos);
	} else {
		pos = mp_player_mgr_get_position();
	}
	duration = mp_player_mgr_get_duration();
	if (duration <= 0) {
		mp_track_info_t *track_info = ad->current_track_info;
		if (track_info)
			duration = track_info->duration;
	}
	ad->music_pos = pos / 1000.;
	ad->music_length = duration / 1000.;
	mp_player_view_update_progressbar(playing_view);
	_mp_player_view_add_progress_timer(playing_view);

	return true;
}

static void
_mp_player_view_progressbar_seek_done_cb(void *data)
{
	startfunc;
	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
	MP_CHECK((int)playing_view == (int)GET_PLAYER_VIEW);

	int pressed = 0;
	if (playing_view->progress_bar)
		pressed = (int)evas_object_data_get(playing_view->progress_bar, "pressed");
	if (!pressed)
		mp_player_view_progress_timer_thaw(playing_view);

	mp_player_view_update_buffering_progress(playing_view, 100);
}


static void
_mp_player_view_progressbar_up_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	if (data == NULL && obj == NULL && event_info == NULL)
		return;

	evas_object_data_set(obj, "pressed", (void *)0);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	DEBUG_TRACE("ad->player_state=%d", ad->player_state);

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);

	Evas_Event_Mouse_Up *ev = event_info;
	Evas_Object *progressbar = obj;
	int w = 0, current = 0, x = 0;
	double ratio = 0.0;

	playing_view->progressbar_dragging = false;
	playing_view->update_pos = -1;

	if (!(ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED
		|| ad->player_state == PLAY_STATE_READY)) {
		ERROR_TRACE("Invaild player_state : %d", ad->player_state);
		return;
	}

	evas_object_geometry_get(progressbar, &x, NULL, &w, NULL);

	current = ev->canvas.x - x;

	if (current < 0)
		current = 0;
	else if (current > w)
		current = w;

	ratio = (double)current / w;

	ad->music_pos = ratio * ad->music_length;
	DEBUG_TRACE("ad->music_pos=%lf", ad->music_pos);
	if (mp_player_mgr_set_position(ad->music_pos * 1000, _mp_player_view_progressbar_seek_done_cb, playing_view)) {
		mp_player_view_update_progressbar(playing_view);
	} else {
		mp_player_view_progress_timer_thaw(playing_view);
	}

#ifdef MP_FEATURE_MUSIC_VIEW
	if (playing_view->timer_wave) {
		ecore_timer_reset(playing_view->timer_wave);
	}
#endif

	endfunc;
}

static void
_mp_player_view_progressbar_move_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	if (data == NULL && obj == NULL && event_info == NULL)
		return;

	int pressed = (int)evas_object_data_get(obj, "pressed");
	if (!pressed) {
		mp_debug("-_- progressbar is not pressed yet!");
		return;
	}

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);

	Evas_Event_Mouse_Move *ev = event_info;
	Evas_Object *progressbar = obj;
	int w = 0, current = 0;
	int x;
	double ratio = 0.0;
	double new_pos;

	evas_object_geometry_get(progressbar, &x, NULL, &w, NULL);

	current = ev->cur.canvas.x - x;

	if (current < 0)
		current = 0;
	else if (current > w)
		current = w;

	ratio = (double)current / w;

	new_pos = ratio * ad->music_length;
	ad->music_pos = new_pos;
	if (playing_view->update_flag == true) /* only collect position data when rotation does not start*/
		playing_view->update_pos = new_pos;
	mp_player_view_update_progressbar(playing_view);
}

static void
_mp_player_view_progess_box_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	CHECK_VIEW(view);

	view->progress_box = NULL;
}

static char *_mp_player_view_progress_bar_tts_info_cb(void *data, Evas_Object *obj)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_NULL(view);

	const char *playing_text = elm_object_part_text_get(view->progress_box, "progress_text_playing");
	const char *total_text =  elm_object_part_text_get(view->progress_box, "progress_text_total");

	char *text = g_strconcat(GET_STR(MP_TTS_PROGRESS_PLAYING), " ", playing_text, GET_STR(MP_TTS_PROGRESS_TOTAL), " ", total_text, NULL);
	return text;
}

static void _mp_player_view_create_progress_layout(void *thiz)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)thiz;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_layout);

	view->progress_box = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_progress_box");
	MP_CHECK(view->progress_box);
	if (mp_util_is_landscape())
		elm_object_signal_emit(view->progress_box, "elm,state,landscape", "");
	evas_object_event_callback_add(view->progress_box, EVAS_CALLBACK_DEL, _mp_player_view_progess_box_del_cb, view);
	elm_object_part_content_set(view->player_view_layout, "progress_box", view->progress_box);

	view->progress_bar = mp_common_load_edj(view->progress_box, PLAY_VIEW_EDJ_NAME, "player_progressbar");
	MP_CHECK(view->progress_bar);
	elm_object_part_content_set(view->progress_box, "progress_bar", view->progress_bar);
	_mp_player_view_progress_val_set(view, 0.0);

	evas_object_event_callback_add(view->progress_bar, EVAS_CALLBACK_MOUSE_DOWN,
					_mp_player_view_progressbar_down_cb, view);
	evas_object_event_callback_add(view->progress_bar, EVAS_CALLBACK_MOUSE_UP,
					_mp_player_view_progressbar_up_cb, view);
	evas_object_event_callback_add(view->progress_bar, EVAS_CALLBACK_MOUSE_MOVE,
					_mp_player_view_progressbar_move_cb, view);

	endfunc;
}

static void
_mp_player_view_play_btn_down_cb(void *data, Evas_Object *obj,	const char *emission,	const char *source)
{
	startfunc;
	return;
}

static void
_mp_player_view_play_btn_up_cb(void *data, Evas_Object *obj,	const char *emission,	const char *source)
{
	startfunc;
	return;
}

static void
_mp_player_view_lyric_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpPlayerView_t *view = data;
	MP_CHECK(view);
	view->lyric = NULL;
}

static bool _mp_player_view_show_lyric(void *data)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(view);
	CHECK_VIEW(view);

	if (view->loaded == false) {
		DEBUG_TRACE("view is not loaded yet");
		return false;
	}

/*lyric not exist in side cast*/
#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER
	if (mp_util_is_landscape()) {
		mp_evas_object_del(view->lyric);
		return true;
	}
#endif
#endif

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);
	MP_CHECK_FALSE(ad->current_track_info);

	if (!ad->b_show_lyric || !view->show_lyric) {
		mp_debug("set hide state");
		edje_object_signal_emit(_EDJ(view->player_view_layout), "lrc_invisible", "lrc");
	} else {
		mp_debug("set show state");
		if (g_strcmp0(mp_lyric_get_path(view->lyric), ad->current_track_info->uri))
			evas_object_del(view->lyric);

		if (!view->lyric) {
			view->lyric = mp_lyric_create(view->layout, ad->current_track_info->uri);

			if (view->lyric) {
				elm_object_part_content_set(view->player_view_layout, "lrc", view->lyric);
				evas_object_event_callback_add(view->lyric, EVAS_CALLBACK_FREE, _mp_player_view_lyric_del_cb,
					view);
				edje_object_signal_emit(_EDJ(view->player_view_layout), "lrc_visible", "lrc");
			} else
				edje_object_signal_emit(_EDJ(view->player_view_layout), "lrc_invisible", "lrc");
		} else
			edje_object_signal_emit(_EDJ(view->player_view_layout), "lrc_visible", "lrc");
	}

	return true;
}

void
mp_player_view_update_dmr_icon(MpPlayerView_t *view)
{
	startfunc;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->inner_navi_it);
	MP_CHECK(view->dmr_button);
	Evas_Object *ic = elm_object_part_content_get(view->dmr_button, "icon");
	MP_CHECK(ic);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	bool button_disabled = elm_object_disabled_get(view->dmr_button);
	if (button_disabled)
		return;

	if (mp_util_mirroring_is_connected()) {
		elm_image_file_set(ic, IMAGE_EDJ_NAME, MP_ICON_NEARBY_DMR_PRESS);
	} else {
		elm_image_file_set(ic, IMAGE_EDJ_NAME, MP_ICON_NEARBY_DMR);
	}
}

#ifdef MP_FEATURE_MUSIC_VIEW
static Eina_Bool
_mp_player_view_update_wave_progressbar_cb(void *data)
{
	TIMER_TRACE();
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(playing_view);

	if (ad->music_length > 0.)
		elm_progressbar_value_set(playing_view->wave_progress_bar, ad->music_pos/ad->music_length);

	return ECORE_CALLBACK_RENEW;

}

static void _mp_player_view_create_wave_progress(void *thiz)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)thiz;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_layout);

	view->wave_progress_bar = elm_progressbar_add(view->player_view_layout);
	elm_object_style_set(view->wave_progress_bar, "wave/list_progres");
	elm_progressbar_horizontal_set(view->wave_progress_bar, EINA_TRUE);
	evas_object_size_hint_align_set(view->wave_progress_bar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(view->wave_progress_bar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(view->wave_progress_bar, 0.0);

	if (NULL == view->wave_progressbar_timer) {
		view->wave_progressbar_timer = ecore_timer_add(0.1, _mp_player_view_update_wave_progressbar_cb, view);
	}

	evas_object_show(view->wave_progress_bar);

	edje_object_signal_emit(_EDJ(view->progress_box), "set_hide", "progress_bar");

	elm_object_part_content_set(view->player_view_layout, "wave_progress", view->wave_progress_bar);

	view->wave_progress_bar_bg = (Evas_Object *)edje_object_part_object_get(_EDJ(view->player_view_layout), "wave_progress_bg");

	evas_object_event_callback_add(view->wave_progress_bar_bg, EVAS_CALLBACK_MOUSE_DOWN,
		_mp_player_view_progressbar_down_cb, view);
	evas_object_event_callback_add(view->wave_progress_bar_bg, EVAS_CALLBACK_MOUSE_UP,
		_mp_player_view_progressbar_up_cb, view);
	evas_object_event_callback_add(view->wave_progress_bar_bg, EVAS_CALLBACK_MOUSE_MOVE,
		_mp_player_view_progressbar_move_cb, view);

	endfunc;
}

static void _draw_wave_view(void *data, int show_status)
{
	/*startfunc;*/

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	MP_CHECK(view->wave_data);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);

	int w = 496 * elm_config_scale_get();
	int h = 160 * elm_config_scale_get();
	int index = 0;
	int max_pos = w;
	int hight = 0;
	double step = view->wave_length/w;

	Evas_Object *image = evas_object_image_add(evas_object_evas_get(view->player_view_layout));
	evas_object_smart_member_add(image , view->player_view_layout);
	view->overlay = image;
	evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_image_alpha_set(image, EINA_TRUE);
	evas_object_image_size_set(image, w , h);
	evas_object_image_colorspace_set(image, EVAS_COLORSPACE_ARGB8888);

	evas_object_resize(view->overlay, w, h);
	evas_object_image_size_set(view->overlay, w, h);
	evas_object_image_fill_set(view->overlay, 0, 0, w, h);

	evas_object_smart_changed(view->player_view_layout);
	evas_object_show(view->overlay);

	elm_object_part_content_set(view->player_view_layout, "wave_graph", view->overlay);

	view->pixels = evas_object_image_data_get(view->overlay, EINA_TRUE);
	view->surface = cairo_image_surface_create_for_data((unsigned char *)view->pixels, CAIRO_FORMAT_RGB24, w, h, w * 4);
	view->cr = cairo_create(view->surface);

	cairo_set_line_width(view->cr, 1.0);

	cairo_set_source_rgba(view->cr, 0.0, 0.0, 0.0, 0.0);
	cairo_set_operator(view->cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(view->cr);
	cairo_set_source_rgba(view->cr, 0.97, 0.96, 0.93, 1.0);
	cairo_set_source_rgb (view->cr, 0.97, 0.96, 0.93);
	cairo_move_to (view->cr, 1, 0);

	while (index < max_pos) {
	/*DEBUG_TRACE("****data: %d", view->wave_data[hight]);*/
		int drawpos = (h-1) - (view->wave_data[hight] * ((h-1)/100));
		cairo_line_to(view->cr, index, drawpos);
		index++;
		hight = index * step;
	}
	cairo_line_to(view->cr, w , 0);
	cairo_line_to(view->cr, 1, 0);
	cairo_stroke_preserve(view->cr);
	cairo_set_source_rgba(view->cr, 0.97, 0.96, 0.93, 1.0);
	cairo_set_source_rgb(view->cr, 0.97, 0.96, 0.93);
	cairo_fill(view->cr);

	cairo_destroy(view->cr);
	evas_object_image_data_set(view->overlay, view->pixels);
	evas_object_image_smooth_scale_set(view->overlay, EINA_TRUE);
	evas_object_image_data_update_add(view->overlay, 0, 0, w, h);

}

static void _mp_player_wave_view_destory(void *data)
{
	startfunc;

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
		if (playing_view->player_view_layout)
			elm_object_signal_emit(playing_view->player_view_layout, "wave_view_hide", "*");

	IF_FREE(playing_view->wave_data);
	mp_ecore_timer_del(playing_view->timer_wave);
	mp_ecore_timer_del(playing_view->wave_progressbar_timer);
		if (playing_view->progress_box)
			elm_object_signal_emit(playing_view->progress_box, "set_show", "progress_bar");

	playing_view->wave_view_status = FALSE;
}

static Eina_Bool _mp_player_view_timer_wave_update_cb(void *data)
{
	TIMER_TRACE();
	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(playing_view);

	_mp_player_wave_view_destory(data);

	return ECORE_CALLBACK_RENEW;
}

static void _mp_player_view_show_wave_view_set(void *data)
{
	startfunc;

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);

	view->wave_data = mp_music_view_mgr_exe(item->uri, &view->wave_length);
		if (!view->wave_data) {
			mp_error("fail to get music_view data[%s]", item->uri);
			mp_widget_text_popup(ad, GET_STR(STR_MP_FILE_TYPE_NOT_SUPPORTED));
			return;
		}

	view->wave_view_status = TRUE;
	_mp_player_view_create_wave_progress(view);
	_draw_wave_view(data, 0);

	edje_object_signal_emit(_EDJ(view->player_view_layout), "wave_view_show", "*");

	if (NULL == view->timer_wave) {
	view->timer_wave = ecore_timer_add(20, _mp_player_view_timer_wave_update_cb, view);
	}

}

static void _mp_player_view_show_wave_view_set_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	_mp_player_view_show_wave_view_set(data);
}
static void _mp_player_view_hide_wave_view_set_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
		mp_evas_object_del(view->more_btn_ctxpopup);

	_mp_player_wave_view_destory(data);
}
#endif

static void
_mp_player_view_ff_rew_btn_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;
	mp_play_control_reset_ff_rew();
}

static void _mp_player_view_shuffle_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("shuffle button clicked");
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	int shuffle_state = 0;
	mp_setting_get_shuffle_state(&shuffle_state);
	shuffle_state = !shuffle_state;
	_mp_player_view_set_shuffle_image(data, shuffle_state);
	mp_play_control_shuffle_set(ad, shuffle_state);
	if (ad->win_minicon)
		mp_minicontroller_update_shuffle_and_repeat_btn(ad);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_NOW_PLAYING);
}

static void _mp_player_view_favor_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("favor button clicked");
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);
	mp_media_info_h favourite_handle = NULL;
	mp_media_info_create(&favourite_handle, item->uid);
	bool favorite = false;
	mp_media_info_get_favorite(favourite_handle, &favorite);
	favorite = !favorite;
	mp_media_info_set_favorite(favourite_handle, favorite);
	_mp_player_view_set_favourite_image(view, favorite);
	mp_media_info_destroy(favourite_handle);
}

static void _mp_player_view_repeat_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("repeat button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);

	int repeat_state = 0;
	mp_setting_get_repeat_state(&repeat_state);
	repeat_state++;
	repeat_state %= 3;
	mp_setting_set_repeat_state(repeat_state);
	_mp_player_view_set_rep_image(view, repeat_state);
	mp_playlist_mgr_set_repeat(ad->playlist_mgr, repeat_state);
#ifdef MP_FEATURE_AVRCP_13
	mp_avrcp_noti_repeat_mode(repeat_state);
#endif
	if (ad->win_minicon)
		mp_minicontroller_update_shuffle_and_repeat_btn(ad);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE_NOW_PLAYING);
}

static void _mp_player_view_volume_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("volume button clicked");
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	mp_player_view_volume_popup_control(view, false);
}

static void _mp_player_view_play_pause_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("play_pause button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);

	if (ad->player_state == PLAY_STATE_NONE && ad->music_pos > 0) {
		DEBUG_TRACE("ad->music_pos = %f", ad->music_pos);
		ad->start_pos = ad->music_pos * 1000;
		mp_play_control_play_pause(ad, true);
		mp_player_view_set_play_image(view, true);
	} else if (ad->player_state == PLAY_STATE_PLAYING) {
		mp_play_control_play_pause(ad, false);
		mp_player_view_set_play_image(view, false);
	} else {
		mp_play_control_play_pause(ad, true);
		/*when player mgr resume failed*/
		if (ad->player_state != PLAY_STATE_PLAYING) {
			mp_player_view_set_play_image(view, false);
		} else {
			mp_player_view_set_play_image(view, true);
		}
	}
}

static void _mp_player_view_prev_btn_pressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("prev button pressed");
	mp_play_control_rew(true, false, true);
}

static void _mp_player_view_prev_btn_unpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("prev button unpressed");
	mp_play_control_rew(false, false, true);
}

static void _mp_player_view_prev_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("prev button clicked");
	if (elm_config_access_get())
		mp_play_control_rew(false, false, true);
}

static void _mp_player_view_next_btn_pressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("next button pressed");
	mp_play_control_ff(true, false, true);

}

static void _mp_player_view_next_btn_unpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("next button unpressed");
	mp_play_control_ff(false, false, true);
}

static void _mp_player_view_next_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("next button clicked");
	if (elm_config_access_get())
		mp_play_control_ff(false, false, true);
}


#ifndef MP_SOUND_PLAYER
static void _mp_player_view_queue_btn_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("queue button clicked");
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	MP_CHECK(!view->trans_queue_list);

	if (!view->queue_list)
		_mp_player_view_create_queue_list(view);
	else
		_mp_player_view_destroy_queue_list(view);
	_mp_player_view_update_control_queue_list_btn(view);

	endfunc;
}
#endif

static Evas_Object *_mp_player_add_btn(void *data, Evas_Object *parent, Evas_Object *btn, char *button_style, char *part, Edje_Signal_Cb func)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_NULL(view);
	MP_CHECK_NULL(parent);

	btn = elm_button_add(parent);
	MP_CHECK_NULL(btn);
	elm_object_focus_allow_set(btn, EINA_FALSE);

	elm_object_style_set(btn, button_style);
	evas_object_show(btn);
	if (!g_strcmp0(part, "control_previous") || !g_strcmp0(part, "control_next")) {
		elm_object_signal_callback_add(btn, SIGNAL_MOUSE_DOWN, "*", _mp_player_view_play_btn_down_cb, view);
		elm_object_signal_callback_add(btn, SIGNAL_MOUSE_UP, "*", _mp_player_view_play_btn_up_cb, view);
		evas_object_event_callback_add(btn, EVAS_CALLBACK_DEL, _mp_player_view_ff_rew_btn_del_cb, view);
	} else {
		elm_object_signal_callback_add(btn, SIGNAL_MOUSE_CLICK, "*", func, data);
	}
	elm_object_part_content_set(parent, part, btn);
	return btn;
}

#ifdef MP_FEATURE_SPLIT_WINDOW

static void _mp_player_view_albumart_cb(void *data, Evas_Object *o, const char *emission, const char *source)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player_view_layout);

	mp_player_view_refresh(view);
	edje_object_signal_callback_del(_EDJ(view->player_view_layout), emission, "*", _mp_player_view_albumart_cb);
}

static void _mp_player_view_resize(void *data)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player_view_layout);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->win_main);

	int w, h;
	int scale = elm_config_scale_get();
	evas_object_geometry_get(ad->win_main, NULL, NULL, &w, &h);
	DEBUG_TRACE("win size w: %d   h: %d", w, h);

#ifdef MP_FEATURE_LANDSCAPE
	if (mp_util_is_landscape()) {
		if (w < MP_MULTI_WINDOW_NO_ALBUMART_WIDTH  * scale) {
			edje_object_signal_callback_add(_EDJ(view->player_view_layout), "set_hide_albumart", "*", _mp_player_view_albumart_cb, view);
			edje_object_signal_emit(_EDJ(view->player_view_layout), "set_hide_albumart", "*");
		} else {
			edje_object_signal_emit(_EDJ(view->player_view_layout), "set_show_albumart", "*");
		}
	} else
#endif
	{
	if (h < MP_MULTI_WINDOW_NO_PROGRESS_HEIGHT * scale) {
		edje_object_signal_callback_add(_EDJ(view->player_view_layout), "set_hide_progress", "*", _mp_player_view_albumart_cb, view);
		edje_object_signal_emit(_EDJ(view->player_view_layout), "set_hide_progress", "*");
	} else if (h < MP_MULTI_WINDOW_NO_OPTION_HEIGHT * scale) {
		edje_object_signal_callback_add(_EDJ(view->player_view_layout), "set_hide_option", "*", _mp_player_view_albumart_cb, view);
		edje_object_signal_emit(_EDJ(view->player_view_layout), "set_hide_option", "*");
	} else {
		edje_object_signal_emit(_EDJ(view->player_view_layout), "set_hide_default", "*");
	}
	}
}
#endif

static void _mp_player_view_add_callbacks(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	struct appdata *ad = mp_util_get_appdata();


#ifdef MP_FEATURE_LANDSCAPE
	bool landscape = mp_util_is_landscape();
	if (landscape) {
		/*side cast has no option area*/
		#ifdef MP_FEATURE_SUGGEST_FOR_YOU
			#ifdef MP_SOUND_PLAYER
			view->player_view_option_layout = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_option_area_ld");
			evas_object_size_hint_align_set(view->player_view_option_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_size_hint_weight_set(view->player_view_option_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			elm_object_part_content_set(view->player_view_layout, "options_area", view->player_view_option_layout);
			evas_object_show(view->player_view_option_layout);
			MP_CHECK(view->player_view_option_layout);
			#else
			view->player_view_option_layout = NULL;
			#endif
		#endif
	} else
#endif
	{
		view->player_view_option_layout = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_option_area");
		evas_object_size_hint_align_set(view->player_view_option_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(view->player_view_option_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_object_part_content_set(view->player_view_layout, "options_area", view->player_view_option_layout);
		evas_object_show(view->player_view_option_layout);
		MP_CHECK(view->player_view_option_layout);
	}

#ifdef MP_FEATURE_LANDSCAPE
	if (landscape) {
		#ifdef MP_FEATURE_SUGGEST_FOR_YOU
			#ifndef MP_SOUND_PLAYER
			view->player_view_control_layout = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_control_area_cast");
			#else
			view->player_view_control_layout = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_control_area_ld");
		#endif
	#else
		view->player_view_control_layout = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_control_area_ld");
	#endif
	} else
#endif
	{
		view->player_view_control_layout = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_control_area");
	}
	evas_object_size_hint_align_set(view->player_view_control_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(view->player_view_control_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_part_content_set(view->player_view_layout, "control_area", view->player_view_control_layout);
	evas_object_show(view->player_view_control_layout);
	mp_util_domain_translatable_part_text_set(view->player_view_option_layout, "volume_text", STR_PLAYER_VIEW_VOLUME);
	MP_CHECK(view->player_view_control_layout);

#ifndef MP_SOUND_PLAYER
	if (view->launch_type == MP_PLAYER_NORMAL) {
		view->control_button[CONTROL_SHUFFLE_ON] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_SHUFFLE_ON], "music/shuffle_on",
				"control_shuffle_on", _mp_player_view_option_btn_click_cb);
		view->control_button[CONTROL_SHUFFLE_OFF] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_SHUFFLE_OFF], "music/shuffle_off",
				"control_shuffle_off", _mp_player_view_option_btn_click_cb);

		view->control_button[CONTROL_REP_A] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_REP_A], "music/rep_A",
				"control_rep_A", _mp_player_view_option_btn_click_cb);
		view->control_button[CONTROL_REP_1] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_REP_1], "music/rep_1",
				"control_rep_1", _mp_player_view_option_btn_click_cb);
		view->control_button[CONTROL_REP_ALL] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_REP_ALL], "music/rep_all",
				"control_rep_all", _mp_player_view_option_btn_click_cb);
	}

#ifndef MP_FEATURE_SUGGEST_FOR_YOU

	view->option_button[OPTION_FAVO_ON] = _mp_player_add_btn(data, view->player_view_layout, view->option_button[OPTION_FAVO_ON], "music/favourite_on",
			"options_favourite_on", _mp_player_view_option_btn_click_cb);
	view->option_button[OPTION_FAVO_OFF] = _mp_player_add_btn(data, view->player_view_layout, view->option_button[OPTION_FAVO_OFF], "music/favourite_off",
			"options_favourite_off", _mp_player_view_option_btn_click_cb);
#else
	if (landscape) {
		view->option_button[OPTION_FAVO_ON] = _mp_player_add_btn(data, view->player_view_option_layout, view->option_button[OPTION_FAVO_ON], "music/favourite_on",
						"options_favourite_on", _mp_player_view_option_btn_click_cb);
		view->option_button[OPTION_FAVO_OFF] = _mp_player_add_btn(data, view->player_view_option_layout, view->option_button[OPTION_FAVO_OFF], "music/favourite_off",
						"options_favourite_off", _mp_player_view_option_btn_click_cb);
	} else {
		view->option_button[OPTION_FAVO_ON] = _mp_player_add_btn(data, view->player_view_layout, view->option_button[OPTION_FAVO_ON], "music/favourite_on",
				"options_favourite_on", _mp_player_view_option_btn_click_cb);
		view->option_button[OPTION_FAVO_OFF] = _mp_player_add_btn(data, view->player_view_layout, view->option_button[OPTION_FAVO_OFF], "music/favourite_off",
				"options_favourite_off", _mp_player_view_option_btn_click_cb);
	}
#endif

#else
	if (ad->samsung_link) {
		view->control_button[CONTROL_SHUFFLE_ON] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_SHUFFLE_ON], "music/shuffle_on",
				"control_shuffle_on", _mp_player_view_option_btn_click_cb);
		view->control_button[CONTROL_SHUFFLE_OFF] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_SHUFFLE_OFF], "music/shuffle_off",
				"control_shuffle_off", _mp_player_view_option_btn_click_cb);
		view->control_button[CONTROL_REP_A] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_REP_A], "music/rep_A",
				"control_rep_A", _mp_player_view_option_btn_click_cb);
		view->control_button[CONTROL_REP_1] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_REP_1], "music/rep_1",
				"control_rep_1", _mp_player_view_option_btn_click_cb);
		view->control_button[CONTROL_REP_ALL] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_REP_ALL], "music/rep_all",
				"control_rep_all", _mp_player_view_option_btn_click_cb);
	}
#endif

	view->option_button[OPTION_VOLUME] = _mp_player_add_btn(data, view->player_view_option_layout, view->option_button[OPTION_VOLUME], "music/control_volume",
			"options_volume", _mp_player_view_option_btn_click_cb);
	view->option_button[OPTION_VOLUME_MUTE] = _mp_player_add_btn(data, view->player_view_option_layout, view->option_button[OPTION_VOLUME], "music/control_volume_mute",
			"options_volume_mute", _mp_player_view_option_btn_click_cb);

#ifndef MP_SOUND_PLAYER
	if (view->launch_type == MP_PLAYER_NORMAL) {
		view->control_button[CONTROL_PREVIOUS] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_PREVIOUS], "music/control_previous",
				"control_previous", NULL);
	}
#endif

	view->control_button[CONTROL_PLAY] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_PLAY], "music/control_play",
			"control_play", _mp_player_view_option_btn_click_cb);
	view->control_button[CONTROL_PAUSE] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_PAUSE], "music/control_pause",
			"control_pause", _mp_player_view_option_btn_click_cb);

#ifndef MP_SOUND_PLAYER
	if (view->launch_type == MP_PLAYER_NORMAL || view->launch_type == MP_PLAYER_ARTIST_PLAY) {
		view->control_button[CONTROL_NEXT] = _mp_player_add_btn(data, view->player_view_control_layout, view->control_button[CONTROL_NEXT], "music/control_next",
			"control_next", NULL);
	}
#endif

#ifndef MP_SOUND_PLAYER
	_mp_player_view_create_control_queue_btn(view);
#endif

#ifndef MP_SOUND_PLAYER
	if (view->launch_type == MP_PLAYER_NORMAL) {
		_mp_player_view_set_rep_image(view, mp_playlist_mgr_get_repeat(ad->playlist_mgr));
		_mp_player_view_set_shuffle_image(view, mp_playlist_mgr_get_shuffle(ad->playlist_mgr));
	}
#endif

	_mp_player_view_volume_update(view);

	if (ad->player_state == PLAY_STATE_PLAYING)
		mp_player_view_set_play_image(view, true);
	else
		mp_player_view_set_play_image(view, false);
	endfunc;
}

static void
_mp_player_view_resume_view_status(void *data)
{

	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);
	mp_player_view_update_progressbar(view);
#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER
	if (mp_util_is_landscape() && mp_player_mgr_get_player_type() != MP_PLAYER_TYPE_ASF)
		mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, false);
	else
#endif
#endif
		mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, true);
	mp_player_view_update_state(view);

	int volume = mp_player_mgr_volume_get_current();
	if (volume != 0)
		view->unmute_vol = 0;
}

static void
_mp_player_view_start_request(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	if (view->start_on_transition_finish) {
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);

		ad->paused_by_user = FALSE;
		/*_mp_player_view_init_progress_bar(data);*/
		if (view->start_new_file) {
			/*start to play from start of the file*/
			ad->start_after_effect = TRUE;
			int ret = mp_play_new_file(ad, TRUE);
			if (ret) {
				ERROR_TRACE("Error: mp_play_new_file..");
			}
		} else {
			/*resume play here*/
			_mp_player_view_resume_view_status(view);
			mp_play_control_play_pause(ad, true);
		}
		view->start_on_transition_finish = false;
	}

	endfunc;

	return;
}

static void
_mp_player_view_eventbox_clicked_cb(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	DEBUG_TRACE("[%d]", ad->b_show_lyric);

	if (ad->b_show_lyric == 0)
		return;
	view->show_lyric = !view->show_lyric;
	_mp_player_view_show_lyric(view);
}

static Eina_Bool
_mp_player_view_eventbox_reader_flick_left_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *action_info)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(view);
	MP_CHECK_FALSE(view->player_view_layout);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);
	mp_play_prev_file(ad);

	return EINA_TRUE;
}

static Eina_Bool
_mp_player_view_eventbox_reader_flick_right_cb(void *data, Evas_Object *obj, Elm_Access_Action_Info *action_info)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(view);
	MP_CHECK_FALSE(view->player_view_layout);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	mp_play_next_file(ad, TRUE);

	return EINA_TRUE;
}

static void
_mp_player_view_eventbox_flick_left_cb(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	_mp_player_view_transit_by_item(view, PLAYER_VIEW_MOVE_LEFT);
}

static void
_mp_player_view_eventbox_flick_right_cb(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);

	_mp_player_view_transit_by_item(view, PLAYER_VIEW_MOVE_RIGHT);
}

static void _mp_player_view_add_event_box(void *data)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_layout);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *event_box = mp_smart_event_box_add(view->player_view_layout);
	MP_CHECK(event_box);
	evas_object_size_hint_weight_set(event_box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(event_box, EVAS_HINT_FILL, EVAS_HINT_FILL);

	mp_smart_event_box_callback_add(event_box, MP_EVENT_CLICK, _mp_player_view_eventbox_clicked_cb, view);
	mp_smart_event_box_callback_add(event_box, MP_EVENT_LEFT, _mp_player_view_eventbox_flick_left_cb, view);
	mp_smart_event_box_callback_add(event_box, MP_EVENT_RIGHT, _mp_player_view_eventbox_flick_right_cb, view);

	evas_object_show(event_box);
	elm_object_part_content_set(view->player_view_layout, "event_box", event_box);
	endfunc;
}


#define PLAYER_LD_RIGHT_W 560

static void _ld_layout_resize_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	MP_CHECK(obj);
	int w, h;
	evas_object_geometry_get(obj, NULL, NULL, &w, &h);

	if (w <= SCALED_SIZE(PLAYER_LD_RIGHT_W)) {
		DEBUG_TRACE("hide albumart");
		elm_object_signal_emit(obj, "hide_album", "*");
	} else {
		DEBUG_TRACE("default");
		elm_object_signal_emit(obj, "set_default", "*");
	}
}

static void _mp_player_view_content_layout_load(void *thiz)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)thiz;
	MP_CHECK(view); CHECK_VIEW(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->win_main);

	bool landscape = mp_util_is_landscape();
	Evas_Object *lyric = NULL;
	/*To reuse lyric*/
	if (view->player_view_layout) {
		lyric = elm_object_part_content_unset(view->player_view_layout, "lrc");
	}

	if (landscape) {
		DEBUG_TRACE("mode orientation 270 or 90");
/*
	-naviframe
	- layout
	- player_view_ld
	- albumart
	- inner naviframe
	- options
	- controls
*/

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifdef MP_SOUND_PLAYER
		view->player_view_layout = mp_common_load_edj(view->layout, PLAY_VIEW_EDJ_NAME, "player_view_ld");
		evas_object_event_callback_add(view->player_view_layout, EVAS_CALLBACK_RESIZE, _ld_layout_resize_cb, view);
		elm_object_part_content_set(view->layout, "list_content", view->player_view_layout);
		view->inner_naviframe = mp_widget_navigation_new(view->player_view_layout);
		MP_CHECK(view->inner_naviframe);

		evas_object_size_hint_min_set(view->inner_naviframe, SCALED_SIZE(PLAYER_LD_RIGHT_W), 0);
		elm_object_focus_set(view->inner_naviframe, FALSE);
		elm_object_focus_allow_set(view->inner_naviframe, FALSE);
		view->inner_navi_it = elm_naviframe_item_push(view->inner_naviframe, NULL, NULL, NULL, NULL, NAVIFRAME_PLAYER_VIEW);
	#else
	/*CAST SIDE do not need inner_navi_it inner_naviframe*/
		view->player_view_layout = mp_common_load_edj(view->layout, PLAY_VIEW_EDJ_NAME, "player_view_side_cast");
		view->inner_navi_it = NULL;
		view->inner_naviframe = NULL;

	#endif
#else
		view->player_view_layout = mp_common_load_edj(view->layout, PLAY_VIEW_EDJ_NAME, "player_view_ld");
		evas_object_event_callback_add(view->player_view_layout, EVAS_CALLBACK_RESIZE, _ld_layout_resize_cb, view);
		elm_object_part_content_set(view->layout, "list_content", view->player_view_layout);
		view->inner_naviframe = mp_widget_navigation_new(view->player_view_layout);
		MP_CHECK(view->inner_naviframe);
		evas_object_size_hint_min_set(view->inner_naviframe, SCALED_SIZE(PLAYER_LD_RIGHT_W), 0);

	elm_object_focus_set(view->inner_naviframe, FALSE);
	elm_object_focus_allow_set(view->inner_naviframe, FALSE);
		view->inner_navi_it = elm_naviframe_item_push(view->inner_naviframe, NULL, NULL, NULL, NULL, NAVIFRAME_PLAYER_VIEW);
#endif

		MP_CHECK(view->player_view_layout);
		elm_object_part_content_set(view->layout, "list_content", view->player_view_layout);

/*cast side has not right title*/
#ifndef MP_FEATURE_SUGGEST_FOR_YOU
		elm_object_part_content_set(view->player_view_layout, "right_title", view->inner_naviframe);
#else
#ifdef MP_SOUND_PLAYER
	elm_object_part_content_set(view->player_view_layout, "right_title", view->inner_naviframe);
#endif
#endif

		view->progress_box = mp_common_load_edj(view->player_view_layout, PLAY_VIEW_EDJ_NAME, "player_view_progress_box_ld");
		MP_CHECK(view->progress_box);
		elm_object_part_content_set(view->player_view_layout, "progress_box", view->progress_box);
		view->progress_bar = mp_common_load_edj(view->progress_box, PLAY_VIEW_EDJ_NAME, "player_progressbar");
		MP_CHECK(view->progress_bar);
		elm_object_part_content_set(view->progress_box, "progress_bar", view->progress_bar);
	} else {
		DEBUG_TRACE("mode orientation 0");
/*
	-naviframe
	- inner_naviframe
	- player_view_portrait_base
	- albumart
	- options
	- controls
*/
		view->inner_naviframe = mp_widget_navigation_new(view->layout);
		MP_CHECK(view->inner_naviframe);

	elm_object_focus_set(view->inner_naviframe, FALSE);
	elm_object_focus_allow_set(view->inner_naviframe, FALSE);

	view->player_view_layout = mp_common_load_edj(view->inner_naviframe, PLAY_VIEW_EDJ_NAME, "player_view_portrait_base");
	MP_CHECK(view->player_view_layout);
	view->inner_navi_it = elm_naviframe_item_push(view->inner_naviframe, NULL, NULL, NULL, view->player_view_layout, NAVIFRAME_PLAYER_VIEW);
	elm_naviframe_item_title_enabled_set(view->inner_navi_it, FALSE, FALSE);

	Evas_Object *content = elm_object_part_content_unset(view->layout, "list_content");
	mp_evas_object_del(content);

	elm_object_part_content_set(view->layout, "list_content", view->inner_naviframe);
	}

	elm_object_focus_allow_set(view->inner_naviframe, TRUE);

	/* reuse lyric*/
	if (lyric)
		elm_object_part_content_set(view->player_view_layout, "lrc", lyric);

	/* album image */
	_mp_player_view_set_album_image(view);


	/* content info icon */
	_mp_player_view_set_content_info_icon(view);

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER
	/* suggestion album*/
	if (landscape) {
		_mp_player_view_suggestion_album_load(view);
	}
#endif
#endif

	/* event box */
	_mp_player_view_add_event_box(view);

	/* process layout */
	_mp_player_view_create_progress_layout(view);

	/* options and control */
	_mp_player_view_add_callbacks(view);

	/* set TTS fous frame order */
	elm_object_focus_custom_chain_append(view->player_view_layout, view->player_view_option_layout, NULL);
	elm_object_focus_custom_chain_append(view->player_view_layout, view->player_view_control_layout, NULL);

	_mp_player_view_set_focused_UI(view);
	endfunc;
}

static int _mp_player_view_start_playback(void *thiz)
{
	startfunc;
	mp_player_view_progress_timer_thaw(thiz);
	mp_player_view_update_state(thiz);
	return 0;
}

static int _mp_player_view_pause_playback(void *thiz)
{
	startfunc;
	mp_player_view_update_progressbar(thiz);
	mp_player_view_progress_timer_freeze(thiz);
	mp_player_view_update_state(thiz);
	return 0;
}

static Eina_Bool _mp_player_view_stop_timer_cb(void *data)
{
	MpPlayerView_t *view = data;
	mp_player_view_update_state(view);
	view->stop_update_timer = NULL;
	return EINA_FALSE;
}

static int _mp_player_view_stop_playback(void *thiz)
{
	startfunc;
	mp_player_view_update_progressbar(thiz);
	mp_player_view_progress_timer_freeze(thiz);
	MpPlayerView_t *view = thiz;
	mp_ecore_timer_del(view->stop_update_timer);

	view->stop_update_timer = ecore_timer_add(1.0, _mp_player_view_stop_timer_cb, view);

	return 0;
}

static void _mp_player_view_resume(void *thiz)
{
	startfunc;
	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, true);
	mp_player_view_progress_timer_thaw(thiz);

	_mp_player_view_show_lyric(thiz);
	mp_player_view_update_state(thiz);

	MpPlayerView_t *view = thiz;
	int volume = mp_player_mgr_volume_get_current();
	if (volume != 0)
		view->unmute_vol = 0;

	endfunc;
}

static void _mp_player_view_pause(void *thiz)
{
	startfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (ad->is_lcd_off)
		mp_player_view_set_title(thiz);

	mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, false);
	mp_player_view_progress_timer_freeze(thiz);
	endfunc;
}

static Eina_Bool
_transit_complete_timer(void *data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(view);
	MP_CHECK_FALSE(view->player_view_layout);

	view->transit_done_timer = NULL;
	ad->freeze_indicator_icon = true;
	mp_play_item_play_current_item(mp_util_get_appdata());
	return EINA_FALSE;
}

static void
_mp_player_view_transit_by_item_complete_cb(void *data, Elm_Transit *transit)
{
	startfunc;
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view);

	view->transition_state = false;

	/* TODO: create image and set as default*/
	Evas_Object *image = NULL;
	if (view->move_direction == PLAYER_VIEW_MOVE_LEFT)
		image = elm_object_part_content_unset(view->player_view_layout, "album_image_right");
	else
		image = elm_object_part_content_unset(view->player_view_layout, "album_image_left");

	elm_object_part_content_set(view->player_view_layout, "album_image", image);

	mp_ecore_timer_del(view->transit_done_timer);
	mp_player_view_refresh(view);
	view->transit_done_timer = ecore_timer_add(0.5, _transit_complete_timer, view);
}

static Evas_Object *
_mp_player_view_create_album_image(Evas_Object *obj, const char *path, int w, int h)
{
	int width, height;
	Evas_Object *thumbnail = evas_object_image_add(evas_object_evas_get(obj));
	MP_CHECK_FALSE(thumbnail);

	evas_object_size_hint_align_set(thumbnail, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	evas_object_image_load_size_set(thumbnail, w, h);
	if (path && strcmp(BROKEN_ALBUMART_IMAGE_PATH, path))
		evas_object_image_file_set(thumbnail, path, NULL);
	else
		evas_object_image_file_set(thumbnail, DEFAULT_PLAYER_THUMBNAIL, NULL);

	evas_object_image_size_get(thumbnail, &width, &height);
	evas_object_image_filled_set(thumbnail, true);

	if (width <= 0 || height <= 0) {
		evas_object_image_file_set(thumbnail, DEFAULT_PLAYER_THUMBNAIL, NULL);
	}
	evas_object_image_preload(thumbnail, EINA_TRUE);

	evas_object_show(thumbnail);
	/*endfunc;*/
	return thumbnail;
}

static Evas_Object *_get_image_by_playlist_item(Evas_Object *parent, mp_plst_item *item)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	char *path = NULL;
	mp_track_info_t *track_info = NULL;

	if (item == NULL) {
		if (ad->current_track_info)
			path = ad->current_track_info->thumbnail_path;
	} else {
		mp_util_load_track_info(ad, item, &track_info);
		path = track_info->thumbnail_path;
	}

	Evas_Object *image = _mp_player_view_create_album_image(parent,
			path, PLAYER_VIEW_ALBUM_SIZE, PLAYER_VIEW_ALBUM_SIZE);

	mp_util_free_track_info(track_info);

	return image;
}

static void _mp_player_view_create_next_album_image(void *thiz, int move_direction)
{
	startfunc;
	MpPlayerView_t *playing_view = (MpPlayerView_t *)thiz;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
	MP_CHECK(playing_view->player_view_layout);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *image = _get_image_by_playlist_item(playing_view->player_view_layout,
			mp_playlist_mgr_get_next(ad->playlist_mgr, true, false));
	elm_object_part_content_set(playing_view->player_view_layout, "album_image_right", image);

	image = _get_image_by_playlist_item(playing_view->player_view_layout,
			mp_playlist_mgr_get_prev(ad->playlist_mgr));
	elm_object_part_content_set(playing_view->player_view_layout, "album_image_left", image);

#ifdef MP_FEATURE_MUSIC_VIEW
	_mp_player_wave_view_destory(thiz);
#endif
	endfunc;
}

#ifdef MP_FEATURE_ALBUM_COVER_BG
static void
_mp_player_view_transit_bg(void *data, Evas_Object *next)
{
	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(next);

	const char *album_path = NULL;
	evas_object_image_file_get(next, &album_path, NULL);
	int *color = mp_collect_color_set_image(album_path, 1);
	MP_CHECK(color);
	int *rgb1 = mp_collect_color_get_RGB(&view->cur_color, 1);
	view->cur_color = *color;
	SAFE_FREE(view->cur_path);
	view->cur_path = g_strdup(album_path);
	int *rgb2 = mp_collect_color_get_RGB(color, 1);
	SAFE_FREE(color);
	Evas_Object *bg = NULL;
#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER
		if (mp_util_is_landscape()) {
			bg = elm_object_part_content_get(view->player_view_layout, "base_bg");
		} else
	#endif
		{
			bg = elm_object_item_part_content_get(view->inner_navi_it, "base_bg");
		}
#else
		bg = elm_object_item_part_content_get(view->inner_navi_it, "base_bg");
#endif

		Elm_Transit *transit1 = elm_transit_add();
		elm_transit_object_add(transit1, bg);
		elm_transit_effect_color_add(transit1, rgb1[0], rgb1[1], rgb1[2], 255, rgb2[0], rgb2[1], rgb2[2], 255);
		DEBUG_TRACE("cur r=%d, g=%d, b=%d", rgb1[0], rgb1[1], rgb1[2]);
		DEBUG_TRACE("next r=%d, g=%d, b=%d", rgb2[0], rgb2[1], rgb2[2]);
		elm_transit_duration_set(transit1, PLAYER_VIEW_TRANSIT_INTERVAL);
		elm_transit_go(transit1);
		SAFE_FREE(rgb1);
		SAFE_FREE(rgb2);
	}
#endif

static void
_mp_player_view_transit_by_item(void *data, int move_direction)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	MpPlayerView_t *view = (MpPlayerView_t *)data;
	MP_CHECK(view); CHECK_VIEW(view);
	MP_CHECK(view->player_view_layout);

	if (view->transition_state)	/* transiton(transition_state)  should be transiit callback */ {
		mp_debug("skip_by transiton effect");
		return;
	}
	view->transition_state = true;

	mp_ecore_timer_del(view->transit_done_timer);

	/*mp_player_mgr_pause(ad);*/
	view->move_direction = move_direction;

	/*mp_player_mgr_destroy(mp_util_get_appdata());*/

	mp_plst_item *it = NULL;
	if (view->move_direction == PLAYER_VIEW_MOVE_LEFT)
		it = mp_playlist_mgr_get_next(ad->playlist_mgr, true, false);
	else
		it = mp_playlist_mgr_get_prev(ad->playlist_mgr);

	mp_playlist_mgr_set_current(ad->playlist_mgr, it);

	if (view->queue_list != NULL) {
		_mp_player_view_transit_by_item_complete_cb(view, NULL);
		return;
	}

	Evas_Object *cur = elm_object_part_content_get(view->player_view_layout, "album_image");
	Evas_Object *fav_off = elm_object_part_content_get(view->player_view_layout, "options_favourite_off");
	Evas_Object *fav_on = elm_object_part_content_get(view->player_view_layout, "options_favourite_on");
	Evas_Object *next = NULL;

	Evas_Coord w, width, h;
	evas_object_geometry_get(cur, NULL, NULL, &width, &h);
	if (view->move_direction == PLAYER_VIEW_MOVE_RIGHT) {

		next = elm_object_part_content_get(view->player_view_layout, "album_image_left");
		w = width;
	} else {
		next = elm_object_part_content_get(view->player_view_layout, "album_image_right");
		w = -width;
	}

	Elm_Transit *transit = elm_transit_add();
	elm_transit_object_add(transit, next);
	elm_transit_effect_translation_add(transit, 0, 80, w, 0);
	elm_transit_duration_set(transit, PLAYER_VIEW_TRANSIT_INTERVAL);
	elm_transit_tween_mode_set(transit, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_objects_final_state_keep_get(transit);
	elm_transit_del_cb_set(transit, _mp_player_view_transit_by_item_complete_cb, view);
	elm_transit_go(transit);

	Elm_Transit *transit1 = elm_transit_add();
	elm_transit_object_add(transit1, fav_off);
	elm_transit_effect_translation_add(transit1, 0, 0, w, 80);
	elm_transit_duration_set(transit1, PLAYER_VIEW_TRANSIT_INTERVAL);
	elm_transit_tween_mode_set(transit1, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_objects_final_state_keep_get(transit1);
	elm_transit_del_cb_set(transit1, _mp_player_view_transit_by_item_complete_cb, view);
	elm_transit_go(transit1);

	Elm_Transit *transit2 = elm_transit_add();
	elm_transit_object_add(transit2, cur);
	elm_transit_effect_translation_add(transit2, 0, 0, w, 80);
	elm_transit_duration_set(transit2, PLAYER_VIEW_TRANSIT_INTERVAL);
	elm_transit_tween_mode_set(transit2, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_objects_final_state_keep_get(transit2);
	elm_transit_go(transit2);

	Elm_Transit *transit3 = elm_transit_add();
	elm_transit_object_add(transit3, fav_on);
	elm_transit_effect_translation_add(transit3, 0, 0, w, 80);
	elm_transit_duration_set(transit3, PLAYER_VIEW_TRANSIT_INTERVAL);
	elm_transit_tween_mode_set(transit3, ELM_TRANSIT_TWEEN_MODE_DECELERATE);
	elm_transit_objects_final_state_keep_get(transit3);
	elm_transit_del_cb_set(transit3, _mp_player_view_transit_by_item_complete_cb, view);
	elm_transit_go(transit3);

#ifdef MP_FEATURE_ALBUM_COVER_BG
	_mp_player_view_transit_bg(view, next);
#endif
	endfunc;
}

static void _mp_player_view_lcd_off(void *thiz)
{
	startfunc;
	mp_player_view_progress_timer_freeze(thiz);
}

static void _mp_player_view_lcd_on(void *thiz)
{
	startfunc;
	mp_player_view_progress_timer_thaw(thiz);
	mp_player_view_refresh(thiz);
}

static void _mp_player_view_on_event(void *thiz, MpViewEvent_e event)
{
	MpPlayerView_t *view = (MpPlayerView_t *)thiz;
	MP_CHECK(view); CHECK_VIEW(view);
	DEBUG_TRACE("event [%d]", event);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	switch (event) {
	case MP_PLAYLIST_MODIFIED:
#ifndef MP_SOUND_PLAYER
		if (view->queue_list) {
			mp_list_update((MpList_t *)view->queue_list);
		}
#endif
		mp_player_view_refresh(view);
		break;
	case MP_VIEW_TRANSITION_FINISHED:
		elm_object_signal_emit(ad->conformant, "elm,state,virtualkeypad,enable", "");
		mp_player_view_refresh(view);
		/*_mp_player_view_start_request(view);*/
		_mp_player_view_show_lyric(view);
		break;
	case MP_ROUTE_CHANGED:
		{
			ERROR_TRACE("MP_ROUTE_CHANGED");

			/*
			** some popup of the view should be deleted only if the view is top view
			** otherwise it will delete popup of setting view or detail view
			*/
			if (mp_view_mgr_get_top_view(GET_VIEW_MGR) == (MpView_t *)view)
				mp_popup_destroy(ad);
		#ifndef MP_SOUND_PLAYER
			/*ms_effect_view_radio_val_set();*/
		#endif

			_mp_player_view_volume_route_change(view);

			mp_player_view_set_snd_path_sensitivity(view);
			break;
		}
	case MP_PLAYLIST_MGR_ITEM_CHANGED:
	{
#ifndef MP_SOUND_PLAYER
		if (view->queue_list) {
			mp_list_update((MpList_t *)view->queue_list);
		}
#endif
		break;
	}
	case MP_QUICKPANNEL_SHOW:
	{
		if (view == (MpPlayerView_t *)mp_view_mgr_get_top_view(GET_VIEW_MGR)) {
			DEBUG_TRACE("MP_QUICKPANNEL_SHOW");
			mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, false);
		}
		break;
	}
	case MP_QUICKPANNEL_HIDE:
	{
		if (view == (MpPlayerView_t *)mp_view_mgr_get_top_view(GET_VIEW_MGR)) {
			DEBUG_TRACE("MP_QUICKPANNEL_HIDE");
			mp_volume_key_grab_condition_set(MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE, true);
		}
		break;
	}
	case MP_SIDE_SYNC_STATUS_CHANGED:
	{
		mp_player_view_set_snd_path_sensitivity(view);
		break;
	}
	case MP_PLAYING_TRACK_CHANGED:
	{
		if (!view->transition_state)
			mp_player_view_refresh(view);

		/* update dmr icon
		if (ad->samsung_link)
			_mp_player_view_update_dmr_icon_state(view);*/
		break;
	}
	case MP_VIEW_EVENT_ALBUMART_CHANGED:
	{
		mp_player_view_refresh(view);
		break;
	}
#ifdef MP_FEATURE_SPLIT_WINDOW
	case MP_WIN_RESIZED:
	{
		_mp_player_view_resize(view);
		break;
	}
#endif
#ifdef MP_FEATURE_LANDSCAPE
	case MP_VIEW_ROTATE_START:
	{
		view->update_flag = false;
		break;
	}
#endif
	case MP_UPDATE_PLAYING_LIST:
	{

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER

		if (mp_util_is_landscape()) {
			_mp_player_view_suggestion_album_load(view);
		}
#endif
#endif
	}

	case MP_LYRIC_UPDATE:
	{
		_mp_player_view_show_lyric(view);
		break;
	}
	case MP_UNSET_NOW_PLAYING:
	{
		MpViewMgr_t *view_mgr = GET_VIEW_MGR;
		MP_CHECK(view_mgr);
		mp_view_mgr_pop_a_view(view_mgr, (MpView_t *)view);
		break;
	}
	case MP_START_PLAYBACK:
	{
#ifndef MP_SOUND_PLAYER
		mp_player_view_refresh(view);
		if (view->queue_list) {
			mp_list_realized_item_part_update((MpList_t *)view->queue_list, "elm.icon.left", ELM_GENLIST_ITEM_FIELD_CONTENT);

		}
#endif
		break;
	}
	case MP_RESUME_PLAYBACK:
	case MP_PAUSE_PLAYBACK:
	case MP_STOP_PLAYBACK:
	{
#ifndef MP_SOUND_PLAYER
		if (view->queue_list) {
			mp_list_realized_item_part_update((MpList_t *)view->queue_list, "elm.icon.left", ELM_GENLIST_ITEM_FIELD_CONTENT);

		}
#endif
		break;
	}
	default:
		/* Not defined*/
		break;
	}
}


static int
_mp_player_view_update_layout(void *data)
{
	startfunc;

	MpPlayerView_t *view = data;
	MP_CHECK_VAL(view, -1);
	/*elm_naviframe_item_title_enabled_set(view->navi_it, FALSE, FALSE);*/

	mp_player_view_set_title_and_buttons(view);
	mp_player_view_refresh(view);

	_mp_player_view_start_request(view);

	view->loaded = true;
	return 0;

}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_player_view_rotate(void *thiz, int init_rotate)
{
	DEBUG_TRACE("mp_player rotated %d", init_rotate);
	MpPlayerView_t *view = thiz;
	MP_CHECK(view);
	CHECK_VIEW(view);

	/*check if player position should update during rotation*/
	int current_pos = mp_player_mgr_get_position();
	/*enable update flag to get down/move for next time*/
	view->update_flag = true;
	/*
	**	view->update_pos >= 0 means update really happened
	**	current_pos != view->update_pos* 1000 update position is not the playing position
	**	it's long press rotate case.
	*/
	if (view->update_pos >= 0 && current_pos != view->update_pos * 1000) {
		mp_player_mgr_set_position(view->update_pos * 1000, _mp_player_view_progressbar_seek_done_cb, view);
	}
	/*cancel drag status for timer progress*/
	view->progressbar_dragging = false;
	/*clean update_pos. every update_pos use only once*/
	view->update_pos = -1;

	_mp_player_view_content_layout_load(view);
	view->queue_list = NULL;

	_mp_player_view_update_layout(view);
	mp_player_view_refresh(view);
#ifdef MP_FEATURE_MUSIC_VIEW
	if (view->wave_view_status)
		_mp_player_view_show_wave_view_set(view);
#endif

	if (view->transition_state) {
		WARN_TRACE("transition is not finished yet for prev/next");
		_mp_player_view_transit_by_item_complete_cb(view, NULL);
	}
}
#endif
/*
static void
_mp_player_view_layout_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK(playing_view);

	playing_view->player_view_layout = NULL;
}
*/

static int
_mp_player_view_init(Evas_Object *parent, MpPlayerView_t *view)
{
	startfunc;
	int ret = 0;
	view->disable_scroller = true;
	view->update_flag = true;
	view->update_pos = -1;
	view->show_lyric = true;

	ret =  mp_view_init(parent, (MpView_t *)view, MP_VIEW_PLAYER);
	MP_CHECK_VAL(ret == 0, -1);

	view->player_view_magic = PLAYER_VIEW_MAGIC;

	view->update = _mp_player_view_update_layout;
	view->update_options = _mp_player_view_update_options;
	view->view_destroy_cb = _mp_player_view_destory_cb;

	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;

	view->start_playback = _mp_player_view_start_playback;
	view->pause_playback = _mp_player_view_pause_playback;
	view->stop_playback = _mp_player_view_stop_playback;

	view->view_resume = _mp_player_view_resume;
	view->view_pause = _mp_player_view_pause;

	view->view_lcd_off = _mp_player_view_lcd_off;
	view->view_lcd_on = _mp_player_view_lcd_on;

	view->on_event = _mp_player_view_on_event;

#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_player_view_rotate;
	view->queue_status = false;
#endif

	return ret;
}

/*access callback for TTS*/
static char *_mp_player_view_shuffle_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("shuffle button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;

	int shuffle_state = 0;
	mp_setting_get_shuffle_state(&shuffle_state);

	if (shuffle_state == 1) {
		operation_txt = GET_SYS_STR(MP_TTS_SHUFFLE_OFF_BUTTON);
	} else {
		operation_txt = GET_SYS_STR(MP_TTS_SHUFFLE_ON_BUTTON);
	}

	char *read_txt = g_strconcat(operation_txt, " ", GET_SYS_STR(MP_SCREEN_BUTTON), NULL);
	return read_txt;
}

static char *_mp_player_view_favor_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("favor button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;

	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_NULL(item);
	mp_media_info_h favourite_handle = NULL;
	mp_media_info_create(&favourite_handle, item->uid);
	MP_CHECK_NULL(favourite_handle);

	bool favorite = false;
	mp_media_info_get_favorite(favourite_handle, &favorite);

	if (favorite == 1) {
		operation_txt = GET_SYS_STR(MP_TTS_FAVOURITE_BUTTON);
	} else {
		operation_txt = GET_SYS_STR(MP_TTS_UNFAVOURITE_BUTTON);
	}
	mp_media_info_destroy(favourite_handle);
	favourite_handle = NULL;

	char *read_txt = g_strconcat(operation_txt, " ", GET_SYS_STR(MP_SCREEN_BUTTON), NULL);

	mp_media_info_destroy(favourite_handle);
	favourite_handle = NULL;

	return read_txt;
}

static char *_mp_player_view_volume_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("volume button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;

	operation_txt = GET_SYS_STR(MP_TTS_VOLUME_BUTTON);

	char *read_txt = g_strconcat(operation_txt, " ", GET_SYS_STR(MP_SCREEN_BUTTON), NULL);
	return read_txt;
}

#ifndef MP_SOUND_PLAYER
static char *_mp_player_view_queue_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("queue button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;

	operation_txt = GET_SYS_STR(MP_TTS_QUEUE_BUTTON);

	char *read_txt = g_strconcat(operation_txt, " ", GET_SYS_STR(MP_SCREEN_BUTTON), NULL);
	return read_txt;
}
#endif

static char *_mp_player_view_play_pause_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("play/pause button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;

	if (ad->player_state == PLAY_STATE_PLAYING) {
		operation_txt = GET_SYS_STR(MP_TTS_PAUSE_BUTTON);
	} else {
		operation_txt = GET_SYS_STR(MP_TTS_PLAY_BUTTON);
	}

	char *read_txt = g_strconcat(operation_txt, " ", GET_SYS_STR(MP_SCREEN_BUTTON), NULL);
	return read_txt;
}

static char *_mp_player_view_repeat_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("repeat button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;

	int repeat_state = 0;
	mp_setting_get_repeat_state(&repeat_state);
	if (MP_PLST_REPEAT_ONE == repeat_state) {
		operation_txt = GET_SYS_STR(MP_TTS_REPEAT_ONE_BUTTON);
	} else if (MP_PLST_REPEAT_NONE == repeat_state) {
		operation_txt = GET_SYS_STR(MP_TTS_REPEAT_OFF_BUTTON);
	} else {
		/*repeat all */
		operation_txt = GET_SYS_STR(MP_TTS_REPEAT_ALL_BUTTON);
	}

	char *read_txt = g_strconcat(operation_txt, " ", GET_SYS_STR(MP_SCREEN_BUTTON), NULL);
	return read_txt;
}

static char *_mp_player_view_ff_rew_access_info_cb(void *data, Evas_Object *obj)
{
	startfunc;
	DEBUG_TRACE("ff/rew button clicked");
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	char *operation_txt = NULL;

	char *source = (char *)data;
	if (!g_strcmp0(source, CONTROLLER_FF_SOURCE)) {
		operation_txt = GET_SYS_STR(MP_TTS_NEXT_BUTTON);
	} else {
		operation_txt = GET_SYS_STR(MP_TTS_PREVIOUS_BUTTON);
	}

	char *read_txt = g_strconcat(operation_txt, " ", GET_SYS_STR(MP_SCREEN_BUTTON), NULL);
	return read_txt;
}

void _mp_player_view_set_focused_UI(void *this)
{
	MpPlayerView_t *view = (MpPlayerView_t *)this;
	Evas_Object *favor_layout = NULL;
	if (!mp_util_is_landscape()) {
		favor_layout = view->player_view_layout;	/*favor_layout = view->player_view_option_layout;*/
	} else {
		favor_layout = view->player_view_option_layout;
	}
	MP_CHECK(favor_layout);

	/*add focused UI*/
	Evas_Object *volume_focus_btn = elm_button_add(view->player_view_option_layout);
	elm_object_style_set(volume_focus_btn, "focus");
	elm_object_part_content_set(view->player_view_option_layout, "volume_focus", volume_focus_btn);
	elm_object_focus_custom_chain_append(view->player_view_option_layout, volume_focus_btn, NULL);
	evas_object_smart_callback_add(volume_focus_btn, "clicked", _mp_player_view_volume_btn_clicked_cb, view);
	elm_object_focus_set(volume_focus_btn, true);
#ifndef MP_SOUND_PLAYER
	Evas_Object *favor_focus_btn = elm_button_add(favor_layout);
	evas_object_repeat_events_set(favor_focus_btn, EINA_FALSE);
	elm_object_style_set(favor_focus_btn, "focus");
	elm_object_part_content_set(favor_layout, "favourite_focus", favor_focus_btn);
	elm_object_focus_custom_chain_append(favor_layout, favor_focus_btn, NULL);
	evas_object_smart_callback_add(favor_focus_btn, "clicked", _mp_player_view_favor_btn_clicked_cb, view);

	Evas_Object *repeat_focus_btn = elm_button_add(view->player_view_control_layout);
	elm_object_style_set(repeat_focus_btn, "focus");
	elm_object_part_content_set(view->player_view_control_layout, "repeat_focus", repeat_focus_btn);
	elm_object_focus_custom_chain_append(view->player_view_control_layout, repeat_focus_btn, NULL);
	evas_object_smart_callback_add(repeat_focus_btn, "clicked", _mp_player_view_repeat_btn_clicked_cb, view);

	Evas_Object *shuffle_focus_btn = elm_button_add(view->player_view_control_layout);
	elm_object_style_set(shuffle_focus_btn, "focus");
	elm_object_part_content_set(view->player_view_control_layout, "shuffle_focus", shuffle_focus_btn);
	elm_object_focus_custom_chain_append(view->player_view_control_layout, shuffle_focus_btn, NULL);
	evas_object_smart_callback_add(shuffle_focus_btn, "clicked", _mp_player_view_shuffle_btn_clicked_cb, view);

	Evas_Object *prev_focus_btn = elm_button_add(view->player_view_control_layout);
	elm_object_style_set(prev_focus_btn, "focus");
	elm_object_part_content_set(view->player_view_control_layout, "previous_focus", prev_focus_btn);
	elm_object_focus_custom_chain_append(view->player_view_control_layout, prev_focus_btn, NULL);
	evas_object_smart_callback_add(prev_focus_btn, "pressed", _mp_player_view_prev_btn_pressed_cb, view);
	evas_object_smart_callback_add(prev_focus_btn, "unpressed", _mp_player_view_prev_btn_unpressed_cb, view);
	evas_object_smart_callback_add(prev_focus_btn, "clicked", _mp_player_view_prev_btn_clicked_cb, view);
#endif

	Evas_Object *play_pause_focus_btn = elm_button_add(view->player_view_control_layout);
	elm_object_style_set(play_pause_focus_btn, "focus");
	elm_object_part_content_set(view->player_view_control_layout, "play_pause_focus", play_pause_focus_btn);
	/*elm_object_focus_custom_chain_append(view->player_view_control_layout, play_pause_focus_btn, NULL);*/
	evas_object_smart_callback_add(play_pause_focus_btn, "clicked", _mp_player_view_play_pause_btn_clicked_cb, view);

#ifndef MP_SOUND_PLAYER
	Evas_Object *next_focus_btn = elm_button_add(view->player_view_control_layout);
	elm_object_style_set(next_focus_btn, "focus");
	elm_object_part_content_set(view->player_view_control_layout, "next_focus", next_focus_btn);
	elm_object_focus_custom_chain_append(view->player_view_control_layout, next_focus_btn, NULL);
	evas_object_smart_callback_add(next_focus_btn, "pressed", _mp_player_view_next_btn_pressed_cb, view);
	evas_object_smart_callback_add(next_focus_btn, "unpressed", _mp_player_view_next_btn_unpressed_cb, view);
	evas_object_smart_callback_add(next_focus_btn, "clicked", _mp_player_view_next_btn_clicked_cb, view);

	Evas_Object *queue_focus_btn = elm_button_add(view->player_view_option_layout);
	elm_object_style_set(queue_focus_btn, "focus");
	elm_object_part_content_set(view->player_view_option_layout, "queue_focus", queue_focus_btn);
	elm_object_focus_custom_chain_append(view->player_view_option_layout, queue_focus_btn, NULL);
	evas_object_smart_callback_add(queue_focus_btn, "clicked", _mp_player_view_queue_btn_clicked_cb, view);

	/*set sequence*/
	elm_object_focus_next_object_set(shuffle_focus_btn, favor_focus_btn, ELM_FOCUS_RIGHT);
	elm_object_focus_next_object_set(favor_focus_btn, repeat_focus_btn, ELM_FOCUS_RIGHT);

	elm_object_focus_next_object_set(repeat_focus_btn, favor_focus_btn, ELM_FOCUS_LEFT);
	elm_object_focus_next_object_set(favor_focus_btn, shuffle_focus_btn, ELM_FOCUS_LEFT);

	elm_object_focus_next_object_set(shuffle_focus_btn, volume_focus_btn, ELM_FOCUS_DOWN);

	elm_object_focus_next_object_set(repeat_focus_btn, queue_focus_btn, ELM_FOCUS_DOWN);
#endif
}
MpPlayerView_t *mp_player_view_create(Evas_Object *parent, int launch_type, bool start_new_file)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpPlayerView_t *view = calloc(1, sizeof(MpPlayerView_t));
	MP_CHECK_NULL(view);

	ret = _mp_player_view_init(parent, view);
	if (ret) goto Error;

	view->launch_type = launch_type;
	view->start_new_file = start_new_file;
	_mp_player_view_content_layout_load(view);

	mp_volume_key_event_callback_add(_mp_player_view_volume_hw_key_cb, view);

	return view;

Error:
	ERROR_TRACE("Error: mp_player_view_create()");
	IF_FREE(view);
	return NULL;
}

bool mp_player_view_refresh(void *data)
{
	startfunc;

	MpPlayerView_t *playing_view = (MpPlayerView_t *)data;
	MP_CHECK_FALSE(playing_view);
	CHECK_VIEW(playing_view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);
	MP_CHECK_FALSE(!ad->is_lcd_off);

	if (playing_view->transition_state) {
		ERROR_TRACE("transition state");
		return false;
	}
	mp_player_view_set_title_and_buttons(playing_view);
#ifndef MP_SOUND_PLAYER
#ifdef MP_FEATURE_SUGGEST_FOR_YOU
	if (!mp_util_is_landscape()) {
		if (playing_view->queue_list)
			mp_now_playing_list_refresh((MpNowPlayingList_t *)playing_view->queue_list);
		if (playing_view->queue_status)
			_mp_player_view_refresh_queue_list(playing_view);
	}
		#else
		if (playing_view->queue_list) {
			mp_now_playing_list_refresh((MpNowPlayingList_t *)playing_view->queue_list);
		}
		if (playing_view->queue_status) {
			_mp_player_view_refresh_queue_list(playing_view);
		}
			/*_mp_player_view_create_queue_list(playing_view);*/
	#endif
#endif
	_mp_player_view_set_album_image(playing_view);
	_mp_player_view_set_content_info_icon(playing_view);
	_mp_player_view_init_progress_bar(playing_view);

	_mp_player_view_show_lyric(playing_view);

	if (ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED)
		mp_player_view_update_state(playing_view);

	/*add favourite begin*/
	bool favourite = FALSE;
	mp_plst_item *item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_FALSE(item);
	mp_media_info_h favourite_handle = NULL;
	mp_media_info_create(&favourite_handle, item->uid);
	mp_media_info_get_favorite(favourite_handle, &favourite);
	_mp_player_view_set_favourite_image(playing_view, favourite);
	mp_media_info_destroy(favourite_handle);
	favourite_handle = NULL;
	/*add favourite end */

	_mp_player_view_create_next_album_image(playing_view, 0);

	endfunc;
	return true;
}

void mp_player_view_update_state(void *data)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	if (ad->is_lcd_off) return;

	MpPlayerView_t *view = data;
	if (!view) return;

	CHECK_VIEW(view);

	if (view->transition_state)
		return;

	if ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PLAYING) {
		mp_player_view_update_buffering_progress(view, 100);
		mp_player_view_set_play_image(view, true);
		/*mp_player_view_set_album_playing(view, true);*/
	} else if ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_READY) {
		/*don't update play/puase button if next track exist.*/
		if (!ad->auto_next) {
			mp_player_view_set_play_image(view, false);
			/*mp_player_view_set_album_playing(view, false);*/
		}
	} else {
		mp_player_view_set_play_image(view, false);
		/*mp_player_view_set_album_playing(view, false);*/
	}

#ifndef MP_SOUND_PLAYER
	_mp_player_view_set_rep_image(view, mp_playlist_mgr_get_repeat(ad->playlist_mgr));
	_mp_player_view_set_shuffle_image(view, mp_playlist_mgr_get_shuffle(ad->playlist_mgr));
#endif
	_mp_player_view_volume_update(view);

	_mp_player_view_update_snd_button_state(view);

	endfunc;
}

void
mp_player_view_update_buffering_progress(void *data, int percent)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(!ad->is_lcd_off);

	MpPlayerView_t *view = data;
	MP_CHECK(view); CHECK_VIEW(view);

	if (percent < 0 || percent >= 100) {
		mp_evas_object_del(view->buffering_progress);
		return;
	}

	if (!view->buffering_progress) {
		view->buffering_progress = mp_widget_loading_icon_add(view->player_view_layout, MP_LOADING_ICON_SIZE_LARGE);
		elm_object_part_content_set(view->player_view_layout, "buffering_area", view->buffering_progress);
	}

	evas_object_show(view->buffering_progress);
}

void mp_player_view_set_data(MpPlayerView_t *view, ...)
{
	startfunc;
	MP_CHECK(view);

	va_list var_args;
	int field;

	va_start(var_args, view);
	do {
		field = va_arg(var_args, int);
		switch (field) {

		case MP_PLAYER_VIEW_LAUNCH_TYPE:
			{
				int val = va_arg((var_args), int);

				view->launch_type = val;
				DEBUG_TRACE("view->launch_type = %d", view->launch_type);
				break;
			}
		default:
			DEBUG_TRACE("Invalid arguments");
		}

	}
	while (field >= 0);

	va_end(var_args);
}




#ifdef MP_FEATURE_SUGGEST_FOR_YOU
#ifndef MP_SOUND_PLAYER

static Evas_Object *_mp_player_view_gengrid_content_get(void *data, Evas_Object *obj, const char *part)
{
	eventfunc;
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = item->handle;
	mp_retvm_if (!track, NULL, "data is null");
	char *thumbpath = NULL;
	if (!g_strcmp0(part, "albumart")) {
		Evas_Object *icon = NULL;
		mp_media_info_get_thumbnail_path(track, &thumbpath);
		icon = mp_util_create_thumb_icon(obj, thumbpath, SUGGEST_TRACK_THUMBNAIL_SIZE, SUGGEST_TRACK_THUMBNAIL_SIZE);
		evas_object_show(icon);
		return icon;
	}
	return NULL;

}
static char *_mp_player_view_gengrid_text_get(void *data, Evas_Object *obj, const char *part)
{

	eventfunc;
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = (mp_media_info_h) (item->handle);
	mp_retvm_if (!track, NULL, "data is null");

	char *title = NULL;
	char *artist = NULL;
	char *album = NULL;
	mp_media_info_get_title(track, &title);
	mp_media_info_get_artist(track, &artist);
	mp_media_info_get_album(track, &album);

	if (!strcmp(part, "elm.text.1")) {
		mp_retv_if (!title, NULL);
		return g_strdup(title);
	} else if (!strcmp(part, "elm.text.2")) {
		mp_retv_if (!artist, NULL);
		return g_strdup(artist);
	} else if (!strcmp(part, "elm.text.3")) {
		mp_retv_if (!album, NULL);
		return g_strdup(album);
	}
	return NULL;

}

static void _mp_player_view_gengrid_item_del_cb(void *data, Evas_Object *obj)
{
	DEBUG_TRACE_FUNC();
	mp_list_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static void _mp_player_view_genrid_create(MpPlayerView_t *thiz)
{
	eventfunc;
	MpPlayerView_t *playing_view = thiz;
	MP_CHECK(playing_view);
	MP_CHECK(playing_view->player_view_layout);


	/*create new genlist*/
	Evas_Object *content = elm_object_part_content_unset(playing_view->player_view_layout, "suggest_song_area");
	mp_evas_object_del(content);
	playing_view->gengrid = elm_gengrid_add(playing_view->player_view_layout);
	MP_CHECK(playing_view->gengrid);

	elm_gengrid_item_size_set(playing_view->gengrid, SUGGEST_GENGRID_ITEM_WIDTH, SUGGEST_GENGRID_ITEM_HEIGHT);
	elm_gengrid_horizontal_set(playing_view->gengrid, TRUE);
	evas_object_size_hint_weight_set(playing_view->gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(playing_view->gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_scroller_policy_set(playing_view->gengrid, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	evas_object_show(playing_view->gengrid);

	if (playing_view->gic == NULL) {
		playing_view->gic = elm_gengrid_item_class_new();
		playing_view->gic->item_style = "music/cast_side";
		playing_view->gic->func.text_get = _mp_player_view_gengrid_text_get;
		playing_view->gic->func.content_get = _mp_player_view_gengrid_content_get;
		playing_view->gic->func.state_get = NULL;
		playing_view->gic->func.del = _mp_player_view_gengrid_item_del_cb;
	}
	elm_object_part_content_set(playing_view->player_view_layout, "suggest_song_area", playing_view->gengrid);
	evas_object_data_set(playing_view->gengrid, "view_data", playing_view);
	eventfunc;
}




static void _mp_player_view_gengrid_item_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(obj);
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK(item);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_plst_item *plst_item = NULL;
	mp_plst_item *to_play = NULL;
	Elm_Object_Item *gli2 = NULL;
	int index = 0;

	if (!ad->playlist_mgr)
		mp_common_create_playlist_mgr();

	mp_playlist_mgr_clear(ad->playlist_mgr);
	/* creat new playlist */
	gli2 = elm_gengrid_first_item_get(obj);
	while (gli2) {

		mp_list_item_data_t *item_data = elm_object_item_data_get(gli2);
		char *uri = NULL;
		char *uid = NULL;
		char *title = NULL;
		char *artist = NULL;

		mp_track_type track_type = MP_TRACK_URI;
		mp_media_info_get_media_id(item_data->handle, &uid);
		mp_media_info_get_file_path(item_data->handle, &uri);
		mp_media_info_get_title(item_data->handle, &title);
		mp_media_info_get_artist(item_data->handle, &artist);
		plst_item = mp_playlist_mgr_item_append(ad->playlist_mgr, uri, uid, title, artist, track_type);
		if (item->index == index)
			to_play = plst_item;
		index++;

		gli2 = elm_gengrid_item_next_get(gli2);
	}

	MP_CHECK(to_play);
	/*play the current track*/
	mp_playlist_mgr_set_current(ad->playlist_mgr, to_play);
	mp_play_destory(ad);
	ad->paused_by_user = FALSE;
	int ret = mp_play_new_file(ad, TRUE);
	if (ret) {
		ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK)
			mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
#endif
		return;
	}

	MpPlayerView_t *player_view = (MpPlayerView_t *)GET_PLAYER_VIEW;
	if (player_view) {
		mp_player_view_refresh(player_view);
	}
	return;
}

static void _mp_player_view_suggestion_album_load(void *thiz)
{
	eventfunc;
	MpPlayerView_t *playing_view = (MpPlayerView_t *)thiz;
	MP_CHECK(playing_view);
	CHECK_VIEW(playing_view);
	MP_CHECK(playing_view->player_view_layout);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	mp_plst_item *cur = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(cur);
	MP_CHECK(cur->uri);



	/*There are 3 method to get the suggestions and has its own priority
	1. use the same mood track (get data from the server DB)
	2. get the same genre track (get data from the other DB)
	3. random track method
	*/

	 _mp_player_mood_method_get_data(playing_view, cur->uri);
	if (g_list_length(playing_view->suggest_list) < SUGGEST_TRACK_MIN_COUNT) {
		DEBUG_TRACE("MOOD method is invalid");
		g_list_free(playing_view->suggest_list);
		playing_view->suggest_list = NULL;
		_mp_player_genre_method_get_data(playing_view, cur->uri);
		if (g_list_length(playing_view->suggest_list) < SUGGEST_TRACK_MIN_COUNT) {
			DEBUG_TRACE("GENRE method is invalid");
			g_list_free(playing_view->suggest_list);
			playing_view->suggest_list = NULL;
			_mp_player_random_method_get_data(playing_view, cur->uri);
		}
	}

	_mp_player_view_suggestion_album_append(playing_view, cur->uri);
	mp_util_domain_translatable_part_text_set(playing_view->player_view_layout, "suggest_album_text", STR_MP_SUGGESTION_STR);
}

static void _mp_player_mood_method_get_data(MpPlayerView_t *view, char *path)
{
	MP_CHECK(view);
	MP_CHECK(path);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_db_client_mgr_create(ad);
	MP_CHECK(ad->dbc_mgr);

	/*get current song mood*/
	int mood = 0;
	mp_db_client_get_mood_by_path(ad->dbc_mgr, &mood, path);
	ERROR_TRACE("wishjox mood:%d", mood);

	/*get the same mood song list*/
	mp_db_client_get_paths_by_mood(ad->dbc_mgr, mood);
	view->suggest_list = ad->dbc_mgr->path_list;
	DEBUG_TRACE("length is %d", g_list_length(view->suggest_list));
	mp_db_client_mgr_destory(ad->dbc_mgr);

}

static void _mp_player_random_method_get_data(MpPlayerView_t *view, char *cur_path)
{
	MP_CHECK(view);
	MP_CHECK(cur_path);
	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	DEBUG_TRACE("count is %d", count);

	GList *sel_list = NULL;
	int prev_index = 0;
	if (count  == 1) {
		sel_list = NULL;
	} else {
		mp_media_list_h svc_handle = NULL;
		mp_media_info_list_create(&svc_handle, MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, count);
		MP_CHECK(svc_handle);
		int index = 0;
		for (index = 0; index < SUGGEST_TRACK_RANDOM_COUNT; index++) {
			int offset = rand() % count;
			char *path = NULL;
			DEBUG_TRACE("Offset is %d", offset);

			mp_media_info_h item = NULL;
			item = mp_media_info_list_nth_item(svc_handle, offset);
			MP_CHECK(item);
			mp_media_info_get_file_path(item, &path);
			if ((!strcmp(path, cur_path)) || (path == NULL) || (offset == prev_index)) {
				continue;
			}
			prev_index = offset;
			sel_list = g_list_append(sel_list, path);

		}

	}
	view->suggest_list = sel_list;
}

static void _mp_player_genre_method_get_data(MpPlayerView_t *view, char *path)
{
	MP_CHECK(view);
	/*get the current track genre*/
	GList *sel_list = NULL;
	mp_media_info_h media_info = NULL;
	char *genre = NULL;
	mp_media_info_create_by_path(&media_info, path);
	mp_media_info_get_genre(media_info, &genre);
	view->genre_str = genre;
	int count = 0;
	int res = mp_media_info_list_count(MP_TRACK_BY_GENRE, view->genre_str, NULL, NULL, -1, &count);
	MP_CHECK(res == 0);
	DEBUG_TRACE("count is %d", count);


	/*get data from DB*/
	mp_media_list_h svc_handle = NULL;
	mp_media_info_list_create(&svc_handle, MP_TRACK_BY_GENRE, view->genre_str, NULL, NULL, 0, 0, count);
	MP_CHECK(svc_handle);

	count = count > (SUGGEST_TRACK_MAX_COUNT+1) ? (SUGGEST_TRACK_MAX_COUNT+1) : count;
	view->suggest_count = count;
	int index = 0;
	for (index = 0; index < view->suggest_count; index++) {
			char *path = NULL;
			mp_media_info_h item = NULL;
			item = mp_media_info_list_nth_item(svc_handle, index);
			MP_CHECK(item);
			mp_media_info_get_file_path(item, &path);
			sel_list = g_list_append(sel_list, path);

	}
	view->suggest_list  = sel_list;

}

static void _mp_player_view_suggestion_album_append(MpPlayerView_t *view, char *cur_path)
{
	MP_CHECK(view);
	MP_CHECK(cur_path);
	/*clear layout*/
	Evas_Object *no_item = elm_object_part_content_unset(view->player_view_layout, "suggest_default_area");
	Evas_Object *item_image = elm_object_part_content_unset(view->player_view_layout, "suggest_song_area");
	mp_evas_object_del(no_item);
	mp_evas_object_del(item_image);
	edje_object_signal_emit(elm_layout_edje_get(view->player_view_layout), "elm,state,text,hidden", "*");

	/*no suggestion album case*/
	if ((!view->suggest_list) || (g_list_length(view->suggest_list) < SUGGEST_TRACK_MIN_COUNT)) {
		Evas_Object *icon = NULL;
		icon = elm_icon_add(view->player_view_layout);
		MP_CHECK(icon);
		elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_NO_ITEM);
		elm_object_part_content_set(view->player_view_layout, "suggest_default_area", icon);
		mp_util_domain_translatable_part_text_set(view->player_view_layout, "help_text", STR_MP_AFTER_YOU_DOWNLOAD_TRACKS);
		edje_object_signal_emit(elm_layout_edje_get(view->player_view_layout), "elm,state,text,shown", "*");
	} else {
		/*create gengrid*/
		_mp_player_view_genrid_create(view);
		MP_CHECK(view->gengrid);
		MP_CHECK(view->gic);
		int index = 0;
		char *path = NULL;

		GList *node = g_list_first(view->suggest_list);
		while (node) {
			path = node->data;
			node = g_list_next(node);
			mp_media_info_h item = NULL;
			mp_media_info_create_by_path(&item, path);

			/*remove the same track or path not exist*/
			if ((!strcmp(path, cur_path)) || (path == NULL) || (!item)) {
				continue;
			}

			mp_list_item_data_t *item_data = calloc(1, sizeof(mp_list_item_data_t));
			MP_CHECK(item_data);
			item_data->handle = item;
			item_data->index = index++;
			item_data->it =  elm_gengrid_item_append(view->gengrid, view->gic, item_data,
								_mp_player_view_gengrid_item_select_cb, item_data);
		}
	}
}


static void _mp_player_view_current_track_info_set(MpPlayerView_t *view)
{
	MP_CHECK(view);

	char *title = NULL;
	char *artist = NULL;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	title = g_strdup_printf("%s", (ad->current_track_info && ad->current_track_info->title ? ad->current_track_info->title : ""));
	artist = g_strdup_printf("%s", (ad->current_track_info && ad->current_track_info->artist ? ad->current_track_info->artist : ""));
	elm_object_part_text_set(view->player_view_layout, "artist_name", artist);

	char *text_format = "<align=center><font_size=%d><color=#%s><color_class=%s>%s</color_class></color></font_size></align>";
	char *text = g_strdup_printf(text_format, 28, "FFFFFFFF", "ATO022", title);
	view->label = mp_widget_slide_title_create(view->player_view_layout, "slide_roll", text);

	if (view->label) {
		elm_object_part_content_set(view->player_view_layout, "song_name", view->label);
		if (ad->is_lcd_off)
			elm_label_slide_mode_set(view->label, ELM_LABEL_SLIDE_MODE_NONE);
		else
			elm_label_slide_mode_set(view->label, ELM_LABEL_SLIDE_MODE_AUTO);
		elm_label_slide_go(view->label);
	}

	IF_G_FREE(title);
	IF_G_FREE(artist);
	IF_G_FREE(text);
}
#endif
#endif

