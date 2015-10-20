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

#ifndef __MC_INDEX_H__
#define __MC_INDEX_H__

#include <Elementary.h>

typedef void (*UgMpIndexCb)(void *data);

Evas_Object *mc_index_create(Evas_Object *parent, int group_type, void *data);
void mc_index_append_item(Evas_Object *index, Evas_Object *list);

#endif

