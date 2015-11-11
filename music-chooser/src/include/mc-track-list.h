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

#ifndef __MC_TRACK_LIST_H__
#define __MC_TRACK_LIST_H__

#include <player.h>
#include "music-chooser.h"
#include "mp-media-info.h"

#define DEF_STR_LEN 512

typedef struct {
	Elm_Object_Item *it;	// Genlist Item pointer
	Eina_Bool checked;	// Check status
	player_state_e state;
	mp_media_info_h media;
	struct app_data *ad;
	int index;
	mc_list_type_e list_type;
	int start_time;
	Eina_Bool checkbox_cb;
} list_item_data_t;


Evas_Object *mc_track_list_create(Evas_Object *parent, struct app_data *ad);
int mc_track_list_set_data(Evas_Object *list, int track_type, const char *type_str, int playlist_id);
void mc_track_list_set_uri_selected(void *thiz, const char *uri);
int mc_track_list_update(Evas_Object *list, Elm_Object_Item *navi_it, Evas_Object *sub_view);
int mc_track_list_get_radio();
bool mc_widget_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc, Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout);
void _mc_popup_view(void *data);
void _mc_track_list_select_cb(void *data, Evas_Object *obj, void *event_info);
void _mc_track_list_select_all_selected_item_data_get(void *data, Evas_Object *obj, void *event_info);
char *mc_create_selectioninfo_text_with_count(int count);
unsigned int _get_select_count(void *data);

#endif

