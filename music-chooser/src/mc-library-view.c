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

#include "mc-library-view.h"
#include "mc-track-list.h"
#include "mc-voice-clip-list.h"
#include "mc-group-list.h"
#include "mc-group-play-list.h"
#include "mc-common.h"
#include "mc-search-view.h"
#include <efl_extension.h>

typedef struct {
	struct app_data *ad;

	Evas_Object *toolbar;
	Evas_Object *list;

} lib_view_data_t;

enum {
	TAB_ALL,
#ifdef MC_ENABLE_PLAYLIST
	TAB_PLAYLIST,
#endif
	TAB_ALBUM,
	TAB_ARTIST,
	TAB_MAX,
};

static Elm_Object_Item *g_tab_item[TAB_MAX];
static Evas_Object *g_ly;
static Elm_Object_Item *g_navi_it;
extern bool detail_view;

Evas_Object *mc_tabbar;

static void _all_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	struct app_data *ad = data;
	MP_CHECK(ad);
	mc_common_obj_domain_text_translate(ad->navi_bar, mc_create_selectioninfo_text_with_count(0));
	Evas_Object *sub_view;
	if (detail_view) {
		elm_naviframe_item_pop(ad->navi_bar);
		ad->track_type = MP_TRACK_ALL;
		detail_view = false;
		mc_library_view_create(ad);
		return;
	}


	sub_view = elm_object_part_content_unset(g_ly, "list-content");
	evas_object_del(sub_view);

	MP_CHECK(g_ly);

	sub_view = mc_track_list_create(g_ly, ad);
	mc_track_list_set_data(sub_view, MP_TRACK_ALL, NULL, 0);
	mc_track_list_update(sub_view, g_navi_it, NULL);

	elm_object_part_content_set(g_ly, "list-content", sub_view);
	evas_object_show(sub_view);
	eext_object_event_callback_add(sub_view, EEXT_CALLBACK_BACK, mc_eext_quit_cb, ad);
	endfunc;
}
#ifdef MC_ENABLE_PLAYLIST
static void _playlist_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	struct app_data *ad = data;
	MP_CHECK(ad);
	mc_common_obj_domain_text_translate(ad->navi_bar, mc_create_selectioninfo_text_with_count(0));
	Evas_Object *sub_view;
	if (detail_view) {
		elm_naviframe_item_pop(ad->navi_bar);
		ad->track_type = MP_TRACK_BY_PLAYLIST;
		detail_view = false;
		mc_library_view_create(ad);
		return;
	}


	sub_view = elm_object_part_content_unset(g_ly, "list-content");
	evas_object_del(sub_view);

	sub_view = mc_group_list_create(g_ly, ad, g_navi_it);
	mc_group_list_set_data(sub_view, MP_GROUP_BY_PLAYLIST, NULL);
	mc_group_list_update(sub_view);

	elm_object_part_content_set(g_ly, "list-content", sub_view);
	evas_object_show(sub_view);
	eext_object_event_callback_add(ad->navi_bar, EEXT_CALLBACK_BACK, mc_eext_quit_cb, ad);
	endfunc;
}
#endif

#ifdef MC_ENABLE_TAB
static void _artist_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	struct app_data *ad = data;
	MP_CHECK(ad);
	mc_common_obj_domain_text_translate(ad->navi_bar, mc_create_selectioninfo_text_with_count(0));
	Evas_Object *sub_view;
	if (detail_view) {
		elm_naviframe_item_pop(ad->navi_bar);
		ad->track_type = MP_TRACK_BY_ARTIST;
		detail_view = false;
		mc_library_view_create(ad);
		return;
	}

	sub_view = elm_object_part_content_unset(g_ly, "list-content");
	evas_object_del(sub_view);

	sub_view = mc_group_list_create(g_ly, ad, g_navi_it);
	mc_group_list_set_data(sub_view, MP_GROUP_BY_ARTIST, NULL);
	mc_group_list_update(sub_view);

	elm_object_part_content_set(g_ly, "list-content", sub_view);
	evas_object_show(sub_view);
	eext_object_event_callback_add(sub_view, EEXT_CALLBACK_BACK, mc_eext_quit_cb, ad);
	endfunc;
}
#endif

static void _album_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	struct app_data *ad = data;
	MP_CHECK(ad);
	mc_common_obj_domain_text_translate(ad->navi_bar, mc_create_selectioninfo_text_with_count(0));
	Evas_Object *sub_view;
	if (detail_view) {
		elm_naviframe_item_pop(ad->navi_bar);
		ad->track_type = MP_TRACK_BY_ALBUM;
		detail_view = false;
		mc_library_view_create(ad);
		return;
	}

	sub_view = elm_object_part_content_unset(g_ly, "list-content");
	evas_object_del(sub_view);

	sub_view = mc_group_list_create(g_ly, ad, g_navi_it);
	mc_group_list_set_data(sub_view, MP_GROUP_BY_ALBUM, NULL);
	mc_group_list_update(sub_view);

	elm_object_part_content_set(g_ly, "list-content", sub_view);
	evas_object_show(sub_view);
	eext_object_event_callback_add(sub_view, EEXT_CALLBACK_BACK, mc_eext_quit_cb, ad);
	endfunc;
}

Evas_Object *_create_tabbar(Evas_Object *parent, struct app_data *ad)
{
	startfunc;
	int selected_tab;
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(ad);
	Evas_Object *obj = NULL;

	/* create toolbar */
	obj = elm_toolbar_add(parent);
	if (obj == NULL) {
		return NULL;
	}
	elm_toolbar_shrink_mode_set(obj, ELM_TOOLBAR_SHRINK_SCROLL);
	elm_toolbar_reorder_mode_set(obj, EINA_FALSE);
	elm_toolbar_transverse_expanded_set(obj, EINA_TRUE);
	elm_toolbar_select_mode_set(obj, ELM_OBJECT_SELECT_MODE_ALWAYS);

	char mc_edj_path[1024] = {0};
	char *path = app_get_resource_path();
	MP_CHECK_NULL(path);
	snprintf(mc_edj_path, 1024, "%s%s", path, MC_EDJ_FILE);
	free(path);
	elm_theme_extension_add(NULL, mc_edj_path);
	elm_object_style_set(obj, "scroll/tabbar");
//	elm_object_style_set(obj, "tabbar/item_with_title");

	if (ad->track_type == MP_TRACK_BY_ALBUM) {
		selected_tab = TAB_ALBUM;
	} else if (ad->track_type == MP_TRACK_BY_ARTIST) {
		selected_tab = TAB_ARTIST;
	} else {
		selected_tab = TAB_ALL;
	}

	g_tab_item[TAB_ALL] = mc_common_toolbar_item_append(obj, NULL, MC_TEXT_ALL, _all_cb, ad);
#ifdef MC_ENABLE_PLAYLIST
	g_tab_item[TAB_PLAYLIST] = mc_common_toolbar_item_append(obj, NULL, MC_TEXT_PLAYLISTS, _playlist_cb, ad);
#endif
	g_tab_item[TAB_ALBUM] = mc_common_toolbar_item_append(obj, NULL, MC_TEXT_ALBUMS, _album_cb, ad);
#ifdef MC_ENABLE_TAB
	g_tab_item[TAB_ARTIST] = mc_common_toolbar_item_append(obj, NULL, MC_TEXT_ARTISTS, _artist_cb, ad);
#endif

	elm_toolbar_item_selected_set(g_tab_item[selected_tab], EINA_TRUE);

	evas_object_show(obj);

	endfunc;
	return obj;
}

void mc_create_search_view_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	struct app_data *ad = data;
	MP_CHECK(ad);

	search_view_data_t *search = mc_search_view_create(g_ly, ad);
	MP_CHECK(search);
	mc_search_view_update_options(search);
}

static void mc_common_item_domain_text_translate(Elm_Object_Item* item, const char *part, const char *label)
{
	elm_object_item_domain_translatable_part_text_set(item, part, DOMAIN_NAME, (const char *)label);
}

void
mc_library_view_create(struct app_data *ad)
{
	startfunc;

	if (ad->select_type == MC_SELECT_VOICE_CLIP) {
		g_ly = mc_voice_clip_list_create(ad->navi_bar, ad);
		g_navi_it = elm_naviframe_item_push(ad->navi_bar, NULL, NULL, NULL, g_ly, NULL);

		mc_voice_clip_list_update(g_ly);
		evas_object_show(g_ly);
	} else if (ad->select_type == MC_SELECT_GROUP_PLAY) {
		g_ly = mc_group_play_list_create(ad->navi_bar, ad);
		g_navi_it = elm_naviframe_item_push(ad->navi_bar, NULL, NULL, NULL, g_ly, NULL);

		mc_group_play_list_update(g_ly, g_navi_it);
		evas_object_show(g_ly);
	} else {
		//g_ly = elm_layout_add(ad->navi_bar);
		//elm_layout_theme_set(g_ly, "layout", "application", "default");
		char mc_edj_path[1024] = {0};
		char *path = app_get_resource_path();
		if (path == NULL) {
			return;
		}
		snprintf(mc_edj_path, 1024, "%s%s", path, MC_EDJ_FILE);
		free(path);
		g_ly = mc_common_load_edj(ad->navi_bar, mc_edj_path, "view_layout_tabbar");
		g_navi_it = elm_naviframe_item_push(ad->navi_bar, NULL, NULL, NULL, g_ly, NULL);
#if  0
		Evas_Object *search_btn = NULL;
		search_btn = mc_widget_create_title_icon_btn(g_ly, IMAGE_EDJ_NAME, MP_ICON_SEARCH,
		             (Evas_Smart_Cb)mc_create_search_view_cb,
		             ad);
		elm_object_item_part_content_set(g_navi_it, "title_right_btn", search_btn);
#endif

		mc_tabbar = _create_tabbar(ad->navi_bar, ad);
		elm_object_part_content_set(g_ly, "tabbar", mc_tabbar);

#ifdef MC_AUTO_RECOMMENDED
		if (ad->auto_recommended_show) {
			elm_object_signal_emit(g_ly, "show.recommended", "*");
			Evas_Object *recommended_area = mc_common_load_edj(ad->navi_bar, mc_edj_path, "recommended_area");
			elm_object_part_content_set(g_ly, "recommended", recommended_area);
			mc_common_obj_domain_translatable_part_text_set(recommended_area, "title_text", MC_TEXT_SET_AS_AUTO_RECOMMEND);
			mc_common_obj_domain_translatable_part_text_set(recommended_area, "description_text", MC_TEXT_SET_AS_RECOMMENDED_TXT);

			/*add check box*/
			Evas_Object *check = elm_check_add(recommended_area);
			elm_check_state_set(check, ad->auto_recommended_on);
			elm_object_part_content_set(recommended_area, "check_box", check);
			evas_object_smart_callback_add(check, "changed", mc_auto_recommended_check_cb, ad);

			evas_object_show(recommended_area);
		}
#endif
	}

	mc_common_item_domain_text_translate(g_navi_it, "elm.text.title", MC_TEXT_SELECT);
	elm_naviframe_item_pop_cb_set(g_navi_it, mc_quit_cb, ad);
	endfunc;
}


