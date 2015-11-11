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

#ifndef __MP_CREATE_PLAYLIST_VIEW_H__
#define __MP_CREATE_PLAYLIST_VIEW_H__

#include "music.h"
#include "mp-list-view.h"

typedef enum {
	MP_PLST_CREATION_TYPE_NORMAL,
#ifdef MP_FEATURE_STORE
	MP_PLST_CREATION_TYPE_WITH_STORE,
#endif
} mp_plst_creation_type_e;

typedef enum {
	MP_PLST_PARENT_NORMAL,
	MP_PLST_PARENT_ALL_VIEW,
	MP_PLST_PARENT_DETAIL_VIEW,
	MP_PLST_PARENT_SQUARE_VIEW
} mp_plst_parent_type_e;


typedef struct {
	INHERIT_MP_VIEW;
	/* extention variables */
	Evas_Object *create_plst_layout;

	/* additional variables */
	Evas_Object *editfiled_new_playlist;
	Evas_Object *editfiled_entry;
	char *new_playlist_name;
	mp_plst_parent_type_e parent_view;
	mp_media_info_h playlist_handle;
	char *name;
	MpList_t *adding_list;
#ifdef MP_FEATURE_STORE
	GList *adding_song_list;
#endif
	char *adding_media_id;
	mp_plst_creation_type_e creation_type;
} MpCreatePlstView_t;

MpCreatePlstView_t *mp_create_plst_view_create(Evas_Object *parent);
int mp_create_plst_view_set_edit_list(MpCreatePlstView_t *view, MpList_t *adding_list);
int mp_create_plst_view_set_media_id(MpCreatePlstView_t *view, const char *adding_media_id);
int mp_create_plst_view_set_creation_type(MpCreatePlstView_t *view, mp_plst_creation_type_e type);
int mp_create_plst_view_destory(MpCreatePlstView_t *view);
#ifdef MP_FEATURE_STORE
int mp_create_plst_view_set_adding_song_list(MpCreatePlstView_t *view, GList *list);
#endif

#endif
