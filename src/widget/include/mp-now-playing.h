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

#ifndef __MP_NOW_PLAYING_H__
#define __MP_NOW_PLAYING_H__

#include <Elementary.h>
#include <stdbool.h>

typedef void (*MpNowplayingCb)(void *data);

Evas_Object *mp_now_playing_create(Evas_Object *parent, MpNowplayingCb play_bt_clicked, MpNowplayingCb clicked, void *data);
void mp_now_playing_thaw_timer(Evas_Object *now_playing);
void mp_now_playing_freeze_timer(Evas_Object *now_playing);
void mp_now_playing_update(Evas_Object *now_playing, const char *title, const char *artist, const char *thumbnail, bool with_title);
bool mp_now_playing_is_landscape(Evas_Object *now_playing);
void mp_now_playing_set_layout(Evas_Object *now_playing);

#endif

