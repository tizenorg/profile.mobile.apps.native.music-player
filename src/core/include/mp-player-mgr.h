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


#ifndef __MP_player_mgr_H_
#define __MP_player_mgr_H_

#include <sound_manager.h>
#include <player.h>
#ifdef GBSBUILD
#include <vconf.h>
#endif
#include "ms-key-ctrl.h"

typedef enum {
	MP_PLAYER_TYPE_MMFW,
} mp_player_type_e;

typedef void (*Seek_Done_Cb)(void *data);
typedef void (*mp_player_started_cb)(void *user_data);
typedef void (*mp_player_paused_cb)(void *user_data);
typedef void (*mp_player_duration_changed_cb)(void *user_data);

player_h mp_player_mgr_get_player(void);
bool mp_player_mgr_is_active(void);
void mp_player_mgr_set_started_db(mp_player_started_cb callback, void *user_data);
void mp_player_mgr_set_completed_cb(player_completed_cb  callback, void *user_data);
void mp_player_mgr_set_interrupted_cb(player_interrupted_cb  callback, void *user_data);
void mp_player_mgr_set_error_cb(player_error_cb  callback, void *user_data);
void mp_player_mgr_set_buffering_cb(player_buffering_cb  callback, void *user_data);
void mp_player_mgr_set_prepare_cb(player_prepared_cb callback, void *user_data);
void mp_player_mgr_set_paused_cb(mp_player_paused_cb callback, void *user_data);
void mp_player_mgr_set_duration_changed_cb(mp_player_duration_changed_cb callback, void *user_data);
void mp_player_mgr_unset_completed_cb(void);
void mp_player_mgr_unset_interrupted_cb(void);
void mp_player_mgr_unset_error_cb(void);
void mp_player_mgr_unset_buffering_cb(void);

int mp_player_mgr_create(void *data, const char * path, mp_player_type_e type, void *extra_data);
int mp_player_mgr_create_with_buffer(void *data, void *buffer, int size);	// buffer will be managed by player_mgr
bool mp_player_mgr_destroy(void *data);
int mp_player_mgr_realize(void *data);
bool mp_player_mgr_unrealize(void *data);
int mp_player_mgr_play(void *data);
bool mp_player_mgr_stop(void *data);
int mp_player_mgr_resume(void *data);
bool mp_player_mgr_pause(void *data);
void mp_player_mgr_set_mute(bool bMuteEnable);
bool mp_player_mgr_is_seeking(void);
bool mp_player_mgr_set_position(unsigned int pos, Seek_Done_Cb done_cb, void *data);
Eina_Bool mp_player_mgr_seek_done(void *data);
void mp_player_mgr_unset_seek_done_cb();
bool mp_player_mgr_set_play_speed(double speed);
int mp_player_mgr_get_position(void);
int mp_player_mgr_get_duration(void);
int mp_player_mgr_vol_type_set(void);
int mp_player_mgr_vol_type_unset(void);
bool mp_player_mgr_session_init(void);
bool mp_player_mgr_session_finish(void);

player_state_e mp_player_mgr_get_state(void);
bool mp_player_mgr_change_player(mp_player_type_e player_type);
mp_player_type_e mp_player_mgr_get_player_type(void);

int mp_player_mgr_volume_get_max();
int mp_player_mgr_volume_get_current();
bool mp_player_mgr_volume_set(int volume);
bool mp_player_mgr_volume_up();
bool mp_player_mgr_volume_down();
int mp_player_mgr_safety_volume_set(int foreground);
bool mp_player_mgr_get_content_info(char **title, char **album, char **artist, char **author, char **genre, char **year);
void mp_playlist_mgr_item_remove_deleted_item(mp_plst_mgr *playlist_mgr);

int mp_player_mgr_set_progressive_download(const char *path, player_pd_message_cb callback, void *user_data);
int _mp_player_mgr_create_common(struct appdata *ad, mp_player_type_e type);
int mp_player_mgr_prepare(void *data);

#endif //__MP_player_mgr_H_
