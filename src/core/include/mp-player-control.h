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

#ifndef __DEF_music_player_contro_H_
#define __DEF_music_player_contro_H_

#include <Elementary.h>
#include "music.h"
#include "mp-player-view.h"

void mp_play_control_play_pause(struct appdata *ad, bool play);
void mp_play_control_resume_via_media_key(struct appdata *ad);
void mp_player_control_stop(struct appdata *ad);
void mp_play_control_ff(int press, bool event_by_mediakey, bool clicked);
void mp_play_control_rew(int press, bool event_by_mediakey, bool clicked);
void mp_play_control_reset_ff_rew(void);
void mp_play_control_menu_cb(void *data, Evas_Object * o, const char *emission, const char *source);
void mp_play_control_end_of_stream(void *data);
int mp_player_control_ready_new_file(void *data, bool check_drm);
void mp_play_control_on_error(struct appdata *ad, int ret, bool add_watch);

void mp_play_stop_and_updateview(void *data, bool mmc_removed);
void mp_play_control_shuffle_set(void *data, bool shuffle_enable);

void mp_play_control_next(void);
void mp_play_control_prev(void);

#endif /*__DEF_music_player_contro_H_*/
