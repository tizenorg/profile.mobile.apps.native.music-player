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

#ifndef __MP_PLAYER_VOLUME_WIDGET_H__
#define __MP_PLAYER_VOLUME_WIDGET_H__

#include <Elementary.h>

typedef enum {
	VOLUME_WIDGET_EVENT_DRAG_START,
	VOLUME_WIDGET_EVENT_DRAG_STOP,

        VOLUME_WIDGET_EVENT_DRAG_MAX,
} volume_widget_event_e;

typedef void (*Volume_Widget_Cb)(void *user_data, Evas_Object *obj, volume_widget_event_e event);

#ifdef __cplusplus
extern "C" {
#endif

Evas_Object *mp_player_volume_widget_add(Evas_Object *parent);
void mp_player_volume_widget_event_callback_add(Evas_Object *obj, Volume_Widget_Cb event_cb, void *user_data);
int mp_player_volume_widget_set_val(Evas_Object *obj, int val);

#ifdef __cplusplus
}
#endif

#endif /* __MP_PLAYER_VOLUME_WIDGET_H__ */

