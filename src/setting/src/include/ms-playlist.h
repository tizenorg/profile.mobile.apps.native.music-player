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

#ifndef __MS_PLAYLIST_H__
#define __MS_PLAYLIST_H__

#include <Elementary.h>
#include "ms-util.h"
#include "mp-setting-view.h"


EXPORT_API Evas_Object* ms_playlist_list_create(MpSettingView_t *view, Evas_Object *parent);
EXPORT_API Evas_Object *_ms_playlist_append_pop_genlist(Evas_Object *genlist, Evas_Object *parent);
EXPORT_API int ms_playlist_check_state_get_val(int *b_val);
EXPORT_API int ms_playlist_check_state_set_val(int b_val);



#endif

