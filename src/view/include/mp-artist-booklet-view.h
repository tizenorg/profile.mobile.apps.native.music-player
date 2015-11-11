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

#ifndef __MP_ARTIST_BOOKLETVIEW_H__
#define __MP_ARTIST_BOOKLETVIEW_H__


#include "music.h"
#include "mp-info-data.h"
enum {
	ARTIST_BOOKLET_VIEW_DISCOGRAPHY = 0,
	ARTIST_BOOKLET_VIEW_BIOGRAPHY,
	ARTIST_BOOKLET_VIEW_SIMILAR_ARTIST,
	ARTIST_BOOKLET_VIEW_MAX,
};
typedef struct {
	INHERIT_MP_VIEW;

	int page_index[ARTIST_BOOKLET_VIEW_MAX];
	int page_count;

	Evas_Object *title_layout;


	Evas_Object *title_index;
	Evas_Object *current_song_title;
	Evas_Object *current_page;
	Evas_Object *current_page_content;
	Evas_Object *next_page_content;

	Evas_Object *genlist[ARTIST_BOOKLET_VIEW_MAX];
	Elm_Genlist_Item_Class *info_itc;
	Elm_Genlist_Item_Class *title_itc;
	Elm_Genlist_Item_Class *content_itc;

	/*page 1*/
	Evas_Object *artist_index;
	Evas_Object *disco_gengrid;
	Elm_Gengrid_Item_Class *disco_itc;
	bool page1_enabled; //flag of page exists or not
	/*page 2*/
	Evas_Object *image;
	Evas_Object *text;
	bool page2_enabled; //flag of page exists or not
	/*page 3*/
	Evas_Object *artist_genlist;
	Elm_Genlist_Item_Class *artist_itc;
	bool page3_enabled; //flag of page exists or not
	/* external objects*/
	mp_info_data_t *info_data;
	char *name;
	char *artist;
	char *thumbnail;

	bool transition_state;
	int page_num;

	Elm_Object_Item *gengrid_item;
} MpArtistBookletView_t;

MpArtistBookletView_t *mp_artist_booklet_view_create(Evas_Object *parent, mp_info_data_t *info_data, char *album, char *artist, char *thumbnail);
#endif
