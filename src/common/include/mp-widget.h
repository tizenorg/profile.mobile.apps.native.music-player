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

#ifndef __MP_WIDGET_H_
#define __MP_WIDGET_H_

#include "music.h"
#include "mp-scroll-page.h"
#include "mp-floating-widget-mgr.h"

#define _EDJ(obj) elm_layout_edje_get(obj) /**< get evas object from elm layout */

typedef enum {
	MP_LOADING_ICON_SIZE_LARGE,		// normal
	MP_LOADING_ICON_SIZE_MEDIUM,
	MP_LOADING_ICON_SIZE_SMALL,
	MP_LOADING_ICON_SIZE_XLARGE,
} mp_loading_icon_size_e;

Evas_Object *mp_widget_navigation_new(Evas_Object * parent);
EXPORT_API Evas_Object *mp_widget_genlist_create(Evas_Object * parent);
Evas_Object *mp_widget_text_popup(void *data, const char *message);
Evas_Object *mp_widget_text_popup_show(void *data, const char *message, bool show);
Evas_Object *mp_widget_text_cb_popup(void *data, const char *message, Evas_Smart_Cb cb);

Evas_Object *mp_widget_create_icon(Evas_Object * obj, const char *path, int w, int h);
Evas_Object *mp_widget_create_button(Evas_Object * parent, char *style, char *caption, Evas_Object * icon,
                                     void (*func)(void *, Evas_Object *, void *), void *data);
char *mp_widget_set_text_theme_color(const char *text, const char *color);
Evas_Object * mp_widget_create_editfield(Evas_Object * parent, int limit_size, char *guide_txt, struct appdata *ad);
Evas_Object * mp_widget_editfield_entry_get(Evas_Object *editfield);
Evas_Object *mp_create_title_text_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data);
Evas_Object * mp_widget_create_title_btn(Evas_Object *parent, const char *text, const char * icon_path, Evas_Smart_Cb func, void *data);
Evas_Object * mp_common_create_naviframe_title_button(Evas_Object *parent, const char * text_id, void *save_cb, void *user_data);
Evas_Object * mp_widget_create_layout_main(Evas_Object * parent);
EXPORT_API Evas_Object *mp_common_load_edj(Evas_Object * parent, const char *file, const char *group);
Evas_Object *mp_create_win(const char *name);
Evas_Object *mp_widget_text_button_popup(void *data, const char *message);
EXPORT_API Evas_Object * mp_widget_notify_cb_popup(void *data, const char *message, Evas_Smart_Cb cb, void *cb_data);

typedef enum {
	MP_NOCONTENT_NORMAL,
	MP_NOCONTENT_TRACKS,
	MP_NOCONTENT_PLAYLIST,
	MP_NOCONTENT_ARTISTS,
	MP_NOCONTENT_ALBUMS,
	MP_NOCONTENT_FOLDERS,
	MP_NOCONTENT_GENRES,
	MP_NOCONTENT_DEVICE,
} MpNocontent_e;

Evas_Object *
mp_widget_create_no_contents(Evas_Object *parent, MpNocontent_e type, void(*callback)(void *data, Evas_Object *obj, void *event_info), void *user_data);
Evas_Object *
mp_widget_create_no_content_playlist(Evas_Object *parent, char *helptext, void(*callback)(void *data, Evas_Object *obj, void *event_info), void *user_data);
Evas_Object *mp_widget_no_contents_default_add(Evas_Object *parent, const char *text, const char *help_text);
Evas_Object *mp_widget_shorcut_box_add(Evas_Object *parent, const char *title, const char *file, const char *group, int w, int h, Edje_Signal_Cb func, void *data);
Evas_Object *mp_widget_create_tabbar(Evas_Object *obj);
Evas_Object *mp_widget_slide_title_create(Evas_Object *parent, const char *style, const char *text);
Evas_Object *mp_widget_device_icon_add(Evas_Object *parent, int device_type);
Evas_Object *mp_widget_connection_info_add(Evas_Object *parent, Evas_Object *src_icon, const char *src_name, Evas_Object *dest_icon, const char *dest_name);
Evas_Object *mp_widget_create_title_icon(Evas_Object *parent, const char *file, const char *group);
Evas_Object *mp_widget_create_bg(Evas_Object *parent);

#define MP_TOOLBAR_BTN_DEFAULT	"naviframe/toolbar/default"
#define MP_TOOLBAR_BTN_LEFT	"naviframe/toolbar/left"
#define MP_TOOLBAR_BTN_RIGHT	"naviframe/toolbar/right"
#define MP_TOOLBAR_BTN_MORE	"naviframe/more/default"
Evas_Object *
mp_widget_create_toolbar_btn(Evas_Object *parent,
                             const char *style, const char *text, Evas_Smart_Cb func, void *data);
Evas_Object *mp_widget_create_naviframe_toolbar(Elm_Object_Item *navi_it);
Elm_Object_Item *mp_widget_create_toolbar_item_btn(Evas_Object *parent, const char *style, const char *text, Evas_Smart_Cb func, void *data);
Evas_Object *mp_widget_create_title_icon_btn(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data);
Evas_Object *mp_widget_create_title_icon_btn_second(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data);
Evas_Object *mp_widget_create_title_icon_btn_black(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data);
Evas_Object *mp_widget_loading_icon_add(Evas_Object *parent, mp_loading_icon_size_e size);
Evas_Object* mp_widget_create_navi_right_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem, Evas_Smart_Cb pFunc, void *pUserData);
Evas_Object* mp_widget_create_navi_left_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem, Evas_Smart_Cb pFunc, void *pUserData);
bool mp_widget_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc, Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout);


#ifdef MP_FEATURE_PERSONAL_PAGE
Evas_Object *mp_widget_lock_icon_create(Evas_Object *obj, const char *thumbpath);
#endif
Evas_Object *mp_widget_rich_info_text_add(Evas_Object *parent, const char *text);

#endif
