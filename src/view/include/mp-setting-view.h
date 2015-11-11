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

#ifndef __MP_SETTING_VIEW__
#define __MP_SETTING_VIEW__

#include "mp-view.h"
#include "ms-key-ctrl.h"

typedef enum {
	MP_SETTING_VIEW_DEFAULT,
	//MP_SETTING_VIEW_SA,
	//MP_SETTING_VIEW_CUSTOM_EQ,
	MP_SETTING_VIEW_TABS,
	MP_SETTING_VIEW_REORDERS,
	MP_SETTING_VIEW_PLAYLISTS,
} MpSettingViewType_e;


typedef enum {
	MP_SETTING_TABS_TRACKS ,
	MP_SETTING_TABS_PLAYLISTS,
	MP_SETTING_TABS_ALBUMS,
	MP_SETTING_TABS_ARTISTS,
	MP_SETTING_TABS_GENRES,
	MP_SETTING_TABS_FOLDERS,
	MP_SETTING_TABS_MUSIC_SQUARE,
	MP_SETTING_TABS_ITEM_MAX,
} MpSettingViewTabs_e;

typedef enum {
	MP_SETTING_REORDER_TABS ,
	MP_SETTING_REORDER_PLAYLISTS,
} MpSettingReorderType_e;

typedef enum {
	MS_MAIN_MENU_PLAYLIST,
	MS_MAIN_MENU_LYRICS,
	MS_MAIN_MENU_ITEM_MAX,
} ms_main_menu_item_t;

typedef enum {
	MS_ITC_TYPE_SEPERATER,
	MS_ITC_TYPE_1TEXT_NO_EXP,
	MS_ITC_TYPE_1TEXT,
	MS_ITC_TYPE_2TEXT,
	MS_ITC_TYPE_2TEXT_NORMAL,
	MS_ITC_TYPE_1TEXT_1ICON,
	MS_ITC_TYPE_NUM,
} ms_itc_type;

typedef enum {
	MS_EFFECT_THREE_D,
	MS_EFFECT_BASS,
	MS_EFFECT_CLARITY,
	MS_EFFECT_MAX,
} ms_effect_tab_type;

enum {
	MS_EF_NONE,
	MS_EF_TUBE,
	MS_EF_VIRTUAL,
	MS_EF_SMALL_ROOM,
	MS_EF_LARGE_ROOM,
	MS_EF_CONCER_HALL,

	MS_EF_MAX,
};

enum {
	MS_TAB_BASIC,
	MS_TAB_ADVANCED,

	MS_TAB_MAX,
};


typedef struct {
	INHERIT_MP_VIEW;
	MpSettingViewType_e setting_type;
	Evas_Object *content;
	Elm_Object_Item *gl_it[MS_MAIN_MENU_ITEM_MAX];
	Elm_Genlist_Item_Class *itc[MS_ITC_TYPE_NUM];
	bool reorder;
	Elm_Genlist_Item_Class *tabs_itc[2];
	Ecore_Timer *back_timer;

	Evas_Object *effect_layout;
	Evas_Object *tabbar;
	Evas_Object *basic_layout;
	Evas_Object *gengrid;
	Evas_Object *effect_gengrid;
	ms_effect_tab_type effcet_tab_status;
	Evas_Object *parent;
	MpSettingReorderType_e reorder_type;
	bool landscape_mode;

	Evas_Object *advanced_layout;
	Evas_Object *progress_layout;
	Evas_Object *effect_toolbar;
	Evas_Object *effect_check[MS_EFFECT_MAX];

	Ecore_Idler *set_effect_idler;
	int current_effect_val;
	int tabbar_status;

	Evas_Object *user_layout;
	Evas_Object *eq_layout;
	Evas_Object *extended_layout;
	Evas_Object *popup;

	int c_eq_val;
	int c_effect_val;
	int c_surround_state;

	Evas_Object *eq_custom_popup;

	ms_eq_custom_t eq_custom_values;
	Evas_Object *eq_valuebar;

	Evas_Object *auto_off_radio_grp;
	bool b_mouse_down;
	Evas_Object *auto_check;
	int init_square;
	int init_effect;
	ms_extended_effect_t init_extended_value;
	ms_eq_custom_t  init_custom_values;
} MpSettingView_t;

typedef struct {
	int x: 8;
	int y: 8;
} ms_effect_position_t;

typedef struct {
	Elm_Object_Item *it;
	int index;
	MpSettingView_t *view;
	ms_effect_position_t position;
	bool b_seleted;
} ms_effect_gengrid_item_data_t;

typedef struct {
	Elm_Object_Item *it;
	int index;
	bool b_seleted;
} ms_effect_ef_gengrid_item_data_t;

typedef struct {
	Elm_Object_Item *it;
	int index;
	char* str;
	int seq;
} mp_setting_genlist_item_data_t;

typedef struct {
	Evas_Object * popup;
	Evas_Object *group_radio;
} mp_setting_lyric_popup;

EXPORT_API MpSettingView_t *mp_setting_view_create(Evas_Object *parent, MpSettingViewType_e type, void *data);
void mp_music_viewas_pop_cb();
int mp_setting_view_destory(MpSettingView_t *view);
EXPORT_API void mp_setting_items_reorder_cb(void *data, Evas_Object *obj, void *event_info);
#endif

