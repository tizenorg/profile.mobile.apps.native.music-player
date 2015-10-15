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

#ifndef __MP_SQUARE_LAYOUT_H__
#define __MP_SQUARE_LAYOUT_H__

#include "music.h"
#include "mp-square-mgr.h"
#include "mp-list.h"

typedef struct
{
        INHERIT_MP_LIST

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

        Ecore_Timer     *init_timer;

	GList *pos_list;
	GList *music_list;
}MpSquareAllList_t;

MpSquareAllList_t *mp_square_all_list_create(Evas_Object *parent);
Evas_Object *mp_square_content_create(void *thiz);
int mp_square_update(void *thiz);
Eina_Bool mp_square_list_update_square(void *data);
void mp_square_gengrid_title_set(MpSquareAllList_t *square);
int mp_square_playlist_position_update(void *thiz);
void mp_square_gengrid_items_state_reset(MpSquareAllList_t *square);
void mp_square_list_gengrid_title_set(MpSquareAllList_t *list);
void mp_square_destory(void * thiz);
void mp_square_change_axis_popup(void *data);
void mp_square_library_update(MpSquareAllList_t *square);
Evas_Object *mp_square_list_get_content(void *thiz);

#endif
