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


#ifndef __mp_common_H__
#define __mp_common_H__

#include <Elementary.h>
#include <app_control.h>
#include "music.h"
#include "mp-list.h"
#include "mp-widget.h"
#include "mp-player-control.h"
#include "mp-all-view.h"

#ifdef MP_FEATURE_STORE
#include "mh-common.h"
#endif

struct text_part {
	char *part;
	char *msgid;
};

#ifdef MP_FEATURE_PERSONAL_PAGE
typedef enum {
	MP_COMMON_ALL_ERROR = -1,
	MP_COMMON_ALL_IN,
	MP_COMMON_ALL_OUT,
	MP_COMMON_PART
} all_in_personal_e;
#endif

void mp_common_show_setting_view(void);
void mp_common_show_set_as_view();
void mp_common_show_edit_view(void *list_view, mp_done_operator_type_t type);
void mp_common_add_to_playlsit_view(void *list_view);
void mp_common_view_cancel_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_create_search_view_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_share_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_genlist_checkbox_tts_double_action_sel_cb(void *data, Evas_Object * obj, Elm_Object_Item *item_data);
void mp_common_genlist_checkbox_sel_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_ctxpopup_setting_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_ctxpopup_end_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_cloud_view_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_playlist_rename_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_playlist_album_update(mp_media_info_h playlist_handle);
void mp_common_ctxpopup_make_offline_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_ctxpopup_add_to_playlist_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_go_to_library_cb(void *data, Evas_Object *obj, void *event_info);
void mp_common_sweep_share_cb(void *data, Evas_Object * obj, void *event_info);

void mp_common_list_delete_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_list_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_list_more_info_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_list_add_to_favorite_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_list_unfavorite_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_list_set_as_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_list_update_albumart_cb(void *data, Evas_Object * obj, void *event_info);

void mp_common_button_add_to_playlist_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_set_list_to_reorder_view(void *list_view);
void mp_common_button_share_list_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_button_delete_list_cb(void *data, Evas_Object * obj, void *event_info);
Evas_Object *mp_common_create_more_ctxpopup(void *view);
void p_common_show_player_view_after_play();
void mp_common_create_default_playlist();
void mp_common_show_player_view(int launch_type, bool disable_effect, bool start_playback, bool start_new_file);
void mp_common_play_track_list_with_playlist_id(mp_list_item_data_t *item, Evas_Object *genlist, int playlist_id);
void mp_common_play_track_list(mp_list_item_data_t *item, Evas_Object *genlist);
void mp_common_search_by(const char *keyword);
void mp_common_list_remove_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_view_check_changed_cb(void *data, Evas_Object *obj, void *event_info);
bool mp_common_track_is_current(mp_media_info_h media, MpList_t *list);
void mp_common_track_delete_cb(void *data, Evas_Object * obj, void *event_info);

#ifdef MP_FEATURE_STORE
MpView_t *mp_common_get_scroll_view(MpViewMgr_t *view_mgr);
void mp_common_go_to_store_cb(void *data, Evas_Object *obj, void *event_info);
#endif
#ifndef MP_SOUND_PLAYER
bool _mp_common_parse_open_shortcut(app_control_h app_control, MpTab_e *tab, char **shortcut_main_info);
#endif
MpView_t *mp_common_get_all_view();
void mp_common_show_add_tracks_view(int playlist_id);
void mp_common_create_playlist_mgr(void);
void mp_common_popup_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
bool mp_common_parse_view_operation(app_control_h app_control);
void mp_common_create_initial_view(void *ad, app_control_h app_control, int *launch_by_shortcut);
void mp_common_playall_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_add_to_home_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_force_close_delete();
int mp_common_get_playlist_totaltime(mp_track_type_e track_type, int playlist_id, int count);

#ifdef MP_FEATURE_PERSONAL_PAGE
all_in_personal_e mp_common_is_all_in_personal_page(Evas_Object *genlist);
void mp_common_add_to_personal_page_cb(void *data, Evas_Object * obj, void *event_info);
void mp_common_remove_from_personal_page_cb(void *data, Evas_Object * obj, void *event_info);
all_in_personal_e mp_common_personal_status(void *thiz);
void  mp_common_longpress_private_move_cb(void *data, Evas_Object * obj, void *event_info);
#endif

#define MP_GENLIST_ITEM_LONG_PRESSED(genlist, popup, obj_item)\
	do{\
		elm_genlist_item_selected_set(obj_item, EINA_FALSE);\
		if(popup){\
			evas_object_data_set(genlist, "popup", popup);\
			elm_object_scroll_freeze_push(genlist);\
			evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, mp_common_popup_del_cb, genlist);\
		}\
	}while(0);

#define MP_GENGRID_ITEM_LONG_PRESSED(genlist, popup, obj_item)\
	do{\
		elm_gengrid_item_selected_set(obj_item, EINA_FALSE);\
		if(popup){\
			evas_object_data_set(genlist, "popup", popup);\
			elm_object_scroll_freeze_push(genlist);\
			evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, mp_common_popup_del_cb, genlist);\
		}\
	}while(0);


#define MP_LIST_ITEM_IGNORE_SELECT(obj)\
	do{\
		Evas_Object *popup = evas_object_data_get(obj, "popup");\
		if (popup) return;\
	}while(0);

#endif // __mp_common_H__
