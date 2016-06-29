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


#ifndef __DEF_music_H_
#define __DEF_music_H_

#include <Elementary.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <glib.h>
#include <notification.h>
#include <Ecore_IMF.h>
//#include <Ecore_X.h>
#include <Edje.h>
#include <errno.h>
#include <libintl.h>
#include <app.h>

#include <sys/times.h>
#include <storage/storage.h>
#include <sound_manager.h>
#include "mp-define.h"
#include "mp-resource.h"
#include "mp-player-debug.h"

#include "mp-define.h"
#include "mp-playlist-mgr.h"

#include "mp-http-mgr.h"
#include "mp-ta.h"
#include "mp-language-mgr.h"
#include "mp-media-info.h"

#include "mp-lyric.h"
#include "mp-widget.h"

#include "mp-view-mgr.h"
#include "mp-popup.h"

//E_DBus_Connection *EDBusHandle;

typedef struct
{

	/* controlbar tab item */
	Elm_Object_Item *ctltab_songs;
	Elm_Object_Item *ctltab_plist;
	Elm_Object_Item *ctltab_album;
	Elm_Object_Item *ctltab_artist;
	Elm_Object_Item *ctltab_genres;
	Elm_Object_Item *ctltab_year;
	Elm_Object_Item *ctltab_composer;
	Elm_Object_Item *ctltab_folder;

	bool allshare;

	bool first_append;
} mp_library;

#define MP_VIEW_DATA_MAGIC	0x801211aa
#define MP_SET_VIEW_DATA_MAGIC(view_data)	((view_data_t *)view_data)->magic = MP_VIEW_DATA_MAGIC
#define MP_CHECK_VIEW_DATA(view_data)	\
do {                                                  \
	if (((view_data_t *)view_data)->magic != MP_VIEW_DATA_MAGIC) {        \
		ERROR_TRACE("\n###########      ERROR   CHECK  #############\nPARAM is not view_data\n###########      ERROR   CHECK  #############\n"); \
		mp_assert(FALSE);}            \
} while (0)


typedef struct
{
	Evas_Object *layout;
	void *EvasPlugin;
	Evas_Object *box;
	Evas_Object *dali_obj;

	Ecore_Timer *mouse_up_timer;
	Ecore_Timer *now_playing_timer;

	Evas_Object *track_list;
	Evas_Object *track_genlist;
	bool show_track_list;

	int        track_count;

	Evas_Object *now_playing;
	Evas_Object *now_playing_icon;
	Evas_Object *all_tracks;
	bool all_tracks_click;
	Evas_Object* ctxpopup;
	int all_tracks_type;

	Evas_Object *back_button;

	int now_playing_album_seq;

	char *cur_artist;
	char *cur_album;

	Ecore_Job *refresh_job;

	struct appdata *ad;
} mp_coverflow_view;

typedef struct
{
	char *uri;
	char *title;
	char *artist;
	char *album;
	char *date;
	char *genre;
	char *location;
	char *format;
	char *media_id;

	int duration;

	char *thumbnail_path;
	char *copyright;

	char *author;
	char *track_num;
	char *year;
	bool favorite;
	int playlist_member_id;

	bool isDiffAP;

	mp_track_type track_type;
}mp_track_info_t;

enum
{
	MP_CREATE_PLAYLIST_MODE_NONE,
	MP_CREATE_PLAYLIST_MODE_NEW,
	MP_CREATE_PLAYLIST_MODE_WITHMUSICS,
	MP_CREATE_PLAYLIST_MODE_SAVEAS,
	MP_CREATE_PLAYLIST_MODE_SWEEP
};

typedef enum
{
	MP_LAUNCH_DEFAULT = 0,	//normal case
	MP_LAUNCH_BY_PATH,		//ug case
	MP_LAUNCH_ADD_TO_HOME,	//add to home
	MP_LAUNCH_PLAY_RECENT,
	MP_LAUNCH_LIVE_BOX,

	MP_LAUNCH_PLAY_FILES,	//play recent
	MP_LAUNCH_PLAY_PLAYLIST,

} mp_launch_type;

typedef enum
{
	LOAD_DEFAULT,
	LOAD_TRACK,		//load by path
	LOAD_GROUP,		//load by shortcut
	LOAD_PLAYLIST,		//load by shortcut
	LOAD_REQUSET_TYPE,
	LOAD_MM_KEY,

} mp_load_type;

typedef enum
{
	MP_SPLIT_VIEW_TYPE_NORMAL = 0,
	MP_SPLIT_VIEW_TYPE_FULL,
} mp_split_view_type;

typedef struct mp_split_view
{
	Evas_Object *layout;
	Evas_Object *left_layout;
	Evas_Object *right_layout;
	Evas_Object *list;
	Evas_Object *fast_index;
	mp_split_view_type current_split_view_type;
	Ecore_Timer *idle_timer;
	Ecore_Idler *idle_idler;
} mp_split_view;

typedef enum
{
	PLAY_STATE_NONE,
	PLAY_STATE_CREATED,
	PLAY_STATE_PREPARING,
	PLAY_STATE_READY,
	PLAY_STATE_PLAYING,
	PLAY_STATE_PAUSED,
} mp_player_state;

typedef enum {
	MP_SEND_TYPE_MESSAGE,
	MP_SEND_TYPE_EMAIL,
	MP_SEND_TYPE_BLUETOOTH,
	MP_SEND_TYPE_WIFI,
	MP_SEND_TYPE_NFC,
	MP_SEND_TYPE_NUM,
} mp_send_type_e;

typedef enum {
	MP_MORE_BUTTON_TYPE_DEFAULT,
	MP_MORE_BUTTON_TYPE_TRACK_LIST,
	MP_MORE_BUTTON_TYPE_MAX,
} mp_more_button_type_e;

typedef enum {
	MP_DEVICE_TYPE_UNKNOWN,
	MP_DEVICE_TYPE_MY_DEVICE,
	MP_DEVICE_TYPE_PHONE,
	MP_DEVICE_TYPE_DESKTOP_PC,
	MP_DEVICE_TYPE_NOTE_PC,
	MP_DEVICE_TYPE_TABLET,
	MP_DEVICE_TYPE_TV,
} mp_device_type_e;

struct appdata
{

	Evas *evas;
	Evas_Object *win_main;
	Evas_Object *bg;
	Evas_Object *popup[MP_POPUP_MAX];
	int win_angle;
	int del_cb_invoked;

	/* App control parameters */
	bool exit_status;
	bool disable_detail_view;

#if 0
	Ecore_X_Window xwin;
#else
	void *xwin;
#endif

#ifdef MP_FEATURE_EXIT_ON_BACK
	Ecore_Event_Handler *callerWinEventHandler;
	unsigned int caller_win_id;
#endif

	/* Layout for each view */
	Evas_Object *conformant;
	Evas_Object *naviframe;

	bool show_optional_menu;
	double music_pos;
	double music_length;
	Ecore_Timer *progressbar_timer;
	int start_pos;
	bool is_sdcard_removed;

	// for Plalying Control
	bool can_play_drm_contents;
	bool show_now_playing;
	bool start_after_effect;

	mp_player_state player_state;

	//mp_drm drm_info;

	mp_plst_mgr *playlist_mgr;

	mp_track_type_e track_type;	// Support voice ui
	mp_group_type_e group_type;	// Support voice ui

	MpViewMgr_t *view_manager;

	sound_stream_info_h stream_info;//support sound_manager

	bool create_view_on_play;
	Evas_Object *preload_player_view;
	Ecore_Idler *create_on_play_lay_idler;
	Ecore_Timer *play_after_transit_timer;
	Ecore_File_Monitor *monitor;

	notification_h noti;

	Evas_Object *editfiled_new_playlist;
	Evas_Object *editfiled_entry;
	char *new_playlist_name;

	struct {
		bool  downed;
        	bool  moving;
		Evas_Coord sx;
		Evas_Coord sy;
	} mouse;

	Evas_Object *radio_group;

	Evas_Object *popup_delete;

	bool b_search_mode;
	Evas_Object *isf_entry;
	Evas_Object *editfield;

	bool freeze_indicator_icon;	//set it true to prevent flickering play icon of indicator.

	int ear_key_press_cnt;
	Ecore_Timer *ear_key_timer;
	unsigned int press_time;

#ifdef MP_FEATURE_LOCKSCREEN
	Evas_Object *win_lockmini;    //lockscreen mini
	Evas_Object *lockmini_layout;
	Evas_Object *lockmini_progress_box;
	Evas_Object *lockmini_progress_layout;
	Evas_Object *lockmini_progress_bar;
	Ecore_Timer *lockmini_progress_timer;
	Ecore_Timer *lockmini_button_timer;
	bool lockmini_visible;
	bool b_lockmini_show;
	bool progress_dragging;
#endif
	Evas_Object *win_minicon;
	Evas_Object *minicontroller_layout;
        Evas_Object *progress_box;
        Evas_Object *progress_layout;
        Evas_Object *progress_bar;
	Evas_Object *minicon_icon;
	Ecore_Timer *minicon_progress_timer;
        Ecore_Timer *minicon_button_timer;
	bool b_minicontroller_show;
	bool minicon_visible;
	int quickpanel_angle;

	mp_split_view *split_view;
	double latest_moved_left_size;

	int current_appcore_rm;
	mp_screen_mode screen_mode;

	int screen_height;	//current screen height
	int screen_width;	//current screen width

	bool paused_by_user;

	mp_http_mgr_t *http_mgr;

	bool app_is_foreground;	// relaunch only available when music is in pause state
	bool is_lcd_off;

	bool is_focus_out;	// update minicontroller in bgm mode.

	mp_snd_path snd_path;	// indicate sound path;

	Ecore_Event_Handler *key_down;
	Ecore_Event_Handler *key_up;
	Ecore_Event_Handler *mouse_button_down;
	Ecore_Event_Handler *focus_in;
	Ecore_Event_Handler *focus_out;
	Ecore_Event_Handler *visibility_change;
	Ecore_Event_Handler *client_msg;
	Ecore_Event_Handler *mouse_button_up;
	Ecore_Event_Handler *mouse_move;
	Ecore_Event_Handler *property;
	Ecore_Event_Handler *hold;

	int motion_handle;

	Evas_Object *info_ug_base;	//if thist is not null, info ug is visible. use this to determine info view is exist or not.
	Evas_Object *info_ug_layout; //do not del this object. if it is deleted, info ug layout will not be displayed properly.
	bool info_click_flag;     //flag for click info button in play view
	bool info_back_play_view_flag;     //flag for info view back  play view

	bool edit_in_progress;	// don't update view in inotify callback while delete operation.

	Ecore_Timer *volume_down_timer;
	Ecore_Timer *mute_popup_show_timer;	//timer for showing mute popup
	bool volume_long_pressed;
	bool mute_flag;		//flag for mute

	double ff_rew_distance;		// for ff and rew when there is no play view

	bool music_setting_change_flag;	//true for change music setting data, false for not

	bool load_play_view; /*set it true if play view must be displayed directly when app is launching*/

	bool is_Longpress;
	bool is_ff;

	Ecore_Idler *setting_idler;
	Ecore_Idler *app_init_idler;
	Ecore_Idler *playview_show_idler;

	Ecore_Timer *longpress_timer;
	Ecore_Timer *live_pos_timer;
	Ecore_Timer *duration_change_timer;

	int b_show_lyric;
	bool vertical_scroll;

	char *shortcut_descrition;

	Ecore_Animator *minfo_ani;
	GList *minfo_list;
	Evas_Object *minfo_genlist;

	bool direct_win_minimize;

	int album_image_w;
	int album_image_h;

	bool paused_by_other_player;

	mp_track_info_t *current_track_info;

#ifdef MP_FEATURE_APP_IN_APP
	bool mini_player_mode;
	int mini_player_current_size;
	Evas_Object *win_mini;
	Evas_Object *mini_layout;
	Evas_Object *title;
	Evas_Object *event_box;

	int click_count;
	Ecore_Timer *click_timer;
	Ecore_Timer *switch_timer;
#endif

#ifdef MP_3D_FEATURE
	mp_coverflow_view *coverflow_view;
#endif

#ifdef MP_FEATURE_AUTO_OFF
	Ecore_Timer *auto_off_timer;
	Ecore_Timer *pause_off_timer;
#endif

#ifdef MP_FEATURE_DESKTOP_MODE
	bool desktop_mode;
#endif


	Evas_Object *more_btn_popup;
	mp_more_button_type_e more_btn_type;

	bool sip_state;

	//to check recording state before play next song
	//0: stop
	//1: start
	bool auto_next;
	bool auto_resume;
	bool camcoder_start;
	bool resume_on_cam_end;

	Ecore_Timer *sleep_unlock_timer;
	bool sleep_locked;

#ifdef MP_SOUND_PLAYER
	char *cookie;
	char *proxy;
#endif

	Ecore_Job *exit_job;

	bool store_enable;
        bool mirror_to_local;

	int samsung_link;
	int disable_change_player;

	Ecore_Timer *app_control_check_timer;

	bool prepare_by_init; //to indicate if player created and prepared to ready state  in init idler
	int externalStorageId;
	app_event_low_battery_status_e low_battery_status;
};


typedef void (*mpOptCallBack) (void *, Evas_Object *, void *);

typedef struct
{
	const char *name;
	mpOptCallBack cb;
} MpOptItemType;

typedef struct
{
	MpOptItemType *l_opt;
	MpOptItemType *m_opt;
	MpOptItemType *r_opt;
} MpOptGroupType;

#include "mp-util.h"

#endif /* __DEF_music_H__ */
