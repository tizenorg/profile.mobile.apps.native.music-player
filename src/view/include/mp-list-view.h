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

#ifndef __MP_LIST_VIEW__
#define __MP_LIST_VIEW__

#include "mp-view.h"
#include "mp-list.h"

#define INHERIT_MP_LIST_VIEW	\
	INHERIT_MP_VIEW\
	int list_view_magic;\
	MpList_t *content_to_show;\
	Evas_Object *select_all_btn;\
	Evas_Object *done_btn;\
	Evas_Object *cancel_btn;\
	Evas_Object *index;\
	Evas_Object *select_all_layout;\
	int (*double_tap)(void *view);\
	int (*set_edit_mode)(void *view,  bool edit);\
	Evas_Object* (*set_select_all)(void *view,  bool flag);\
	Evas_Object* (*set_done_button)(void *view,  bool flag);\
	Evas_Object* (*set_cancel_button)(void *view,  bool flag);\

typedef struct
{
	INHERIT_MP_VIEW;
	int list_view_magic;

	/* selected content */
	MpList_t *content_to_show;

	Evas_Object *select_all_btn;
	Evas_Object *done_btn;
	Evas_Object *cancel_btn;
	Evas_Object *index;
	Evas_Object *select_all_layout;

	int (*double_tap)(void *view);
	int (*set_edit_mode)(void *view,  bool edit);
	Evas_Object* (*set_select_all)(void *view,  bool flag);
	Evas_Object* (*set_done_button)(void *view,  bool flag,mp_done_operator_type_t type);
	Evas_Object* (*set_cancel_button)(void *view,  bool flag);

}MpListView_t;

int mp_list_view_init(Evas_Object *parent, MpListView_t *view, MpViewType_e view_type, ...);
int mp_list_view_fini(MpListView_t *view);
int mp_list_view_is_list_view(MpListView_t *view, bool *val);
int mp_list_view_set_edit_mode(MpListView_t *view, bool edit);
Evas_Object *mp_list_view_set_select_all(MpListView_t *view, bool flag);
int mp_list_view_double_tap(MpListView_t *view);
Evas_Object *mp_list_view_set_cancel_btn(MpListView_t *view,  bool flag);
Evas_Object *mp_list_view_set_done_btn(MpListView_t *view,  bool flag,mp_done_operator_type_t type);
#endif

