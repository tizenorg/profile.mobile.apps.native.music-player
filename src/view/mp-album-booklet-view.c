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

#include "mp-album-booklet-view.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-util.h"
#include "mp-smart-event-box.h"
#include "mp-scroll-page.h"

#define MP_MAX_TEXT_PRE_FORMAT_LEN 256
#define MP_MAX_ARTIST_NAME_WIDTH 320
#define MP_LABEL_SLIDE_DURATION 5
#define MP_ALBUM_INDEX_ICON_SIZE (202 * elm_config_scale_get())
#define MP_ALBUM_BOOKLET_THUMB_ICON_SIZE 312
#define HD_SCREEN_WIDTH 720.0
#define HD_INFO_RIGHT_WIDTH 880.0


typedef struct {
	char *header;
	char *detail;
} mp_media_info_t;

typedef struct {
	char *name;
	char *year;
	char *thumb;
} mp_album_info_t;

static void
_mp_album_booklet_view_destory_cb(void *thiz)
{
	eventfunc;
	MpAlbumBookletView_t *view = thiz;
	MP_CHECK(view);
	mp_view_fini((MpView_t *)view);

	/* TODO: release resource..*/
	IF_G_FREE(view->name);
	IF_G_FREE(view->artist);
	IF_G_FREE(view->thumbnail);

	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_evas_object_del(view->track_genlist);
	mp_elm_genlist_item_class_free(view->info_itc);
	mp_elm_genlist_item_class_free(view->title_itc);
	mp_elm_genlist_item_class_free(view->track_itc);

	free(view);
}

int _mp_album_booklet_view_update(void *thiz)
{
	startfunc;
	MpAlbumBookletView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_update_options((MpView_t *)view);
	return 0;
}

static void _mp_album_booklet_view_search_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpAlbumBookletView_t *view =  data;
	MP_CHECK(view);
	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_common_search_by(view->name);
}

static void _mp_album_booklet_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	/* Todo: supports multi-language */

	/*Search */
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_SEARCH, MP_PLAYER_MORE_BTN_SEARCH, _mp_album_booklet_view_search_cb, view);

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, mp_common_ctxpopup_setting_cb, view);
#ifndef MP_FEATURE_NO_END
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
	                             STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif
	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);
}


/***************	functions for track list update 	*******************/
Eina_Bool _mp_album_booklet_view_tracklist_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	DEBUG_TRACE("");
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	mp_view_mgr_pop_view(view_mgr, false);

	return EINA_TRUE;
}

static int _mp_album_booklet_view_update_options(void *thiz)
{
	startfunc;
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	mp_view_clear_options((MpView_t *)view);
	Evas_Object *btn = NULL;

	btn = mp_widget_create_toolbar_btn(view->layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_album_booklet_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);
	/*view->toolbar_options[MP_OPTION_MORE] = btn;*/
	elm_naviframe_item_title_enabled_set(view->navi_it, EINA_FALSE, EINA_FALSE);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_album_booklet_view_tracklist_back_cb, view);

	endfunc;
	return 0;
}

/***page 1***/
static char *_mp_album_booklet_view_get_year(void *thiz)
{
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)thiz;
	MP_CHECK_NULL(view);

	int ret = 0;

	mp_media_list_h svc_handle = NULL;
	ret = mp_media_info_list_create(&svc_handle, MP_TRACK_BY_ALBUM, view->name, NULL, NULL, -1, 0, 1);
	MP_CHECK_NULL(ret == 0);

	mp_media_info_h item = NULL;
	char *year = NULL;
	char *get_year = NULL;
	item = mp_media_info_list_nth_item(svc_handle, 0);
	if (item) {
		ret = mp_media_info_get_year(item, &year);
	}
	DEBUG_TRACE("year=%s", year);
	get_year = year ? g_strdup(year) : g_strdup("");

	mp_media_info_list_destroy(svc_handle);

	return get_year;
}

static char*
_mp_album_booklet_view_album_index_text_get(void *data, Evas_Object * obj, const char *part)
{
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)evas_object_data_get(obj, "view");
	MP_CHECK_NULL(view);

	char *text = NULL;
	if (!g_strcmp0(part, "elm.text.1")) {
		if (view->info_data) {
			text = view->info_data->album_title;
		}
	} else if (!g_strcmp0(part, "elm.text.2")) {
		return _mp_album_booklet_view_get_year(view);
	} else if (!g_strcmp0(part, "elm.text.3")) {
		text = view->artist;
	}

	return g_strdup(text);
}

static Evas_Object *
_mp_album_booklet_view_album_index_content_get(void *data, Evas_Object *obj, const char *part)
{
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)evas_object_data_get(obj, "view");
	MP_CHECK_NULL(view);

	Evas_Object *icon = mp_util_create_thumb_icon(obj, view->thumbnail, MP_ALBUM_INDEX_ICON_SIZE, MP_ALBUM_INDEX_ICON_SIZE);

	return icon;
}

static char*
_mp_album_booklet_view_title_text_get(void *data, Evas_Object * obj, const char *part)
{
	int type = (int)data;
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)evas_object_data_get(obj, "view");
	MP_CHECK_NULL(view);

	const char *title = NULL;
	if (type == ALBUM_BOOKLET_VIEW_TRACK_LIST) {
		title = STR_MP_TRACK_LIST;
	} else if (type == ALBUM_BOOKLET_VIEW_REVIEW) {
		title = STR_MP_REVIEW;
	}

	char *text = GET_STR(title);
	return g_strdup(text);
}

static Evas_Object *
_mp_album_booklet_view_title_content_get(void *data, Evas_Object *obj, const char *part)
{
	int type = (int)data;
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)evas_object_data_get(obj, "view");
	MP_CHECK_NULL(view);

	Evas_Object *icon = mp_scroll_page_index_icon_add(obj, view->page_count, view->page_index[type]);
	return icon;
}

static char*
_mp_album_booklet_view_track_gl_text_get(void *data, Evas_Object * obj, const char *part)
{
	mp_media_info_t *info = data;
	MP_CHECK_NULL(info);

	if (!g_strcmp0(part, "elm.text.2")) {
		return g_strdup(info->header);
	} else if (!g_strcmp0(part, "elm.text.1")) {
		return g_strdup(info->detail);
	}
	return NULL;
}

static void
_mp_album_booklet_vew_list_item_del(void *data, Evas_Object *obj)
{
	mp_media_info_t *info = data;
	MP_CHECK(info);

	IF_FREE(info->header);
	IF_FREE(info->detail);
	free(info);
}

static void
_mp_album_booklet_view_append_track_item(Evas_Object *genlist, char *header, char *detail)
{
	/*startfunc;*/
	MpAlbumBookletView_t *view = NULL;
	mp_media_info_t *info = NULL;

	MP_CHECK(genlist);

	view = (MpAlbumBookletView_t *)evas_object_data_get(genlist, "view");
	MP_CHECK(view);
	info = calloc(1, sizeof(mp_media_info_t));
	MP_CHECK(info);
	info->header = g_strdup(header);
	info->detail = g_strdup(detail);

	Elm_Object_Item *item;

	item = elm_genlist_item_append(view->track_genlist, view->track_itc,
	                               info,
	                               NULL,
	                               ELM_GENLIST_ITEM_NONE,
	                               NULL, NULL);
	if (item) {
		elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
	}
}

static Evas_Object *
_mp_album_booklet_view_create_genlist(Evas_Object *parent)
{
	startfunc;
	Evas_Object *genlist = NULL;
	genlist = mp_widget_genlist_create(parent);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_homogeneous_set(genlist, EINA_FALSE);

	endfunc;
	return genlist;
}

static Evas_Object *
_mp_album_booklet_view_create_track_list(Evas_Object *parent, MpAlbumBookletView_t *view)
{
	startfunc;
	MP_CHECK_NULL(view);

	Evas_Object *genlist = NULL;
	genlist = _mp_album_booklet_view_create_genlist(parent);
	MP_CHECK_NULL(genlist);
	view->track_genlist = genlist;
	/*elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);*/
	evas_object_data_set(genlist, "view", view);

	/* album info(index) */
	Elm_Object_Item *item = elm_genlist_item_append(genlist, view->info_itc, (void *)ALBUM_BOOKLET_VIEW_TRACK_LIST, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	/* title */
	item = elm_genlist_item_append(genlist, view->title_itc, (void *)ALBUM_BOOKLET_VIEW_TRACK_LIST, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

	/*data part*/
	mp_info_data_t *info_data = view->info_data;
	int i = 0;
	for (i = 0; i < info_data->track_list_count; i++) {
		if (info_data->track_list[i].track_title) {
			char duration_format[10] = { 0, };
			int dur_sec = info_data->track_list[i].duration;
			int sec = dur_sec % 60;
			int min = dur_sec / 60;
			snprintf(duration_format, sizeof(duration_format), "%02u:%02u", min, sec);
			_mp_album_booklet_view_append_track_item(genlist, duration_format, info_data->track_list[i].track_title);
		} else {
			DEBUG_TRACE("credits have error");
		}
	}

	return genlist;
}
/***page 2***/

static char *
_mp_album_booklet_album_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_album_info_t *item = (mp_album_info_t *) data;
	MP_CHECK_NULL(item);

	if (!strcmp(part, "elm.text")) {

		return g_strdup(item->name);

	} else if (!strcmp(part, "elm.text.2")) {
		return g_strdup(item->year);
	}

	DEBUG_TRACE("Unusing part: %s", part);
	return NULL;
}



Evas_Object *
_mp_album_booklet_album_icon_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *icon = NULL;

	mp_album_info_t *item = (mp_album_info_t *) data;
	MP_CHECK_NULL(item);

	if (!strcmp(part, "elm.icon") || !strcmp(part, "elm.swallow.icon")) {

		icon = mp_util_create_thumb_icon(obj, item->thumb, MP_ALBUM_BOOKLET_THUMB_ICON_SIZE, MP_ALBUM_BOOKLET_THUMB_ICON_SIZE);
	}
	return icon;
}

static void
_mp_album_booklet_album_item_del_cb(void *data, Evas_Object *obj)
{
	mp_album_info_t *info = data;
	MP_CHECK(info);

	IF_FREE(info->name);
	IF_FREE(info->thumb);
	IF_FREE(info->year);
	free(info);
}

static void _mp_album_booklet_append_album_item(Evas_Object *gengrid, char *name, char *year, char *thumb)
{
	/*startfunc;*/
	MpAlbumBookletView_t *view = NULL;
	mp_album_info_t *info = NULL;

	MP_CHECK(gengrid);

	view = (MpAlbumBookletView_t *)evas_object_data_get(gengrid, "view");
	MP_CHECK(view);
	info = calloc(1, sizeof(mp_album_info_t));
	MP_CHECK(info);
	info->name = g_strdup(name);
	info->year = g_strdup(year);
	info->thumb = g_strdup(thumb);

	elm_gengrid_item_append(view->album_gengrid, view->album_itc,
	                        info,
	                        NULL,
	                        NULL);
}


static Evas_Object *
_mp_album_booklet_view_gengrid_create(Evas_Object *parent, MpAlbumBookletView_t *view)
{
	startfunc;
	MP_CHECK_NULL(view);
	/*create new gengrid*/
	mp_evas_object_del(view->album_gengrid);
	view->album_gengrid = NULL;

	mp_info_data_t *info_data = view->info_data;
	if (info_data->discography_count > 0) {
		view->album_gengrid = elm_gengrid_add(parent);
		evas_object_data_set(view->album_gengrid, "view", view);
		evas_object_size_hint_weight_set(view->album_gengrid, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(view->album_gengrid, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_freeze_events_set(view->album_gengrid, EINA_TRUE);
		evas_object_show(view->album_gengrid);

		if (!view->album_itc) {
			view->album_itc = elm_gengrid_item_class_new();
			view->album_itc->item_style = "music/album_grid";
			view->album_itc->func.text_get = _mp_album_booklet_album_label_get;
			view->album_itc->func.content_get = _mp_album_booklet_album_icon_get;
			view->album_itc->func.del = _mp_album_booklet_album_item_del_cb;
		}

		double scale = elm_config_scale_get();
		int w = (int)((MP_ALBUM_BOOKLET_THUMB_ICON_SIZE + 33) * scale);
		int h = (int)((MP_ALBUM_BOOKLET_THUMB_ICON_SIZE + 97) * scale);
		elm_gengrid_item_size_set(view->album_gengrid, w, h);
		elm_gengrid_align_set(view->album_gengrid, 0.5, 0.0);

		int i = 0;
		for (i = 0; i < info_data->discography_count; i++) {
			if (info_data->discography[i].album_title && info_data->discography[i].thumbpath) {
				_mp_album_booklet_append_album_item(view->album_gengrid, info_data->discography[i].album_title,
				                                    info_data->discography[i].year, info_data->discography[i].thumbpath);
			} else {
				DEBUG_TRACE("credits have error");
			}
		}
	} else {
		view->album_gengrid = mp_widget_create_no_contents(view->current_page, MP_NOCONTENT_NORMAL, NULL, NULL);
	}

	endfunc;
	return view->album_gengrid;
}

static Evas_Object *
_mp_album_booklet_view_add_review(Evas_Object *parent, MpAlbumBookletView_t *view)
{
	startfunc;
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(view);

	Evas_Object *scroller = elm_scroller_add(parent);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);

	Evas_Object *box = elm_box_add(scroller);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_object_content_set(scroller, box);

	/* title */
	Evas_Object *title = mp_common_load_edj(box, THEME_NAME, "elm/genlist/item/music_player/rich_info/page_title/default");
	evas_object_size_hint_weight_set(title, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(title, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_data_set(title, "view", view);

	char *text = _mp_album_booklet_view_title_text_get((void *)ALBUM_BOOKLET_VIEW_REVIEW, title, "elm.text");
	elm_object_part_text_set(title, "elm.text", text);
	IF_FREE(text);
	Evas_Object *index = _mp_album_booklet_view_title_content_get((void *)ALBUM_BOOKLET_VIEW_REVIEW, title, "elm.icon");
	elm_object_part_content_set(title, "elm.icon", index);
	elm_box_pack_end(box, title);

	/* review */
	Evas_Object *review = mp_widget_rich_info_text_add(box, view->info_data->review);
	evas_object_size_hint_weight_set(review, EVAS_HINT_EXPAND, EVAS_HINT_FILL);
	evas_object_size_hint_align_set(review, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_box_pack_end(box, review);

	return scroller;
}

static Evas_Object *
_mp_similar_album_create(Evas_Object *parent, MpAlbumBookletView_t *view)
{
	return _mp_album_booklet_view_gengrid_create(parent, view);
}

static Evas_Object *
_mp_review_create(Evas_Object *parent, MpAlbumBookletView_t *view)
{
	return _mp_album_booklet_view_add_review(parent, view);
}

static void _mp_album_booklet_view_content_load(void *thiz)
{
	startfunc;
	MpAlbumBookletView_t *view = (MpAlbumBookletView_t *)thiz;
	MP_CHECK(view);

	/*set itc*/
	if (!view->info_itc) {
		view->info_itc = elm_genlist_item_class_new();
		view->info_itc->item_style = "music_player/album_index";
		view->info_itc->func.text_get = _mp_album_booklet_view_album_index_text_get;
		view->info_itc->func.content_get = _mp_album_booklet_view_album_index_content_get;
		view->info_itc->func.del = NULL;
	}

	if (!view->title_itc) {
		view->title_itc = elm_genlist_item_class_new();
		view->title_itc->item_style = "music_player/rich_info/page_title";
		view->title_itc->func.text_get = _mp_album_booklet_view_title_text_get;
		view->title_itc->func.content_get = _mp_album_booklet_view_title_content_get;
		view->title_itc->func.del = NULL;
	}

	if (!view->track_itc) {
		view->track_itc = elm_genlist_item_class_new();
		view->track_itc->item_style = "2text";
		view->track_itc->func.text_get = _mp_album_booklet_view_track_gl_text_get;
		view->track_itc->func.content_get = NULL;
		view->track_itc->func.del = _mp_album_booklet_vew_list_item_del;
	}

	/*scroll page*/
	Evas_Object *scroll_page = mp_scroll_page_add(view->layout);
	mp_scroll_page_hide_scroll_bar(scroll_page);

	/* calc page total becasue of title index */
	if (view->info_data->track_list_count > 0) {
		++view->page_count;
	}
	if (view->info_data->review) {
		++view->page_count;
	}

	Evas_Object *page = NULL;
	int page_count = 0;

	if (view->info_data->track_list_count > 0) {
		view->page_index[ALBUM_BOOKLET_VIEW_TRACK_LIST] = page_count;
		page = _mp_album_booklet_view_create_track_list(scroll_page, view);
		evas_object_show(page);
		mp_scroll_page_content_append(scroll_page, page);
		page_count++;
	}

	if (0) {
		view->page_index[ALBUM_BOOKLET_VIEW_SIMILAR_ALBUM] = page_count;
		page = _mp_similar_album_create(scroll_page, view);
		evas_object_show(page);
		mp_scroll_page_content_append(scroll_page, page);
		page_count++;
	}

	if (view->info_data->review) {
		view->page_index[ALBUM_BOOKLET_VIEW_REVIEW] = page_count;
		page = _mp_review_create(scroll_page, view);
		evas_object_show(page);
		mp_scroll_page_content_append(scroll_page, page);
		page_count++;
	}

	elm_object_part_content_set(view->layout, "list_content", scroll_page);

}

static int
_mp_album_booklet_view_init(Evas_Object *parent, MpAlbumBookletView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_view_init(parent, (MpView_t *)view, MP_VIEW_ALBUM_BOOKLET);
	MP_CHECK_VAL(ret == 0, -1);

	view->disable_title_icon = true;

	view->update = _mp_album_booklet_view_update;
	view->update_options = _mp_album_booklet_view_update_options;
	view->update_options_edit = NULL;
	view->view_destroy_cb = _mp_album_booklet_view_destory_cb;

	return ret;
}

MpAlbumBookletView_t *mp_album_booklet_view_create(Evas_Object *parent, mp_info_data_t *info_data, char *album, char *artist, char *thumbnail)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpAlbumBookletView_t *view = calloc(1, sizeof(MpAlbumBookletView_t));
	MP_CHECK_NULL(view);

	ret = _mp_album_booklet_view_init(parent, view);
	if (ret) {
		goto Error;
	}

	view->info_data = info_data;
	view->name = g_strdup(album);
	view->artist = g_strdup(artist);
	view->thumbnail = g_strdup(thumbnail);

	_mp_album_booklet_view_content_load(view);
	return view;

Error:
	ERROR_TRACE("Error: mp_album_booklet_view_create()");
	IF_FREE(view);
	return NULL;
}


