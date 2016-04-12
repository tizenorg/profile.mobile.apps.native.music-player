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

#ifndef __MP_INDEX_H__
#define __MP_INDEX_H__

#include <Elementary.h>
#include <mp-list.h>

typedef void (*MpIndexCb)(void *data);


Evas_Object *mp_index_create(Evas_Object *parent, int group_type, void *data);
void mp_index_append_item(Evas_Object *index, MpList_t *list);

#endif

