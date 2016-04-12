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

#include "mp-index.h"
#include "mp-player-debug.h"
#include "mp-util.h"
#include <system_settings.h>

typedef struct {
	char *first;

} MpIndexData_t;

static char *non_latin_lan[] = {
     "ar_AE.UTF-8",
     "as_IN.UTF-8",
     "bg_BG.UTF-8",
     "bn_IN.UTF-8",
     "el_GR.UTF-8",
     "fa_IR.UTF-8",
     "gu_IN.UTF-8",
     "he_IL.UTF-8",
     "hi_IN.UTF-8",
     "hy_AM.UTF-8",
     "ja_JP.UTF-8",
     "ka_GE.UTF-8",
     "kk_KZ.UTF-8",
     "km_KH.UTF-8",
     "kn_IN.UTF-8",
     "ko_KR.UTF-8",
     "lo_LA.UTF-8",
     "mk_MK.UTF-8",
     "ml_IN.UTF-8",
     "mn_MN.UTF-8",
     "mr_IN.UTF-8",
     "ne_NP.UTF-8",
     "or_IN.UTF-8",
     "pa_IN.UTF-8",
     "ru_RU.UTF-8",
     "si_LK.UTF-8",
     "ta_IN.UTF-8",
     "te_IN.UTF-8",
     "th_TH.UTF-8",
     "uk_UA.UTF-8",
     "ur_PK.UTF-8",
     "zh_TW.UTF-8",
     NULL
};

#define GET_WIDGET_DATA(o) evas_object_data_get(o, "widget_d");

static void
_index_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *item = (Elm_Object_Item*)event_info;

	elm_index_item_selected_set(item, EINA_FALSE);
}

static void
_mp_index_item_selected_cb(void *data, Evas_Object *obj, void *event_info)
{
	const char *index_letter = NULL, *label = NULL;
	MpList_t *list = data;
	MP_CHECK(list);
	Evas_Object *genlist = list->genlist;
	MP_CHECK(genlist);
	MP_CHECK(mp_list_get_display_mode(list) == MP_LIST_DISPLAY_MODE_NORMAL);

	MpIndexData_t *wd = GET_WIDGET_DATA(obj);
	MP_CHECK(wd);

	index_letter = elm_index_item_letter_get(event_info);
	MP_CHECK(index_letter);

	Elm_Object_Item *matched = NULL;
	Elm_Object_Item *closed = NULL;

	Elm_Object_Item *gl_item = elm_genlist_first_item_get(genlist);

	if (!g_strcmp0(index_letter, wd->first)) {
		DEBUG_TRACE("%s selected", wd->first);	//"#" case
		matched = gl_item;
		goto END;
	}

	gunichar uni_index = g_utf8_get_char_validated(index_letter, g_utf8_strlen(index_letter, -1));
	if (uni_index == (gunichar) - 1 || uni_index == (gunichar) - 2) {
		DEBUG_TRACE("failed to convert a sequence of bytes encoded as UTF-8 to a Unicode character.");
		return;
	}

	while (gl_item) {
		gunichar uni = 0;
		label = mp_list_get_list_item_label(list, gl_item);

		if (label) {
			char *capital = g_utf8_strup(label, -1);
			uni = mp_util_get_utf8_initial_value(capital);

			if (uni == uni_index) {
				matched = gl_item;
				break;
			} else if (closed == NULL && (g_unichar_isalpha(uni) || uni > 0x0400)) {
				/*
				char first[10] = {0,}, index[10]= {0,};
				g_unichar_to_utf8(uni, first);
				g_unichar_to_utf8(uni, index);
				DEBUG_TRACE("uni[0x%x, %s], uni_index[0x%x, %s]", uni, first, uni_index, index);
				*/
				if (capital && uni > uni_index) { //move to most close item
					closed = gl_item;
				}
				IF_FREE(capital);
			}
		}
		gl_item = elm_genlist_item_next_get(gl_item);
	}

END:
	if (matched) {
		elm_genlist_item_show(matched, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	} else if (closed) {
		elm_genlist_item_show(closed, ELM_GENLIST_ITEM_SCROLLTO_TOP);
	}
}

Eina_Bool ea_locale_latin_get(const char *locale)
{
	if (!locale) {
		return EINA_FALSE;
	}

	int i = 0;

	while (non_latin_lan[i]) {
		if (!strcmp(non_latin_lan[i], locale)) {
			return EINA_FALSE;
		}
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
	char *locale = NULL;
	MpList_t *list = (MpList_t *)data;
	MP_CHECK(obj);
	elm_index_item_clear(obj);

	MpIndexData_t *wd = GET_WIDGET_DATA(obj);
	MP_CHECK(wd);
	//1. Special character & Numbers
	elm_index_item_append(obj, "#", _mp_index_item_selected_cb, list);
	if (!wd->first) {
		wd->first = g_strdup("#");
	}

	//2. Local language
	str = dgettext("efl-extension", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ");
	MP_CHECK(str);
	len = strlen(str);
	if (len == 0) {
		return;
	}
	while (i < len) {
		j = i;
		uni = eina_unicode_utf8_next_get(str, &i);
		MP_CHECK(uni);
		snprintf(buf, i - j + 1, "%s", str + j);
		buf[i - j + 1] = 0;

		elm_index_item_append(obj, buf, _mp_index_item_selected_cb, list);
		//elm_index_item_priority_set(it, 0);
	}

	//3. English - in case of non-latin
	int retcode = system_settings_get_value_string(SYSTEM_SETTINGS_KEY_LOCALE_LANGUAGE, &locale);
	if (retcode != SYSTEM_SETTINGS_ERROR_NONE) {
		ERROR_TRACE("Unable to fetch the current language setting with return value %d", retcode);
	}
	MP_CHECK(locale);
	if (!ea_locale_latin_get(locale)) {
		str = dgettext("efl-extension", "IDS_EA_BODY_ABCDEFGHIJKLMNOPQRSTUVWXYZ_SECOND");
		MP_CHECK(str);
		len = strlen(str);

		i = 0;
		while (i < len) {
			j = i;
			uni = eina_unicode_utf8_next_get(str, &i);
			MP_CHECK(uni);
			snprintf(buf, i - j + 1, "%s", str + j);
			buf[i - j + 1] = 0;

			elm_index_item_append(obj, buf, _mp_index_item_selected_cb, list);
			//elm_index_item_priority_set(it, 1);
		}

		setlocale(LC_MESSAGES, locale);
	}
	IF_FREE(locale);

}
static void _append_item(Evas_Object *index, MpList_t *list)
{
	_mp_fastscoller_append_item((void*)list, index);
	elm_index_level_go(index, 0);
	evas_object_smart_callback_add(index, "changed", _mp_index_item_selected_cb, list);
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
	_mp_fastscoller_append_item(data, obj);
	elm_index_level_go(obj, 0);
	evas_object_smart_callback_add(obj, "changed", _mp_index_item_selected_cb, (MpList_t *)data);
	evas_object_smart_callback_add(obj, "selected", _index_selected_cb, NULL);
}

Evas_Object *mp_index_create(Evas_Object *parent, int group_type, void *data)
{
	Evas_Object *index = NULL;
	MpIndexData_t *wd = NULL;
	MpList_t *list = (MpList_t *)data;
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

	evas_object_data_set(index, "widget_d", wd);

	evas_object_event_callback_add(index, EVAS_CALLBACK_FREE, _widget_del_cb, wd);
	//language change the fast scroll language will change too
	evas_object_smart_callback_add(index, "language,changed", _language_changed, list);
	return index;
}

void mp_index_append_item(Evas_Object *index, MpList_t *list)
{
	MP_CHECK(list);
	_append_item(index, list);
	elm_index_level_go(index, 0);
}


