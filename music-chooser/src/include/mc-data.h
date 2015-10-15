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

#ifndef __MC_DATA_H__
#define __MC_DATA_H__

typedef struct _mc_list_data *app_data;

typedef enum
{
	MP_DATA_ARTIST,
	MP_DATA_ALBUM,
	MP_DATA_PLAYLIST,
}mp_data_e;

int app_data_count(mp_data_e type, char *filter_text);
int app_data_create(mp_data_e type, char *filter_text, app_data *data);
int app_data_destory(app_data data);

int app_data_get_main_text();

#endif

