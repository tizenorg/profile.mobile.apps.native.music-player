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


#ifndef __MP_popup_H_
#define __MP_popup_H_

#include <Elementary.h>
#include "music.h"

typedef enum _mp_popup_t
{
	MP_POPUP_PV_SET_AS = 0,
	MP_POPUP_PV_SET_AS_INCLUDE_ADD_TO_HOME,
	MP_POPUP_PV_SHARE,
	MP_POPUP_LIST_SHARE,
	MP_POPUP_SOUND_PATH,
	MP_POPUP_ADD_TO_PLST,
	MP_POPUP_SETTING_PLAYLIST,
	MP_POPUP_LIST_LONGPRESSED,
	MP_POPUP_MORE_INFO,

#ifdef MP_FEATURE_CLOUD
	MP_POPUP_CLOUD_VIEW,
#endif
	MP_POPUP_CHANGE_LIST_DISPLAY_MODE,
	MP_POPUP_EDIT_IMAGE,
	MP_POPUP_INFO_LIST,
	MP_POPUP_UPDATE_ALBUM_ART,
	MP_POPUP_SEARCH,
	MP_POPUP_DELETE_TRACK,
} mp_popup_t;

typedef enum {
	MP_POPUP_BTN_1,
	MP_POPUP_BTN_2,
	MP_POPUP_BTN_3,
	MP_POPUP_BTN_MAX,
} popup_button_t;

typedef enum
{
	MP_POPUP_NORMAL = 0,
	MP_POPUP_GENLIST,
	MP_POPUP_CTX,
	MP_POPUP_PROGRESS,
	MP_POPUP_PROGRESS_NO_BUTTON,
	MP_POPUP_NOTIFY,	// NOT destroyed by mp_popup_destroy()
	MP_POPUP_CONTENT_EXPAND,
	MP_POPUP_ENTRY,
	MP_POPUP_PRIVACY,
	MP_POPUP_PROGRESS_WITH_CANCEL,
	MP_POPUP_PROGRESS_WITHOUT_CANCEL,
	MP_POPUP_OPERATION_PROGRESS,
#ifdef MP_FEATURE_PERSONAL_PAGE
	MP_POPUP_CHECK_INFO_PERSONAL,
#endif

	MP_POPUP_MAX,
} mp_popup_type;

typedef struct {
	char *label;	/*label for String ID*/
	char *content;	/*real content get from db*/
	void *item_data;
} Popup_genlist_item;

#define mp_popup_desc_set(obj, desc) elm_object_text_set((obj), (desc))

Elm_Object_Item *mp_genlist_popup_item_append(Evas_Object * popup, char *label, char *content, Evas_Object * icon, void *item_data, void *cb,
					       void *data);
Evas_Object *mp_genlist_popup_create(Evas_Object * parent, mp_popup_t type, void *user_data, struct appdata *ad);
Evas_Object *mp_entry_popup_create(char *title);
Evas_Object *mp_popup_create(Evas_Object * parent, mp_popup_type type, char *title, void *user_data, Evas_Smart_Cb response_cb,
			     void *ad);
void mp_popup_destroy(struct appdata *ad);
void mp_longpress_popup_destroy(struct appdata *ad);

void mp_popup_response_callback_set(Evas_Object *popup, Evas_Smart_Cb cb, void *user_data);
void mp_popup_response(Evas_Object *popup, int response);
bool mp_popup_button_set(Evas_Object *popup, popup_button_t btn_index, const char *text, int response);
void mp_popup_timeout_set(Evas_Object *popup, double timeout);
bool mp_popup_max_length(Evas_Object *entry, const char *text);
void mp_popup_back_cb(void *data, Evas_Object *obj, void *event_info);

Evas_Object *mp_popup_multi_window_center_add(Evas_Object *parent);
Evas_Object *elm_popup_win_get(Evas_Object *popup);
Evas_Object *mp_popup_message_create(Evas_Object * parent, mp_popup_type type, char *title,char *message, void *user_data, Evas_Smart_Cb response_cb,
					void *ad);


#endif // __MP_contextpopup_H_
