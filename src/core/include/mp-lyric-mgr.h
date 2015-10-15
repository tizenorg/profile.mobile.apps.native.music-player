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


#ifndef __MP_LYRIC_MGR_H__
#define __MP_LYRIC_MGR_H__

#include "mp-define.h"

#define MP_LRC_LINE_BUF_LEN (int)255 /* The max length of one line string buffer */


typedef enum
{
	MP_LYRIC_SOURCE_BUFFER=0,
	MP_LYRIC_SOURCE_LIST,
	MP_LYRIC_SOURCE_FILE,
}mp_lyric_source_type;

typedef struct
{
	long time;
	char *lyric;
}mp_lrc_node_t;

typedef struct
{
	char *title;
	char *artist;
	char *album;
	long offset; /* The offset of all time tags */
	Eina_List *synclrc_list;
	Eina_List *unsynclrc_list;
}mp_lyric_mgr_t;

mp_lyric_mgr_t *mp_lyric_mgr_create(const char *path);
void mp_lyric_mgr_destory(mp_lyric_mgr_t *lyric_mgr);

#endif /* __MP_LYRIC_MGR_H__ */
