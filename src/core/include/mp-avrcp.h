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

#ifndef __MP_AVRCP_H__
#define __MP_AVRCP_H__

#include <stdbool.h>

typedef enum {
	MP_AVRCP_STATE_STOPPED,
	MP_AVRCP_STATE_PLAYING,
	MP_AVRCP_STATE_PAUSED,
	MP_AVRCP_STATE_FF,
	MP_AVRCP_STATE_REW,
} mp_avrcp_player_state_e;

typedef enum {
	MP_AVRCP_REPEAT_OFF,
	MP_AVRCP_REPEAT_ONE,
	MP_AVRCP_REPEAT_ALL,
} mp_avrcp_repeat_mode_e;

typedef enum {
	MP_AVRCP_SHUFFLE_OFF,
	MP_AVRCP_SHUFFLE_ON,
} mp_avrcp_shuffle_mode_e;

typedef enum {
	MP_AVRCP_EQ_OFF,
	MP_AVRCP_EQ_ON,
} mp_avrcp_eq_state_e;

typedef enum {
	MP_AVRCP_ERROR_NONE,
	MP_AVRCP_ERROR,
	MP_AVRCP_ERROR_PERMISSION_DENIED,
} MpAvrcpErr_e;

typedef void (*mp_avrcp_connection_state_changed_cb)(bool connected, const char *remote_address, void *user_data);
typedef void (*mp_avrcp_shuffle_changed_cb)(mp_avrcp_shuffle_mode_e mode, void *user_data);
typedef void (*mp_avrcp_repeat_changed_cb)(mp_avrcp_repeat_mode_e mode, void *user_data);
typedef void (*mp_avrcp_eq_changed_cb)(mp_avrcp_eq_state_e state, void *user_data);

MpAvrcpErr_e mp_avrcp_target_initialize(void);
int mp_avrcp_target_finalize(void);

MpAvrcpErr_e mp_avrcp_noti_player_state(mp_avrcp_player_state_e state);
MpAvrcpErr_e mp_avrcp_noti_eq_state(mp_avrcp_eq_state_e eq);
MpAvrcpErr_e mp_avrcp_noti_repeat_mode(mp_avrcp_repeat_mode_e repeat);
MpAvrcpErr_e mp_avrcp_noti_shuffle_mode(mp_avrcp_shuffle_mode_e shuffle);
MpAvrcpErr_e mp_avrcp_noti_track(const char *title, const char *artist, const char *album, const char *genre, unsigned int duration);
MpAvrcpErr_e mp_avrcp_noti_track_position(unsigned int position);

MpAvrcpErr_e mp_avrcp_set_mode_change_cb(mp_avrcp_connection_state_changed_cb connection_cb, mp_avrcp_shuffle_changed_cb s_cb,
        mp_avrcp_repeat_changed_cb r_cb, mp_avrcp_eq_changed_cb e_cb, void *user_data);

#endif

