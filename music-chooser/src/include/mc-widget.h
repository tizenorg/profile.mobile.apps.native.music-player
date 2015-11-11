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


#ifndef __MC_WIDGET_H__
#define __MC_WIDGET_H__

#include <Elementary.h>

typedef enum {
	NO_CONTENT_SONG,
	NO_CONTENT_PLAYLIST,
	NO_CONTENT_ALBUM,
	NO_CONTENT_ARTIST,
} NoContentType_e;

Evas_Object *
mc_widget_create_title_icon_btn(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data);

Evas_Object *
mc_create_win(const char *name);

Evas_Object *
mc_widget_no_content_add(Evas_Object *parent, NoContentType_e type);

Evas_Object *
mc_widget_create_naviframe_toolbar(Elm_Object_Item *navi_it);

Elm_Object_Item *
mc_widget_create_toolbar_item_btn(Evas_Object *parent,
                                  const char *style, const char *text, Evas_Smart_Cb func, void *data);

Evas_Object* mc_widget_create_navi_left_btn(Evas_Object *pParent,
        Elm_Object_Item *pNaviItem, Evas_Smart_Cb pFunc, void *pUserData);
Evas_Object* mc_widget_create_navi_right_btn(Evas_Object *pParent,
        Elm_Object_Item *pNaviItem, Evas_Smart_Cb pFunc, void *pUserData);
#endif /* __MC_WIDGET_H__ */

