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

#ifndef __MP_ALL_VIEW_H__
#define __MP_ALL_VIEW_H__

#include "mp-list-view.h"
#include "mp-track-list.h"
#include "mp-playlist-list.h"
#include "mp-album-list.h"
#include "mp-artist-list.h"
#include "mp-genre-list.h"
#include "mp-folder-view.h"
#include "mp-search-view.h"
#include "mp-edit-callback.h"

#include "music.h"

typedef struct
{
	INHERIT_MP_LIST_VIEW;
	/* extention variables */
	Evas_Object *all_view_layout;
	Evas_Object *all_view_tabbar;
	Evas_Object *all_view_genlist;

	MpTab_e tab_status;
	int history;
	bool reorder_flag;

	/* external objects*/
	Evas_Object *radio_main;

	/* useful flags */
	//MpAllViewTab_e content_tab;
#ifdef MP_FEATURE_PERSONAL_PAGE
	bool personal_page_status;
#endif
	MpListDisplayMode_e display_mode[MP_TAB_MAX];

	Ecore_Idler *show_last_idler;
	Ecore_Timer *bringin_timer;

	/* extention functions */
}MpAllView_t;

MpAllView_t *mp_all_view_create(Evas_Object *parent, MpTab_e init_tab);
int mp_all_view_destory(MpAllView_t *view);
int mp_all_view_select_tab(MpAllView_t *view, MpTab_e tab);

#endif
