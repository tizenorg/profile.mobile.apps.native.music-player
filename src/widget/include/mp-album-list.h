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

#ifndef __MP_ALBUM_LIST_H__
#define __MP_ALBUM_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum {
	MP_ALBUM_LIST_FUNC,	//indicate in all-view or add-track-view
	MP_ALBUM_LIST_DISPLAY_MODE,
};

typedef struct __MpAlbumList {
	INHERIT_MP_LIST

	Elm_Genlist_Item_Class *itc;
	Elm_Gengrid_Item_Class *gengrid_itc;
	Elm_Genlist_Item_Class *itc_group_index;
	Elm_Object_Item *group_it;
	void (*set_edit_default)(void *thiz, bool edit);

	mp_media_list_h album_list;
	int album_list_count;

} MpAlbumList_t;

MpAlbumList_t * mp_album_list_create(Evas_Object *parent);
void mp_album_list_set_data(MpAlbumList_t *list, ...);
void mp_album_list_copy_data(MpAlbumList_t*src, MpAlbumList_t *dest);
void mp_album_list_set_reorder(MpAlbumList_t *list, bool reorder);

#endif

