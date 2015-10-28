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

#include "music.h"
#include "mp-search.h"
#include "mp-player-debug.h"
#include "mp-util.h"

#define MP_SEARCHBAR_W 400*elm_config_scale_get()

void
_mp_search_view_activated_cb(void *data, Evas_Object * obj, void *event_info)
{
	MP_CHECK(data);
	elm_object_focus_set(obj, FALSE);
}

void
_mp_search_view_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	MP_CHECK(data);
	MP_CHECK(obj);
	elm_object_focus_allow_set(obj,EINA_TRUE);
	elm_object_focus_set(obj, EINA_TRUE);
}

static void
_mp_search_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	startfunc;
	Evas_Object *entry = data;
	MP_CHECK(entry);

	elm_entry_entry_set(entry, "");
	elm_object_focus_set(entry, TRUE);
}

static void
_mp_search_entry_maxlength_reached_cb(void *data, Evas_Object * obj, void *event_info)
{
	mp_popup_max_length(obj, STR_NH_COM_POPUP_CHARACTERS_MAXNUM_REACHED);
}

static void
_mp_searchfield_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *editfield = (Evas_Object *)data;

	if (!elm_entry_is_empty(obj) && elm_object_focus_get(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
	else
		elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

static void
_mp_searchfield_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *editfield = (Evas_Object *)data;
	elm_object_signal_emit(editfield, "elm,state,focused", "");

	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(editfield, "elm,action,show,button", "");
}

static void
_mp_searchfield_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *editfield = (Evas_Object *)data;
	elm_object_signal_emit(editfield, "elm,state,unfocused", "");
	elm_object_signal_emit(editfield, "elm,action,hide,button", "");
}

static void	_mp_search_cancel_button_clicked(void *data, Evas_Object *o, const char *emission, const char *source)
{
	Evas_Object *en = (Evas_Object *) data;
	elm_object_text_set(en, "");
}

Evas_Object *
mp_search_create_new(Evas_Object * parent, Evas_Smart_Cb change_cb, void *change_cb_data, Evas_Smart_Cb cancel_cb, void *cancel_cb_data,
		Evas_Smart_Cb focus_cb, void *focus_cb_data, Evas_Smart_Cb unfocus_cb, void *unfocus_cb_data)
{
	startfunc;

	Evas_Object *en = NULL;
	Evas_Object *searchfield = NULL;

	searchfield = elm_layout_add(parent);
	Elm_Theme *th = elm_theme_new();
	elm_theme_extension_add(th, THEME_NAME);
	elm_layout_theme_set(searchfield, "layout", "searchbar", "cancel_button_case");
	//elm_layout_theme_set(searchfield, "layout", "searchfield", "singleline");
	const char *style ="DEFAULT='font=tizen; font_size=45'";

	en = elm_entry_add(searchfield);
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_entry_single_line_set(en, EINA_TRUE);
	elm_entry_prediction_allow_set(en, EINA_FALSE);
	elm_object_part_content_set(searchfield, "elm.swallow.content", en);
	evas_object_data_set(searchfield, "entry", en);

	elm_entry_input_panel_layout_set(en, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_input_panel_return_key_type_set(en, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);

	evas_object_size_hint_weight_set(searchfield, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(searchfield, EVAS_HINT_FILL, 0.0);
	elm_entry_text_style_user_push(en, style);
	mp_util_domain_translatable_part_text_set(en, "elm.guide",STR_SEARCH_GUIDE);
	evas_object_smart_callback_add(en, "changed", _mp_searchfield_changed_cb, searchfield);
	elm_object_signal_callback_add(searchfield, "elm,eraser,clicked", "elm", _mp_search_eraser_clicked_cb, en);

	evas_object_smart_callback_add(en, "changed", change_cb, change_cb_data);
	evas_object_smart_callback_add(en, "activated", _mp_search_view_activated_cb, searchfield);

	evas_object_smart_callback_add(en, "focused", _mp_searchfield_focused_cb, focus_cb_data);
	evas_object_smart_callback_add(en, "unfocused", _mp_searchfield_unfocused_cb, focus_cb_data);
	evas_object_smart_callback_add(en, "preedit,changed", change_cb, change_cb_data);
	evas_object_smart_callback_add(en, "clicked", _mp_search_view_clicked_cb, searchfield);

	edje_object_signal_callback_add(_EDJ(searchfield), "elm,action,click", "button_cancel_image",  _mp_search_cancel_button_clicked, en);

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = MP_SEARCH_MAX_CHAR_COUNT;
	limit_filter_data.max_byte_count = 0;
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data);
	evas_object_smart_callback_add(en, "maxlength,reached", _mp_search_entry_maxlength_reached_cb,NULL);
	evas_object_show(searchfield);

	return searchfield;
}

void
mp_search_hide_imf_pannel(Evas_Object * search)
{
	MP_CHECK(search);
	Evas_Object *en = mp_search_entry_get(search);
	elm_object_focus_set(en,EINA_FALSE);
}

void
mp_search_show_imf_pannel(Evas_Object * search)
{
	MP_CHECK(search);
	Evas_Object *en = mp_search_entry_get(search);
	elm_object_focus_set(en,EINA_TRUE);
}

Evas_Object *
mp_search_entry_get(Evas_Object *search)
{
	MP_CHECK_NULL(search);

	Evas_Object *entry = evas_object_data_get(search, "entry");

	return entry;
}

char *
mp_search_text_get(Evas_Object *search)
{
	MP_CHECK_NULL(search);
	Evas_Object *entry = evas_object_data_get(search, "entry");
	MP_CHECK_NULL(entry);

	const char *text = elm_entry_entry_get(entry);
        char *markup_text = elm_entry_markup_to_utf8(text);

	return markup_text;
}

void
mp_search_text_set(Evas_Object *search, const char *text)
{
	MP_CHECK(search);
	Evas_Object *entry = evas_object_data_get(search, "entry");
	MP_CHECK(entry);

	if (text == NULL)
		text = "";

	DEBUG_TRACE("Text: %s", text);
	elm_entry_entry_set(entry, text);
	elm_entry_cursor_end_set(entry);
	//elm_object_text_set(entry, text);
}


