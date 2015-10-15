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

#include "mc-search-view.h"
#include "mc-search-list.h"
#include "mc-search.h"
#include "mc-common.h"
#include "music-chooser.h"

#define MC_SEARCHBAR_W 400*elm_config_scale_get()

static void _mc_search_view_content_load(void *thiz);


/***************	functions for track list update 	*******************/
static Eina_Bool _mc_search_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	search_view_data_t *search = (search_view_data_t *) data;
	MP_CHECK_VAL(search, EINA_TRUE);

	IF_G_FREE(search->needle);
	mp_ecore_timer_del(search->search_timer);

	free(search);

	return EINA_TRUE;
}

int mc_search_view_update_options(void *thiz)
{
	startfunc;
	search_view_data_t *search = (search_view_data_t *)thiz;
	MP_CHECK_VAL(search, -1);

	/*add search bar into title part of navi frame*/
	elm_object_item_part_content_set(search->navi_it, "title", search->search_bar);
	elm_naviframe_item_pop_cb_set(search->navi_it, _mc_search_view_back_cb, search);

	return 0;
}

static Eina_Bool
_mc_search_view_update_list_timer_cb(void *data)
{
	eventfunc;
	search_view_data_t *search = (search_view_data_t *) data;
	MP_CHECK_FALSE(search);

	_mc_search_view_content_load(search);

	DEBUG_TRACE("view->needle: %s", search->needle);

	search->search_timer = NULL;

	return EINA_FALSE;
}


static void
_mc_search_view_keyword_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	search_view_data_t *search = (search_view_data_t *) data;
	MP_CHECK(search);
	char *search_str = NULL;

	search_str = mc_search_text_get(search->search_bar);

	EVENT_TRACE("search_str: %s", search_str);
	if (search_str)
	{
		if (search->needle)
			free(search->needle);
		search->needle = search_str;
		//signal = "hide.screen";
	}
	else
	{
		if (search->needle)
			free(search->needle);
		//signal = "show.screen";
	}

	mp_ecore_timer_del(search->search_timer);
	search->search_timer = ecore_timer_add(0.1, _mc_search_view_update_list_timer_cb, search);
}


static void
_mc_search_view_create_search_bar(void *thiz)
{
	startfunc;
	search_view_data_t *search = (search_view_data_t *)thiz;
	MP_CHECK(search);
	MP_CHECK(search->layout);

	search->search_bar = mc_search_create_new(search->layout,
			_mc_search_view_keyword_changed_cb, search, NULL, NULL,
			NULL, search, NULL, search);
	MP_CHECK(search->search_bar);
	evas_object_show(mc_search_entry_get(search->search_bar));
	endfunc;
}

static void _mc_search_view_content_load(void *thiz)
{
	startfunc;
	search_view_data_t *search = (search_view_data_t *)thiz;
	MP_CHECK(search);
	MP_CHECK(search->layout);

	Evas_Object *content = elm_object_part_content_unset(search->layout, "list-content");
	evas_object_del(content);

	UgMpSearchList_t *list = mc_search_list_create(search->layout, search->ad);
	MP_CHECK(list);
	mc_search_list_set_data(list, MC_SEARCH_LIST_FILTER_STR, search->needle, -1);

	mc_search_list_update(list);

	MP_CHECK(list->layout);
	elm_object_part_content_set(search->layout, "list-content", list->layout);
	free(list);
	endfunc;
}

static void _mc_search_layout_del_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	startfunc;
}

static void
_mc_search_init(Evas_Object *parent, void *thiz)
{
	startfunc;

        search_view_data_t *search = (search_view_data_t *)thiz;

        Evas_Object *layout = mc_common_load_edj(parent, MC_EDJ_FILE, "view_layout");

        search->layout = layout;

	/* search bar */
	_mc_search_view_create_search_bar(search);

        evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _mc_search_layout_del_cb, search);

	char *keyword = NULL;
	search->needle= g_strdup(keyword);
	mc_search_text_set(search->search_bar, keyword);

	_mc_search_view_content_load(search);

	return;
}

search_view_data_t *mc_search_view_create(Evas_Object *parent, struct app_data *ad)
{
	eventfunc;
        search_view_data_t *search = NULL;

        search = calloc(1, sizeof(search_view_data_t));
	MP_CHECK_NULL(search);

        search->ad = ad;

        _mc_search_init(ad->navi_bar, search);

        Evas_Object *searchbar_layout = NULL;
        elm_object_part_content_set(search->layout, "searchbar", searchbar_layout);
        search->searchbar_layout = searchbar_layout;

        Elm_Object_Item *navi_it = elm_naviframe_item_push(ad->navi_bar, NULL, NULL, NULL, search->layout, "empty/music");
        search->navi_it = navi_it;

        elm_naviframe_item_pop_cb_set(navi_it, mc_quit_cb, ad);

	return search;
}

void
mc_search_view_set_keyword(search_view_data_t *search, const char *keyword)
{
	MP_CHECK(search);
	IF_FREE(search->needle);
	search->needle= g_strdup(keyword);
	mc_search_text_set(search->search_bar, keyword);
}
