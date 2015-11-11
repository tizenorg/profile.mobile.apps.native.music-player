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


#ifndef __MP_MENU_H_
#define __MP_MENU_H_

#include "mp-media-info.h"
typedef enum {
	MP_MENU_FUNC_ADD_TO_LIST = 0,
	MP_MENU_FUNC_DELETE,
} mp_menu_func_type;

void mp_menu_ctxpopup_share_select_cb(void *data, Evas_Object * obj, void *event_info);
void mp_menu_ctxpopup_set_as_select_cb(void *data, Evas_Object * obj, void *event_info);
void mp_menu_genlist_set_as_select_cb(void *data, Evas_Object * obj, void *event_info);

void mp_menu_genlist_popup_share_select_cb(void *data, Evas_Object * obj, void *event_info);
void mp_menu_genlist_popup_list_share_select_cb(void *data, Evas_Object * obj, void *event_info);

#ifdef MP_FEATURE_ADD_TO_HOME
void mp_menu_add_to_home(int type, void *main_info, void *extra_info1, void *extra_info2);
#endif
void mp_menu_share_cb(void *data, Evas_Object * obj, void *event_info);
void mp_menu_set_cb(void *data, Evas_Object * obj, void *event_info);
#ifndef MP_SOUND_PLAYER
void mp_menu_delete_cb(void *data, Evas_Object * obj, void *event_info);
void mp_menu_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info);
mp_track_type_e mp_menu_get_track_type_by_group(mp_group_type_e group_type);
void mp_menu_add_to_play_list_cancel_create_cb(void *data, Evas_Object * obj, void *event_info);
void mp_menu_add_to_play_list_done_create_cb(void *data, Evas_Object * obj, void *event_info);
#endif

#define ADD_TO_HOME         ("IDS_MUSIC_SK2_ADD_TO_HOME")
#define CALLER_RINGTONE   ("IDS_MUSIC_OPT_SETAS_CALLER_RINGTONE")
#define ALARM_TONE            ("IDS_MUSIC_OPT_SETAS_ALARM_TONE")
#define CALL_RINGTONE       ("IDS_MUSIC_POP_PHONE_RINGTONE")

//system string
#define BLUETOOTH_SYS      GET_SYS_STR("IDS_COM_BODY_BLUETOOTH")
#define ALLSHARE_SYS         GET_SYS_STR("IDS_COM_BODY_ALLSHARE")
#define EMAIL_SYS               GET_SYS_STR("IDS_COM_BODY_EMAIL")
#define MESSAGE_SYS		GET_SYS_STR("IDS_COM_BODY_MESSAGE")
#define WIFI_SYS 		GET_SYS_STR("IDS_COM_BODY_WI_FI")
#define NFC_SYS			GET_SYS_STR("IDS_COM_BODY_NFC")

#endif // __MP_MENU_H_
