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

#ifndef __MC_COMMON_H__
#define __MC_COMMON_H__

#include <Elementary.h>
#include <stdbool.h>

void mc_common_push_track_view_by_group_name(void *ad, int track_type, const char *name, int playlist_id, const char *folder_name);
bool mc_check_image_valid(Evas *evas, const char *path);
Elm_Object_Item *mc_common_toolbar_item_append(Evas_Object *obj, const char *icon,
        const char *label, Evas_Smart_Cb func,
        const void *data);
void mc_post_status_message(const char *text);
bool mc_is_call_connected(void);
void mc_common_obj_domain_text_translate(Evas_Object *obj, const char *label);
void mc_common_obj_domain_translatable_part_text_set(Evas_Object *obj, const char* part, const char* label);
bool mc_check_file_exist(const char *path);
Evas_Object *mc_widget_genlist_create(Evas_Object * parent);
bool mc_is_mmc_removed(void);
char *mc_artist_text_get(void *data, Evas_Object *obj, const char *part);
char *mc_album_text_get(void *data, Evas_Object *obj, const char *part);
char * mc_playlist_text_get(void *data, Evas_Object *obj, const char *part);
char *mc_folder_list_label_get(void *data, Evas_Object * obj, const char *part);
Evas_Object * mc_group_content_get(void *data, Evas_Object *obj, const char *part);
void mc_eext_quit_cb(void *data, Evas_Object *obj, void *event_info);
Eina_Bool mc_quit_cb(void *data, Elm_Object_Item *it);
void mc_auto_recommended_check_cb(void *data, Evas_Object *obj, void *event_info);
void mc_quit_select_cb(void *data, Evas_Object *obj, void *event_info);
Evas_Object *mc_common_load_edj(Evas_Object * parent, const char *file, const char *group);
Evas_Object *mc_widget_navigation_new(Evas_Object * parent);
void mc_common_create_fastscroller(Evas_Object *parent, Eina_Bool multiple, Evas_Object *genlist);
Evas_Object *mc_common_create_processing_popup(void *data);
const char *mc_commonl_search_markup_keyword(const char *string, char *searchword, bool *result);
Evas_Object *mc_common_create_thumb_icon(Evas_Object * obj, const char *path, int w, int h);

const char *mc_common_search_markup_keyword(const char *string, char *searchword, bool *result);

#endif
