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

#ifndef __MP_SET_AS_VIEW__
#define __MP_SET_AS_VIEW__

#include "mp-view.h"
#include "mp-media-info.h"
#include <player.h>

typedef enum
{
	MP_SET_AS_FROM_START = 0,
	MP_SET_AS_RECOMMEND,
	MP_SET_AS_RECOMMEND_PRE_LISTEN,
	MP_SET_AS_TITLE,
	MP_SET_AS_PHONE_RINGTONE,
	MP_SET_AS_CALLER_RINGTONE,
	MP_SET_AS_ALARM_TONE,
	MP_SET_AS_MAX,
} set_as_item_type;


typedef struct
{
	INHERIT_MP_VIEW;
	Evas_Object *content;
	Evas_Object *progress_popup;
	Elm_Genlist_Item_Class *radio_itc;
	Elm_Genlist_Item_Class *title_itc;
	Elm_Genlist_Item_Class *recommend_itc_full;
	Elm_Genlist_Item_Class *recommend_itc_text;
	char* path;
	bool button_enable;

	int recommended;
	int set_as_type;

	int duration;
	int position;
	player_h player;
	bool need_to_resume;
	Ecore_Pipe *smat_pipe;

	Ecore_Idler *move_idler;
}MpSetAsView_t;

EXPORT_API MpSetAsView_t *mp_set_as_view_create(Evas_Object *parent,char* path);
int mp_set_as_view_destory(MpSetAsView_t *view);
#endif







