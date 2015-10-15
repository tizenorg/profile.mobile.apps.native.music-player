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

#include "ms-play-speed.h"
#include "ms-key-ctrl.h"
#include "ms-util.h"
#include "mp-widget.h"

static double g_slider_val;
Elm_Object_Item *g_parent_it;
Evas_Object *g_layout;
static Ecore_Timer *longpress_timer = NULL;

static void _slider_change_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *slider = (Evas_Object*)data;
	double val = elm_slider_value_get(slider);
	int temp = 0;
	temp = (val+0.05)*10;
	val = (double)temp/10.;
	if (g_slider_val != val) {
		DEBUG_TRACE("%1.1f", val);
		g_slider_val = val;
		ms_key_set_play_speed(val);
		elm_genlist_item_fields_update(g_parent_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);
	}
}

static Evas_Object *
_ms_play_speed_create_slider(Evas_Object * parent)
{
	Evas_Object *slider = NULL;

	slider = elm_slider_add(parent);
	elm_object_style_set(slider, "music/tap_to_drag");
	elm_slider_indicator_show_set(slider, EINA_TRUE);
	evas_object_size_hint_weight_set(slider, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(slider, EVAS_HINT_FILL, 0.5);
	elm_slider_indicator_format_set(slider, "%1.1f");

	elm_slider_min_max_set(slider, 0.5, 2.0);
	elm_slider_value_set(slider, g_slider_val);

	evas_object_smart_callback_add(slider, "changed", _slider_change_cb, slider);

	return slider;
}

static void
_ms_play_speed_slider_value(MsSpeedOption_e option)
{
	startfunc;

        Evas_Object *slider = elm_object_part_content_get(g_layout, "ps_slider");
        MP_CHECK(slider);

        double val = elm_slider_value_get(slider);
        int temp = 0;

        if (option == MS_PLAY_SPEED_PLUS)
        {
		if (val >1.95 && val <= 2.0)
                {
                        mp_ecore_timer_del(longpress_timer);
			return;
                }
		val = val + 0.1;
		temp = (int)(val * 10.0) / 10.0;
        }
        else if (option == MS_PLAY_SPEED_MINUS)
        {
		if (val == 0.5)
                {
                        mp_ecore_timer_del(longpress_timer);
			return;
                }
		temp = (val-0.05)*10;
		val = (double)temp/10.;
        }
        g_slider_val = val;
        ms_key_set_play_speed(val);
        elm_slider_value_set(slider, val);
        elm_genlist_item_fields_update(g_parent_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);

}

static Eina_Bool
_ms_play_speed_long_press_timer_cb(void *data)
{
	startfunc;

        char *emission = NULL;
        emission = data;

        Evas_Object *slider = elm_object_part_content_get(g_layout, "ps_slider");
        MP_CHECK_FALSE(slider);

	if (!g_strcmp0(emission, "ps_minus_bt_press_down")) {
                _ms_play_speed_slider_value(MS_PLAY_SPEED_MINUS);
	}  else if (!g_strcmp0(emission, "ps_plus_bt_press_down")) {
                _ms_play_speed_slider_value(MS_PLAY_SPEED_PLUS);
	}

	if (longpress_timer)
		ecore_timer_interval_set(longpress_timer, 0.5);

	return ECORE_CALLBACK_RENEW;
}

static void _ms_play_speed_control_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	startfunc;
	MP_CHECK(emission);
	DEBUG_TRACE("emission=%s", emission);
	DEBUG_TRACE("source=%s", source);

	Evas_Object *slider = (Evas_Object*)data;
	MP_CHECK(slider);
	WARN_TRACE("slider=%p", slider);

	if (!g_strcmp0(emission, "ps_minus_bt_press_down")) {
                _ms_play_speed_slider_value(MS_PLAY_SPEED_MINUS);
                if (!longpress_timer)
                        longpress_timer = ecore_timer_add(0.3, _ms_play_speed_long_press_timer_cb, emission);
	} else if (!g_strcmp0(emission, "ps_plus_bt_press_down")) {
                _ms_play_speed_slider_value(MS_PLAY_SPEED_PLUS);
                if (!longpress_timer)
                        longpress_timer = ecore_timer_add(0.3, _ms_play_speed_long_press_timer_cb, emission);
	} else if (!g_strcmp0(emission, "ps_minus_bt_up") || !g_strcmp0(emission, "ps_plus_bt_up")) {
                mp_ecore_timer_del(longpress_timer);
        }

	endfunc;
}

static const char *g_play_speed_btn_part[]= {"ps_minus_bt", "ps_plus_bt", NULL};

static const char *
_ms_play_speed_screen_reader_find_button_part(const Evas_Object *edje_obj, const Evas_Object *part_obj)
{
	startfunc;

	MP_CHECK_NULL(edje_obj);
	MP_CHECK_NULL(part_obj);

	WARN_TRACE("edje_obj:%p",edje_obj);
	WARN_TRACE("part_obj:%p",part_obj);

	Evas_Object *obj = NULL;
	int index = 0;

	while (g_play_speed_btn_part[index]) {
		WARN_TRACE("g__now_playing_btn_part[index]:%s",g_play_speed_btn_part[index]);
		obj = (Evas_Object *)edje_object_part_object_get(edje_obj, g_play_speed_btn_part[index]);
		WARN_TRACE("obj:%p",obj);
		if (obj == part_obj) return g_play_speed_btn_part[index];
		++index;
	}

	return NULL;
	endfunc;
}

static void
_ms_play_speed_tts_action_double_cb(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	startfunc;

	Evas_Object *slider = (Evas_Object*)data;
	MP_CHECK(slider);
	WARN_TRACE("slider=%p", slider);

	const char *part = _ms_play_speed_screen_reader_find_button_part(elm_layout_edje_get(g_layout), obj);
	MP_CHECK(part);

        WARN_TRACE("part is:%s",part);

        if (!g_strcmp0(part, "ps_minus_bt")) {
		double val = elm_slider_value_get(slider);
		WARN_TRACE("val=%f", val);
		WARN_TRACE("g_slider_val=%f", g_slider_val);
		if (val == 0.5)
			return;
		int temp = 0;
		temp = (val-0.05)*10;
		val = (double)temp/10.;
		DEBUG_TRACE("%1.1f", val);
		g_slider_val = val;
		ms_key_set_play_speed(val);
		elm_slider_value_set(slider, val);
		elm_genlist_item_fields_update(g_parent_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);
        }
        else if (!g_strcmp0(part, "ps_plus_bt"))
        {
		double val = elm_slider_value_get(slider);
		WARN_TRACE("val=%f", val);
		WARN_TRACE("g_slider_val=%f", g_slider_val);
		if (val >1.95 && val <= 2.0)
			return;
		val = val + 0.1;
		DEBUG_TRACE("%1.1f", val);
		g_slider_val = val;
		ms_key_set_play_speed(val);
		elm_slider_value_set(slider, val);
		elm_genlist_item_fields_update(g_parent_it, "elm.text.2", ELM_GENLIST_ITEM_FIELD_TEXT);
        }

}

static void _ms_play_speed__realized(void *data, Evas_Object *obj, void *ei)
{

	MP_CHECK(ei);
	Elm_Object_Item *item = ei;

	const Elm_Genlist_Item_Class *itc = elm_genlist_item_item_class_get(item);
	if (!strcmp(itc->item_style, "dialogue/1icon")) {
		/* unregister item itself */
		elm_object_item_access_unregister(item);

		/* convey highlight to its content */
		Evas_Object *content;
		content = elm_object_item_part_content_get(item, "elm.icon");
		MP_CHECK(content);

		//add accesss to layout
		//Eina_List *items = NULL;
		//Evas_Object * tmp_obj1 = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(content), "ps_minus_bt");
		//Evas_Object * tmp_obj2 = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(content), "ps_plus_bt");
		//Evas_Object * tmp_obj3 = (Evas_Object *)edje_object_part_object_get(elm_layout_edje_get(content), "ps_slider");
	}
}
