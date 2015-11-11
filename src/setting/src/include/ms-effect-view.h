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

#ifndef __ms_effect_view_H__
#define __ms_effect_view_H__
#include <Elementary.h>
#include "ms-util.h"
#include "mp-setting-view.h"

typedef struct {

	Elm_Object_Item *selected_item;
	Evas_Object *layout;

} MsEffectView_t;

EXPORT_API Evas_Object * ms_effect_view_create(Evas_Object *parent, MpSettingView_t *view, Elm_Object_Item *parent_item);
EXPORT_API void ms_effect_view_update_option(MpSettingView_t *view);
EXPORT_API void ms_effect_view_cancel_btn_cb(void *data, Evas_Object *obj, void *event_info);
EXPORT_API void ms_effect_view_save_btn_cb(void *data, Evas_Object *obj, void *event_info);
EXPORT_API void ms_auto_check_refresh(MpSettingView_t *view);
EXPORT_API void ms_effect_view_refresh(MpSettingView_t *data);
#ifdef MP_FEATURE_LANDSCAPE
EXPORT_API void ms_effect_view_rotate(MpSettingView_t *view, bool landscape_mode);
#endif

#endif //__ms_effect_view_H__
