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
//#include <ui-gadget-module.h>
//#include <shortcut.h>

#include "mc-select-view.h"
#include "mc-text.h"
#include "mp-media-info.h"
#include "mc-common.h"

//#define SHOW_SEARCHBAR

typedef struct{
	struct app_data *ad;

	Evas_Object *layout;
	Evas_Object *searchbar_layout;
	Evas_Object *entry;
	Evas_Object *list_object;
	Evas_Object *no_content;
	Evas_Object *genlist;

	mp_group_type_e type;

	char *filter_text; 	//free
	char *title;		//not free

	int count;
	mp_media_list_h media_list;
	mp_media_list_h defualt_playlist;

}sel_view_data_t;

static Elm_Genlist_Item_Class *itc;
static void _mc_list_update(sel_view_data_t *vd);

static void _mc_layout_del_cb(void *data, Evas *evas, Evas_Object *obj, void *event_info)
{
	sel_view_data_t *vd = data;
	MP_CHECK(vd);
	IF_FREE(vd->filter_text);
	if (vd->media_list)
		mp_media_info_group_list_destroy(vd->media_list);
	free(vd);
}

#ifdef SHOW_SEARCHBAR
static void _mc_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	sel_view_data_t *vd = data;
	MP_CHECK(vd);

	const char *text = elm_object_text_get(vd->entry);
	IF_FREE(vd->filter_text);
	vd->filter_text = g_strdup(text);

	if (elm_object_focus_get(vd->searchbar_layout)) {
		if (elm_entry_is_empty(obj))
			elm_object_signal_emit(vd->searchbar_layout, "elm,state,eraser,hide", "elm");
		else
			elm_object_signal_emit(vd->searchbar_layout, "elm,state,eraser,show", "elm");
	}

	_mc_list_update(vd);
}

static void _mc_focused_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	sel_view_data_t *vd = data;
	MP_CHECK(vd);

	if (!elm_entry_is_empty(obj))
		elm_object_signal_emit(vd->searchbar_layout, "elm,state,eraser,show", "elm");
	elm_object_signal_emit(vd->searchbar_layout, "elm,state,guidetext,hide", "elm");
	elm_object_signal_emit(vd->searchbar_layout, "cancel,in", "");
}

static void _mc_unfocused_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	sel_view_data_t *vd = data;
	MP_CHECK(vd);
	if (elm_entry_is_empty(obj))
		elm_object_signal_emit(vd->searchbar_layout, "elm,state,guidetext,show", "elm");
	elm_object_signal_emit(vd->searchbar_layout, "elm,state,eraser,hide", "elm");
	elm_object_signal_emit(vd->searchbar_layout, "cancel,out", "");
}

static void _mc_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	DEBUG_TRACE("");
	sel_view_data_t *vd = data;
	MP_CHECK(vd);
	elm_entry_entry_set(vd->entry, "");
}

static void _mc_bg_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	DEBUG_TRACE("");
	sel_view_data_t *vd = data;
	MP_CHECK(vd);
	elm_object_focus_set(vd->entry, EINA_TRUE);
}

static void _mc_cancel_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	sel_view_data_t *vd = data;
	MP_CHECK(vd);
	const char* text;
	evas_object_hide(obj);
	elm_object_signal_emit(vd->searchbar_layout, "cancel,out", "");
	text = elm_entry_entry_get(vd->entry);
	if (text != NULL && strlen(text) > 0)
		elm_entry_entry_set(vd->entry, NULL);
	elm_object_focus_set(vd->entry, EINA_FALSE);
	elm_object_focus_set(vd->list_object, EINA_TRUE);
}

static void _mc_searchsymbol_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	DEBUG_TRACE("");
	sel_view_data_t *vd = data;
	MP_CHECK(vd);
	elm_object_focus_set(vd->entry, EINA_TRUE);
}

static Evas_Object *
_mc_create_searchbar(sel_view_data_t *vd)
{
	Evas_Object *entry = NULL, *cancel_btn = NULL;
	Evas_Object *searchbar_layout = NULL;
	MP_CHECK_NULL(vd);

	searchbar_layout = elm_layout_add(vd->layout);
	elm_layout_theme_set(searchbar_layout, "layout", "searchbar", "cancel_button");

	entry = elm_entry_add(searchbar_layout);
	vd->entry = entry;
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_entry_single_line_set(entry, EINA_TRUE);
	evas_object_smart_callback_add(entry, "changed", _mc_changed_cb, vd);
	evas_object_smart_callback_add(entry, "focused", _mc_focused_cb, vd);
	evas_object_smart_callback_add(entry, "unfocused", _mc_unfocused_cb, vd);
	elm_object_part_content_set(searchbar_layout, "elm.swallow.content", entry);
	//elm_object_part_text_set(searchbar_layout, "elm.guidetext", GET_STR(MC_TEXT_SEARCH));
        mc_common_obj_domain_translatable_part_text_set(searchbar_layout, "elm.guidetext", MC_TEXT_SEARCH);
	elm_object_signal_callback_add(searchbar_layout, "elm,eraser,clicked", "elm", _mc_eraser_clicked_cb, vd);
	elm_object_signal_callback_add(searchbar_layout, "elm,bg,clicked", "elm", _mc_bg_clicked_cb, vd);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	evas_object_size_hint_weight_set(searchbar_layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(searchbar_layout, EVAS_HINT_FILL, EVAS_HINT_FILL);

	cancel_btn = elm_button_add(searchbar_layout);
	elm_object_part_content_set(searchbar_layout, "button_cancel", cancel_btn);
	elm_object_style_set(cancel_btn, "searchbar/default");
	elm_object_text_set(cancel_btn, GET_SYS_STR(MC_TEXT_CANCEL));

	evas_object_smart_callback_add(cancel_btn, "clicked", _mc_cancel_clicked_cb, vd);
	elm_object_signal_callback_add(searchbar_layout, "elm,action,click", "", _mc_searchsymbol_clicked_cb, vd);

	return searchbar_layout;
}
#endif

static Evas_Object *
_mc_create_genlist(sel_view_data_t *vd)
{
	Evas_Object *genlist = NULL;

	MP_CHECK_NULL(vd);

	genlist = elm_genlist_add(vd->layout);
	elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

	return genlist;
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	sel_view_data_t *vd = data;
	MP_CHECK(vd);

	elm_genlist_item_selected_set(event_info, EINA_FALSE);

	char *title = NULL;
	int playlist_id = 0;
	char *thumbnail_path = NULL;
	mp_media_info_h media = elm_object_item_data_get(event_info);
	MP_CHECK(media);
	mp_media_info_group_get_main_info(media, &title);
	mp_media_info_group_get_thumbnail_path(media, &thumbnail_path);

	if (vd->type == MP_GROUP_BY_PLAYLIST)
		mp_media_info_group_get_playlist_id(media, &playlist_id);


	//_add_to_home(vd, title, playlist_id, thumbnail_path);
}

static void _mc_append_items(sel_view_data_t *vd)
{
	startfunc;
	int i;
	mp_media_list_h media_list = NULL;

	if (vd->type == MP_GROUP_BY_PLAYLIST)
	{
		mp_media_info_h media = NULL;
		mp_media_info_group_list_create(&media_list, MP_GROUP_BY_SYS_PLAYLIST, NULL, NULL, 0, 0);
		i = 0;
		for (i = 0; i<3; i++) {
			media = mp_media_info_group_list_nth_item(media_list, i);
			if (!media)
				break;

			elm_genlist_item_append(vd->genlist, itc, media,
				NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, vd);
		}

		if (vd->defualt_playlist)
			mp_media_info_group_list_destroy(vd->defualt_playlist);
		vd->defualt_playlist = media_list;
	}

	mp_media_info_group_list_create(&media_list, vd->type, NULL, vd->filter_text, 0, vd->count);

	for (i = 0; i < vd->count; i++)
	{
		elm_genlist_item_append(vd->genlist, itc,
			mp_media_info_group_list_nth_item(media_list, i),
			NULL, ELM_GENLIST_ITEM_NONE, _gl_sel, vd);
	}

	if (vd->media_list)
		mp_media_info_group_list_destroy(vd->media_list);
	vd->media_list = media_list;
	endfunc;
}

static void
_mc_list_update(sel_view_data_t *vd)
{
	Evas_Object *list_object = NULL;
	int item_count = 0;

	MP_CHECK(vd);

	mp_media_info_group_list_count(vd->type, NULL, vd->filter_text, &item_count);
	DEBUG_TRACE("count: %d", item_count);

	vd->count = item_count;

	if (item_count == 0) {
		if (vd->genlist) {
			evas_object_del(vd->genlist);
			vd->genlist = NULL;
		}
		if (!vd->no_content) {
			NoContentType_e type = NO_CONTENT_SONG;
			if (vd->type == MP_GROUP_BY_PLAYLIST)
				type = NO_CONTENT_PLAYLIST;
			else if (vd->type == MP_GROUP_BY_ARTIST)
				type = NO_CONTENT_ARTIST;
			else if (vd->type == MP_GROUP_BY_ALBUM)
				type = NO_CONTENT_ALBUM;

			vd->no_content = mc_widget_no_content_add(vd->layout, type);
		}

		list_object = vd->no_content;
	}
	else
	{
		if (vd->no_content) {
			evas_object_del(vd->no_content);
			vd->no_content = NULL;
		}
		if (!vd->genlist) {
			vd->genlist = _mc_create_genlist(vd);
		}

		list_object = vd->genlist;
		elm_genlist_clear(vd->genlist);
		_mc_append_items(vd);
	}

	elm_object_part_content_set(vd->layout, "elm.swallow.content", list_object);
	vd->list_object = list_object;
}

static void
_mc_select_view_init(int type, sel_view_data_t *vd)
{
	MP_CHECK(vd);

	itc = elm_genlist_item_class_new();
	itc->func.content_get = mc_group_content_get;
	switch (type)
	{
	case MC_SHORTCUT_ALBUM:
		itc->item_style = "music/3text.1icon.2/";
		itc->func.text_get = mc_album_text_get;
		vd->type = MP_GROUP_BY_ALBUM;
		vd->title = MC_TEXT_SELECT_ALBUM;
		break;
	case MC_SHORTCUT_ARTIST:
		itc->item_style = "music/2text.1icon";
		itc->func.text_get = mc_artist_text_get;
		vd->type = MP_GROUP_BY_ARTIST;
		vd->title =  MC_TEXT_SELECT_ARTIST;
		break;
	case MC_SHORTCUT_PLAYLIST:
		itc->item_style = "music/2text.1icon";
		itc->func.text_get = mc_playlist_text_get;
		vd->type = MP_GROUP_BY_PLAYLIST;
		vd->title =  MC_TEXT_SELECT_PLAYLIST;
		break;
	default:
		ERROR_TRACE("Invalid vd->type: %d", type);
		break;
	}
}

Evas_Object *
mc_select_view_create(struct app_data *ad)
{
	startfunc;
	Evas_Object *layout = NULL;
	Evas_Object *btn = NULL;
	sel_view_data_t *vd = NULL;

	MP_CHECK_NULL(ad);

	layout = elm_layout_add(ad->navi_bar);
	MP_CHECK_NULL(layout);
	elm_layout_theme_set(layout, "layout", "application", "searchbar_base");
#ifdef SHOW_SEARCHBAR
	elm_object_signal_emit(layout, "elm,state,show,searchbar", "elm");
#endif
	vd = calloc(1, sizeof(sel_view_data_t));
	MP_CHECK_NULL(vd);
	vd->ad = ad;
	vd->layout = layout;
	evas_object_data_set(layout, "view_data", vd);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_DEL, _mc_layout_del_cb, vd);

	_mc_select_view_init(ad->select_type, vd);

#ifdef SHOW_SEARCHBAR
	Evas_Object *searchbar_layout = NULL;
	searchbar_layout = _mc_create_searchbar(vd);
	elm_object_part_content_set(layout, "searchbar", searchbar_layout);
	vd->searchbar_layout = searchbar_layout;
#endif

	_mc_list_update(vd);

	btn = elm_button_add(ad->navi_bar);
	elm_object_style_set(btn, "naviframe/back_btn/default");

	evas_object_smart_callback_add(btn, "clicked", mc_quit_select_cb, ad);

	Elm_Object_Item *navi_it = elm_naviframe_item_push(ad->navi_bar, GET_STR(vd->title), btn, NULL, layout, NULL);
	MP_CHECK_NULL(navi_it);

	elm_naviframe_item_pop_cb_set(navi_it, mc_quit_cb, ad);

	endfunc;
	return layout;
}

