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


#ifndef __MP_APP_IN_APP_H__
#define __MP_APP_IN_APP_H__

#include "music.h"

typedef enum
{
        WINDOW_SIZE_0,
        WINDOW_SIZE_1,
        WINDOW_SIZE_2,
        WINDOW_SIZE_3,
        WINDOW_SIZE_4
} MULTI_WINDOW_SIZE;


typedef struct
{
	INHERIT_MP_VIEW;
	/* extention variables */
	Evas_Object *mini_player_view_layout;
	/* lyric */
	mp_lyric_view_t *lyric_view;
	/* screen mode */
	mp_screen_mode play_view_screen_mode;

	bool mini_player_mode;
	int mini_player_current_size;
	Evas_Object *win_mini;
        Evas_Object *bg;
#if 0
        Ecore_X_Window *xwin;
#else
        void *xwin;
#endif
	Evas_Object *title;
	Evas_Object *event_box;
        Evas_Object *mini_lyric_view;

	Ecore_Timer *play_delay_timer;
	Ecore_Timer *switch_timer;
}MpMwView_t;


void mp_mini_player_show(struct appdata *ad, int num);
void mp_mini_player_hide(void *data);
void mp_mini_player_mode_set(void *data, int is_set);
void mp_mini_player_refresh(void *data);
void mp_mini_player_window_drag_resize(void *data, int start_x, int start_y, unsigned int button);
void mp_mini_player_window_drag_start(void *data, int start_x, int start_y, unsigned int button);
int mp_mini_player_rotation_cb(app_device_orientation_e mode, void *data);

#endif
