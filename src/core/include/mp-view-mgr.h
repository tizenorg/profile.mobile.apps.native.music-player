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

#ifndef __MP_VIEW_MGR_H__
#define __MP_VIEW_MGR_H__

#include <Elementary.h>
#include <glib.h>
#include "mp-view.h"

typedef struct {
	Evas_Object *navi;
} MpViewMgr_t;

#define GET_VIEW_MGR	mp_view_mgr_get_view_manager()
#define GET_PLAYER_VIEW mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYER)
#define GET_ALL_VIEW mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_ALL)
#define GET_NAVIFRAME	((mp_view_mgr_get_view_manager()) ? mp_view_mgr_get_view_manager()->navi : NULL)

MpViewMgr_t *mp_view_mgr_create(Evas_Object *parent);
int mp_view_mgr_destory(MpViewMgr_t *view_mgr);
EXPORT_API MpViewMgr_t *mp_view_mgr_get_view_manager();

MpView_t * mp_view_mgr_get_top_view(MpViewMgr_t *view_mgr);
EXPORT_API MpView_t * mp_view_mgr_get_view(MpViewMgr_t *view_mgr, MpViewType_e type);
EXPORT_API MpView_t * mp_view_mgr_get_view_prev(MpViewMgr_t *view_mgr, MpViewType_e type);

EXPORT_API int mp_view_mgr_push_view_with_effect(MpViewMgr_t *view_mgr, MpView_t *view, const char *item_style, bool disable_effect);
EXPORT_API int mp_view_mgr_push_view(MpViewMgr_t *view_mgr, MpView_t *view, const char *item_style);
int mp_view_mgr_pop_view(MpViewMgr_t *view_mgr, bool pop_view);
int mp_view_mgr_pop_to_view(MpViewMgr_t *view_mgr, MpViewType_e type);
int mp_view_mgr_delete_view(MpViewMgr_t *view_mgr, MpViewType_e type);
int mp_view_mgr_count_view(MpViewMgr_t *view_mgr);
int mp_view_mgr_pop_a_view(MpViewMgr_t *view_mgr, MpView_t *view);

void mp_view_mgr_post_event(MpViewMgr_t *view_mgr, MpViewEvent_e event);

#endif

