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


#ifndef __MP_DETAIL_VIEW__
#define __MP_DETAIL_VIEW__

#include "mp-view.h"
#include <glib.h>
//#include <libsoup/soup.h>
//#include "mp-info-popup.h"

enum {
	DETAIL_VIEW_MOVE_NONE,
	DETAIL_VIEW_MOVE_LEFT,
	DETAIL_VIEW_MOVE_RIGHT,
};

enum {
	DETAIL_VIEW_PAGE_VIDEO = 1,
	DETAIL_VIEW_PAGE_CREDIT,
	DETAIL_VIEW_PAGE_META,
};

typedef enum {
	DETAIL_VIEW_ITC_NO_META,
	DETAIL_VIEW_ITC_VIDEO,
	DETAIL_VIEW_ITC_CREDIT,
	DETAIL_VIEW_ITC_META_INFO,
} mp_detail_view_itc;

typedef struct {
	INHERIT_MP_VIEW;
	Elm_Object_Item *inner_navi_it;
	//add pagecontrol begin
	Evas_Object *title_index;
	Evas_Object *index_layout;
	//add pagecontrol end
	Evas_Object *content;
	Evas_Object *current_song_title;
	Evas_Object *current_page;
	Evas_Object *current_page_content;
	Evas_Object *next_page_content;

	Evas_Object *album_booklet;
	Evas_Object *artist_booklet;

	Evas_Object *waiting_popup;	//add waiting popup
	Evas_Object *info_list_popup;	//add waiting popup
	Evas_Object *popup;	//add  popup

	/*data part related*/
	char *xml_path;

	int mp_info_file_count;	//the sequence of file which is used as the identification of the file
	//used when xml exists
	bool transition_state;
	unsigned int page_total;
	int page_num;
	Elm_Genlist_Item_Class *video_itc;
	Evas_Object *video_genlist;
	bool page1_enabled; //flag of page1 exists or not
	bool fail_to_get_related_vided;		// prohibit infinite loop
	Elm_Genlist_Item_Class *credit_itc;
	Evas_Object *credit_genlist;
	bool page2_enabled; //flag of page2 exists or not
	Elm_Genlist_Item_Class *meta_itc;
	Evas_Object *meta_genlist;
	bool page3_enabled; //flag of page3 exists or not
	//used when xml does not exist
	//Elm_Genlist_Item_Class *minfo_itc;
	Evas_Object *minfo_genlist;

	char *title;
	char *uri;
	char *albumart;
	char *artist;
	char *album;
	char *id;
	char *thumb;
} MpDetailView_t;

MpDetailView_t *mp_detail_view_create(Evas_Object *parent);
int mp_detail_view_destory(MpDetailView_t *view);
bool mp_player_mgr_is_active(void);
player_h mp_player_mgr_get_player(void);

#endif


