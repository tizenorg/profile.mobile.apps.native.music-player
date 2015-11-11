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


#ifndef __MP_MW_LYRIC_VIEW_H__
#define __MP_MW_LYRIC_VIEW_H__

#include "mp-define.h"

typedef struct {
	struct appdata *ad;
	int win_w;
	int win_h;

	Evas_Object *layout;
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *cur_line;
	Evas_Object *prev_line;

	int cur_line_index;
	int prev_line_index;

#ifdef MP_FEATURE_SUPPORT_ID3_TAG
	char *lyric_buffer;
	Eina_List *synclrc_list;
#else
	char *lyric_path;
#endif

	char *music_path;
	bool b_drag;
	bool b_show;
} mp_mw_lyric_view_t;

void mp_mw_lyric_view_create(void *data);
void mp_mw_lyric_view_destroy(void *data);
void mp_mw_lyric_view_show(void *data);
void mp_mw_lyric_view_hide(void *data);
void mp_test_lyric_view_hide(void *data);
void mp_mw_lyric_view_refresh(void *data);

#endif /* __MP_LYRIC_VIEW_H__ */
