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

#include "mc-index.h"
#include "mc-debug.h"
#ifdef GBSBUILD
#include <vconf.h>
#endif
#include "mc-track-list.h"
#include <system_settings.h>

typedef struct{
	char *first;
	Eina_Bool multiple_selection;
}MpIndexData_t;

#define GET_WIDGET_DATA(o) evas_object_data_get(o, "widget_d");
static const char *_mc_list_item_get_label(Elm_Object_Item *event_info)
{
	char *title = NULL;

	list_item_data_t *item_data =  elm_object_item_data_get(event_info);
	if (!item_data || item_data->list_type < MC_TRACK || item_data->list_type > MC_FOLDER_TRACK) {
		return NULL;
	}
	if ((item_data->list_type == MC_TRACK)
	|| (item_data->list_type == MC_ALBUM_TRACK)
	|| (item_data->list_type == MC_ARTIST_TRACK)
	|| (item_data->list_type == MC_FOLDER_TRACK))
	{
		mp_media_info_get_title(item_data->media, &title);
	}
	else if ((item_data->list_type == MC_ALBUM)
		||(item_data->list_type == MC_ARTIST)
		||(item_data->list_type == MC_FOLDER))
	{
		mp_media_info_group_get_main_info(item_data->media, &title);
	}
	return title;
}

static gchar *
_mc_util_get_utf8_initial(const char *name)
{
	gunichar first;
	char *next = NULL;
	MP_CHECK_NULL(name);

	if (g_utf8_strlen(name, -1) <= 0)
	{
		return strdup("");
	}

	first = g_utf8_get_char_validated(name, g_utf8_strlen(name, -1));
	if (first == (gunichar) - 1 || first == (gunichar) - 2) {
		DEBUG_TRACE ("failed to convert a sequence of bytes encoded as UTF-8 to a Unicode character.");
		return strdup("");
	}

	next = (char *)name;

	while (!g_unichar_isgraph(first))
	{
		next = g_utf8_next_char(next);
		first = g_utf8_get_char_validated(next, g_utf8_strlen(name, -1));
		if (first == (gunichar) - 1 || first == (gunichar) - 2) {
			DEBUG_TRACE ("failed to convert a sequence of bytes encoded as UTF-8 to a Unicode character.");
			return strdup("");
		}
	}

	if (first >= 0xAC00 && first <= 0xD7A3)
	{			//korean
		int index = 0;
		index = ((((first - 0xAC00) - ((first - 0xAC00) % 28)) / 28) / 21);
		if (index < 20 && index >= 0)
		{
			const gunichar chosung[20] = { 0x3131, 0x3132, 0x3134, 0x3137, 0x3138,
				0x3139, 0x3141, 0x3142, 0x3143, 0x3145,
				0x3146, 0x3147, 0x3148, 0x3149, 0x314a,
				0x314b, 0x314c, 0x314d, 0x314e, 0
			};

			gchar result[10] = { 0, };
			int len = 0;
			len = g_unichar_to_utf8(chosung[index], result);
			return strndup(result, len + 1);
		}
	}
	else
	{
		gchar result[10] = { 0, };
		int len = 0;
		len = g_unichar_to_utf8(first, result);
		return strndup(result, len + 1);
	}
	return NULL;
}

static void
_index_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
        Elm_Object_Item *item = (Elm_Object_Item*)event_info;

        elm_index_item_selected_set(item, EINA_FALSE);
}

static void
_mc_index_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	const char *index_letter = NULL, *label = NULL;
	Evas_Object *genlist = (Evas_Object *)data;
	MP_CHECK(genlist);

	MpIndexData_t *wd = GET_WIDGET_DATA(obj);
	MP_CHECK(wd);

	index_letter = elm_index_item_letter_get(event_info);
	MP_CHECK(index_letter);

	Elm_Object_Item *gl_item = elm_genlist_first_item_get(genlist);
	if (wd->multiple_selection) {
		gl_item = elm_genlist_item_next_get(gl_item);
	}
	while (gl_item)
	{
		char *uni = NULL;
		label = _mc_list_item_get_label(gl_item);
		if (!label) {
			gl_item = elm_genlist_item_next_get(gl_item);
			continue;
		}

		uni = _mc_util_get_utf8_initial(label);
		if (uni == NULL) {
			ERROR_TRACE("ERROR :label is not found");
			return;
		}
#if 0 //print code
		char *text = NULL;
		char code[16] = {0,};
		text = g_strconcat("0x", NULL);
		for (i= 0; i<strlen(uni) ; i++)
		{
			snprintf(code, 16, "%x", uni[i]);
			code[15] = 0;
			text = g_strconcat(text, code, NULL);
		}
		DEBUG_TRACE("uni: %s, code: %s, A: 0x%x", uni, code, 'A');
#endif
		if (!g_strcmp0(index_letter, wd->first))
		{
			if (uni[0] < 'A'||uni[0] > 'z')
			{
				elm_genlist_item_bring_in(gl_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
				IF_FREE(uni);
				break;
			}
		}

		if (!strcasecmp(uni, index_letter))
		{
			elm_genlist_item_bring_in(gl_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			IF_FREE(uni);
			break;
		}
		else
		{
			char *capital = g_utf8_strup(uni, -1);
			if (!capital)
			{
					IF_FREE(uni);
					continue;
			}

			if (capital[0] > index_letter[0]) //move to most close item
			{
				elm_genlist_item_bring_in(gl_item, ELM_GENLIST_ITEM_SCROLLTO_TOP);
				IF_FREE(uni);
				IF_FREE(capital);
				break;
			}
			IF_FREE(capital);
		}
		gl_item = elm_genlist_item_next_get(gl_item);
		IF_FREE(uni);
	}
}

Eina_Bool ea_locale_latin_get(const char *locale)
{
   if (!locale) return EINA_FALSE;

   int i = 0;

   while(non_latin_lan[i])
     {
        if (!strcmp(non_latin_lan[i], locale)) return EINA_FALSE;
        i++;
     }
   return EINA_TRUE;
}


static void _mp_fastscoller_append_item(void *data, Evas_Object *obj)
{
	int i = 0, j, len;
	char *str = NULL;
	char buf[PATH_MAX] = {0, };
	Eina_Unicode uni;
	Elm_Object_Item *it = NULL;
	char *locale = NULL;
	Evas_Object *list = (Evas_Object *)data;
	MP_CHECK(obj);
	elm_index_item_clear(obj);

	MpIndexData_t *wd = GET_WIDGET_DATA(obj);
	MP_CHECK(wd);
	//1. Special character & Numbers
	elm_index_item_append(obj, "#", _mc_index_item_selected_cb, list);
	if (!wd->first)
		wd->first = g_strdup("#");

	//2. Local language
	str = dgettext("efl-extension", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	MP_CHECK(str);
	len = strlen(str);
	if (len == 0)
		return;
	while (i < len)
	{
		j = i;
		uni = eina_unicode_utf8_next_get(str, &i);
		MP_CHECK(uni);
		snprintf(buf, i - j + 1, "%s", str + j);
		buf[i - j + 1] = 0;

		it = elm_index_item_append(obj, buf, _mc_index_item_selected_cb, list);
		//elm_index_item_priority_set(it, 0);
	}

	//3. English - in case of non-latin
	int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	MP_CHECK(locale);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ERROR_TRACE("Unable to fetch the current language setting with return value %d", retcode);
	}
	if (!ea_locale_latin_get(locale))
	{
		str = dgettext("efl-extension", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ_SECOND");
		MP_CHECK(str);
		len = strlen(str);

		i = 0;
		while (i < len)
		{
			j = i;
			uni = eina_unicode_utf8_next_get(str, &i);
			MP_CHECK(uni);
			snprintf(buf, i - j + 1, "%s", str + j);
			buf[i - j + 1] = 0;

			it = elm_index_item_append(obj, buf, _mc_index_item_selected_cb, list);
			//elm_index_item_priority_set(it, 1);
		}

		setlocale(LC_MESSAGES, locale);
	}
	IF_FREE(locale);

}
static void _append_item(Evas_Object *index, Evas_Object *list)
{
	_mp_fastscoller_append_item((void*)list, index);
	elm_index_level_go(index, 0);
	evas_object_smart_callback_add(index, "changed", _mc_index_item_selected_cb, list);
        evas_object_smart_callback_add(index, "selected", _index_selected_cb, NULL);
}

static Evas_Object* _create_fastscroll(Evas_Object* parent)
{
	Evas_Object *index;
	index = elm_index_add(parent);
	elm_index_omit_enabled_set(index, EINA_TRUE);
	return index;
}

static void
_widget_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpIndexData_t *wd = data;
	MP_CHECK(wd);
	IF_FREE(wd->first);
	IF_FREE(wd);
}

void _language_changed(void *data, Evas_Object *obj, void *event_info)
{
	MP_CHECK(data);
	MP_CHECK(obj);
	_mp_fastscoller_append_item(data,obj);
	elm_index_level_go(obj, 0);
	evas_object_smart_callback_add(obj, "changed", _mc_index_item_selected_cb, (Evas_Object *)data);
	evas_object_smart_callback_add(obj, "selected", _index_selected_cb, NULL);
}

Evas_Object *mc_index_create(Evas_Object *parent, int group_type, void *data)
{
	Evas_Object *index = NULL;
	MpIndexData_t *wd = NULL;
	Evas_Object *list = (Evas_Object *)data;
	MP_CHECK_NULL(list);

	// Create index
	index = _create_fastscroll(parent);
	elm_index_autohide_disabled_set(index, EINA_TRUE);

	wd = calloc(1, sizeof(MpIndexData_t));
	if (!wd) {
		ERROR_TRACE("Error: memory alloc failed");
		evas_object_del(index);
		return NULL;
	}

	if (group_type == 1) {
		wd->multiple_selection = EINA_TRUE;
	} else {
		wd->multiple_selection = EINA_FALSE;
	}

	evas_object_data_set(index, "widget_d", wd);

	evas_object_event_callback_add(index, EVAS_CALLBACK_FREE, _widget_del_cb, wd);
	//language change the fast scroll language will change too
	evas_object_smart_callback_add(index, "language,changed", _language_changed, list);
	return index;
}

void mc_index_append_item(Evas_Object *index, Evas_Object *list)
{
	MP_CHECK(list);
	_append_item(index, list);
	elm_index_level_go(index, 0);
}



