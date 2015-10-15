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

#ifndef __MP_LYRIC_H__
#define __MP_LYRIC_H__

#include <Elementary.h>

Evas_Object *mp_lyric_create(Evas_Object *parent, const char *path);
void mp_lyric_sync_update(Evas_Object *lyric);
const char *mp_lyric_get_path(Evas_Object *lyric);

#endif

