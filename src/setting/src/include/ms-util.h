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


#ifndef __ms_utils_H__
#define __ms_utils_H__
#include <Elementary.h>
#include <stdbool.h>
#include "mp-player-debug.h"
#include "mp-util.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__((__visibility__("default")))
#endif

#ifndef DOMAIN_NAME
#define DOMAIN_NAME "music-player"
#endif

#ifndef GET_SYS_STR
#define GET_SYS_STR(str) dgettext(DOMAIN_NAME, str)
#endif


Eina_Bool ms_util_is_earjack_connected(void);
bool ms_util_is_sound_device_connected(void);
void ms_util_domain_translatable_part_text_set(Evas_Object *obj, const char* part, const char* text);
void ms_util_object_item_translate_set(Elm_Object_Item *item, const char *ID);
void ms_util_domain_translatable_text_set(Evas_Object *obj, const char* text);
Evas_Object *ms_screen_reader_set_sub_obj_info_full(Evas_Object *parent, Evas_Object *obj,
        const char *part, const char *info,
        const char *type, const char *context,
        Elm_Access_Activate_Cb activate_callback, void *activate_data,
        Elm_Access_Info_Cb info_callback, void *info_data);

#endif //__ms_utils_H__
