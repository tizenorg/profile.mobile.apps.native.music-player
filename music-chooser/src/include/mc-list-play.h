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

#ifndef __DEF_MC_LIST_PLAY_H_
#define __DEF_MC_LIST_PLAY_H_

#include <player.h>
#include <sound_manager.h>
#include "mc-track-list.h"

#define MC_PRE_PLAY_BUTTON_PART_NAME "elm.edit.icon.2"

typedef enum {
	MC_PLAYER_CB_TYPE_STARTED,
	MC_PLAYER_CB_TYPE_PAUSED,
	MC_PLAYER_CB_TYPE_COMPLETED,
	MC_PLAYER_CB_TYPE_INTURRUPTED,
	MC_PLAYER_CB_TYPE_ERROR,
	MC_PLAYER_CB_TYPE_BUFFERING,
	MC_PLAYER_CB_TYPE_PREPARE,
	MC_PLAYER_CB_TYPE_MAX,
} mc_player_cb_type;

typedef enum {
	MC_VOLUME_NONE,
	MC_VOLUME_ALERT,
	MC_VOLUME_NOTIFICATION,
	MC_VOLUME_RINGTONE,
	MC_VOLUME_NUM,
} mc_player_volume_type;

typedef struct __mc_player_cbs {
	/* player callbacks */
	/*	player_started_cb started_cb;
		player_paused_cb paused_cb;*/
	player_completed_cb completed_cb;
	player_interrupted_cb interrupted_cb;
	player_error_cb error_cb;
	player_buffering_cb buffering_cb;
	player_prepared_cb prepare_cb;

	/* callback user data */
	void *user_data[MC_PLAYER_CB_TYPE_MAX];
} mc_player_cbs;

typedef struct {
	mc_player_cb_type cb_type;

	union {
		player_interrupted_code_e interrupted_code;
		int error_code;
		int percent;
	} param;
} mc_player_cb_extra_data;

void mc_pre_play_mgr_play_song(void *data);
void mc_pre_play_mgr_reset_song(void *data);
void mc_pre_play_mgr_reset_song_without_stop(void *data);
void mc_pre_play_control_clear_pre_item_data(void);
void mc_pre_play_mgr_play_control(void *data);
player_state_e mc_pre_play_get_player_state(void);
void mc_pre_play_mgr_destroy_play(void);
bool mc_player_mgr_is_active(void);
void mc_player_pause(void);
void mc_player_play(void);
void mc_pre_play_control_play_no_pause_music_item(list_item_data_t *item_data);
void mc_pre_play_control_play_music_item(list_item_data_t *item_data);
void mc_vol_reset_default_value(void* data);
void mc_vol_type_set(sound_type_e type);
int mc_get_volume(sound_type_e type);



#endif /* __DEF_MC_LIST_PLAY_H_ */

