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

#ifndef __MP_ADD_TRACK_VIEW_H__
#define __MP_ADD_TRACK_VIEW_H__

#include "mp-list-view.h"
#include "mp-track-list.h"
#include "mp-playlist-list.h"
#include "mp-album-list.h"
#include "mp-artist-list.h"
#include "mp-folder-view.h"
#include "mp-edit-callback.h"

#include "music.h"

typedef enum
{
	MP_ADD_TRACK_VIEW_TAB_ALL,
	MP_ADD_TRACK_VIEW_TAB_PLAYLIST,
	MP_ADD_TRACK_VIEW_TAB_SONGS,
	MP_ADD_TRACK_VIEW_TAB_ALBUMS,
	MP_ADD_TRACK_VIEW_TAB_ARTIST,
	MP_ADD_TRACK_VIEW_TAB_FOLDERS,
	/*
	MP_ADD_TRACK_VIEW_TAB_GENRE,
	MP_ADD_TRACK_VIEW_TAB_YEAR,
	MP_ADD_TRACK_VIEW_TAB_COMPOSER,
	MP_ADD_TRACK_VIEW_TAB_FOLDER,
	MP_ADD_TRACK_VIEW_TAB_SQUARE,
	MP_ADD_TRACK_VIEW_TAB_ALLSHARE,
	*/
}MpAddTrackViewTab_e;

typedef struct
{
	INHERIT_MP_LIST_VIEW;
	/* extention variables */
	Evas_Object *add_track_view_layout;
	Evas_Object *add_track_view_tabbar_layout;
	Evas_Object *add_track_view_tabbar;

	/* controlbar tab item */
	Elm_Object_Item *ctltab_songs;
        #ifdef MP_FEATURE_ADD_TO_INCLUDE_PLAYLIST_TAB
	Elm_Object_Item *ctltab_plist;
        #endif
	Elm_Object_Item *ctltab_album;
	Elm_Object_Item *ctltab_artist;
	Elm_Object_Item *ctltab_folders;

	/* useful flags */
	MpAddTrackViewTab_e content_tab;
	bool first_start;
	int playlist_id;
	//char *playlist_name;

	/* extention functions */
}MpAddTrackView_t;

MpAddTrackView_t *mp_add_track_view_create(Evas_Object *parent, int playlist_id);
int mp_add_track_view_destory(MpAddTrackView_t *view);
int mp_add_track_view_select_tab(MpAddTrackView_t *view, MpAddTrackViewTab_e tab);
void mp_add_track_view_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info);

#endif
