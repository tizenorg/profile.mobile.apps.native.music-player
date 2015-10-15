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

#ifndef __MP_LIST_H__
#define __MP_LIST_H__

#include <Elementary.h>
#include "music.h"
#include "mp-player-debug.h"
#include "mp-floating-widget-mgr.h"
#include "mp-popup.h"

typedef enum
{
	MP_LIST_TYPE_TRACK,
	MP_LIST_TYPE_GROUP,
	MP_LIST_TYPE_PLAYLIST,
        MP_LIST_TYPE_ALBUM_DETAIL,
	MP_LIST_TYPE_ARTIST_DETAIL,
	MP_LIST_TYPE_ALL,
        MP_LIST_TYPE_SQUARE,
	MP_LIST_TYPE_MAX,
}MpListType_e;

typedef enum
{
	MP_LIST_FUNC_NORMAL,	//in all-view
	MP_LIST_FUNC_ADD_TRACK,	//in add-track-view
	MP_LIST_FUNC_MAX,
}MpListFunction_e;

typedef enum
{
	MP_LIST_DISPLAY_MODE_NORMAL,
	MP_LIST_DISPLAY_MODE_THUMBNAIL,
	MP_LIST_DISPLAY_MODE_MAX,
} MpListDisplayMode_e;				// view as

typedef enum
{
	MP_LIST_EDIT_TYPE_NORMAL,
	MP_LIST_EDIT_TYPE_SHARE,
} MpListEditType_e;

typedef enum
{
	MP_LIST_ITEM_TYPE_NORMAL	= 0,
        MP_LIST_ITEM_TYPE_SHUFFLE,
	MP_LIST_ITEM_TYPE_GROUP_TITLE,
	MP_LIST_ITEM_TYPE_SELECTABLE_GROUP_TITLE,
	MP_LIST_ITEM_TYPE_ALBUMART_INDEX,
	MP_LIST_ITEM_TYPE_BOTTOM_COUNTER,
} MpListItemType_e;

typedef enum
{
	MP_TRACK_LIST_VIEW_ALL,
	MP_TRACK_LIST_VIEW_LOCAL,
	MP_TRACK_LIST_VIEW_CLOUD,
	MP_TRACK_LIST_VIEW_MAX,
}MpCloudView_e;

typedef enum {
	MP_TRACK_LIST_INDEX_NONE,
	MP_TRACK_LIST_INDEX_ALBUM_ART_LIST,
} MpTrackListIndex_t;

#ifdef MP_FEATURE_PERSONAL_PAGE
typedef enum {
	MP_LIST_PERSONAL_PAGE_NONE,
	MP_LIST_PERSONAL_PAGE_ADD,
	MP_LIST_PERSONAL_PAGE_REMOVE,
} MpListPersonalPage_t;

typedef enum {
	MP_LIST_PERSONAL_PAGE_NORMAL,
	MP_LIST_PERSONAL_PAGE_PRIVATE,
} MpListPersonalPageLocation_t;


typedef enum {
	MP_LIST_PERSONAL_ALL_ERROR = -1,
	MP_LIST_PERSONAL_ALL_IN,
	MP_LIST_PERSONAL_ALL_OUT,
	MP_LIST_PERSONAL_PART
} MpListPersonalStatus_e;
#endif

#define INHERIT_MP_LIST \
	MpListType_e list_type;\
	Evas_Object *layout;\
	Evas_Object *box;\
	Evas_Object *genlist;\
	Evas_Object *no_content;\
	Evas_Object *fast_scroll;\
	Elm_Object_Item *bottom_counter_item; \
	char *(*bottom_counter_text_get_cb)(void *thiz); \
        mp_popup_t popup_type;\
        Ecore_Timer *pop_delay_timer;\
	int edit_mode;\
	MpListEditType_e edit_type;\
	int reorderable;\
	bool scroll_drag_status;\
	bool display_mode_changable;\
	MpListDisplayMode_e display_mode; \
	MpCloudView_e cloud_view_type; \
	MpListPersonalPage_t personal_page_type; \
	MpListPersonalPageLocation_t personal_page_storage; \
	void (*update)(void *thiz);\
        void (*realized_item_update)(void *thiz, const char *part, int field);\
	void (*set_edit)(void *thiz, bool edit);\
	void (*set_reorder)(void *thiz, bool reorder);\
	void (*rotate)(void *thiz);\
	int (*show_fastscroll)(void *thiz);\
	int (*hide_fastscroll)(void *thiz);\
	void (*edit_mode_sel)(void *list, void *data);\
	mp_group_type_e (*get_group_type)(void *thiz);\
	mp_track_type_e (*get_track_type)(void *thiz);\
	void *(*get_playlist_handle)(void *thiz);\
	void (*destory_cb)(void *thiz);\
	unsigned int (*get_count)(void *thiz, MpListEditType_e type);\
	unsigned int (*get_select_count)(void *thiz);\
	void (*flick_left_cb)(void *thiz, Evas_Object * obj, void *event_info);\
	void (*flick_right_cb)(void *thiz, Evas_Object * obj, void *event_info);\
	void (*flick_stop_cb)(void *thiz, Evas_Object * obj, void *event_info);\
	void (*mode_left_cb)(void *thiz, Evas_Object * obj, void *event_info);\
	void (*mode_right_cb)(void *thiz, Evas_Object * obj, void *event_info);\
	void (*mode_cancel_cb)(void *thiz, Evas_Object * obj, void *event_info);\
	void (*longpressed_cb)(void *data, Evas_Object *obj, void *event_info);\
        void (*drag_start_cb)(void *data, Evas_Object *obj, void *event_info);\
        void (*drag_stop_cb)(void *data, Evas_Object *obj, void *event_info);\
	void (*change_display_mode)(void *thiz, MpListDisplayMode_e mode); \
	void (*selected_item_data_get)(void *thiz, GList **selected);\
        void (*all_item_data_get)(void *thiz, GList **selected);\
	const char *(*get_label)(void *thiz, void *event_info);\
	mp_group_type_e group_type; \
	char *type_str; \
	char *type_str2; \
	char *filter_str; \
	int playlist_id; \
	mp_track_type_e track_type; \
	MpTrackListIndex_t index_type; \
	MpListFunction_e function_type; \
	GList *checked_path_list; \

typedef struct __MpList{
	INHERIT_MP_LIST
}MpList_t;

#define INHERIT_MP_LIST_ITEM_DATA \
	int index; \
	MpListItemType_e item_type; \
	Elm_Object_Item *it; \
	Eina_Bool checked; \
	mp_group_type_e group_type; \
	mp_media_info_h handle; \
	bool unregister_lang_mgr; \
	MpListDisplayMode_e display_mode; \
	int artist_album_page; \
	void *plst_item;

typedef struct
{
	// for mh_list_item_data;
	INHERIT_MP_LIST_ITEM_DATA;

} mp_list_item_data_t;

typedef struct
{
	int item_count;
	mp_list_item_data_t **item_data;
} mp_grid_item_data_t;

#define MP_LIST_OBJ_SET_AS_GENGRID(obj) (evas_object_data_set(obj, "is_gengrid", (void *)1))
#define MP_LIST_OBJ_IS_GENGRID(obj) ((int)evas_object_data_get(obj, "is_gengrid"))

void mp_list_init(MpList_t *list, Evas_Object *parent, MpListType_e list_type);
Evas_Object *mp_list_get_layout(MpList_t *list);
int mp_list_hide_fast_scroll(MpList_t *list);
int mp_list_show_fast_scroll(MpList_t *list);
void mp_list_update(MpList_t *list);
void mp_list_realized_item_part_update(MpList_t *list, const char *part, int field);
void mp_list_set_edit(MpList_t *list, bool edit);
bool mp_list_get_edit(MpList_t *list);
void mp_list_set_reorder(MpList_t *list, bool reorder);
bool mp_list_get_reorder(MpList_t *list);
void mp_list_set_edit_type(MpList_t *list, MpListEditType_e type);
MpListEditType_e mp_list_get_edit_type(MpList_t *list);
void mp_list_edit_mode_sel(MpList_t *list, void *data);
mp_group_type_e mp_list_get_group_type(MpList_t *list);
mp_track_type_e mp_list_get_track_type(MpList_t *list);
void *mp_list_get_playlist_handle(MpList_t *list);
unsigned int mp_list_get_editable_count(MpList_t *list, MpListEditType_e type);
unsigned int _mp_list_get_count(void *thiz, MpListEditType_e type);
unsigned int mp_list_get_checked_count(MpList_t *list);
bool mp_list_is_display_mode_changable(MpList_t *list);
MpListDisplayMode_e mp_list_get_display_mode(MpList_t *list);
void mp_list_change_display_mode(MpList_t *list, MpListDisplayMode_e mode);
void mp_list_selected_item_data_get(MpList_t *list, GList **selected);
void mp_list_all_item_data_get(MpList_t *list, GList **selected);
Elm_Object_Item *mp_list_first_item_get(Evas_Object *obj);
Elm_Object_Item *mp_list_item_next_get(Elm_Object_Item *item);
void mp_list_select_mode_set(Evas_Object *obj, Elm_Object_Select_Mode select_mode);
Elm_Object_Select_Mode mp_list_select_mode_get(Evas_Object *obj);
void mp_list_item_select_mode_set(Elm_Object_Item *item, Elm_Object_Select_Mode select_mode);
Elm_Object_Select_Mode mp_list_item_select_mode_get(Elm_Object_Item *item);
void mp_list_reorder_mode_set(Evas_Object *obj, Eina_Bool reorder_mode);
void mp_list_item_selected_set(Elm_Object_Item *item, Eina_Bool selected);
Eina_Bool mp_list_item_selected_get(Elm_Object_Item *item);
const char * mp_list_get_list_item_label(MpList_t *list, Elm_Object_Item *item);
void mp_list_double_tap(MpList_t *list);
void mp_list_rotate(MpList_t *list);

mp_list_item_data_t *mp_list_item_data_create(MpListItemType_e item_type);
void mp_list_item_check_set(Elm_Object_Item *item, Eina_Bool checked);
Elm_Object_Item *mp_list_bottom_counter_item_append(MpList_t *list);

GList *mp_list_get_checked_path_list(MpList_t *list);
bool mp_list_is_in_checked_path_list(GList *path_list, char *file_path);
void mp_list_free_checked_path_list(GList *path_list);

void mp_list_item_reorder_moved_cb(void *data, Evas_Object *obj, void *event_info);
int _mp_list_set_fastscroll(void *thiz);

#endif

