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

#ifndef __MP_ALBUM_DETAIL_LIST_LIST_H__
#define __MP_ALBUM_DETAIL_LIST_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum
{
	MP_ALBUM_DETAIL_LIST_TYPE,	//mp_track_type_e
	MP_ALBUM_DETAIL_TYPE_STR,	//type_str for db query
	MP_ALBUM_DETAIL_ARTIST,
	MP_ALBUM_DETAIL_THUMBNAIL,
};


typedef struct __MpAlbumDetailList{
	INHERIT_MP_LIST

	void (*set_edit_default)(void *thiz, bool edit);

	Elm_Genlist_Item_Class *itc;
	Elm_Genlist_Item_Class *itc_album;
	Elm_Genlist_Item_Class *itc_shuffle;

	mp_media_list_h track_list[2];
	Ecore_Timer *load_timer;

	Elm_Object_Item *shuffle_it;

	int track_count;
	bool various_name;
	int total_duration;
	char *artist;
	char *thumbnail;

}MpAlbumDetailList_t;

MpAlbumDetailList_t * mp_album_detail_list_create(Evas_Object *parent);
void mp_album_detail_list_set_data(MpAlbumDetailList_t *list, ...);
void mp_album_detail_list_set_reorder(MpAlbumDetailList_t *list, bool reorder);
void mp_album_detail_list_show_shuffle(void *thiz, bool show);
void mp_album_detail_list_update_genlist(void *thiz);
void mp_album_detail_list_popup_delete_genlist_item(void *thiz);
void mp_album_detail_list_copy_data(MpAlbumDetailList_t *src, MpAlbumDetailList_t *dest);
#endif

