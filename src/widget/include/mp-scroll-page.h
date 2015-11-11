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

#ifndef __MP_SCROLL_PAGE_H__
#define __MP_SCROLL_PAGE_H__

#include <Elementary.h>

typedef enum {
	SCROLL_PAGE_MIN = -1,
	SCROLL_PAGE_STORE,
	SCROLL_PAGE_RADIO,
	SCROLL_PAGE_PLAYER,
	SCROLL_PAGE_MAX
} MpScrollPageType_e;

typedef void (*page_change_callback)(int page, void *user_data);

Evas_Object *mp_scroll_page_add(Evas_Object *parent);
void mp_scroll_page_content_append(Evas_Object *obj, Evas_Object *content);
void mp_scroll_page_content_append_typed(Evas_Object *obj, Evas_Object *content, MpScrollPageType_e page_type);
void mp_scroll_page_content_pre_append(Evas_Object *obj, Evas_Object *content);
void mp_scroll_page_remove(Evas_Object *obj, MpScrollPageType_e page_type);
void mp_scroll_page_set_page_change_callback(Evas_Object *scroll_page, page_change_callback callback, void *userdata);
void mp_scroll_page_set_page_location(Evas_Object *obj, MpScrollPageType_e page_type);
MpScrollPageType_e
mp_scroll_page_get_current_page_type(Evas_Object *obj);
Evas_Object *
mp_scroll_page_index_icon_add(Evas_Object *parent, unsigned int total, int index);
void mp_scroll_page_hide_scroll_bar(Evas_Object *obj);

#endif /* __MP_SCROLL_PAGE_H__ */
