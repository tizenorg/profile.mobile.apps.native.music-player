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


#include <glib.h>
#include "ms-effect-view.h"
#include "ms-key-ctrl.h"
#include "ms-util.h"
#include "ms-eq-view.h"
#include <efl_extension.h>

int _g_auto_off_time[KEY_MUSIC_AUTO_OFF_TIME_MAX] = {
	0,
	15,
	30,
	60,
	90,
	120,
	-1,
};

static Evas_Object *g_radio_grp;
static Elm_Object_Item *g_auto_off_gl_it;
static int g_hour, g_min;

static void
_ms_auto_off_radio_del_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE("radio group delected");
	g_radio_grp = NULL;
	return;
}

static void
_ms_auto_off_radio_change_cb(void *data, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE("");
	mp_retm_if (!g_radio_grp, "radio group is null");

	int type = elm_radio_value_get(g_radio_grp);
	DEBUG_TRACE("selected radio val : %d type, %d min", type, _g_auto_off_time[type]);

	ms_key_set_auto_off_val(type);

	if (type == KEY_MUSIC_AUTO_OFF_TIME_CUSTOM)
		ms_key_set_auto_off_time( 60*g_hour + g_min);
	else
		ms_key_set_auto_off_time(_g_auto_off_time[type]);

	if (g_auto_off_gl_it)
		elm_genlist_item_update(g_auto_off_gl_it);

	elm_genlist_item_selected_set(g_auto_off_gl_it, EINA_FALSE);
	bool status = !elm_genlist_item_expanded_get(g_auto_off_gl_it);
	elm_genlist_item_expanded_set(g_auto_off_gl_it, status);
	elm_genlist_item_subitems_clear(g_auto_off_gl_it);
	elm_genlist_realized_items_update(obj);
}

static char *
_ms_auto_off_gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	int param = (int)data;

	char *txt = ms_key_get_auto_off_time_text(param);
	return (txt) ? strdup(txt) : NULL;
}
static inline void _picker_back_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	elm_object_signal_emit(data, "picker,hide", "");
}

static void _ms_auto_off_customize_edit_start_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = (Evas_Object *)data;
	MP_CHECK(genlist);
	eext_object_event_callback_add(genlist, EEXT_CALLBACK_BACK, (Eext_Event_Cb)_picker_back_cb, obj);
	evas_object_data_set(genlist, "customized_on", (void *)1);
}

static void _ms_auto_off_customize_edit_end_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *genlist = (Evas_Object *)data;
	MP_CHECK(genlist);
	eext_object_event_callback_del(genlist, EEXT_CALLBACK_BACK, (Eext_Event_Cb)_picker_back_cb);
	evas_object_data_set(genlist, "customized_on", (void *)0);

	elm_genlist_item_selected_set(g_auto_off_gl_it, EINA_FALSE);
	bool status = !elm_genlist_item_expanded_get(g_auto_off_gl_it);
	elm_genlist_item_expanded_set(g_auto_off_gl_it, status);
	elm_genlist_item_subitems_clear(g_auto_off_gl_it);
	elm_genlist_realized_items_update(genlist);
	endfunc;
}

static void _set_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("Set clicked");
	Evas_Object *genlist = (Evas_Object *)data;
	struct tm time1;

	elm_datetime_value_get(obj, &time1);
	DEBUG_TRACE("hour: %d, min: %d", time1.tm_hour, time1.tm_min);

	int min = time1.tm_hour * 60 + time1.tm_min;
	ms_key_set_auto_off_custom_time(min);

	g_hour = time1.tm_hour;
	g_min = time1.tm_min;

	if (60*g_hour + g_min == 0)
	{
		ms_key_set_auto_off_val(KEY_MUSIC_AUTO_OFF_TIME_OFF);
		elm_radio_value_set(g_radio_grp, KEY_MUSIC_AUTO_OFF_TIME_OFF);
	}
	else
	{
		ms_key_set_auto_off_val(KEY_MUSIC_AUTO_OFF_TIME_CUSTOM);
		elm_radio_value_set(g_radio_grp, KEY_MUSIC_AUTO_OFF_TIME_CUSTOM);
	}

	ms_key_set_auto_off_time( 60*g_hour + g_min);

	if (g_auto_off_gl_it)
		elm_genlist_item_update(g_auto_off_gl_it);

	elm_genlist_realized_items_update(genlist);
}

static Evas_Object *
_ms_auto_off_gl_content_get(void *data, Evas_Object * obj, const char *part)
{
	int param = (int)data;

	mp_retvm_if (param >= KEY_MUSIC_AUTO_OFF_TIME_MAX, NULL, "invalid param: %d", param);

	if (!g_strcmp0(part, "elm.icon") || !g_strcmp0(part, "elm.icon.1"))
	{
		Evas_Object *radio_btn = elm_radio_add(obj);
		evas_object_propagate_events_set(radio_btn, FALSE);

		elm_radio_state_value_set(radio_btn, param);

		elm_radio_group_add(radio_btn, g_radio_grp);

		int type = ms_key_get_auto_off_val();
		if (g_radio_grp)
		{
			elm_radio_value_set(g_radio_grp, type);
		}

		evas_object_smart_callback_add(radio_btn, "changed", _ms_auto_off_radio_change_cb, NULL);
		evas_object_show(radio_btn);
		return radio_btn;
	}
	return NULL;
}

static void
_ms_auto_off_gl_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	int param = (int)data;
	DEBUG_TRACE("data: %d", param);

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	if (param != KEY_MUSIC_AUTO_OFF_TIME_CUSTOM)
	{
		elm_radio_value_set(g_radio_grp, param);
		evas_object_smart_callback_call(g_radio_grp, "changed", NULL);
	}
	else
	{	Evas_Object *datetime = elm_datetime_add(obj);
		if (datetime != NULL)
		{
			elm_object_style_set(datetime, "pickerstyle");
			elm_datetime_format_set(datetime, "%H:%M");

			struct tm time1;
			memset(&time1, 0, sizeof(time1));
			time1.tm_year = 2;
			time1.tm_mon = 1;
			time1.tm_mday = 1;

			int min = ms_key_get_auto_off_custom_time();
			DEBUG_TRACE("min: %d", min);
			time1.tm_hour = min/60;
			time1.tm_min = min%60;
			elm_datetime_value_set(datetime, &time1);

			//evas_object_smart_callback_add(datetime, "changed", _ms_auto_off_time_changed_cb, NULL);
			evas_object_smart_callback_add(datetime, "picker,value,set", _set_clicked_cb, obj);
			evas_object_smart_callback_add(datetime, "edit,start", _ms_auto_off_customize_edit_start_cb, obj);
			evas_object_smart_callback_add(datetime, "edit,end", _ms_auto_off_customize_edit_end_cb, obj);
			elm_object_signal_emit(datetime, "timepicker,show", "");
		}
	}

}

EXPORT_API void
ms_auto_off_list_create(Elm_Object_Item *parent_item)
{
	mp_retm_if (parent_item == NULL, "parent_item is null");

	Evas_Object *genlist = elm_object_item_widget_get(parent_item);

	g_auto_off_gl_it = parent_item;

	//elm_genlist_tree_effect_enabled_set(genlist, EINA_TRUE);

	g_radio_grp = elm_radio_add(genlist);
	elm_radio_state_value_set(g_radio_grp, -1);
	evas_object_smart_callback_add(g_radio_grp, "changed", _ms_auto_off_radio_change_cb, NULL);
	evas_object_event_callback_add(g_radio_grp, EVAS_CALLBACK_DEL, _ms_auto_off_radio_del_cb, NULL);
	evas_object_hide(g_radio_grp);

	static Elm_Genlist_Item_Class itc, custom_itc;

	itc.func.text_get = _ms_auto_off_gl_label_get;
	itc.func.content_get = _ms_auto_off_gl_content_get;
	itc.item_style = "dialogue/1text.1icon.3";
	itc.version = ELM_GENGRID_ITEM_CLASS_VERSION;
	itc.refcount = 0;
	itc.delete_me = EINA_FALSE;

	custom_itc.func.text_get = _ms_auto_off_gl_label_get;
	custom_itc.func.content_get = _ms_auto_off_gl_content_get;
	custom_itc.item_style = "dialogue/1text.1icon.3";
	custom_itc.version = ELM_GENGRID_ITEM_CLASS_VERSION;
	custom_itc.refcount = 0;
	custom_itc.delete_me = EINA_FALSE;

	int type = KEY_MUSIC_AUTO_OFF_TIME_OFF;
	while (type < KEY_MUSIC_AUTO_OFF_TIME_CUSTOM)
	{
		elm_genlist_item_append(genlist, &itc, (void *)type, parent_item,
					ELM_GENLIST_ITEM_NONE, _ms_auto_off_gl_sel_cb, (void *)type);
		type ++;
	}

	//custom
	Elm_Object_Item *item = elm_genlist_item_append(genlist, &custom_itc, (void *)type, parent_item,
					ELM_GENLIST_ITEM_NONE, _ms_auto_off_gl_sel_cb, (void *)type);
	evas_object_data_set(genlist, "customized_item", (void *)item);
}


