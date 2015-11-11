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

#ifndef __MP_ARTIST_LIST_H__
#define __MP_ARTIST_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum {
	MP_ARTIST_LIST_TYPE,	//mp_track_type_e
	MP_ARTIST_LIST_FUNC,
	MP_ARTIST_LIST_PLAYLIT_ID,
	MP_ARTIST_LIST_TYPE_STR,	//type_str for db query
	MP_ARTIST_LIST_FILTER_STR,
	MP_ARTIST_LIST_DISPLAY_MODE,
};

typedef struct __MpArtistList {
	INHERIT_MP_LIST

	Elm_Genlist_Item_Class *itc;
	Elm_Gengrid_Item_Class *gengrid_itc;
	Elm_Genlist_Item_Class *itc_group_index;
	Elm_Object_Item *group_it;
	void (*set_edit_default)(void *thiz, bool edit);
	mp_media_list_h artist_list;
	int artist_list_count;
} MpArtistList_t;

MpArtistList_t * mp_artist_list_create(Evas_Object *parent);
void mp_artist_list_set_data(MpArtistList_t *list, ...);
void mp_artist_list_copy_data(MpArtistList_t*src, MpArtistList_t *dest);
void mp_artist_list_set_reorder(MpArtistList_t *list, bool reorder);


#endif

