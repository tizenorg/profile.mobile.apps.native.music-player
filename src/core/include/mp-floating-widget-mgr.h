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

#ifndef __MP_FLOATING_WIDGET_MGR__
#define __MP_FLOATING_WIDGET_MGR__

#include "music.h"

typedef void (*mp_floaing_widget_cb)(bool show, int x, int y, int w, int h, void *data);

typedef struct MpFWMgr_t *MpFwMgr;

MpFwMgr mp_floating_widget_mgr_create(Evas_Object *genlist);
void mp_floating_widget_mgr_destroy(MpFwMgr FwMgr);
/**
*	position - position in list
*	order - order between floating widgets
**/
void mp_floating_widget_callback_add(MpFwMgr FwMgr,
                                     int content_h, int position, int index, mp_floaing_widget_cb cb , void *data);

void mp_floating_widget_mgr_widget_deleted(MpFwMgr FwMgr, int index);
bool mp_floating_widget_mgr_visible_get(MpFwMgr FwMgr, int index);

#endif

