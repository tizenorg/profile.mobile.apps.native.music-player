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

#ifndef __MP_PLAYLIST_LIST_H__
#define __MP_PLAYLIST_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum
{
	MP_PLAYLIST_LIST_TYPE,	//mp_track_type_e
	MP_PLAYLIST_LIST_FUNC,	//indicate in all-view or add-track-view
	MP_PLAYLIST_LIST_PLAYLIT_ID,
	MP_PLAYLIST_LIST_TYPE_STR,	//type_str for db query
	MP_PLAYLIST_LIST_FILTER_STR,
	MP_PLAYLIST_LIST_DISPLAY_MODE,
};

enum
{
	MP_PLAYLIST_GROUP_INDEX_DEFAULT,
	MP_PLAYLIST_GROUP_INDEX_MY_PLAYLIST,
	MP_PLAYLIST_GROUP_INDEX_NUM,
};

typedef struct __MpPlaylistList{
	INHERIT_MP_LIST

	Elm_Genlist_Item_Class *itc_group_index_default;
	Elm_Genlist_Item_Class *itc_group_index_user;
	Elm_Genlist_Item_Class *itc_auto;
	Elm_Genlist_Item_Class *itc_user;
	Elm_Gengrid_Item_Class *gengrid_itc;
	Elm_Gengrid_Item_Class *gengrid_add_itc;

	int auto_playlist_count;

	mp_media_list_h playlists_user;
	mp_media_list_h playlists_auto;
	mp_media_info_h playlist_handle;

	Elm_Object_Item *group_index[MP_PLAYLIST_GROUP_INDEX_NUM + 1];
	//int edit_mode;

}MpPlaylistList_t;

MpPlaylistList_t * mp_playlist_list_create(Evas_Object *parent);
void mp_playlist_list_set_data(MpPlaylistList_t *list, ...);
void mp_playlist_list_copy_data(MpPlaylistList_t *src, MpPlaylistList_t *dest);
void mp_playlist_list_set_reorder(MpPlaylistList_t *list, bool reorder);
int mp_playlist_list_set_playlist(mp_plst_mgr *plst_mgr, int playlist_id);


#endif

