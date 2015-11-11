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

#ifndef __MP_EDIT_CALLBACK_H__
#define	__MP_EDIT_CALLBACK_H__

typedef enum {
#ifdef MP_FEATURE_PERSONAL_PAGE
	MP_EDIT_MOVE,
#endif
	MP_EDIT_DELETE,
	MP_EDIT_ADD_TO_PLAYLIST,
} mp_edit_operation_t;

#include "music.h"
void mp_edit_create_delete_popup(void *data);
void mp_edit_create_add_to_playlist_popup(void *data);
#ifndef MP_SOUND_PLAYER
void *mp_edit_get_delete_thread();
#endif

/* if playlist_name is not null, playlist detail view will be displayed after add to playlist*/
void mp_edit_cb_excute_add_to_playlist(void *data, int playlist_id, char *playlist_name, bool selected);
void mp_edit_cb_excute_delete(void *data);
void mp_edit_cb_cencel_cb(void *data, Evas_Object * obj, void *event_info);
void mp_edit_cb_excute_make_offline_available(void *data);
void mp_edit_cb_excute_move(void *data);
void mp_edit_create_track_delete_popup(void *data);

#endif
