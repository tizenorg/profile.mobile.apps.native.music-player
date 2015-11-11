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

#ifndef __MP_VIEW_H__
#define __MP_VIEW_H__

#include <Elementary.h>
#include <stdbool.h>
#include "mp-define.h"
#include "mp-menu.h"

enum {
	MP_VIEW_MOVE_NONE,
	MP_VIEW_MOVE_LEFT,
	MP_VIEW_MOVE_RIGHT,
};

typedef enum {
	MP_VIEW_ALL,
	MP_VIEW_ALBUM_DETAIL,
	MP_VIEW_ARTIST_DETAIL,
	MP_VIEW_FOLDER_DETAIL,
	MP_VIEW_PLAYLIST_DETAIL,
	MP_VIEW_GENRE_DETAIL,
	MP_VIEW_ALLSHARE_SEVER,
	MP_VIEW_ALLSHARE_DETAIL,
	MP_VIEW_SEARCH,
	MP_VIEW_SQUARE,
	MP_VIEW_FOLDER,
	MP_VIEW_SETTING,
	MP_VIEW_SET_AS,
	MP_VIEW_DETAIL,
	MP_VIEW_ALBUM_BOOKLET,
	MP_VIEW_ARTIST_BOOKLET,
	MP_VIEW_PLAYER,
	MP_VIEW_CREATE_PLAYLIT,
	MP_VIEW_NOW_PLAYING_LIST,
	MP_VIEW_ADD_TRACK,
	MP_VIEW_SELECT_TRACK,
	MP_VIEW_MAKE_OFFLINE,
	MP_VIEW_EDIT,
	MP_VIEW_MAX,
} MpViewType_e;

typedef enum {
	MP_UPDATE_NOW_PLAYING = 0, //0
	MP_UNSET_NOW_PLAYING,
	MP_START_PLAYBACK,
	MP_RESUME_PLAYBACK,
	MP_PAUSE_PLAYBACK,
	MP_PLAYING_TRACK_CHANGED,
	MP_STOP_PLAYBACK,
	MP_PLAYLIST_CREATED,
	MP_PLAYLIST_MODIFIED,
	MP_PLAYLIST_REORDER_DONE,
	MP_DOUBLE_TAP,
	MP_LCD_OFF,
	MP_LCD_ON,

	MP_UPDATE = 50,
	MP_ADD_TO_PLAYLIST_DONE,
	MP_DELETE_DONE,
	MP_POPUP_DELETE_DONE,
	MP_UPDATE_PLAYING_LIST,
	MP_SETTING_PLAYLIST_CHANGED,
	MP_PLAY_TIME_COUNT_UPDATED,
	MP_ROUTE_CHANGED,
	MP_DB_UPDATED,
#ifdef MP_FEATURE_LANDSCAPE
	MP_VIEW_ROTATE,
	MP_VIEW_ROTATE_START,	//to indicate rotation start
#endif
	MP_MMC_MOUNTED,
	MP_MMC_REMOVED,
	MP_LANG_CHANGED,
	MP_WIN_RESIZED,


	MP_SIP_STATE_CHANGED = 100,
	MP_VIEW_TRANSITION_REQUESTED,
	MP_VIEW_TRANSITION_FINISHED,
	MP_PLAYLIST_RENAMED,
	MP_PLAYLIST_IMAGE_UPDATED,
	MP_POPUP_CANCEL,
	MP_NETWORK_STATE_CHANGED,
	MP_UPDATE_FAVORITE_LIST,
	MP_PLAYLIST_MGR_ITEM_CHANGED,
#ifdef MP_FEATURE_PERSONAL_PAGE
	MP_PERSONAL_PAGE_OFF,
	MP_PERSONAL_PAGE_ON,
#endif
	MP_SIDE_SYNC_STATUS_CHANGED,
	MP_VIEW_EVENT_ALBUMART_CHANGED,

	MP_TABS_REORDER_DONE,
	MP_TABS_ITEM_CHANGED,
	MP_PLAYLISTS_REORDER_DONE,
	MP_REORDER_ENABLE,
	MP_REORDER_DISABLE,
	MP_LYRIC_UPDATE,
	MP_QUICKPANNEL_SHOW,
	MP_QUICKPANNEL_HIDE,

} MpViewEvent_e;

typedef enum {
	MP_OPTION_LEFT,
	MP_OPTION_MIDDLE,
	MP_OPTION_RIGHT,
	MP_OPTION_MORE,
	MP_OPTION_BACK,
	MP_OPTION_MAX,
} MpOptionType_e;

typedef enum {
	MP_TITLE_OPTION_SAVE,
	MP_TITLE_OPTION_MAX,
} MpTitleOption_e;

#define VIEW_MAGIC 0x37373700
#define LIST_VIEW_MAGIC 0x37373701

#define MP_STORE_MEMBER

#define INHERIT_MP_VIEW	\
	int view_magic;\
	Elm_Object_Item * navi_it;\
	Evas_Object *layout;\
	Evas_Object *inner_naviframe;\
	Evas_Object *nowplaying_bar;\
	Evas_Object *more_btn_ctxpopup; \
	Elm_Object_Item *toolbar_options[MP_OPTION_MAX];\
	Evas_Object *title_options[MP_TITLE_OPTION_MAX];\
	Evas_Object *selection_info;\
	Evas_Object *radio_btn;\
	MpViewType_e view_type;\
	Evas_Object *scroller;\
	bool disable_scroller;\
	bool paused;\
	bool rotate_flag;\
	bool disable_title_icon; \
	bool push_transition; \
	int cloud_view;\
	int (*update)(void *view);\
	int (*clear_options)(void *view);\
	int (*update_options)(void *view);\
	int (*update_options_edit)(void *view);\
	int (*set_title)(void *view, char *text_id);\
	int (*set_subtitle)(void *view, char *title);\
	int (*title_slide_go)(void *view);\
	int (*set_nowplaying)(void *view);\
	int (*unset_nowplaying)(void *view);\
	int (*update_nowplaying)(void *view, bool with_title);\
	int (*freeze_nowplaying)(void *view, int freeze);\
	int (*start_playback)(void *view);\
	int (*pause_playback)(void *view);\
	int (*stop_playback)(void *view);\
	void (*view_destroy_cb)(void *view); \
	void (*view_pause)(void *view);\
	void (*view_resume)(void *view);\
	void (*view_lcd_off)(void *view);\
	void (*view_lcd_on)(void *view);\
	void (*on_event)(void *view, MpViewEvent_e event);\
	int (*is_rotate_available)(void *view);\
	void (*rotate)(void *view, int randscape);\
	MP_STORE_MEMBER

typedef struct _mp_view {
	INHERIT_MP_VIEW
} MpView_t;

int mp_view_init(Evas_Object *parent, MpView_t *view, MpViewType_e view_type, ...);
int mp_view_fini(MpView_t *view);
EXPORT_API int mp_view_update(MpView_t *view);
int mp_view_update_options(MpView_t *view);
int mp_view_update_options_edit(MpView_t *view);
EXPORT_API int mp_view_set_title(MpView_t *view, char *title);
int mp_view_set_sub_title(MpView_t *view, char *title);
int mp_view_set_title_visible(MpView_t *view, int visible);
int mp_view_title_slide_go(MpView_t *view);
int mp_view_set_nowplaying(MpView_t *view);
int mp_view_unset_nowplaying(MpView_t *view);
int mp_view_update_nowplaying(MpView_t *view, bool with_title);
int mp_view_freeze_nowplaying(MpView_t *view, int freeze);
int mp_view_get_nowplaying_show_flag(MpView_t *view);
int mp_view_start_playback(MpView_t *view);
int mp_view_pause_playback(MpView_t *view);
int mp_view_stop_playback(MpView_t *view);
int mp_view_view_lcd_off(MpView_t *view);
int mp_view_view_lcd_on(MpView_t *view);
int mp_view_view_pause(MpView_t *view);
int mp_view_view_resume(MpView_t *view);
int mp_view_clear_options(MpView_t *view);
Evas_Object *mp_view_get_base_obj(MpView_t *view);
EXPORT_API int mp_view_on_event(MpView_t *view, MpViewEvent_e event);
#ifdef MP_FEATURE_LANDSCAPE
int mp_view_is_rotate_available(MpView_t *view);
int mp_view_rotate(MpView_t *view);
#endif
int mp_view_is_now_push_transit(MpView_t *view, bool *now_transit);

#endif

