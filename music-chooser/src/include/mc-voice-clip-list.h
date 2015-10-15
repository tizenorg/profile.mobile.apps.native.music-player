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

#ifndef __MC_VOICE_CLIP_LIST_H__
#define __MC_VOICE_CLIP_LIST_H__

#include "music-chooser.h"
#include "mp-media-info.h"

Evas_Object *mc_voice_clip_list_create(Evas_Object *parent, struct app_data *ad);
int mc_voice_clip_list_update(Evas_Object *list);

#endif

