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

#ifndef __MP_TRACK_LIST_H__
#define __MP_TRACK_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

enum {
	MP_TRACK_LIST_TYPE,	//mp_track_type_e
	MP_TRACK_LIST_PLAYLIT_ID,
	MP_TRACK_LIST_TYPE_STR,	//type_str for db query
	MP_TRACK_LIST_TYPE_STR2,	//type_str for db query
	MP_TRACK_LIST_FILTER_STR,
	MP_TRACK_LIST_INDEX_TYPE,	// album art list, ....
	MP_TRACK_LIST_CLOUD_TYPE,
	MP_TRACK_LIST_CHECKED_LIST,
};

typedef struct __MpTrackList {
	INHERIT_MP_LIST

	void (*set_edit_default)(void *thiz, bool edit);

	Elm_Genlist_Item_Class *itc;
	Elm_Genlist_Item_Class *itc_shuffle;

	mp_media_list_h track_list[2];
	Ecore_Timer *load_timer;

	Elm_Object_Item *albumart_index_item;
	Elm_Object_Item *shuffle_it;
	GList *albumart_index_list;

	mp_media_list_h playlists;
	mp_media_info_h playlist_handle;

	int track_count;
	bool  get_by_view;

} MpTrackList_t;

MpTrackList_t * mp_track_list_create(Evas_Object *parent);
void mp_track_list_set_data(MpTrackList_t *list, ...);
void mp_track_list_set_reorder(MpTrackList_t *list, bool reorder);
void mp_track_list_update_albumart_index(MpTrackList_t *list);
void mp_track_list_show_shuffle(void *thiz, bool show);
MpCloudView_e mp_track_list_get_cloud_view(MpTrackList_t *list);
void mp_track_list_update_genlist(void *thiz);
void mp_track_list_popup_delete_genlist_item(void *thiz);
void mp_track_list_copy_data(MpTrackList_t *src, MpTrackList_t *dest);
#endif

