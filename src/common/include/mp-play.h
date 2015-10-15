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


#ifndef __MP_PLAY_H_
#define __MP_PLAY_H_

#include <Elementary.h>
#include "mp-define.h"

#define MP_PLAY_ERROR_NO_SONGS -1

#define MP_PLAY_ERROR_NETWORK -101
#define MP_PLAY_ERROR_STREAMING -102

bool mp_play_item_play_current_item(void *data);
bool mp_play_start_in_ready_state(void *data);
int mp_play_new_file(void *data, bool check_drm);
void mp_play_prev_file(void *data);
void mp_play_next_file(void *data, bool forced);
void mp_play_prepare(void  *data);
void mp_play_start(void *data);
void mp_play_pause(void *data);
void mp_play_stop(void *data);
void mp_play_resume(void *data);
bool mp_play_fast_destory(void *data);
bool mp_play_destory(void *data);
#endif /*__DEF_music_player_contro_H_*/
