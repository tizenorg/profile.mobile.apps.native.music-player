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
#ifndef __MC_SEARCH_VIEW_H__
#define __MC_SEARCH_VIEW_H__

#include "mc-search-list.h"
#include "music-chooser.h"

#define MC_SEARCH_VIEW_STYLE_EMPTY "empty/music"

typedef struct{
	struct app_data *ad;

	Elm_Object_Item *navi_it;
	Evas_Object *layout;
	Evas_Object *searchbar_layout;
	Evas_Object *search_layout;
	Evas_Object *search_bar;
	Evas_Object *entry;
	Evas_Object *list_object;
	Evas_Object *no_content;
	Evas_Object *genlist;

	Ecore_Timer *search_timer;
	char *needle;

	UgMpSearchList_t *list;

	mp_group_type_e type;

	char *filter_text; 	//free
	char *title;		//not free

	int count;
	mp_media_list_h media_list;
	mp_media_list_h defualt_playlist;

}search_view_data_t;


search_view_data_t *mc_search_view_create(Evas_Object *parent, struct app_data *ad);
int mc_search_view_update_options(void *thiz);
void mc_search_view_set_keyword(search_view_data_t *search, const char *keyword);
int mc_search_view_destory(search_view_data_t *search);
#endif
