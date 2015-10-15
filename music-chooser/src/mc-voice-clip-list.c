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

#include "mc-track-list.h"
#include "mp-media-info.h"
#include "mc-common.h"

typedef struct{
	struct app_data *ad;

	Evas_Object *no_content;
	Evas_Object *genlist;

	Elm_Genlist_Item_Class itc;
	mp_media_list_h voice_clip_list;

	Ecore_Timer *destroy_timer;
}voice_clip_list_data_t;

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

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *text = NULL;

	int ret = 0;
	if (strcmp(part, "elm.text") == 0) {
		ret = mp_media_info_get_title(data, &text);
		MP_CHECK_NULL(ret==0);
		return g_strdup(text);
	}
	return NULL;
}

static Eina_Bool
_destory_timer_cb(void *data)
{
	voice_clip_list_data_t *ld  = data;
	MP_CHECK_FALSE(ld);
	ld->destroy_timer = NULL;
	elm_exit();
	return EINA_FALSE;
}

static void _gl_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	voice_clip_list_data_t *ld  = data;
	char *path = NULL;

	elm_genlist_item_selected_set(event_info, EINA_FALSE);
	MP_CHECK(ld);
	MP_CHECK(!ld->destroy_timer);

	mp_media_info_h media = elm_object_item_data_get(event_info);
	MP_CHECK(media);
	mp_media_info_get_file_path(media, &path);
	DEBUG_TRACE("path: %s", path);

	app_control_h service = NULL;
	app_control_create(&service);
	app_control_add_extra_data(service, APP_CONTROL_DATA_PATH, path);

	app_control_reply_to_launch_request(service,ld->ad->service, APP_CONTROL_RESULT_SUCCEEDED);

	ld->destroy_timer = ecore_timer_add(0.1, _destory_timer_cb, ld);

	app_control_destroy(service);
}

static void
_layout_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	voice_clip_list_data_t *ld  = data;
	MP_CHECK(ld);

	free(ld);
}

Evas_Object *mc_voice_clip_list_create(Evas_Object *parent, struct app_data *ad)
{
	startfunc;
	Evas_Object *layout ;
	voice_clip_list_data_t *ld = NULL;

	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(ad);

	layout = elm_layout_add(parent);
	elm_layout_theme_set(layout, "layout", "application", "default");
	MP_CHECK_NULL(layout);

	ld = calloc(1, sizeof(voice_clip_list_data_t));
	MP_CHECK_NULL(ld);

	ld->ad = ad;

	evas_object_data_set(layout, "list_data", ld);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, _layout_del_cb, ld);

	ld->itc.func.content_get = NULL;
	ld->itc.item_style = "1text";
	ld->itc.func.text_get = _gl_text_get;

	return layout;
}

int mc_voice_clip_list_update(Evas_Object *list)
{
	startfunc;
	Evas_Object *content;

	int count = 0;
	voice_clip_list_data_t *ld  = GET_LIST_DATA(list);
	MP_CHECK_VAL(ld, -1);

	if (ld->voice_clip_list)
	{
		mp_media_info_list_destroy(ld->voice_clip_list);
		ld->voice_clip_list = NULL;
	}

	content = elm_layout_content_get(list, "elm.swallow.content");
	evas_object_del(content);

	mp_media_info_list_count(MP_TRACK_BY_VOICE_CLIP, NULL, NULL, NULL, 0, &count);
	if (count)
	{
		content = _mc_create_genlist(list);
		mp_media_info_list_create(&ld->voice_clip_list, MP_TRACK_BY_VOICE_CLIP, NULL, NULL, NULL, 0, 0, count);
		int i = 0;
		for (i=0; i<count; i++)
		{
			mp_media_info_h media =  mp_media_info_list_nth_item(ld->voice_clip_list, i);
			elm_genlist_item_append(content, &ld->itc, media, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_cb, ld);
		}
	}
	else
		content = mc_widget_no_content_add(list, NO_CONTENT_SONG);

	elm_layout_content_set(list, "elm.swallow.content", content);

	return 0;
}


