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



#ifndef __MP_VOICE_CONTROL_MANAGER_H__
#define __MP_VOICE_CONTROL_MANAGER_H__

#include <stdbool.h>

typedef enum {
	MP_VOICE_CTRL_ACTION_NONE,
	MP_VOICE_CTRL_ACTION_NEXT,
	MP_VOICE_CTRL_ACTION_PREVIOUS,
	MP_VOICE_CTRL_ACTION_PAUSE,
	MP_VOICE_CTRL_ACTION_PLAY,
	MP_VOICE_CTRL_ACTION_VOLUME_UP,
	MP_VOICE_CTRL_ACTION_VOLUME_DOWN,
} mp_voice_ctrl_action_e;

typedef void (*Mp_Voice_Ctrl_Action_Cb)(mp_voice_ctrl_action_e action, void *user_data);

#ifdef __cplusplus
extern "C" {
#endif

void mp_voice_ctrl_mgr_set_action_callback(Mp_Voice_Ctrl_Action_Cb action_cb, void *user_data);
bool mp_voice_ctrl_mgr_start_listening();
void mp_voice_ctrl_mgr_stop_listening();

#ifdef __cplusplus
}
#endif

#endif /*__MP_VOICE_CONTROL_MANAGER_H__ */

