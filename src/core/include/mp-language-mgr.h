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

#ifndef __MP_LANGUAGE_CHANGER_H
#define __MP_LANGUAGE_CHANGER_H

#include <Elementary.h>

typedef enum {
	OBJ_TYPE_ELM_OBJECT, 		//elm_object_text_set(obj, text)
	OBJ_TYPE_EDJE_OBJECT, 	//edje_object_part_text_set(obj, part, text)
	OBJ_TYPE_MAX,
} obj_type;

typedef void (*Mp_Language_Update_Func)(Evas_Object *obj, const char *part, const char *ids);

int mp_language_mgr_create();
int mp_language_mgr_destroy();

/*part and string_id must be static*/
void mp_language_mgr_register_object(Evas_Object *obj, obj_type type, const char *part, const char *string_id);
void mp_language_mgr_register_object_func_set(Evas_Object *obj, const char *part, const char *text_id, Mp_Language_Update_Func func);
void mp_language_mgr_unregister_object_item(Elm_Object_Item *object_item);
void mp_language_mgr_object_item_text_ID_set(Elm_Object_Item *object_item, const char *text_ID);

void mp_language_mgr_unregister_genlist_item(Elm_Object_Item *item);

void mp_language_mgr_register_gengrid_item(Elm_Object_Item *item);
void mp_language_mgr_unregister_gengrid_item(Elm_Object_Item *item);

void mp_language_mgr_update();

#endif

