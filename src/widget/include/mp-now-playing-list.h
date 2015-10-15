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

#ifndef __MP_NOW_PLAYING_LIST_H__
#define __MP_NOW_PLAYING_LIST_H__

#include "mp-list.h"

enum {
	MP_NOW_PLAYING_LIST_ATTR_HIGHLIGHT_CURRENT,
};

typedef struct {
	INHERIT_MP_LIST

	Elm_Genlist_Item_Class *itc;

	bool highlight_current;

	Ecore_Timer *loading_timer;
	Evas_Object *loading_progress;
        Ecore_Idler *sel_idler;
}MpNowPlayingList_t;

MpNowPlayingList_t * mp_now_playing_list_create(Evas_Object *parent);
void mp_now_playing_list_set_data(MpNowPlayingList_t *list, ...);
void mp_now_playing_list_remove_selected_item(MpNowPlayingList_t *list);
void mp_now_playing_list_refresh(MpNowPlayingList_t *list);
void mp_now_playing_list_current_item_show(MpNowPlayingList_t *list);

#endif

