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

#include "mc-search.h"
#include "music-chooser.h"

void
_mc_search_view_activated_cb(void *data, Evas_Object * obj, void *event_info)
{
	MP_CHECK(data);
	elm_object_focus_set(obj, FALSE);
}

void
_mc_search_view_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	MP_CHECK(data);
	MP_CHECK(obj);
	elm_object_focus_allow_set(obj, EINA_TRUE);
	elm_object_focus_set(obj, EINA_TRUE);
}

static void
_mc_search_entry_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	Evas_Object *searchbar = data;
	MP_CHECK(searchbar);
	Evas_Object *entry = obj;
	MP_CHECK(entry);

	const char *signal = NULL;
	if (elm_entry_is_empty(entry)) {
		signal = "elm,state,eraser,hide";
	} else {
		signal = "elm,state,eraser,show";
	}

	elm_object_signal_emit(searchbar, signal, "elm");
}

static void
_mc_search_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	startfunc;
	Evas_Object *entry = data;
	MP_CHECK(entry);

	elm_entry_entry_set(entry, "");
	elm_object_focus_set(entry, TRUE);
}

static void _entry_text_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	Evas_Object *searchbar = data;
	MP_CHECK(searchbar);
	Evas_Object *entry = obj;
	MP_CHECK(entry);

	const char *signal = NULL;
	if (elm_entry_is_empty(entry)) {
		signal = "elm,state,eraser,hide";
	} else {
		signal = "elm,state,eraser,show";
	}

	elm_object_signal_emit(searchbar, signal, "elm");
}

static void
_mc_search_entry_maxlength_reached_cb(void *data, Evas_Object * obj, void *event_info)
{
	//mp_popup_max_length(obj, STR_NH_COM_POPUP_CHARACTERS_MAXNUM_REACHED);
}


Evas_Object *
mc_search_create_new(Evas_Object * parent, Evas_Smart_Cb change_cb, void *change_cb_data, Evas_Smart_Cb cancel_cb, void *cancel_cb_data,
                     Evas_Smart_Cb focus_cb, void *focus_cb_data, Evas_Smart_Cb unfocus_cb, void *unfocus_cb_data)
{
	startfunc;
	Evas_Object *sb = NULL;
	Evas_Object *en = NULL;

	sb = elm_layout_add(parent);
	MP_CHECK_NULL(sb);

	const char *style = (cancel_cb) ? "cancel_button" : "default";
	elm_layout_theme_set(sb, "layout", "searchbar", style);

	if (cancel_cb) {
		Evas_Object *cancel_btn = elm_button_add(sb);
		elm_object_style_set(cancel_btn, "searchbar/default");
		//elm_object_text_set(cancel_btn, GET_SYS_STR("IDS_COM_SK_CANCEL"));
		//mp_language_mgr_register_object(cancel_btn, OBJ_TYPE_ELM_OBJECT, NULL, "IDS_COM_SK_CANCEL");
		mc_common_obj_domain_text_translate(cancel_btn, "IDS_COM_SK_CANCEL");
		evas_object_smart_callback_add(cancel_btn, "clicked", cancel_cb, cancel_cb_data);

		elm_object_part_content_set(sb, "button_cancel", cancel_btn);
		elm_object_signal_emit(sb, "cancel,show", "");
	}
	//en = elm_entry_add(sb);
	en = elm_entry_add(sb);
	elm_entry_scrollable_set(en, EINA_TRUE);
	elm_entry_single_line_set(en, EINA_TRUE);
	//elm_entry_prediction_allow_set(en, EINA_FALSE);
	elm_object_part_content_set(sb, "elm.swallow.content", en);
	evas_object_data_set(sb, "entry", en);
	//elm_object_part_text_set(en, "elm.guide", GET_STR(STR_SEARCH_GUIDE));
	mc_common_obj_domain_translatable_part_text_set(en, "elm.guide", MC_TEXT_SEARCH);

	elm_entry_input_panel_layout_set(en, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_input_panel_return_key_type_set(en, ELM_INPUT_PANEL_RETURN_KEY_TYPE_SEARCH);

	evas_object_size_hint_weight_set(sb, EVAS_HINT_EXPAND, 0);
	evas_object_size_hint_align_set(sb, EVAS_HINT_FILL, 0.0);

	evas_object_smart_callback_add(en, "changed", _mc_search_entry_changed_cb, sb);
	elm_object_signal_callback_add(sb, "elm,eraser,clicked", "elm", _mc_search_eraser_clicked_cb, en);

	evas_object_smart_callback_add(en, "changed", change_cb, change_cb_data);
	evas_object_smart_callback_add(en, "activated", _mc_search_view_activated_cb, sb);

	evas_object_smart_callback_add(en, "focused", focus_cb, focus_cb_data);
	evas_object_smart_callback_add(en, "unfocused", unfocus_cb, focus_cb_data);

	evas_object_smart_callback_add(en, "preedit,changed", _entry_text_changed_cb, sb);
	evas_object_smart_callback_add(en, "preedit,changed", change_cb, change_cb_data);
	evas_object_smart_callback_add(en, "clicked", _mc_search_view_clicked_cb, sb);

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = 0;
	limit_filter_data.max_byte_count = 193;
	elm_entry_markup_filter_append(en, elm_entry_filter_limit_size, &limit_filter_data);
	evas_object_smart_callback_add(en, "maxlength,reached", _mc_search_entry_maxlength_reached_cb, NULL);

	evas_object_show(sb);
	//elm_object_focus_set(en,EINA_TRUE);

	return sb;
}

void
mc_search_hide_imf_pannel(Evas_Object * search)
{
	MP_CHECK(search);
	Evas_Object *en = mc_search_entry_get(search);
	elm_object_focus_set(en, EINA_FALSE);
}

void
mc_search_show_imf_pannel(Evas_Object * search)
{
	MP_CHECK(search);
	Evas_Object *en = mc_search_entry_get(search);
	elm_object_focus_set(en, EINA_TRUE);
}

Evas_Object *
mc_search_entry_get(Evas_Object *search)
{
	MP_CHECK_NULL(search);

	Evas_Object *entry = evas_object_data_get(search, "entry");

	return entry;
}

char *
mc_search_text_get(Evas_Object *search)
{
	MP_CHECK_NULL(search);
	Evas_Object *entry = evas_object_data_get(search, "entry");
	MP_CHECK_NULL(entry);

	const char *text = elm_entry_entry_get(entry);
	char *markup_text = elm_entry_markup_to_utf8(text);

	return markup_text;
}

void
mc_search_text_set(Evas_Object *search, const char *text)
{
	MP_CHECK(search);
	Evas_Object *entry = evas_object_data_get(search, "entry");
	MP_CHECK(entry);

	if (text == NULL) {
		text = "";
	}

	DEBUG_TRACE("Text: %s", text);
	elm_entry_entry_set(entry, text);
	elm_entry_cursor_end_set(entry);
	//elm_object_text_set(entry, text);
}

