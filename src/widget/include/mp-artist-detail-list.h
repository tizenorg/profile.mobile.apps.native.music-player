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

#ifndef __MP_ARTIST_DETAIL_LIST_H__
#define __MP_ARTIST_DETAIL_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum {
	MP_ARTIST_DETAIL_LIST_TYPE,	//mp_track_type_e
	MP_ARTIST_DETAIL_LIST_TYPE_STR,	//type_str for db query
	MP_ARTIST_DETAIL_LIST_FILTER_STR,
};

typedef struct __MpArtistDetailList {
	INHERIT_MP_LIST

	void (*set_edit_default)(void *thiz, bool edit);

	Elm_Genlist_Item_Class *itc_track;
	Elm_Genlist_Item_Class *itc_album;
	Elm_Genlist_Item_Class *itc_shuffle;
	Elm_Object_Item *shuffle_it;

	int album_list_count;
	mp_media_list_h album_list;
	GList *track_lists;
	int count_album;
	//int edit_mode;

} MpArtistDetailList_t;

MpArtistDetailList_t * mp_artist_detail_list_create(Evas_Object *parent);
void mp_artist_detail_list_set_data(MpArtistDetailList_t *list, ...);
void mp_artist_detail_list_set_reorder(MpArtistDetailList_t *list, bool reorder);
void mp_artist_detail_list_copy_data(MpArtistDetailList_t *src, MpArtistDetailList_t *dest);
void mp_artist_detail_list_update_genlist(void *thiz);

#endif

