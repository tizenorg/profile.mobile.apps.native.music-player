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

#ifndef __MP_MAIN_VIEW_H__
#define __MP_MAIN_VIEW_H__

#include "mp-list-view.h"
#include "mp-all-view.h"
#include "mp-track-list.h"
#include "mp-playlist-list.h"
#include "mp-album-list.h"
#include "mp-artist-list.h"
#include "mp-folder-view.h"
#include "mp-search-view.h"
#include "mp-edit-callback.h"
#include "mp-scroll-page.h"

#include "music.h"

typedef struct
{
	INHERIT_MP_LIST_VIEW;

	Evas_Object *scroll_page;
#ifdef MP_FEATURE_BOTTOM_PAGE_CONTROL
	Evas_Object *indicator;
#endif

	MpView_t *sub_views[MH_SCROLL_MAX];

	bool store_initialized;
	bool radio_initialized;

	Ecore_Timer *load_timer;
	Ecore_Timer *rotate_timer;

	/* extention functions */
}MpMainView_t;

MpMainView_t *mp_main_view_create(Evas_Object *parent, MpTab_e inint_tab);
int mp_main_view_select_tab(MpMainView_t *view, MpTab_e tab);

#endif




