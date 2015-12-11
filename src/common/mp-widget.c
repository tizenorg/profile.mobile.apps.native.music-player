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


#include "mp-widget.h"
#include "mp-player-debug.h"
#include "mp-util.h"
#include "mp-popup.h"
#ifndef MP_SOUND_PLAYER
#include "mp-common.h"
#endif
#include "mp-file-util.h"
#include "mp-define.h"

#define MAX_LEN_VIB_DURATION 0.5
#define DEF_BUF_LEN            (512)

Evas_Object *
mp_widget_navigation_new(Evas_Object * parent)
{
	Evas_Object *navi_bar;
	mp_retv_if(parent == NULL, NULL);
	navi_bar = elm_naviframe_add(parent);
	mp_retvm_if(navi_bar == NULL, NULL, "Fail to create navigation bar");
	elm_naviframe_event_enabled_set(navi_bar, EINA_FALSE);

	evas_object_show(navi_bar);
	return navi_bar;
}

#ifdef MP_WATCH_DOG
#include "mp-watch-dog.h"
static void _unrealized_cb(void *data, Evas_Object *obj, void *event_info)
{
	mp_watch_dog_reset();
}
#endif

EXPORT_API Evas_Object *
mp_widget_genlist_create(Evas_Object * parent)
{
	Evas_Object *list = NULL;

	list = elm_genlist_add(parent);
	MP_CHECK_NULL(list);
#ifdef MP_WATCH_DOG
	evas_object_smart_callback_add(list, "unrealized", _unrealized_cb, list);
#endif
	elm_scroller_bounce_set(list, EINA_FALSE, EINA_TRUE);
	return list;
}

static void
_mp_widget_text_popup_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	mp_evas_object_del(obj);
}

EXPORT_API Evas_Object *
mp_widget_text_popup(void *data, const char *message)
{
	struct appdata *ad = mp_util_get_appdata();
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main, MP_POPUP_NOTIFY, NULL, ad, _mp_widget_text_popup_timeout_cb, ad);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	char *text = g_strconcat("<align=center>", message, "</align>", NULL);
	elm_object_text_set(popup, (const char *)text);
	mp_popup_timeout_set(popup, MP_POPUP_TIMEOUT);
	evas_object_show(popup);
	IF_FREE(text);
	return popup;
}

EXPORT_API Evas_Object *
mp_widget_text_popup_show(void *data, const char *message, bool show)
{
	struct appdata *ad = mp_util_get_appdata();
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main, MP_POPUP_NOTIFY, NULL, ad, _mp_widget_text_popup_timeout_cb, ad);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	char *text = g_strconcat("<align=center>", message, "</align>", NULL);
	elm_object_text_set(popup, (const char *)text);
	mp_popup_timeout_set(popup, MP_POPUP_TIMEOUT);
	if (show) {
		evas_object_show(popup);
	}
	IF_FREE(text);
	return popup;
}

EXPORT_API Evas_Object *
mp_widget_notify_cb_popup(void *data, const char *message, Evas_Smart_Cb cb, void *cb_data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main, MP_POPUP_NOTIFY, NULL, cb_data, cb, ad);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	char *text = g_strconcat("<align=center>", message, "</align>", NULL);
	elm_object_text_set(popup, (const char *)text);
	mp_popup_timeout_set(popup, MP_POPUP_TIMEOUT);
	evas_object_show(popup);
	IF_FREE(text);
	return popup;
}


EXPORT_API Evas_Object *
mp_widget_text_cb_popup(void *data, const char *message, Evas_Smart_Cb cb)
{
	struct appdata *ad = mp_util_get_appdata();
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main, MP_POPUP_NOTIFY, NULL, ad, cb, ad);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	char *text = g_strconcat("<align=center>", message, "</align>", NULL);
	elm_object_text_set(popup, (const char *)text);
	mp_popup_timeout_set(popup, MP_POPUP_TIMEOUT);
	evas_object_show(popup);
	IF_FREE(text);
	return popup;
}

EXPORT_API Evas_Object* mp_widget_text_button_popup(void *data, const char *message)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	ad->popup[MP_POPUP_NORMAL] = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, NULL, NULL, ad);
	MP_CHECK_NULL(ad->popup[MP_POPUP_NORMAL]);

	mp_popup_desc_set(ad->popup[MP_POPUP_NORMAL], message);
	mp_popup_button_set(ad->popup[MP_POPUP_NORMAL], MP_POPUP_BTN_1, STR_MP_OK, MP_POPUP_YES);
	evas_object_show(ad->popup[MP_POPUP_NORMAL]);
	return ad->popup[MP_POPUP_NORMAL];
}

Evas_Object *
mp_widget_create_tabbar(Evas_Object *obj)
{
	Evas_Object *tabbar;

	/* create controlbar */
	PROFILE_IN("elm_toolbar_add");
	tabbar = elm_toolbar_add(obj);
	PROFILE_OUT("elm_toolbar_add");
	elm_toolbar_shrink_mode_set(tabbar, ELM_TOOLBAR_SHRINK_SCROLL);
	elm_toolbar_reorder_mode_set(tabbar, EINA_FALSE);
	elm_toolbar_transverse_expanded_set(tabbar, EINA_TRUE);
	elm_toolbar_select_mode_set(tabbar, ELM_OBJECT_SELECT_MODE_ALWAYS);

	PROFILE_IN("elm_object_style_set: toolbar");
	elm_object_style_set(tabbar, "scroll/tabbar_with_title");
	PROFILE_OUT("elm_object_style_set: toolbar");

	return tabbar;
}

char *
mp_widget_set_text_theme_color(const char *text, const char *color)
{
	ERROR_TRACE("");
	MP_CHECK_NULL(text);
	MP_CHECK_NULL(color);

	int r = 0;
	int g = 0;
	int b = 0;
	int a = 0;
	static char return_str[DEF_BUF_LEN + 1] = { 0, };

	memset(return_str, 0x00, DEF_BUF_LEN + 1);

	snprintf(return_str, DEF_BUF_LEN,
	         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, text);

	ERROR_TRACE("return_str %s", return_str);

	return return_str;
}


Evas_Object *
mp_widget_create_button(Evas_Object * parent, char *style, char *caption, Evas_Object * icon,
                        void (*func)(void *, Evas_Object *, void *), void *data)
{
	if (!parent) {
		return NULL;
	}

	Evas_Object *btn;

	btn = elm_button_add(parent);

	if (style) {
		elm_object_style_set(btn, style);
	}

	if (caption) {
		mp_util_domain_translatable_text_set(btn, caption);
	}

	if (icon) {
		elm_object_content_set(btn, icon);
	}

	elm_object_focus_allow_set(btn, EINA_TRUE);
	evas_object_propagate_events_set(btn, EINA_FALSE);

	evas_object_smart_callback_add(btn, "clicked", func, (void *)data);

	return btn;
}

static void
_mp_widget_entry_maxlength_reached_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = data;
	MP_CHECK(ad);
	mp_popup_max_length(obj, STR_NH_COM_POPUP_CHARACTERS_MAXNUM_REACHED);
}

static void
_mp_widget_entry_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	struct appdata *ad = data;
	MP_CHECK(ad);

	Evas_Object *editfield = data;
	MP_CHECK(editfield);

	Evas_Object *entry = obj;
	MP_CHECK(entry);

	Eina_Bool entry_empty = elm_entry_is_empty(entry);
	const char *eraser_signal = NULL;
	const char *guidetext_signal = NULL;
	if (entry_empty) {
		eraser_signal =	"elm,state,eraser,hide";
		guidetext_signal = "elm,state,guidetext,show";
	} else {
		eraser_signal =	"elm,state,eraser,show";
		guidetext_signal = "elm,state,guidetext,hide";
	}
	elm_object_signal_emit(editfield, eraser_signal, "elm");
	elm_object_signal_emit(editfield, guidetext_signal, "elm");
}

static void
_mp_widget_entry_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source) // When X marked button is clicked, empty entry's contents.
{
	Evas_Object *entry = data;
	MP_CHECK(entry);

	elm_entry_entry_set(entry, "");
}


Evas_Object *
mp_widget_create_editfield(Evas_Object * parent, int limit_size, char *guide_txt, struct appdata *ad)
{
	startfunc;
	Evas_Object *editfield = NULL;
	Evas_Object *entry = NULL;
	editfield = elm_layout_add(parent);
	elm_layout_theme_set(editfield, "layout", "editfield", "default");
	MP_CHECK_NULL(editfield);
	evas_object_size_hint_weight_set(editfield, EVAS_HINT_EXPAND,
	                                 EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(editfield, EVAS_HINT_FILL,
	                                EVAS_HINT_FILL);

	entry = elm_entry_add(editfield);
	MP_CHECK_NULL(entry);
	elm_object_style_set(entry, "default");
	elm_entry_single_line_set(entry, EINA_TRUE);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_autocapital_type_set(entry, ELM_AUTOCAPITAL_TYPE_NONE);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);

	evas_object_data_set(editfield, "entry", entry);
	elm_object_part_content_set(editfield, "elm.swallow.content", entry);

	evas_object_smart_callback_add(entry, "changed", _mp_widget_entry_changed_cb, editfield);
	elm_object_signal_callback_add(editfield, "elm,eraser,clicked", "elm", _mp_widget_entry_eraser_clicked_cb, entry);

	if (limit_size > 0) {
		static Elm_Entry_Filter_Limit_Size limit_filter_data;

		limit_filter_data.max_char_count = 0;
		limit_filter_data.max_byte_count = limit_size;
		elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
		evas_object_smart_callback_add(entry, "maxlength,reached", _mp_widget_entry_maxlength_reached_cb,
		                               ad);
	}

	if (guide_txt) {
		elm_object_part_text_set(editfield, "elm.guidetext", guide_txt);
	}

	return editfield;

}

Evas_Object *
mp_widget_editfield_entry_get(Evas_Object *editfield)
{
	MP_CHECK_NULL(editfield);

	Evas_Object *entry = evas_object_data_get(editfield, "entry");

	return entry;
}

Evas_Object *
mp_widget_create_title_btn(Evas_Object *parent, const char *text, const char * icon_path, Evas_Smart_Cb func, void *data)
{
	Evas_Object *btn = elm_button_add(parent);
	Evas_Object * icon = NULL;
	MP_CHECK_NULL(btn);

	if (text) {
		elm_object_text_set(btn, text);
	} else if (icon_path) {
		icon = elm_icon_add(btn);
		MP_CHECK_NULL(icon);
		elm_image_file_set(icon, IMAGE_EDJ_NAME, icon_path);
		elm_object_content_set(btn, icon);
		elm_object_style_set(btn, "title_button");
	}
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}

Evas_Object *
mp_widget_create_layout_main(Evas_Object * parent)
{
	Evas_Object *layout;

	mp_retv_if(parent == NULL, NULL);
	layout = elm_layout_add(parent);
	mp_retvm_if(layout == NULL, NULL, "Failed elm_layout_add.");

	elm_layout_theme_set(layout, "layout", "application", "default");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(parent, layout);

	evas_object_show(layout);

	return layout;
}

EXPORT_API Evas_Object *
mp_common_load_edj(Evas_Object * parent, const char *file, const char *group)
{
	Evas_Object *eo = NULL;
	int r = -1;

	eo = elm_layout_add(parent);
	if (eo) {
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", file);

		MP_CHECK_NULL(edje_path);
		r = elm_layout_file_set(eo, edje_path, group);
		free(path);
		if (!r) {
			evas_object_del(eo);
			return NULL;
		}
		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(eo);
	}
	evas_object_name_set(eo, group);

	return eo;
}

static void
_mp_common_win_del(void *data, Evas_Object * obj, void *event)
{
	elm_exit();
}

Evas_Object *
mp_create_win(const char *name)
{
	startfunc;
	Evas_Object *eo;
	int w, h;

	eo = elm_win_add(NULL, name, ELM_WIN_BASIC);
	if (eo) {
		elm_win_title_set(eo, name);
		evas_object_smart_callback_add(eo, "delete,request", _mp_common_win_del, NULL);
		elm_win_screen_size_get(eo, NULL, NULL, &w, &h);
		evas_object_resize(eo, w, h);
		//set indicator as transparent
		elm_win_indicator_mode_set(eo, ELM_WIN_INDICATOR_SHOW);
		elm_win_indicator_opacity_set(eo, ELM_WIN_INDICATOR_TRANSPARENT);
		elm_win_conformant_set(eo, EINA_TRUE);
	}
	return eo;
}

Evas_Object *
mp_common_create_naviframe_title_button(Evas_Object *parent, const char * text_id, void *save_cb, void *user_data)
{
	Evas_Object *btn_save = NULL;
	btn_save = elm_button_add(parent);
	elm_object_style_set(btn_save, "naviframe/title1/default");
	evas_object_size_hint_align_set(btn_save, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(btn_save, "clicked", save_cb, user_data);

	elm_object_text_set(btn_save, GET_SYS_STR(text_id));
	mp_language_mgr_register_object(btn_save, OBJ_TYPE_ELM_OBJECT, NULL, text_id);
	evas_object_show(btn_save);

	return btn_save;

}

Evas_Object *
mp_widget_create_toolbar_btn(Evas_Object *parent,
                             const char *style, const char *text, Evas_Smart_Cb func, void *data)
{
	startfunc;
	Evas_Object *btn = elm_button_add(parent);
	MP_CHECK_NULL(btn);

	elm_object_style_set(btn, style);
	elm_object_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", func, data);

	return btn;
}

Evas_Object *mp_widget_create_naviframe_toolbar(Elm_Object_Item *navi_it)
{
	startfunc;
	Evas_Object *toolbar = elm_object_item_part_content_get(navi_it, "toolbar");
	if (!toolbar) {
		toolbar = elm_toolbar_add(GET_NAVIFRAME);
		MP_CHECK_NULL(toolbar);

		elm_object_style_set(toolbar, "default");
		elm_toolbar_shrink_mode_set(toolbar, ELM_TOOLBAR_SHRINK_EXPAND);
		elm_toolbar_transverse_expanded_set(toolbar, EINA_TRUE);
		elm_object_item_part_content_set(navi_it, "toolbar", toolbar);
	}

	Elm_Object_Item *item = elm_toolbar_first_item_get(toolbar);
	while (item) {
		elm_object_item_del(item);
		item = elm_toolbar_first_item_get(toolbar);
	};

	return toolbar;
}

Elm_Object_Item *mp_widget_create_toolbar_item_btn(Evas_Object *parent, const char *style, const char *text, Evas_Smart_Cb func, void *data)
{
	startfunc;
	Evas_Object *toolbar = parent;
	MP_CHECK_NULL(toolbar);

	Elm_Object_Item *toolbar_item = NULL;
	//Evas_Object *toolbar_obj_item = NULL;

	toolbar_item = elm_toolbar_item_append(toolbar, NULL, NULL, func, data);
	//toolbar_obj_item = elm_toolbar_item_object_get(toolbar_item);
	mp_util_item_domain_translatable_part_text_set(toolbar_item, "elm.text", text);
	return toolbar_item;
}

Evas_Object *mp_widget_create_title_icon_btn_black(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data)
{
	startfunc;
	Evas_Object *ic;
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "music/naviframe/title_icon_black");

	ic = elm_icon_add(parent);
	elm_image_file_set(ic, file, group);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);

	evas_object_smart_callback_add(btn, "clicked", func, data);
	evas_object_show(btn);
	return btn;
}

Evas_Object *mp_widget_create_title_icon_btn_second(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data)
{
	startfunc;
	Evas_Object *ic;
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "music/naviframe/title_icon_second");

	ic = elm_icon_add(parent);
	elm_image_file_set(ic, file, group);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);

	evas_object_smart_callback_add(btn, "clicked", func, data);
	evas_object_show(btn);
	return btn;
}

Evas_Object *mp_create_title_text_btn(Evas_Object *parent, const char *text, Evas_Smart_Cb func, void *data)
{
	startfunc;
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "naviframe/title_text");
	mp_util_domain_translatable_text_set(btn, text);
	evas_object_smart_callback_add(btn, "clicked", func, data);
	return btn;
}
Evas_Object *mp_widget_create_title_icon_btn(Evas_Object *parent, const char *file, const char *group, Evas_Smart_Cb func, void *data)
{
	startfunc;
	Evas_Object *ic;
	Evas_Object *btn = elm_button_add(parent);
	if (!btn) {
		return NULL;
	}
	elm_object_style_set(btn, "naviframe/title_icon");

	ic = elm_image_add(parent);
	elm_image_file_set(ic, file, group);
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_BOTH, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(btn, "icon", ic);

	evas_object_smart_callback_add(btn, "clicked", func, data);
	evas_object_show(btn);
	return btn;
}

Evas_Object* mp_widget_create_navi_left_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
        Evas_Smart_Cb pFunc, void *pUserData)
{
	startfunc;

	if (!pParent || !pNaviItem) {
		ERROR_TRACE("parent is NULL.");
		return NULL;
	}
	Evas_Object *pLeftbtn = NULL;
	pLeftbtn = elm_button_add(pParent);
	elm_object_style_set(pLeftbtn, "naviframe/title_left");
	evas_object_smart_callback_add(pLeftbtn, "clicked", pFunc, pUserData);
	mp_util_domain_translatable_text_set(pLeftbtn, STR_MP_NAVI_CANCEL);

	if (!pLeftbtn) {
		ERROR_TRACE("[ERR] Fail to create pLeftbtn");
		return NULL;
	}

	elm_object_item_part_content_set(pNaviItem, "title_left_btn", pLeftbtn);

	evas_object_show(pLeftbtn);

	return pLeftbtn;
}
Evas_Object* mp_widget_create_navi_right_btn(Evas_Object *pParent, Elm_Object_Item *pNaviItem,
        Evas_Smart_Cb pFunc, void *pUserData)
{
	startfunc;

	if (!pParent || !pNaviItem) {
		ERROR_TRACE("parent is NULL.");
		return NULL;
	}

	Evas_Object *pRightbtn = NULL;

	pRightbtn = elm_button_add(pParent);
	elm_object_style_set(pRightbtn, "naviframe/title_right");
	evas_object_smart_callback_add(pRightbtn, "clicked", pFunc, pUserData);
	mp_util_domain_translatable_text_set(pRightbtn, STR_MP_NAVI_DONE);

	if (!pRightbtn) {
		ERROR_TRACE("[ERR] Fail to create pRightbtn");
		return NULL;
	}

	elm_object_item_part_content_set(pNaviItem, "title_right_btn", pRightbtn);

	evas_object_show(pRightbtn);

	return pRightbtn;

}


Evas_Object *mp_widget_create_title_icon(Evas_Object *parent, const char *file, const char *group)
{
	startfunc;
	Evas_Object *pIcon = elm_icon_add(parent);
	elm_image_file_set(pIcon, file, group);
	evas_object_size_hint_aspect_set(pIcon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(pIcon, EINA_TRUE, EINA_TRUE);

	return pIcon;
}
Evas_Object *mp_widget_create_bg(Evas_Object *parent)
{
	startfunc;
	Evas_Object *bg = NULL;
	bg = evas_object_rectangle_add(evas_object_evas_get(parent));
	evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_repeat_events_set(bg, EINA_FALSE);
	evas_object_color_set(bg, 248, 246, 239, 255);
	evas_object_show(bg);
	return bg;
}
Evas_Object *
_mp_widget_no_content_w_help_add(Evas_Object *parent, char *text, char *helptext, char *btntext, void *cb, void *data)
{
	Evas_Object *lay = NULL, *btn = NULL;

	/* Full view layout */
	lay = elm_layout_add(parent);
	elm_layout_theme_set(lay, "layout", "nocontents", "default");
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(lay, EVAS_HINT_FILL, EVAS_HINT_FILL);

	mp_util_domain_translatable_part_text_set(lay, "elm.text", text);
	elm_layout_signal_emit(lay, "text,disabled", "");
	elm_layout_signal_emit(lay, "align.center", "elm");

	if (helptext) {
		mp_util_domain_translatable_part_text_set(lay, "elm.help.text", helptext);
	}

	if (cb) {
		btn = elm_button_add(lay);
		elm_object_style_set(btn, "style1");
		mp_util_domain_translatable_text_set(btn, btntext);
		evas_object_smart_callback_add(btn, "clicked", cb, data);

		elm_object_part_content_set(lay, "swallow_area", btn);
	}
	evas_object_show(lay);

	return lay;

}

Evas_Object *mp_widget_create_no_contents(Evas_Object *parent, MpNocontent_e type, void(*callback)(void *data, Evas_Object *obj, void *event_info), void *user_data)
{
	MP_CHECK_NULL(parent);
	startfunc;

	Evas_Object *content = NULL;

	if (type == MP_NOCONTENT_NORMAL || type == MP_NOCONTENT_PLAYLIST || type == MP_NOCONTENT_DEVICE) {
		content = elm_layout_add(parent);
		elm_layout_theme_set(content, "layout", "nocontents", "default");
		evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
		const char *text = NULL;
		if (type == MP_NOCONTENT_PLAYLIST) {
			text = STR_MP_NO_PLAYLISTS;
		} else if (type == MP_NOCONTENT_DEVICE) {
			text = STR_MP_NO_DEVICES;
		} else {
			text = STR_MP_NO_TRACKS;
		}
		mp_util_domain_translatable_text_set(content, text);
		evas_object_show(content);
	} else if (type == MP_NOCONTENT_ARTISTS || type == MP_NOCONTENT_ALBUMS) {
		char *text = (type == MP_NOCONTENT_ARTISTS) ? STR_MP_NO_ARTISTS : STR_MP_NO_ALBUMS;
		content = _mp_widget_no_content_w_help_add(parent, text,
		          STR_MP_AFTER_ADD_TRACKS_HELP, NULL, NULL, NULL);
	}

	else {
		content = _mp_widget_no_content_w_help_add(parent, STR_MP_NO_TRACKS,
		          STR_MP_AFTER_YOU_ADD_TRACKS, NULL, NULL, NULL);
	}

	return content;
}

Evas_Object *mp_widget_create_no_content_playlist(Evas_Object *parent,
        char *helptext, void(*callback)(void *data, Evas_Object *obj, void *event_info), void *user_data)
{
	MP_CHECK_NULL(parent);
	startfunc;

	Evas_Object *lay = NULL, *btn = NULL;
	lay = elm_layout_add(parent);
	elm_layout_theme_set(lay, "layout", "nocontents", "default");
	evas_object_size_hint_weight_set(lay, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(lay, EVAS_HINT_FILL, EVAS_HINT_FILL);

	//elm_object_part_text_set(lay, "elm.text", GET_STR(STR_MP_NO_TRACKS));
	mp_util_domain_translatable_part_text_set(lay, "elm.text", STR_MP_NO_TRACKS);
	elm_layout_signal_emit(lay, "text,disabled", "");
	elm_layout_signal_emit(lay, "align.center", "elm");

	if (helptext) {
		mp_util_domain_translatable_part_text_set(lay, "elm.help.text", (const char*)helptext);
		//avoid text skip
		edje_object_message_signal_process(elm_layout_edje_get(lay));
	}

	if (callback) {
		btn = elm_button_add(lay);
		elm_object_style_set(btn, "style1");
		mp_util_domain_translatable_text_set(btn, STR_MP_ADD_TRACKS);
		evas_object_smart_callback_add(btn, "clicked", callback, user_data);
		//elm_object_part_content_set(lay, "swallow_area", btn);
	}
	evas_object_show(lay);
	return lay;
}

Evas_Object *mp_widget_no_contents_default_add(Evas_Object *parent, const char *text, const char *help_text)
{
	MP_CHECK_NULL(parent);

	Evas_Object *content = _mp_widget_no_content_w_help_add(parent, (char *)text, (char *)help_text, NULL, NULL, NULL);
	return content;
}

Evas_Object *mp_widget_shorcut_box_add(Evas_Object *parent, const char *title, const char *file, const char *group, int w, int h, Edje_Signal_Cb func, void *data)
{
	Evas_Object *layout = NULL;
	layout = mp_common_load_edj(parent, MP_EDJ_NAME, "shortcut_box");
	MP_CHECK_NULL(layout);
	Evas_Object *image = NULL;

	if (group) {
		image = mp_util_create_thumb_icon(layout, NULL, w, h);
		elm_bg_file_set(image, file, group);
	} else {
		image = mp_util_create_thumb_icon(layout, file, w, h);
	}

	elm_object_part_content_set(layout, "bg", image);
	if (title) {
		elm_object_part_text_set(layout, "label", GET_STR(title));
		mp_language_mgr_register_object(layout, OBJ_TYPE_EDJE_OBJECT, "label", title);
	}

	if (func) {
		elm_object_signal_callback_add(layout, "clicked", "*", func, data);
	}

	return layout;
}

Evas_Object *mp_widget_slide_title_create(Evas_Object *parent, const char *style, const char *text)
{
	Evas_Object *label = elm_label_add(parent);

	// set the label style
	elm_object_style_set(label, style);

	// set the label for the title slide mode
	elm_label_slide_mode_set(label, ELM_LABEL_SLIDE_MODE_AUTO);
	elm_label_wrap_width_set(label, 1);
	// set the label text
	elm_object_text_set(label, text);
	evas_object_show(label);
	/*
		double duration = strlen(text) / (double)10;
		if (duration < 1.0)
			duration = 1.0;
		elm_label_slide_duration_set(label, duration);
	*/

	//elm_label_ellipsis_set(label, EINA_TRUE);

	return label;
}

Evas_Object *mp_widget_device_icon_add(Evas_Object *parent, int device_type)
{
	MP_CHECK_NULL(parent);

	const char *group = NULL;
	if (device_type == MP_DEVICE_TYPE_MY_DEVICE) {
		group = MP_ICON_CONNECTED_MY_DEVICE;
	} else {
		group = MP_ICON_CONNECTED_TV;
	}

	Evas_Object *icon = mp_common_load_edj(parent, IMAGE_EDJ_NAME, group);
	evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);

	return icon;
}

static Eina_Bool
_mp_widget_connection_info_timer_cb(void *data)
{
	TIMER_TRACE();
	Evas_Object *layout = data;
	MP_CHECK_VAL(layout, ECORE_CALLBACK_CANCEL);

	elm_object_signal_emit(layout, "sig_update_progress_image", "");

	return ECORE_CALLBACK_RENEW;
}

static void
_mp_widget_connection_info_del_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Ecore_Timer *timer = data;
	MP_CHECK(timer);

	ecore_timer_del(timer);
}

Evas_Object *mp_widget_connection_info_add(Evas_Object *parent, Evas_Object *src_icon, const char *src_name, Evas_Object *dest_icon, const char *dest_name)
{
	MP_CHECK_NULL(parent);

	Evas_Object *layout = mp_common_load_edj(parent, MP_EDJ_NAME, "connection_info_layout");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	/* from */
	elm_object_part_content_set(layout, "elm.icon.source", src_icon);
	elm_object_part_text_set(layout, "elm.text.source", src_name);

	/* to */
	elm_object_part_content_set(layout, "elm.icon.destination", dest_icon);
	elm_object_part_text_set(layout, "elm.text.destination", dest_name);

	Ecore_Timer *progress_timer = ecore_timer_add(0.5, _mp_widget_connection_info_timer_cb, layout);
	evas_object_data_set(layout, "timer", progress_timer);

	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _mp_widget_connection_info_del_cb, progress_timer);

	return layout;
}

Evas_Object *mp_widget_loading_icon_add(Evas_Object *parent, mp_loading_icon_size_e size)
{
	Evas_Object *progressbar = NULL;
	progressbar = elm_progressbar_add(parent);

	const char *style = NULL;
	if (size == MP_LOADING_ICON_SIZE_MEDIUM) {
		style = "process_medium";
	} else if (size == MP_LOADING_ICON_SIZE_SMALL) {
		style = "process_small";
	} else if (size == MP_LOADING_ICON_SIZE_XLARGE) {
		style = "process_large";
	} else {
		style = "process_large";
	}
	elm_object_style_set(progressbar, style);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);
	elm_progressbar_pulse(progressbar, EINA_TRUE);

	return progressbar;
}
#ifdef MP_FEATURE_PERSONAL_PAGE
Evas_Object *mp_widget_lock_icon_create(Evas_Object *obj, const char *thumbpath)
{
	MP_CHECK_NULL(obj);
	/*create layout*/
	Evas_Object *layout = NULL;
	layout = mp_common_load_edj(obj, MP_EDJ_NAME, "thumbnail_only");
	if (layout == NULL) {
		mp_error("layout create failed");
	}
	/*create thumbnail*/
	Evas_Object *thumb = elm_image_add(layout);
	elm_object_focus_set(thumb, EINA_FALSE);
	if (thumbpath != NULL) {
		elm_image_file_set(thumb, thumbpath, NULL);
	} else {
		elm_image_file_set(thumb, DEFAULT_THUMBNAIL, NULL);
	}

	elm_object_part_content_set(layout, "default_thumbnail", thumb);
	/*check if it is in personal page*/
	Evas_Object *personal_page = NULL;
	personal_page = elm_image_add(layout);
	elm_object_focus_set(personal_page, EINA_FALSE);
	elm_image_file_set(personal_page, IMAGE_EDJ_NAME, MP_ICON_LOCK);
	evas_object_show(personal_page);
	elm_object_part_content_set(layout, "lock.sub", personal_page);

	return layout;
}
#endif

Evas_Object *mp_widget_rich_info_text_add(Evas_Object *parent, const char *text)
{
	MP_CHECK_NULL(parent);

	Evas_Object *obj = mp_common_load_edj(parent, MP_EDJ_NAME, "rich_info_label_layout");
	MP_CHECK_NULL(obj);

	Evas_Object *label = elm_label_add(obj);
	elm_label_line_wrap_set(label, ELM_WRAP_WORD);

	char *label_text = g_strdup_printf("<font_size=32><color=#444444FF><align=left>%s</align></color></font_size>", text);
	elm_object_text_set(label, label_text);
	IF_FREE(label_text);

	elm_object_part_content_set(obj, "elm.label", label);
	return obj;
}

bool mp_widget_create_select_all_layout(Evas_Object *pParent, Evas_Smart_Cb pChangeFunc,
                                        Evas_Object_Event_Cb pMouseDownFunc, void *pUserData, Evas_Object **pCheckBox, Evas_Object **pSelectLayout)
{
	if (!pParent) {
		ERROR_TRACE("parent is NULL.");
		return FALSE;
	}

	Evas_Object *pSelectAllLayout = elm_layout_add(pParent);
	elm_layout_theme_set(pSelectAllLayout, "genlist", "item", "select_all/default");
	evas_object_size_hint_weight_set(pSelectAllLayout, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(pSelectAllLayout, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_event_callback_add(pSelectAllLayout, EVAS_CALLBACK_MOUSE_DOWN, pMouseDownFunc, pUserData);
	*pSelectLayout = pSelectAllLayout;

	Evas_Object *pSelectAllCheckbox = elm_check_add(pSelectAllLayout);
	evas_object_smart_callback_add(pSelectAllCheckbox, "changed", pChangeFunc, pUserData);
	evas_object_propagate_events_set(pSelectAllCheckbox, EINA_FALSE);
	elm_object_part_content_set(pSelectAllLayout, "elm.icon", pSelectAllCheckbox);
	mp_util_domain_translatable_part_text_set(pSelectAllLayout, "elm.text.main", MP_SCREEN_READER_SELECT_ALL);
	evas_object_show(pSelectAllLayout);
	*pCheckBox = pSelectAllCheckbox;
	return TRUE;

}

