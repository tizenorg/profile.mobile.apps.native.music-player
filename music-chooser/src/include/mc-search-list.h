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

#ifndef __MC_SEARCH_LIST_H__
#define __MC_SEARCH_LIST_H__

//#include "mp-list.h"
#include <Elementary.h>

#include "mp-media-info.h"

enum
{
	MC_SEARCH_LIST_FILTER_STR,
};

typedef struct __UgMpSearchList{
	struct app_data *ad;

	Elm_Genlist_Item_Class *itc_track;
	Elm_Genlist_Item_Class *itc_album;
	Elm_Genlist_Item_Class *itc_artist;
	Elm_Genlist_Item_Class *itc_group_title;

	Elm_Object_Item *search_group_git;
	int track_count;

	Evas_Object *layout;
	Evas_Object *box;
	Evas_Object *genlist;
	Evas_Object *no_content;
	char *filter_str;

	mp_media_list_h track_handle;	//for search
	mp_media_list_h artist_handle;	//for search
	mp_media_list_h album_handle;	//for search
}UgMpSearchList_t;

typedef enum
{
	MP_LIST_ITEM_TYPE_NORMAL	= 0,
        MP_LIST_ITEM_TYPE_SHUFFLE,
	MP_LIST_ITEM_TYPE_GROUP_TITLE,
	MP_LIST_ITEM_TYPE_SELECTABLE_GROUP_TITLE,
	MP_LIST_ITEM_TYPE_ALBUMART_INDEX,
	MP_LIST_ITEM_TYPE_BOTTOM_COUNTER,
} MpListItemType_e;

UgMpSearchList_t * mc_search_list_create(Evas_Object *parent, struct app_data *ad);
void mc_search_list_set_data(UgMpSearchList_t *list, ...);
void mc_search_list_update(void *thiz);
void mc_search_list_set_reorder(UgMpSearchList_t *list, bool reorder);

#endif

