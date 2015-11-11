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

#ifndef __MP_SETTING_CTRL_H_
#define __MP_SETTING_CTRL_H_

#ifdef GBSBUILD
#include <vconf.h>
#endif

#ifdef IDEBUILD
#include "idebuild.h"
#endif


typedef void (*MpSettingPlaylist_Cb)(int state, void *data);
typedef void (*MpSettingSaChange_Cb)(int state, void *data);
typedef void (*MpSettingAutoOff_Cb)(int min, void *data);
typedef void (*MpSettingPlaySpeed_Cb)(double speed, void *data);

int mp_setting_init(struct appdata *ad);
int mp_setting_deinit(struct appdata *ad);
int mp_setting_set_shuffle_state(int b_val);
int mp_setting_get_shuffle_state(int *b_val);
int mp_setting_set_repeat_state(int val);
int mp_setting_get_repeat_state(int *val);
void mp_setting_set_nowplaying_id(int val);
int mp_setting_get_nowplaying_id(void);
int mp_setting_playlist_get_state(int *state);
int mp_setting_playlist_set_callback(MpSettingPlaylist_Cb func, void *data);
void mp_setting_set_player_state(int val);
void mp_setting_save_now_playing(void *ad);
void mp_setting_save_playing_info(void *ad);
void mp_setting_get_now_playing_path_from_file(char **path);
void mp_setting_save_shortcut(char *shortcut_title, char *artist, char *shortcut_description,
                              char *shortcut_image_path);

void mp_setting_remove_now_playing_shared_status(void);
void mp_setting_remove_now_playing(void);
int
mp_setting_read_playing_status(char *uri, char *status);
void
mp_setting_write_playing_status(char *uri, char *status);

#ifdef MP_FEATURE_AUTO_OFF
int mp_setting_auto_off_set_callback(MpSettingAutoOff_Cb func, void *data);
void mp_setting_reset_auto_off_time();
int mp_setting_get_auto_off_time();
#endif

#ifdef MP_FEATURE_PLAY_SPEED
int mp_setting_set_play_speed_change_callback(MpSettingPlaySpeed_Cb func, void *data);
int mp_setting_reset_play_speed();
double mp_setting_get_play_speed();
#endif

int mp_setting_get_side_sync_status(void);

void mp_setting_update_active_device();
#ifdef MP_FEATURE_PERSONAL_PAGE
bool mp_setting_set_personal_dont_ask_again(bool bAsked);
bool mp_setting_get_personal_dont_ask_again(bool *bAsked);
#endif

#endif // __MP_SETTING_CTRL_H_
