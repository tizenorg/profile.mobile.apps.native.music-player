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

#ifndef __MP_PLAYER_VIEW_H_
#define __MP_PLAYER_VIEW_H_

#include "music.h"
#ifdef MP_FEATURE_MUSIC_VIEW
#include <cairo.h>
#endif

enum
{
	PLAYER_VIEW_MOVE_NONE,
	PLAYER_VIEW_MOVE_LEFT,
	PLAYER_VIEW_MOVE_RIGHT,
};

/*OPTION_VOLUME,*/
/*OPTION_INFO,*/
enum
{
	OPTION_VOLUME,
	OPTION_VOLUME_MUTE,
	OPTION_FAVO_ON,
	OPTION_FAVO_OFF,
	OPTION_QUEUE,

	OPTION_MAX
};

enum
{

    CONTROL_SHUFFLE_ON,
    CONTROL_SHUFFLE_OFF,
	CONTROL_PREVIOUS,
	CONTROL_PLAY,
	CONTROL_PAUSE,
	CONTROL_NEXT,
	CONTROL_REP_A,
	CONTROL_REP_1,
	CONTROL_REP_ALL,

	CONTROL_MAX
};

enum
{
	MP_PLAYER_NORMAL,
	MP_PLAYER_RADIO_PLAY,
	MP_PLAYER_ARTIST_PLAY
};

enum
{
	MP_PLAYER_VIEW_LAUNCH_TYPE
};

#define PLAYER_VIEW_MAGIC	0x7877

typedef struct
{
	INHERIT_MP_VIEW;
	int player_view_magic;
	bool start_on_transition_finish;
	/* extention variables */
	Evas_Object *player_view_layout;
	Elm_Object_Item *inner_navi_it;
	/* title button */
	Evas_Object *dmr_button;
	Evas_Object *snd_button;
	Evas_Object *queue_button;
	/* progress */
	Evas_Object *progress_box;
	Evas_Object *progress_bar;
	Evas_Object *buffering_progress;
	Ecore_Timer *progressbar_timer;
	bool update_flag;		//to indicate if we are in rotation open state. when rotation starts, we will block update_pos for long press case
	double update_pos;	//the position before rotation. used to update rotated view for long pressed case. rotation will do same action as progress up
	/* option */
	Evas_Object *player_view_option_layout;
	Evas_Object *option_button[OPTION_MAX];
	/* control */
	Evas_Object *player_view_control_layout;
	/* start to play new file */
	bool start_new_file;

	Evas_Object *control_button[CONTROL_MAX];
	Evas_Object *volume_popup;
	Ecore_Timer *volume_popup_timer;
	bool volume_popup_now_dragging;
	/* more button */
	mp_media_info_h set_as_handle;
	mp_media_info_h add_to_plst_handle;
	/* other */
	/* focus object */
	Evas_Object *current_focus_btn;
	Evas_Object *pre_focus_btn;
	/* lyric */
	Evas_Object * lyric;
	/* queue list */
	void *queue_list;
	bool queue_status;
	/* screen mode */
	mp_screen_mode play_view_screen_mode;
	/*move direction*/
	int move_direction;
	bool transition_state;
	bool loaded;
#ifdef MP_FEATURE_MUSIC_VIEW
	/* wave view */
	bool wave_view_status;
	char *wave_data;
	int wave_length;
	void *pixels;
	cairo_t *cr;
	cairo_surface_t *surface;
	Evas_Object *overlay;
	Evas_Object *wave_progress_bar;
	Evas_Object *wave_progress_bar_bg;
	Ecore_Timer *wave_progressbar_timer;
	Ecore_Timer *timer_wave;
#endif
#ifdef MP_IMAGE_ROTATE_FEATURE
	/* rotate image & timer */
	int imgId;
	Evas_Object *rotation_image;
	Ecore_Timer *timer_rotate;
#endif

#ifdef MP_FEATURE_SPLIT_WINDOW
        Evas_Object *popup_win;
        int nPosY;
        int nHeight;
        int nWidth;
#endif

	/* launch type: music file / radio / artist*/
	int launch_type;
	int available_route;
	int available_route_count;

	int progressbar_dragging;

	Ecore_Timer *stop_update_timer;
	Ecore_Timer *transit_done_timer;

	int  unmute_vol;

#ifdef MP_FEATURE_ALBUM_COVER_BG
        int  cur_color;
        char *cur_path;
#endif
	Elm_Transit *trans_queue_list;
	Ecore_Idler *queue_title_idler;
	Ecore_Idler *queue_list_update_idler;
	Ecore_Idler * del_old_detail_handle;

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
	Evas_Object* gengrid;
	Elm_Gengrid_Item_Class *gic;
	const char*  genre_str;
	int suggest_count;
	GList* suggest_list;
	Evas_Object* label;
#endif
        bool show_lyric;
}MpPlayerView_t;

MpPlayerView_t *mp_player_view_create(Evas_Object *parent, int launch_type, bool start_new_file);
void mp_player_view_update_progressbar(void *data);
bool mp_player_view_refresh(void *data);
void mp_player_view_progress_timer_freeze(void *data);
void mp_player_view_progress_timer_thaw(void *data);
void mp_player_view_set_snd_path_sensitivity(void *data);
void mp_player_view_update_state(void *data);
void mp_player_view_set_album_playing(void *thiz, bool playing);
void mp_player_view_set_play_image(void *data, bool playing);
void mp_player_view_set_title(void *thiz);
void mp_player_view_volume_popup_control(void *data, bool force_show);
void mp_player_view_update_buffering_progress(void *data, int percent);
void mp_player_view_set_data(MpPlayerView_t *view, ...);
void mp_player_view_update_dmr_icon(MpPlayerView_t *view);
#endif

