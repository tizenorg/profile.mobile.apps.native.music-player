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

#ifndef __MP_SEARCH_VIEW_H__
#define __MP_SEARCH_VIEW_H__

#include "mp-list-view.h"
#include "mp-track-list.h"
#include "mp-search-list.h"
#include "music.h"

#define MP_SEARCH_VIEW_STYLE_EMPTY NULL//"empty/music"

typedef struct
{
	INHERIT_MP_LIST_VIEW;

	Evas_Object *search_view_layout;
	Evas_Object *search_bar;
	Evas_Object *search_base_layout;

	char *needle;
	bool needle_change;
	bool first_called;
        bool transition;

	Ecore_Timer *search_timer;
	/* extention functions */
	void (*content_set)(void *view);
}MpSearchView_t;

MpSearchView_t *mp_search_view_create(Evas_Object *parent, const char *keyword);
void mp_search_view_set_keyword(MpSearchView_t *view, const char *keyword);
int mp_search_view_destory(MpSearchView_t *view);
#endif
