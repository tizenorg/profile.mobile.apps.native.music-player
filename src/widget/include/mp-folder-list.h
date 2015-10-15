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

#ifndef __MP_FOLDER_LIST_H__
#define __MP_FOLDER_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum
{
	MP_FOLDER_LIST_TYPE,	//mp_track_type_e
	MP_FOLDER_LIST_TYPE_STR,	//type_str for db query
	MP_FOLDER_LIST_FUNC,
	MP_FOLDER_LIST_FILTER_STR,
};

typedef struct __MpFolderList{
	INHERIT_MP_LIST

	Elm_Genlist_Item_Class *itc;
	mp_media_list_h folder_list;
	//int edit_mode;

}MpFolderList_t;

void mp_folder_list_set_data(MpFolderList_t *list, ...);
void mp_folder_list_copy_data(MpFolderList_t*src, MpFolderList_t *dest);
MpFolderList_t * mp_folder_list_create(Evas_Object *parent);

#endif

