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

#include "mc-group-play-list.h"
#include "mp-media-info.h"
#include "mc-common.h"

typedef struct{
	struct app_data *ad;

	Evas_Object *no_content;
	Evas_Object *genlist;
	Elm_Object_Item *btn_done;

	Elm_Genlist_Item_Class itc;

	mp_media_list_h track_list;

	Ecore_Timer *destroy_timer;
	Elm_Object_Item *win_navi_it;
}group_play_list_data_t;

typedef struct
{
	//Elm_Object_Item *it;	// Genlist Item pointer
	Eina_Bool checked;	// Check status
	mp_media_info_h media;
} list_item_data_t;

#define GET_LIST_DATA(obj)	evas_object_data_get(obj, "list_data")

static Evas_Object *
_mc_create_genlist(Evas_Object *parent)
{
	Evas_Object *genlist = NULL;
	MP_CHECK_NULL(parent);

	genlist = elm_genlist_add(parent);
	elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

	return genlist;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *content = NULL;
	char *thumbpath = NULL;

	list_item_data_t *it_data = data;
	MP_CHECK_NULL(it_data);

	mp_media_info_h media = it_data->media;

	if (!strcmp(part, "elm.icon"))
	{
		content = elm_bg_add(obj);
		elm_bg_load_size_set(content, ICON_SIZE, ICON_SIZE);

		mp_media_info_get_thumbnail_path(media, &thumbpath);

		if (mc_check_image_valid(	evas_object_evas_get(obj), thumbpath))
			elm_bg_file_set(content, thumbpath, NULL);
		else
			elm_bg_file_set(content, DEFAULT_THUMBNAIL, NULL);
	}

	if (elm_genlist_decorate_mode_get(obj) )
	{			// if edit mode
		if (!strcmp(part, "elm.edit.icon.1"))
		{		// swallow checkbox or radio button
			content = elm_check_add(obj);
			elm_check_state_pointer_set(content, &it_data->checked);
			elm_object_style_set(content, "default/genlist");
			evas_object_repeat_events_set(content, EINA_TRUE);
			evas_object_propagate_events_set(content, FALSE);
 			return content;
		}
	}

	return content;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *text = NULL;

	list_item_data_t *it_data = data;
	MP_CHECK_NULL(it_data);

	mp_media_info_h media = it_data->media;

	int ret = 0;
	if (strcmp(part, "elm.text.1") == 0) {
		ret = mp_media_info_get_title(media, &text);
		MP_CHECK_NULL(ret==0);
		return g_strdup(text);
	} else if (strcmp(part, "elm.text.2") == 0) {
		ret = mp_media_info_get_artist(media, &text);
		MP_CHECK_NULL(ret==0);
		return g_strdup(text);
	}
	return NULL;
}

static void _gl_del(void *data, Evas_Object *obj)
{
	list_item_data_t *it_data = data;
	IF_FREE(it_data);
}

static Eina_Bool
_destroy_timer_cb(void *data)
{
	group_play_list_data_t *ld  = data;
	MP_CHECK_FALSE(ld);
	ld->destroy_timer = NULL;
	elm_exit();
	return EINA_FALSE;
}

static unsigned int
_get_select_count(void *data)//(Evas_Object *genlist)
{
	startfunc;
	unsigned int count = 0;

	group_play_list_data_t *ld  = data;
	Elm_Object_Item *item;
	MP_CHECK_VAL(ld, 0);
	MP_CHECK_VAL(ld->genlist, 0);

	item = elm_genlist_first_item_get(ld->genlist);
	while (item)
	{
		list_item_data_t *it_data = elm_object_item_data_get(item);
		item = elm_genlist_item_next_get(item);
		if (it_data && it_data->checked)
		{
			count++;
		}
	}
	return count;

	endfunc;
}

static void _gl_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	group_play_list_data_t *ld  = data;
	char *path = NULL;

	elm_genlist_item_selected_set(event_info, EINA_FALSE);
	MP_CHECK(ld);
	MP_CHECK(!ld->destroy_timer);

	list_item_data_t *it_data = elm_object_item_data_get(event_info);
	MP_CHECK(it_data);

	if (elm_genlist_decorate_mode_get(obj) )
	{
		it_data->checked = !it_data->checked;
 		elm_genlist_item_fields_update(event_info, "elm.edit.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);

		if (ld->btn_done)
		{
			if (_get_select_count(ld))
				elm_object_item_disabled_set(ld->btn_done, false);
			else
				elm_object_item_disabled_set(ld->btn_done, true);
		}
		DEBUG_TRACE("_get_select_count = %d", _get_select_count(ld));
 		return;
	}

	mp_media_info_h media = it_data->media;
	MP_CHECK(media);
	mp_media_info_get_file_path(media, &path);
	DEBUG_TRACE("path: %s", path);

	app_control_h service = NULL;
	app_control_create(&service);
	app_control_add_extra_data(service, APP_CONTROL_DATA_PATH, path);
	app_control_add_extra_data(service, APP_CONTROL_DATA_SELECTED, path);

	app_control_reply_to_launch_request(service,ld->ad->service, APP_CONTROL_RESULT_SUCCEEDED);

	ld->destroy_timer = ecore_timer_add(0.1, _destroy_timer_cb, ld);

	app_control_destroy(service);
}

static void
_layout_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	group_play_list_data_t *ld  = data;
	MP_CHECK(ld);

	Evas_Object * right_btn = elm_object_item_part_content_get(ld->win_navi_it, "title_right_btn");
	evas_object_hide(right_btn);

	free(ld);
}

static void
_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	char *fmt = ";%s";
	GString *path = NULL;
	group_play_list_data_t *ld  = data;
	Elm_Object_Item *item;
	MP_CHECK(ld);

	item = elm_genlist_first_item_get(ld->genlist);
	while (item)
	{
		list_item_data_t *it_data = elm_object_item_data_get(item);
		item = elm_genlist_item_next_get(item);
		if (it_data && it_data->checked)
		{
			char *tmp = NULL;
			mp_media_info_h media = it_data->media;
			MP_CHECK(media);
			mp_media_info_get_file_path(media, &tmp);
			DEBUG_TRACE("path: %s", tmp);
			if (path == NULL)
				path = g_string_new(tmp);
			else
				g_string_append_printf(path, fmt, tmp);
		}
	}

	MP_CHECK(path);

	app_control_h service = NULL;
	app_control_create(&service);
	app_control_add_extra_data(service, APP_CONTROL_DATA_PATH, path->str);
	app_control_add_extra_data(service, APP_CONTROL_DATA_SELECTED, path->str);

	app_control_reply_to_launch_request(service,ld->ad->service, APP_CONTROL_RESULT_SUCCEEDED);

	ld->destroy_timer = ecore_timer_add(0.1, _destroy_timer_cb, ld);

	app_control_destroy(service);

	g_string_free(path, TRUE);

}

static void _mc_track_list_select_all_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	group_play_list_data_t *ld  = data;
	Elm_Object_Item *item;
	MP_CHECK(ld);

	Eina_Bool all_selected = EINA_FALSE;

	item = elm_genlist_first_item_get(ld->genlist);

	while (item)
	{
		list_item_data_t *it_data = elm_object_item_data_get(item);

		if (!it_data->checked)
		{
			all_selected = EINA_TRUE;
			break;
		}
		item = elm_genlist_item_next_get(item);
	}

	item = elm_genlist_first_item_get(ld->genlist);
	while (item)
	{
		list_item_data_t *it_data = elm_object_item_data_get(item);

		it_data->checked = all_selected;
		elm_genlist_item_fields_update(item, "elm.edit.icon.1", ELM_GENLIST_ITEM_FIELD_CONTENT);
		item = elm_genlist_item_next_get(item);
	}
	if (ld->btn_done)
	{
		if (_get_select_count(ld) && all_selected)
			elm_object_item_disabled_set(ld->btn_done, false);
		else
			elm_object_item_disabled_set(ld->btn_done, true);
	}
}


Evas_Object *mc_group_play_list_create(Evas_Object *parent, struct app_data *ad)
{
	startfunc;
	Evas_Object *layout ;
	group_play_list_data_t *ld = NULL;

	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(ad);

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "application", "default");
	MP_CHECK_NULL(layout);

	ld = calloc(1, sizeof(group_play_list_data_t));
	MP_CHECK_NULL(ld);

	ld->ad = ad;

	evas_object_data_set(layout, "list_data", ld);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, _layout_del_cb, ld);

	ld->itc.func.content_get = _gl_content_get;
	ld->itc.item_style = "2text.1icon.4";
	ld->itc.func.text_get = _gl_text_get;
	ld->itc.func.del = _gl_del;
	ld->itc.decorate_all_item_style = "edit_default";

	return layout;
}

int mc_group_play_list_update(Evas_Object *list, Elm_Object_Item *navi_it)
{
	startfunc;
	Evas_Object *content;

	int count = 0;
	group_play_list_data_t *ld  = GET_LIST_DATA(list);
	MP_CHECK_VAL(ld, -1);

	struct app_data *ad  = ld->ad;
	MP_CHECK_VAL(ad, -1);

	ld->win_navi_it = navi_it;

	DEBUG_TRACE("add done button");
	Evas_Object *toolbar = mc_widget_create_naviframe_toolbar(ld->win_navi_it);
		ld->btn_done = mc_widget_create_toolbar_item_btn(toolbar,
		"naviframe/toolbar/default", GET_SYS_STR("IDS_COM_POP_DONE"), _done_cb, ld);

	elm_object_item_disabled_set(ld->btn_done, true);

	Evas_Object *ic;
	Evas_Object *select_btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(select_btn, "naviframe/title_icon");
	ic = elm_icon_add(ad->navi_bar);
	elm_image_file_set(ic, IMAGE_EDJ_NAME, "00_icon_edit.png");
	evas_object_size_hint_aspect_set(ic, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
	elm_image_resizable_set(ic, EINA_TRUE, EINA_TRUE);
	elm_object_part_content_set(select_btn, "icon", ic);
	evas_object_smart_callback_add(select_btn, "clicked", _mc_track_list_select_all_cb, ld);

	evas_object_show(select_btn);
	elm_object_item_part_content_set(ld->win_navi_it, "title_right_btn", select_btn);

	if (ld->track_list)
	{
		mp_media_info_list_destroy(ld->track_list);
		ld->track_list = NULL;
	}

	content = elm_layout_content_get(list, "elm.swallow.content");
	evas_object_del(content);

	mp_media_info_list_count(MP_TRACK_BY_GROUP_PLAY, NULL, NULL, NULL, 0, &count);
	if (count)
	{
		ld->genlist = content = _mc_create_genlist(list);
		mp_media_info_list_create(&ld->track_list, MP_TRACK_BY_GROUP_PLAY, NULL, NULL, NULL, 0, 0, count);
		int i = 0;
		for (i = 0; i < count; i++)
		{
			mp_media_info_h media = mp_media_info_list_nth_item(ld->track_list, i);
			list_item_data_t *data = calloc(1, sizeof(list_item_data_t));
			if (data) {
				data->media = media;

				elm_genlist_item_append(content, &ld->itc, data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_cb, ld);
			}
		}

		elm_genlist_decorate_mode_set(content, true);
	}
	else
		content = mc_widget_no_content_add(list, NO_CONTENT_SONG);

	elm_layout_content_set(list, "elm.swallow.content", content);

	return 0;
}



