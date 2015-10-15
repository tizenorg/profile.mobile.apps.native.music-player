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

#ifndef __MP_EDIT_VIEW_H__
#define __MP_EDIT_VIEW_H__

#include "mp-list-view.h"
#ifdef 	MP_FEATURE_PERSONAL_PAGE
enum
{
	MP_EDIT_VIEW_PERSONAL_PAGE,
};

typedef enum
{
	MP_EDIT_VIEW_PERSONAL_PAGE_NONE = 0,
	MP_EDIT_VIEW_PERSONAL_PAGE_ADD,
	MP_EDIT_VIEW_PERSONAL_PAGE_REMOVE,
}MpEditViewPersonalPageType;
#endif

typedef enum
{
	MP_EDIT_VIEW_NORMAL = 0,
	MP_EDIT_VIEW_EDIT,
	MP_EDIT_VIEW_REORDER,
}MpEditViewList_Mode;

typedef struct
{
	INHERIT_MP_LIST_VIEW;
	MpList_t *ref_list;
	bool share;
        bool create_playlist;
        Ecore_Timer *back_timer;
        MpEditViewList_Mode list_mode;
	bool reorder;
#ifdef 	MP_FEATURE_PERSONAL_PAGE
	MpEditViewPersonalPageType person_page_sel;
#endif
}MpEditView_t;

#ifdef MP_FEATURE_PERSONAL_PAGE
MpEditView_t *mp_edit_view_create(Evas_Object *parent, MpList_t *list, bool share, MpEditViewPersonalPageType person_page_sel);
void mp_edit_view_notify_popup(void *data);
#else
MpEditView_t *mp_edit_view_create(Evas_Object *parent, MpList_t *list, bool share);
#endif
void mp_edit_view_delete_cb(void *data, Evas_Object * obj, void *event_info);
void mp_edit_view_remove_cb(void *data, Evas_Object * obj, void *event_info);
void mp_edit_view_list_item_reorder_update_cb(void *data, Evas_Object * obj, void *event_info);
void mp_edit_view_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info);
#endif

