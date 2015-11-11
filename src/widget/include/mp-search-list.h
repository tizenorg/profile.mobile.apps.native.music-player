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

#ifndef __MP_SEARCH_LIST_H__
#define __MP_SEARCH_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum {
	MP_SEARCH_LIST_FILTER_STR,
};

enum {
	MP_SEARCH_ARTIST_GROUP,
	MP_SEARCH_ALBUM_GROUP,
	MP_SEARCH_TRACK_GROUP,
	MP_SEARCH_MAX_GROUP,
};


typedef struct __MpSearchList {
	INHERIT_MP_LIST

	Elm_Genlist_Item_Class *itc_track;
	Elm_Genlist_Item_Class *itc_album;
	Elm_Genlist_Item_Class *itc_artist;
	Elm_Genlist_Item_Class *itc_group_title;

	Elm_Object_Item *search_group_git[MP_SEARCH_MAX_GROUP];
	int track_count;

	mp_media_list_h track_handle;	//for search
	mp_media_list_h artist_handle;	//for search
	mp_media_list_h album_handle;	//for search
	void (*refresh)(void *thiz);
	GList* artist_list;
	GList* album_list;
	GList* track_list;
} MpSearchList_t;

MpSearchList_t * mp_search_list_create(Evas_Object *parent);
void mp_search_list_set_data(MpSearchList_t *list, ...);
void mp_search_list_set_reorder(MpSearchList_t *list, bool reorder);

#endif

