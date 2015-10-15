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

#ifndef __MP_MUSIC_RECOMMEND_H__
#define __MP_MUSIC_RECOMMEND_H__

typedef struct _MpRcmd MpRcmd;

typedef void(*MpRcmdCb)(int position, void *userdata);

MpRcmd *
mp_recommend_create(const char *path);

int
mp_recommend_extractable(MpRcmd *rcmd);

void
mp_recommend_extract(MpRcmd *rcmd, MpRcmdCb cb, void *user_data);

void
mp_recommend_destory(MpRcmd *rcmd);


#endif

