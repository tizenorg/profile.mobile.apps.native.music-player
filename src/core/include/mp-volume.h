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

#ifndef __MP_VOLMUE_H__
#define __MP_VOLMUE_H__

#include <stdbool.h>

typedef enum {
	MP_VOLUME_KEY_DOWN,
	MP_VOLUME_KEY_UP,
	MP_VOLUME_KEY_MUTE,
} mp_volume_key_e;

typedef enum {
	MP_VOLUME_KEY_GRAB_COND_WINDOW_FOCUS,
	MP_VOLUME_KEY_GRAB_COND_VIEW_VISIBLE,
	MP_VOLUME_KEY_GRAB_COND_MAX,
} mp_volume_key_grab_condition_e;


typedef void (*Mp_Volume_Key_Event_Cb)(void *user_data, mp_volume_key_e key, bool released);
typedef void (*Mp_Volume_Change_Cb)(int volume, void *user_data);

#ifdef __cplusplus
extern "C" {
#endif

#if 0
void mp_volume_init(Ecore_X_Window xwin, Elm_Win *Win);
#else
void mp_volume_init(void *xwin, Elm_Win *Win);
#endif
void mp_volume_finalize(void);
void mp_volume_key_grab_condition_set(mp_volume_key_grab_condition_e condition, bool enabled);
bool mp_volume_key_grab_start();
void mp_volume_key_grab_end();
bool mp_volume_key_is_grabed();
void mp_volume_key_event_send(mp_volume_key_e type, bool released);
void mp_volume_key_event_callback_add(Mp_Volume_Key_Event_Cb event_cb, void *user_data);
void mp_volume_key_event_callback_del();
void mp_volume_key_event_timer_del();
void mp_volume_add_change_cb(Mp_Volume_Change_Cb cb, void *user_data);
void _mp_volume_handle_change(unsigned int volume);

#ifdef __cplusplus
}
#endif


#endif /* __MP_VOLMUE_H__ */

