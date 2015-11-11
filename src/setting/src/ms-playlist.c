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

#include "ms-playlist.h"
#include "ms-key-ctrl.h"
#include "ms-util.h"

enum {
	MS_PLAYLIST_QUICK_LIST,
	MS_PLAYLIST_RECENTLEY_ADDED,
	MS_PLAYLIST_MOST_PLAYED,
	MS_PLAYLIST_RECENTLEY_PLAYED,

	MS_PLAYLIST_MAX,
};

static int playlist_state = 0;

static char *playlist_names[MS_PLAYLIST_MAX] = {
	"IDS_IV_BODY_FAVOURITE",
	"IDS_MH_MBODY_RECENTLY_ADDED_M_SONG",
	"IDS_MUSIC_BODY_MOST_PLAYED",
	"IDS_MUSIC_BODY_RECENTLY_PLAYED",
};

static Evas_Object *check_boxs[MS_PLAYLIST_MAX];

/*static void
_ms_playlist_set_cb(void *data, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE("");
	ms_key_set_playlist_val(playlist_state);
	return;
}*/

EXPORT_API int
ms_playlist_check_state_get_val(int *b_val)
{
	*b_val = playlist_state;

	return 0;
}

EXPORT_API int
ms_playlist_check_state_set_val(int b_val)
{
	playlist_state = b_val;

	return 0;
}

static void
_ms_playlist_view_check_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	int index = (int)evas_object_data_get(obj, "index");
	DEBUG_TRACE("index:%d", index);

	if (playlist_state & (1 << index)) {
		playlist_state &= ~(1 << index);
	} else {
		playlist_state |= (1 << index);
	}

	DEBUG_TRACE("set to 0x%x", playlist_state);
	//_ms_playlist_set_cb(data, obj, event_info);

	return;
}

static char *
_ms_playlist_view_gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_setting_genlist_item_data_t* item_data = data;

	char *txt = NULL;

	if (strcmp(part, "elm.text.main.left") == 0) {
		txt = GET_STR(playlist_names[item_data->index - 1]);
		if (txt) {
			return strdup(txt);
		}
	}
	return NULL;
}

static Evas_Object *
_ms_playlist_view_gl_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_setting_genlist_item_data_t* item_data = data;
	int param = item_data->index - 1;



	if (strcmp(part, "elm.icon.2") == 0) {
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);

		Evas_Object *check_box = elm_check_add(obj);
		elm_object_style_set(check_box, "default");
		evas_object_data_set(check_box, "index", (void *)param);

		evas_object_repeat_events_set(check_box, EINA_TRUE);
		evas_object_propagate_events_set(check_box, FALSE);
		elm_check_state_set(check_box, playlist_state & (1 << param));

		evas_object_smart_callback_add(check_box, "changed", _ms_playlist_view_check_changed_cb, NULL);
		evas_object_show(check_box);

		check_boxs[param] = check_box;

		elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
		elm_layout_content_set(content, "elm.swallow.content", check_box);
		return content;
	}



	return NULL;
}

static void
_ms_playlist_view_gl_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	mp_setting_genlist_item_data_t* item_data = data;
	int param = item_data->index - 1;
	DEBUG_TRACE("data: %d", param);

	Elm_Object_Item *item = (Elm_Object_Item *) event_info;

	elm_genlist_item_selected_set(item, EINA_FALSE);

	if (elm_check_state_get(check_boxs[param])) {
		elm_check_state_set(check_boxs[param], FALSE);
	} else {
		elm_check_state_set(check_boxs[param], TRUE);
	}

	evas_object_smart_callback_call(check_boxs[param], "changed", NULL);

}


static void
_ms_playlist_view_gl_sel_item_cb(void *data, Evas_Object * obj, void *event_info)
{
	int param = (int)data;
	DEBUG_TRACE("data: %d", param);
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);
}


static void
_mp_playlist_item_del_cb(void *data, Evas_Object *obj)
{
	mp_setting_genlist_item_data_t *item_data = data;
	MP_CHECK(item_data);
	IF_FREE(item_data->str);
	IF_FREE(item_data);
}

static Evas_Object *
_ms_playlist_create_genlist(MpSettingView_t *view, Evas_Object *parent)
{

	MP_CHECK_VAL(view, NULL);
	Evas_Object *genlist = mp_widget_genlist_create(parent);
	int index = 0;

	static Elm_Genlist_Item_Class itc;
	if (view->setting_type == MP_SETTING_VIEW_PLAYLISTS) {
		itc.item_style = "dialogue/1text.1icon/expandable2";
	} else if (view->setting_type == MP_SETTING_VIEW_REORDERS) {
		itc.item_style = "dialogue/1text";
		evas_object_smart_callback_add(genlist, "moved", mp_setting_items_reorder_cb, view);
	}


	char* str = NULL;
	ms_key_get_playlist_str(&str);
	DEBUG_TRACE("str is %s", str);


	int value = atoi(str);
	int playlist[MS_PLAYLIST_MAX] = {0};
	DEBUG_TRACE("value %d", value);
	int j = 0;
	for (j = MS_PLAYLIST_MAX - 1; j >= 0 ; j--) {
		playlist[j] = value % 10;
		value = value / 10;
		DEBUG_TRACE("index  %d  %d", j, playlist[j]);
	}

	itc.func.text_get = _ms_playlist_view_gl_label_get;
	itc.func.content_get = _ms_playlist_view_gl_icon_get;
	itc.func.del = _mp_playlist_item_del_cb;

	itc.version = ELM_GENGRID_ITEM_CLASS_VERSION;
	itc.refcount = 0;
	itc.delete_me = EINA_FALSE;

	for (index = 0; index < MS_PLAYLIST_MAX; index++) {
		int m = playlist[index];
		mp_setting_genlist_item_data_t* item_data = calloc(1, sizeof(mp_setting_genlist_item_data_t));
		if (item_data) {
			item_data->index = m;
			item_data->seq = index;
			if (view->setting_type == MP_SETTING_VIEW_PLAYLISTS) {
				item_data->it  = elm_genlist_item_append(genlist, &itc, (void *)item_data, NULL, ELM_GENLIST_ITEM_NONE,
				                 _ms_playlist_view_gl_sel_cb, (void *)item_data);
			} else if (view->setting_type == MP_SETTING_VIEW_REORDERS) {

				item_data->it  = elm_genlist_item_append(genlist, &itc, (void *)item_data, NULL, ELM_GENLIST_ITEM_NONE,
				                 _ms_playlist_view_gl_sel_item_cb, (void *)item_data);
			}
			elm_object_item_data_set(item_data->it, item_data);
		}
	}
	return genlist;
}

EXPORT_API Evas_Object *
_ms_playlist_append_pop_genlist(Evas_Object *genlist, Evas_Object *parent)
{

	MP_CHECK_VAL(genlist, NULL);
	mp_retvm_if(parent == NULL, NULL, "parent is NULL");
	ms_key_get_playlist_val(&playlist_state);

	int index = 0;

	static Elm_Genlist_Item_Class itc;
	itc.item_style = "1line";


	char* str = NULL;
	ms_key_get_playlist_str(&str);


	int value = atoi(str);
	int playlist[MS_PLAYLIST_MAX] = {0};
	DEBUG_TRACE("playlist display order value %d", value);

	int j = 0;
	for (j = MS_PLAYLIST_MAX - 1; j >= 0 ; j--) {
		playlist[j] = value % 10;
		value = value / 10;
		DEBUG_TRACE("playlist display order index  %d  %d", j, playlist[j]);
	}

	itc.func.text_get = _ms_playlist_view_gl_label_get;
	itc.func.content_get = _ms_playlist_view_gl_icon_get;
	itc.func.del = _mp_playlist_item_del_cb;

	itc.version = ELM_GENGRID_ITEM_CLASS_VERSION;
	itc.refcount = 0;
	itc.delete_me = EINA_FALSE;

	for (index = 0; index < MS_PLAYLIST_MAX; index++) {
		int m = playlist[index];
		mp_setting_genlist_item_data_t* item_data = calloc(1, sizeof(mp_setting_genlist_item_data_t));
		if (item_data) {
			item_data->index = m;
			item_data->seq = index;
			item_data->it  = elm_genlist_item_append(genlist, &itc, (void *)item_data, NULL, ELM_GENLIST_ITEM_NONE,
			                 _ms_playlist_view_gl_sel_cb, (void *)item_data);
			elm_object_item_data_set(item_data->it, item_data);
		}
	}
	return genlist;
}


EXPORT_API Evas_Object*
ms_playlist_list_create(MpSettingView_t *view, Evas_Object *parent)
{
	mp_retvm_if(parent == NULL, NULL, "parent is NULL");
	ms_key_get_playlist_val(&playlist_state);
	return _ms_playlist_create_genlist(view, parent);
}
