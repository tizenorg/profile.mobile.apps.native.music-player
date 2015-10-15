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

#ifndef __MP_SQUARE_VIEW_NEW_H__
#define __MP_SQUARE_VIEW_NEW_H__

#include "music.h"
#include "mp-square-mgr.h"

typedef struct
{
	INHERIT_MP_VIEW;
	/* extention variables */
	Evas_Object *square_layout;
	Evas_Object *gengrid;

        mp_square_item_t *current_item;

	/* external objects*/
	Evas_Object *radio_main;
	Evas_Object *popup_update_library;
	Evas_Object *popup_update_library_progress;
	Evas_Object *layout_update_library_progress;
	Evas_Object *update_library_progressbar;
	Evas_Object *popup;

	/* additional variables */
	int screen_mode;
	bool popup_status;
	mp_square_type_t type;
	mp_square_position_t now_playing_position;
	int radio_index; /* for change axis popup */

	Ecore_Timer *update_library_timer;
	bool b_mouse_down;

	GList *pos_list;
	GList *music_list;
}MpSquareView_t;

MpSquareView_t *mp_square_view_new_create(Evas_Object *parent);
int mp_square_view_new_destory(MpSquareView_t *view);

#endif
