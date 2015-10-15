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

#ifndef __MP_EDIT_PLAYLIST_H__
#define __MP_EDIT_PLAYLIST_H__

#include "music.h"
#include "mp-list-view.h"

typedef enum {
	MP_PLST_CREATIE_TYPE_NORMAL,
} mp_plst_create_type_e;

typedef enum {
        MP_PLST_NORMAL,
        MP_PLST_CREATE,
        MP_PLST_CREATE_TO_ADD_TRACK,
	MP_PLST_RENAME,
	MP_PLST_SAVE_AS,
} mp_plst_operation_type;


typedef struct {
	struct appdata *ad;
	mp_plst_operation_type type;

        char *new_playlist_name;
        char *name;
	char *oldname;

        Evas_Object *popup;
        Evas_Object *layout;
        Evas_Object *editfiled_entry;
        Evas_Object *btn_ok;

        bool add_to_selected;
        mp_media_info_h playlist_handle;
	MpList_t *adding_list;

	char *adding_media_id;
        mp_plst_create_type_e creation_type;

        Ecore_Timer *entry_show_timer;
        Ecore_Idler *set_line_end_idler;
        bool         set_to_end;
} Mp_Playlist_Data;

void *mp_edit_playlist_create(mp_plst_operation_type type);
void mp_edit_playlist_content_create(void *thiz);
void mp_edit_playlist_add_to_selected_mode(void *data, bool selected);
int mp_edit_playlist_set_edit_list(Mp_Playlist_Data *mp_playlist_data, MpList_t *adding_list);
int mp_edit_playlist_set_media_id(Mp_Playlist_Data *mp_playlist_data, const char *adding_media_id);
int mp_edit_playlist_set_create_type(Mp_Playlist_Data *mp_playlist_data, mp_plst_create_type_e type);

#endif
