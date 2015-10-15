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


#ifndef __MP_DB_CLIENT_H__
#define __MP_DB_CLIENT_H__

#include <glib.h>
#include <sqlite3.h>
#include "mp-square-mgr.h"

typedef struct
{
	struct appdata *ad;

	sqlite3 *sqlite_handle;

        GList *path_list;

} mp_dbc_mgr_t;


bool mp_db_client_get_mood_by_path(mp_dbc_mgr_t *dbc_mgr, int *mood, char *path);
int mp_db_client_get_paths_by_mood(mp_dbc_mgr_t *dbc_mgr, int mood);
int mp_db_client_mgr_create(struct appdata *ad);
bool mp_db_client_mgr_destory(mp_dbc_mgr_t *dbc_mgr);

#endif

