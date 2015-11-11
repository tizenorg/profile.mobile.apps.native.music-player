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

#ifndef __MP_ALL_LIST_H__
#define __MP_ALL_LIST_H__

#include "mp-list.h"
#include "mp-media-info.h"

typedef struct {
	INHERIT_MP_LIST

	Evas_Object *parent;

	//tabbar
	MpTab_e tab_status;
	bool first_change;

	//key layouts
	Evas_Object *shortcut_layout;
	Evas_Object *tabbar_layout;
	int shortcut_index;

	//thumbnail view mode
	Evas_Object *gengrid;

	//item class
	Elm_Genlist_Item_Class *itc_icon;
	Elm_Genlist_Item_Class *itc;
	Elm_Genlist_Item_Class *itc_shuffle;
	Elm_Gengrid_Item_Class *gengrid_itc;

	//genlist items
	Elm_Object_Item *shortcut_it;
	Elm_Object_Item *tabbar_it;
	Elm_Object_Item *first_item; //first item of track, playlist, album, or artist list
	Elm_Object_Item *shuffle_it;

	bool tabbar_realized;
	bool drag_status;
	int track_count;

	//track related
	mp_media_list_h track_list[2];
	Ecore_Timer *load_timer;
#ifdef MP_FEATURE_PERSONAL_PAGE
	bool personal_page_status;
#endif

	//playlist related
	mp_media_list_h playlists_user;
	mp_media_list_h playlists_auto;
	int auto_playlist_count;

	//album, artist related
	mp_media_list_h group_list;
	MpListDisplayMode_e album_disp_mode;
	MpListDisplayMode_e artist_disp_mode;

	MpFwMgr FwMgr;
	Evas_Object *floating_tabbar;
} MpAllList_t;

MpAllList_t * mp_all_list_create(Evas_Object *parent, MpTab_e init_tab);
void mp_all_list_update_shortcut(MpAllList_t *list);
void mp_all_list_rotate_shortcut(MpAllList_t *list);
void mp_all_list_update_data(void *thiz);
void mp_all_list_update_genlist(void *thiz);
MpTab_e mp_all_list_get_selected_tab(MpAllList_t *list);
void mp_all_list_select_tab(MpAllList_t *list, MpTab_e tab);
void mp_all_list_update_favourite(MpAllList_t *list);
void mp_all_list_set_display_mode(MpAllList_t *list, MpListDisplayMode_e mode);

#endif

