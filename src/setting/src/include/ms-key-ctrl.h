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


#ifndef __ms_key_ctrl_H__
#define __ms_key_ctrl_H__

#include "mp-vconf-private-keys.h"
#include "ms-util.h"

enum
{
	MS_MENU_ALBUMS = 0,
	MS_MENU_ARTISTS,
	MS_MENU_GENRES,
	MS_MENU_COMPOSERS,
	MS_MENU_YEARS,
	MS_MENU_FOLDERS,
	MS_MENU_SQUARE,
	MS_MENU_NUMS,
};

typedef struct _ms_eq_custom_t
{
	double band_1;
	double band_2;
	double band_3;
	double band_4;
	double band_5;
	double band_6;
	double band_7;
	double band_8;
} ms_eq_custom_t;

typedef struct {
	double three_d;
	double bass;
	double room_size;
	double reverb_level;
	double clarity;
} ms_extended_effect_t;

int ms_key_set_tabs_val(int b_val);
int ms_key_get_tabs_val(int *b_val);


int ms_key_set_playlist_val(int b_val);
int ms_key_get_playlist_val(int *b_val);
int ms_key_set_menu_changed(void);
int ms_key_set_eq_custom(ms_eq_custom_t custom_val);

int ms_key_set_extended_effect(ms_extended_effect_t *extended_val);

void ms_key_set_user_effect(int value);

int ms_key_set_auto_off_time(int min);
int ms_key_get_auto_off_time(void);
int ms_key_set_auto_off_custom_time(int min);
int ms_key_get_auto_off_custom_time(void);
int ms_key_get_auto_off_val(void);
int ms_key_set_auto_off_val(int type);

char* ms_key_get_auto_off_time_text(int index);

double ms_key_get_play_speed(void);
void ms_key_set_play_speed(double speed);

EXPORT_API int ms_key_get_playlist_str(char **b_str);
EXPORT_API int ms_key_get_tabs_str(char **b_str);
EXPORT_API int ms_key_set_tabs_str(char* b_str);
EXPORT_API int ms_key_set_playlist_str(char* b_str);

#endif //__ms_key_ctrl_H__
