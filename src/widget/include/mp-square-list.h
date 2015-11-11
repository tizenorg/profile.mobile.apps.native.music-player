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

#ifndef __MP_SQUARE_LIST_H__
#define __MP_SQUARE_LIST_H__

#include "mp-list.h"

enum {
	MP_SQUARE_LIST_ATTR_HIGHLIGHT_CURRENT,
};

typedef struct {
	INHERIT_MP_LIST

	void (*set_edit_default)(void *thiz, bool edit);

	Elm_Genlist_Item_Class *itc;
	Elm_Genlist_Item_Class *itc_shuffle;
	Elm_Object_Item *shuffle_it;

	bool highlight_current;
} MpSquareList_t;

MpSquareList_t * mp_square_list_create(Evas_Object *parent);
void mp_square_list_set_data(MpSquareList_t *list, ...);
void mp_square_list_remove_selected_item(MpSquareList_t *list);
void mp_square_list_refresh(MpSquareList_t *list);
void mp_square_list_current_item_bring_in(MpSquareList_t *list);
void mp_square_list_update_genlist(void *thiz);

#endif

