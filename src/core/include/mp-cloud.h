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

#ifndef __MP_CLOUD_H__
#define __MP_CLOUD_H__

#include "mp-playlist-mgr.h"

typedef enum
{
	MP_CLOUD_PLAY_OFFLINE = 0,
	MP_CLOUD_PLAY_STREAMING = 1,
	MP_CLOUD_PLAY_UNAVAILABLE = 2,
}mp_cloud_playable_e;

int mp_cloud_create(void);
int mp_cloud_destroy(void);
mp_cloud_playable_e mp_cloud_play_available(const char *uuid, mp_plst_item *plst_item);
int mp_cloud_delete_content(const char *uuid, int delete_server);
int mp_cloud_is_on(int *is_on);
int mp_cloud_cancel_request(int cancel_id);
int mp_cloud_make_offline_available(const char *uuid);

#endif

