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

#include "mp-recommend.h"
#include "mp-define.h"

struct _MpRcmd{
	MpRcmdCb cb;
	void *user_data;
	Ecore_Timer *timer;
};

MpRcmd *
mp_recommend_create(const char *path)
{
	MpRcmd *rcmd = calloc(1, sizeof(MpRcmd));

	return rcmd;
}

int
mp_recommend_extractable(MpRcmd *rcmd)
{
	int ret = true;

	return ret;
}

static Eina_Bool
_dummy_complete_cb(void *data)
{
	MpRcmd *rcmd = data;
	MP_CHECK_FALSE(rcmd);

	if (rcmd->cb)
		rcmd->cb(5000, rcmd->user_data);

	rcmd->timer = NULL;
	return EINA_FALSE;
}

void
mp_recommend_extract(MpRcmd *rcmd, MpRcmdCb cb, void *user_data)
{
	MP_CHECK(rcmd);
	MP_CHECK(cb);

	rcmd->cb = cb;
	rcmd->user_data = user_data;

	rcmd->timer = ecore_timer_add(3, _dummy_complete_cb, rcmd);
}

void
mp_recommend_destory(MpRcmd *rcmd)
{
	MP_CHECK(rcmd);

	mp_ecore_timer_del(rcmd->timer);

	free(rcmd);
}

