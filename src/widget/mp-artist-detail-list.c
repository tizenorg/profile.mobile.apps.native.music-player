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

#include "mp-artist-detail-list.h"
#include "mp-create-playlist-view.h"
#include "mp-player-view.h"
#include "mp-popup.h"
#include "mp-ctxpopup.h"
#include "mp-util.h"
#include "mp-common.h"
#include "mp-widget.h"
#include "mp-play.h"
#include "mp-common.h"

static char *
_mp_artist_detail_list_album_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *name = NULL;
	int ret = 0;
	MpArtistDetailList_t *list = NULL;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	mp_retv_if(svc_item == NULL, NULL);

	if (!g_strcmp0(part, "elm.text")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}
		return g_strdup(elm_entry_utf8_to_markup(name));
	}
	DEBUG_TRACE("Unusing part: %s", part);
	return NULL;
}

static void _mp_aritst_detail_list_update_check(mp_list_item_data_t *it_data)
{
	MP_CHECK(it_data);
	Elm_Object_Item *gli = (Elm_Object_Item *)it_data->it;

	if (it_data->item_type == MP_LIST_ITEM_TYPE_SELECTABLE_GROUP_TITLE) {
		Eina_Bool checked = it_data->checked;

		Elm_Object_Item *item = elm_genlist_item_next_get(gli);
		while (item) {
			mp_list_item_data_t *item_data = elm_object_item_data_get(item);
			if (item_data && item_data->item_type == MP_LIST_ITEM_TYPE_NORMAL) {
				mp_list_item_check_set(item, checked);
			} else {
				break;
			}

			item = elm_genlist_item_next_get(item);
		}
	} else if (it_data->item_type == MP_LIST_ITEM_TYPE_NORMAL) {
		Elm_Object_Item *parent_item = elm_genlist_item_parent_get(gli);
		if (parent_item) {
			Eina_Bool checked_all = EINA_TRUE;
			Elm_Object_Item *item = elm_genlist_item_next_get(parent_item);
			while (item) {
				mp_list_item_data_t *item_data = elm_object_item_data_get(item);
				if (item_data && item_data->item_type == MP_LIST_ITEM_TYPE_NORMAL) {
					if (!item_data->checked) {
						checked_all = EINA_FALSE;
						break;
					}
				} else {
					break;
				}

				item = elm_genlist_item_next_get(item);
			}

			mp_list_item_check_set(parent_item, checked_all);
		}
	}
}

static void _mp_artist_detail_list_check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK(item);
	_mp_aritst_detail_list_update_check(item);

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	MP_CHECK(view_mgr);
	MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
	MP_CHECK(view);
	mp_view_update_options_edit(view);
}

Evas_Object *
_mp_artist_detail_list_album_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);
	mp_retv_if(svc_item == NULL, NULL);
	Evas_Object *content = NULL;
	Evas_Object *icon = NULL;

	MpArtistDetailList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	if (!strcmp(part, "elm.swallow.icon")) {
		content = elm_layout_add(obj);

		char *thumb_name = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumb_name, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);

		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		return content;
	}
	if (list->edit_mode) {
		if (!strcmp(part, "elm.swallow.end")) {
			icon = elm_check_add(obj);
			elm_object_style_set(icon, "default");
			evas_object_propagate_events_set(icon, EINA_FALSE);
			elm_check_state_pointer_set(icon, &item->checked);
			evas_object_smart_callback_add(icon, "changed", _mp_artist_detail_list_check_changed_cb, item);

			return icon;
		}
	}
	return NULL;
}

/*sweep button callbacks*/
static void
_mp_artist_detail_list_set_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	if (data == NULL) {
		return;
	}
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_ctxpopup_create(obj, MP_CTXPOPUP_PV_SET_AS, data, ad);

	return;
}

/*end of sweep button callbacks*/

static char *
_mp_artist_detail_list_track_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = (mp_media_info_h)(item->handle);
	mp_retvm_if(!track, NULL, "data is null");

	MpArtistDetailList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);


	static char result[DEF_STR_LEN + 1] = { 0, };

	if (!strcmp(part, "elm.text")) {

		bool match = mp_common_track_is_current(track, (MpList_t *)list);
		char *title = NULL;

		mp_media_info_get_title(track,	&title);

		mp_retv_if(!title, NULL);
		char *markup = NULL;

		if (match) {
			char *info = elm_entry_utf8_to_markup(title);

			int r = 21;
			int g = 108;
			int b = 148;
			int a = 255 ;

			memset(result, 0x00, DEF_STR_LEN + 1);
			snprintf(result, DEF_STR_LEN,
			         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, info);
			IF_FREE(info);

			return g_strdup(result);
		} else {
			markup = elm_entry_utf8_to_markup(title);
		}

		return markup;
	}
	if (!list->edit_mode) {
		if (!strcmp(part, "elm.text.end")) {
			int duration;
			char time[16] = "";
			bool match = mp_common_track_is_current(track, (MpList_t *)list);

			mp_media_info_get_duration(track, &duration);
			mp_util_format_duration(time, duration);
			time[15] = '\0';
			if (match) {
				int r = 21;
				int g = 108;
				int b = 148;
				int a = 255 ;

				memset(result, 0x00, DEF_STR_LEN + 1);
				snprintf(result, DEF_STR_LEN,
				         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, time);

				return g_strdup(result);
			}
			return g_strdup(time);
		}
	}
	return NULL;
}

Evas_Object *
_mp_artist_detail_list_track_icon_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h track = item->handle;
	mp_retvm_if(!track, NULL, "data is null");

	MpArtistDetailList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);
	/*
		if (!strcmp(part, "elm.icon"))
		{
			char *thumbpath = NULL;
			Evas_Object *icon;

			mp_media_info_get_thumbnail_path(track, &thumbpath);
			icon = mp_util_create_thumb_icon(obj, thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
			return icon;
		}
	*/
	Evas_Object *button;

	if (!strcmp(part, "elm.slide.swallow.3")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		//elm_object_text_set(button, GET_STR(STR_MP_ADD_TO));
		//mp_language_mgr_register_object(button, OBJ_TYPE_ELM_OBJECT, NULL, STR_MP_ADD_TO);
		mp_util_domain_translatable_text_set(button, STR_MP_ADD_TO);
		evas_object_smart_callback_add(button, "clicked", mp_common_button_add_to_playlist_cb, evas_object_data_get(obj, "list_data"));
		return button;
	} else if (!strcmp(part, "elm.slide.swallow.1")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		//elm_object_text_set(button, GET_SYS_STR("IDS_COM_BUTTON_SHARE"));
		//mp_language_mgr_register_object(button, OBJ_TYPE_ELM_OBJECT, NULL, "IDS_COM_BUTTON_SHARE");
		mp_util_domain_translatable_text_set(button, "IDS_COM_BUTTON_SHARE");
		evas_object_smart_callback_add(button, "clicked", mp_common_sweep_share_cb, track);
		return button;
	} else if (!strcmp(part, "elm.slide.swallow.2")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		//elm_object_text_set(button, GET_SYS_STR("IDS_COM_SK_SET"));
		//mp_language_mgr_register_object(button, OBJ_TYPE_ELM_OBJECT, NULL, "IDS_COM_SK_SET");
		mp_util_domain_translatable_text_set(button, "IDS_COM_SK_SET");
		evas_object_smart_callback_add(button, "clicked", _mp_artist_detail_list_set_cb, track);
		return button;
	} else if (!strcmp(part, "elm.slide.swallow.4")) {
		button = elm_button_add(obj);
		elm_object_style_set(button, "style1/delete");

		//elm_object_text_set(button, GET_SYS_STR("IDS_COM_OPT_DELETE"));
		//mp_language_mgr_register_object(button, OBJ_TYPE_ELM_OBJECT, NULL, "IDS_COM_OPT_DELETE");
		mp_util_domain_translatable_text_set(button, "IDS_COM_OPT_DELETE");
		evas_object_smart_callback_add(button, "clicked", mp_common_button_delete_list_cb, evas_object_data_get(obj, "list_data"));
		return button;
	}

	if (list->edit_mode) {
		// if edit mode
		DEBUG_TRACE("edit mode starts");
		if (!strcmp(part, "elm.swallow.end")) {		// swallow checkbox or radio button
			Evas_Object *icon = NULL;

			icon = elm_check_add(obj);
			elm_object_style_set(icon, "default");
			evas_object_propagate_events_set(icon, EINA_FALSE);
			elm_check_state_pointer_set(icon, &item->checked);
			evas_object_smart_callback_add(icon, "changed", _mp_artist_detail_list_check_changed_cb, item);

			return icon;
		}
	}
	return NULL;
}



static void
_mp_artist_detail_list_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	elm_genlist_item_selected_set(gli, FALSE);

	MpArtistDetailList_t *list = (MpArtistDetailList_t *)data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	DEBUG_TRACE("item selected");

	if (list->edit_mode) {
		mp_list_edit_mode_sel((MpList_t *)list, item);

		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		ERROR_TRACE("update options of edit view");
		mp_view_update_options_edit(view);
		ERROR_TRACE("set selected count");
		return;
	}

	mp_common_play_track_list(item, obj);

	return;
}

static void _mp_artist_detail_list_album_title_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_LIST_ITEM_IGNORE_SELECT(obj);
	elm_genlist_item_selected_set(gli, FALSE);

	MpArtistDetailList_t *list = data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	if (list->edit_mode) {
		mp_list_edit_mode_sel((MpList_t *)list, item);

		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		ERROR_TRACE("update options of edit view");
		mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count((MpList_t *)list));
		mp_view_update_options_edit(view);
		ERROR_TRACE("set selected count");
		return;
	}
}

static int _mp_artist_detail_list_append_album_tracks(void *thiz, char *name, Elm_Object_Item *parent_group)
{
	MpArtistDetailList_t *list = thiz;
	MP_CHECK_VAL(list, 0);


	/*media-svc related*/
	mp_media_list_h svc_handle;

	/*clear genlist*/
	/*
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	if (item)
	{
		elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
		elm_genlist_clear(list->genlist);
	}
	*/
	/*get data from DB*/
	int ret = 0;
	int count = 0;
	int real_count = 0;
	ret = mp_media_info_list_count(MP_TRACK_BY_ARTIST_ALBUM, name, list->type_str, NULL, 0, &count);
	MP_CHECK_VAL(ret == 0, 0);
	ret = mp_media_info_list_create(&svc_handle, MP_TRACK_BY_ARTIST_ALBUM, name, list->type_str, NULL, -1, 0, count);
	MP_CHECK_VAL(ret == 0, 0);

	DEBUG_TRACE("count is %d", count);
	real_count = count;
	int index = 0;
	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		char *title = NULL;

		item = mp_media_info_list_nth_item(svc_handle, index);
		ret = mp_media_info_get_title(item, &title);
		if (ret != 0) {
			DEBUG_TRACE("Fail to mp_media_info_get_title, ret[%d], index[%d]", ret, index);
			goto END;
		}

		/* check DRM FL */
		mp_list_item_data_t *item_data;
#ifdef MP_FEATURE_PERSONAL_PAGE
		char *path = NULL;
		mp_media_info_get_file_path(item, &path);
		if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_NONE) {
			goto append_artist_items;
		}

		if (mp_util_is_in_personal_page((const char *)path)) {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_ADD) {
				real_count--;
				continue;
			}
		} else {
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_REMOVE) {
				real_count--;
				continue;
			}
		}
append_artist_items:
#endif
		item_data = mp_list_item_data_create(MP_LIST_ITEM_TYPE_NORMAL);
		MP_CHECK_VAL(item_data, real_count);
		item_data->handle = item;
		item_data->index = index;

		item_data->it = elm_genlist_item_append(list->genlist, list->itc_track, item_data, parent_group,
		                                        ELM_GENLIST_ITEM_NONE, _mp_artist_detail_list_select_cb, list);
	}

	list->track_lists = g_list_append(list->track_lists, svc_handle);
	return real_count;
END:
	return 0;
	endfunc;
}

static void
_free_track_lists(void *data)
{
	mp_media_info_list_destroy(data);
}


static char *
_mp_artist_detail_list_shuffle_text_get(void *data, Evas_Object *obj, const char *part)
{
	startfunc;
	int res = -1;
	int count = 0;
	char *markup = NULL;
	static char result[DEF_STR_LEN + 1] = { 0, };

	if (!strcmp(part, "elm.text")) {
		MpArtistDetailList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);

		int r = 21;
		int g = 108;
		int b = 148;
		int a = 255 ;

		res = mp_media_info_list_count(MP_TRACK_BY_ARTIST, list->type_str, NULL,  NULL, 0, &count);
		MP_CHECK_NULL(res == 0);

		markup = (count == 1) ? g_strdup(GET_STR(STR_MP_SHUFFLE_1_TRACK)) : g_strdup_printf(GET_STR(STR_MP_SHUFFLE_PD_TRACKS), count);

		memset(result, 0x00, DEF_STR_LEN + 1);
		snprintf(result, DEF_STR_LEN, "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, markup);
		IF_FREE(markup);

		return g_strdup(result);
	}
	return NULL;
}

Evas_Object *
_mp_artist_detail_list_shuffle_icon_get(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.swallow.icon")) {
		Evas_Object *content = NULL;
		content = elm_layout_add(obj);

		Evas_Object *icon = NULL;
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", IMAGE_EDJ_NAME);

		icon = mp_util_create_image(obj, edje_path, MP_LITE_SHUFFLE_ICON, MP_LIST_SHUFFLE_ICON_SIZE, MP_LIST_SHUFFLE_ICON_SIZE);
		evas_object_color_set(icon, 21, 108, 148, 255);
		free(path);

		elm_layout_theme_set(content, "layout", "list/B/type.3", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		return content;
	}
	return NULL;
}


static void
_mp_artist_detail_list_shuffle_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	elm_genlist_item_selected_set(gli, FALSE);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);

	mp_play_control_shuffle_set(NULL, true);
	mp_common_play_track_list(item, obj);

	return;
}

void mp_artist_detail_list_update_genlist(void *thiz)
{
	startfunc;

	MP_CHECK(thiz);
	MpArtistDetailList_t *list = thiz;
	MP_CHECK(list->genlist);

	int count = 0;
	mp_media_info_list_count(MP_TRACK_BY_ARTIST, list->type_str, NULL,  NULL, 0, &count);

	if (count <= 0) {
		mp_list_update(thiz);
	} else {
		elm_genlist_realized_items_update(list->genlist);
	}
}

static void
_mp_artist_detail_list_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static void _mp_artist_detail_list_append_shuffle_item(MpArtistDetailList_t *list)
{
	startfunc;
	MP_CHECK(list);

	list->itc_shuffle = elm_genlist_item_class_new();

	if (list->itc_shuffle == NULL) {
		ERROR_TRACE("Cannot create artist detail list");
		return;
	}
	//list->itc_shuffle->item_style = "music/1line";//"music/1text.2icon.3";//"music/3text.1icon.2"
	list->itc_shuffle->item_style = "default";//"music/1text.2icon.3";//"music/3text.1icon.2"
	list->itc_shuffle->func.text_get = _mp_artist_detail_list_shuffle_text_get;
	list->itc_shuffle->decorate_all_item_style = NULL;
	list->itc_shuffle->func.content_get = _mp_artist_detail_list_shuffle_icon_get;
	list->itc_shuffle->func.del = _mp_artist_detail_list_item_del_cb;

	mp_list_item_data_t *item_data;
	item_data = mp_list_item_data_create(MP_LIST_ITEM_TYPE_SHUFFLE);
	MP_CHECK(item_data);
	item_data->it = list->shuffle_it = elm_genlist_item_append(list->genlist, list->itc_shuffle, item_data, NULL,
	                                   ELM_GENLIST_ITEM_NONE, _mp_artist_detail_list_shuffle_cb, list);
	elm_object_item_data_set(item_data->it, item_data);

	endfunc;
}

static void _mp_artist_detail_list_load_list(void *thiz, int count)
{
	MpArtistDetailList_t *list = thiz;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle;

	/*clear genlist*/
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	if (item) {
		elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
		elm_genlist_clear(list->genlist);
	}

	gint index = 0;
	int ret = 0;

	DEBUG_TRACE("count: %d", count);

	if (count < 0) {
		goto END;
	}

	if (list->album_list) {
		mp_media_info_group_list_destroy(list->album_list);
		g_list_free_full(list->track_lists, _free_track_lists);
		list->track_lists = NULL;
	}

	ret = mp_media_info_group_list_create(&list->album_list, MP_GROUP_BY_ARTIST_ALBUM, list->type_str, list->filter_str, 0, count);

	if (ret != 0) {
		DEBUG_TRACE("Fail to get items");
		goto END;
	}


	svc_handle = list->album_list ;

	_mp_artist_detail_list_append_shuffle_item(list);

	for (index = 0; index < count; index++) {
		mp_media_info_h item = NULL;
		Elm_Object_Item *list_item = NULL;
		char *title = NULL;

		item = mp_media_info_group_list_nth_item(svc_handle, index);
		if (!item) {
			DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
			goto END;
		}
		mp_media_info_group_get_main_info(item, &title);
		mp_list_item_data_t *item_data;
		item_data = mp_list_item_data_create(MP_LIST_ITEM_TYPE_SELECTABLE_GROUP_TITLE);
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->group_type = list->group_type;
		item_data->index = index;

		item_data->it = list_item = elm_genlist_item_append(list->genlist, list->itc_album, item_data, NULL,
		                            ELM_GENLIST_ITEM_NONE, _mp_artist_detail_list_album_title_select_cb, (void *)list);

		if (!list->edit_mode) {
			elm_genlist_item_select_mode_set(list_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}

		/* append tracks of the album */
		int appended_track_num = _mp_artist_detail_list_append_album_tracks(list, title, list_item);
		if (!appended_track_num) {
			elm_object_item_del(list_item);
			list->count_album--;
		}
	}

END:
	endfunc;
}

void _mp_artist_detail_list_destory_cb(void *thiz)
{
	eventfunc;
	MpArtistDetailList_t *list = thiz;
	MP_CHECK(list);

	if (list->album_list) {
		mp_media_info_group_list_destroy(list->album_list);
		g_list_free_full(list->track_lists, _free_track_lists);
		list->track_lists = NULL;
	}

	if (list->itc_track) {
		elm_genlist_item_class_free(list->itc_track);
		list->itc_track = NULL;
	}
	if (list->itc_album) {
		elm_genlist_item_class_free(list->itc_album);
		list->itc_album = NULL;
	}
	if (list->itc_shuffle) {
		elm_genlist_item_class_free(list->itc_shuffle);
		list->itc_shuffle = NULL;
	}

	IF_FREE(list->type_str);
	IF_FREE(list->filter_str);

	free(list);
}


/*static void
_mp_artist_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpArtistDetailList_t *list = (MpArtistDetailList_t*)data;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	char *title = NULL;
	char *file_path = NULL;
	int pop_item_count = 5;

	Elm_Object_Item *temp = NULL;
	Evas_Object *popup = NULL;
	mp_list_item_data_t *item_data = NULL;

	if (list->scroll_drag_status || list->edit_mode == 1 || list->shuffle_it == item)
		return;

	temp = elm_genlist_item_next_get(list->shuffle_it);
	while (temp) {
		item_data = elm_object_item_data_get(temp);
		item_data->checked = false;
		temp = elm_genlist_item_next_get(temp);
	}

	item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);

	item_data->checked = true;

	mp_media_info_get_title(item_data->handle, &title);
	mp_media_info_get_file_path(item_data->handle, &file_path);

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

        char *up_title = g_strdup(title);

        elm_object_part_text_set(popup, "title,text", up_title);
        IF_FREE(up_title);

        mp_genlist_popup_item_append(popup, STR_MP_SET_AS, NULL, NULL, NULL,
                                     mp_common_list_set_as_cb, list);
        mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PLAYLIST, NULL, NULL, NULL,
                                     mp_common_list_add_to_playlist_cb, list);
        bool favourite = false;
        char *str = NULL;
        Evas_Smart_Cb cb = NULL;

        mp_media_info_get_favorite(item_data->handle, &favourite);
        if (favourite) {
        	str = STR_MP_POPUP_REMOVE_FROM_FAVORITE;
        	cb = mp_common_list_unfavorite_cb;
        } else {
        	str = STR_MP_ADD_TO_FAVOURITES;
        	cb = mp_common_list_add_to_favorite_cb;
        }
        mp_genlist_popup_item_append(popup, str, NULL, NULL, NULL, cb, list);
        //mp_genlist_popup_item_append(popup, GET_STR(STR_MP_REMOVE), NULL, NULL,
          //                           mp_common_list_delete_cb, list);
        mp_genlist_popup_item_append(popup, STR_MP_DELETE, NULL, NULL, NULL,
                                 mp_common_list_delete_cb, list);
#ifdef MP_FEATURE_PERSONAL_PAGE
	if (mp_util_is_personal_page_on())
	{
		if (mp_util_is_in_personal_page((const char *)file_path))
		{
			DEBUG_TRACE("remove from personal page");
			list->personal_page_storage = MP_LIST_PERSONAL_PAGE_PRIVATE;
			mp_genlist_popup_item_append(popup, STR_MP_REMOVE_FROM_PERSONAL_PAGE, NULL, NULL, NULL,
												 mp_common_longpress_private_move_cb, list);
		}
		else
		{
			DEBUG_TRACE("add to personal page");
			list->personal_page_storage = MP_LIST_PERSONAL_PAGE_NORMAL;
			mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PERSONAL_PAGE, NULL, NULL, NULL,
												 mp_common_longpress_private_move_cb, list);
		}
	}
#endif

        mp_genlist_popup_item_append(popup, STR_MP_POPUP_MORE_INFO, NULL, NULL, NULL,
                                     mp_common_list_more_info_cb, list);
#ifdef MP_FEATURE_ALBUMART_UPDATE
	mp_genlist_popup_item_append(popup, STR_MP_UPDATE_ALBUM_ART, NULL, NULL, NULL,
                                     mp_common_list_update_albumart_cb, list);
#endif
	MP_GENLIST_ITEM_LONG_PRESSED(obj, popup, event_info);
}*/

static void _mp_artist_detail_list_item_highlighted(void *data, Evas_Object *obj, void *event_info)
{
	MpArtistDetailList_t *list = data;
	MP_CHECK(list);

	Elm_Object_Item *it = event_info;
	MP_CHECK(it);

	if (list->shuffle_it == it) {
		Evas_Object *icon = elm_object_item_part_content_get(it, "elm.icon.2");
		if (icon) {
			elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SHUFFLE_PRESS);
		}
	}
}

static void _mp_artist_detail_list_item_unhighlighted(void *data, Evas_Object *obj, void *event_info)
{
	MpArtistDetailList_t *list = data;
	MP_CHECK(list);

	Elm_Object_Item *it = event_info;
	MP_CHECK(it);

	if (list->shuffle_it == it) {
		Evas_Object *icon = elm_object_item_part_content_get(it, "elm.icon.2");
		if (icon) {
			elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SHUFFLE);
		}
	}
}

static void _mp_artist_detail_list_set_edit(void *thiz, bool edit)
{
	startfunc;
	MpArtistDetailList_t *list = thiz;
	MP_CHECK(list);

	if (edit) {
		mp_elm_object_item_del(list->shuffle_it);

		Elm_Object_Item *item = mp_list_first_item_get(list->genlist);
		while (item) {
			mp_list_item_data_t *item_data = (mp_list_item_data_t *) elm_object_item_data_get(item);
			if (item_data && item_data->item_type == MP_LIST_ITEM_TYPE_SELECTABLE_GROUP_TITLE) {
				mp_list_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DEFAULT);
			}
			item = mp_list_item_next_get(item);
		}

		mp_list_select_mode_set(list->genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	} else {
		mp_list_select_mode_set(list->genlist, ELM_OBJECT_SELECT_MODE_DEFAULT);

		Elm_Object_Item *item = mp_list_first_item_get(list->genlist);
		while (item) {
			mp_list_item_data_t *item_data = (mp_list_item_data_t *) elm_object_item_data_get(item);
			if (item_data) {
				item_data->checked = EINA_FALSE;
				if (item_data->item_type == MP_LIST_ITEM_TYPE_SELECTABLE_GROUP_TITLE) {
					mp_list_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
				}
			}
			item = mp_list_item_next_get(item);
		}

		// TODO: restore shuffle item
	}

	elm_genlist_realized_items_update(list->genlist);
}

void _mp_artist_detail_list_update(void *thiz)
{
	startfunc;
	int res = 0;
	int count_album = 0;
	MpArtistDetailList_t *list = thiz;
	MP_CHECK(list);

	res = mp_media_info_group_list_count(MP_GROUP_BY_ARTIST_ALBUM, list->type_str, list->filter_str, &count_album);
	MP_CHECK(res == 0);
	DEBUG_TRACE("count is %d", count_album);
	list->count_album = count_album;

	mp_evas_object_del(list->genlist);
	mp_evas_object_del(list->no_content);

	if (count_album) {
		/*create new genlist*/
		list->genlist = mp_widget_genlist_create(list->box);
		evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		elm_genlist_mode_set(list->genlist, ELM_LIST_COMPRESS);
		evas_object_show(list->genlist);
		/*packet genlist to box*/
		elm_box_pack_end(list->box, list->genlist);

		evas_object_data_set(list->genlist, "list_data", list);
#ifdef MP_FEATURE_ADD_TO_HOME
		const char *group_slide_style = "mode/slide4";
		//group_slide_style = "mode/slide4";
#endif
		list->itc_album = elm_genlist_item_class_new();
		if (list->itc_album) {
			list->itc_album->item_style = "default";
			list->itc_album->func.text_get = _mp_artist_detail_list_album_label_get;
			list->itc_album->func.content_get = _mp_artist_detail_list_album_icon_get;
			list->itc_album->func.del = _mp_artist_detail_list_item_del_cb;

			list->itc_track = elm_genlist_item_class_new();
			if (list->itc_track) {
				list->itc_track->item_style = "type1";
				list->itc_track->func.text_get = _mp_artist_detail_list_track_label_get;
				list->itc_track->func.content_get = _mp_artist_detail_list_track_icon_get;
				list->itc_track->func.del = _mp_artist_detail_list_item_del_cb;
			}
		}
		evas_object_smart_callback_add(list->genlist, "drag,start,left", list->flick_left_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,right", list->flick_right_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,stop", list->flick_stop_cb, NULL);

		evas_object_smart_callback_add(list->genlist, "drag,start,right", list->mode_right_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,left", list->mode_left_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,up", list->mode_cancel_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,down", list->mode_cancel_cb, NULL);

		evas_object_smart_callback_add(list->genlist, "scroll,drag,start", list->drag_start_cb, list);
		evas_object_smart_callback_add(list->genlist, "scroll,drag,stop", list->drag_stop_cb, list);
		evas_object_smart_callback_add(list->genlist, "highlighted", _mp_artist_detail_list_item_highlighted, list);
		evas_object_smart_callback_add(list->genlist, "unhighlighted", _mp_artist_detail_list_item_unhighlighted, list);

		/* load list */
		_mp_artist_detail_list_load_list(thiz, count_album);

	} else {
		DEBUG_TRACE("count is 0");
		list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_NORMAL, NULL, NULL);
		elm_box_pack_end(list->box, list->no_content);
	}

}

static mp_track_type_e _mp_artist_detail_list_get_track_type(void *thiz)
{
	MpArtistDetailList_t *list = thiz;
	MP_CHECK_VAL(list, MP_TRACK_ALL);
	return MP_TRACK_BY_ARTIST;
}

static unsigned int
_mp_artist_detail_list_get_count(void *thiz, MpListEditType_e type)
{
	MpArtistDetailList_t *list = thiz;
	MP_CHECK_VAL(list->genlist, 0);

	int count = elm_genlist_items_count(list->genlist);

	if (list->shuffle_it) {
		--count;
	}

	if (list->bottom_counter_item) {
		--count;
	}
	count = count - list->count_album ;

	return count;
}

static void
_mp_aritst_detail_list_edit_mode_sel(void *thiz, void *data)
{
	startfunc;
	MpList_t *list = (MpList_t *)thiz;
	mp_list_item_data_t *it_data = (mp_list_item_data_t *)data;
	Elm_Object_Item *gli = (Elm_Object_Item *)it_data->it;
	mp_list_item_selected_set(gli, EINA_FALSE);


	if (elm_genlist_item_flip_get(gli) || elm_genlist_item_select_mode_get(gli) == ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY) {
		return;
	}

	mp_list_item_check_set(gli, !it_data->checked);

	_mp_aritst_detail_list_update_check(it_data);

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
	if (view) {
		mp_util_create_selectioninfo_with_count(view, mp_list_get_checked_count(list));
	}
}

static void
_mp_artist_detail_list_selected_item_data_get(void *thiz, GList **selected)
{
	startfunc;
	MpList_t *list = (MpList_t *)thiz;
	GList *sel_list = NULL;

	if (!list->genlist) {
		goto END;
	}

	Elm_Object_Item *item = mp_list_first_item_get(list->genlist);

	if (!item) {
		goto END;
	}

	while (item) {
		mp_list_item_data_t *item_data = elm_object_item_data_get(item);
		if (item_data && item_data->item_type == MP_LIST_ITEM_TYPE_NORMAL) {
			if (item_data->checked) {
				sel_list = g_list_append(sel_list, item_data);
			}
		}

		item = mp_list_item_next_get(item);
	}
END:
	if (selected) {
		*selected = sel_list;
	}
}

static char *
_mp_artist_detail_list_bottom_counter_text_cb(void *thiz)
{
	MpArtistDetailList_t *list = thiz;
	MP_CHECK_NULL(list);

	unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));
	char *text = NULL;
	if (count == 1) {
		text = g_strdup(GET_STR(STR_MP_1_SONG));
	} else {
		text = g_strdup_printf(GET_STR(STR_MP_PD_SONGS), count);
	}

	return text;
}

static mp_group_type_e _mp_artist_detail_list_get_group_type(void *thiz)
{
	MpArtistDetailList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);
	return MP_GROUP_NONE;
}


MpArtistDetailList_t * mp_artist_detail_list_create(Evas_Object *parent)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpArtistDetailList_t *list = calloc(1, sizeof(MpArtistDetailList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_ARTIST_DETAIL);

	list->update = _mp_artist_detail_list_update;
	list->destory_cb = _mp_artist_detail_list_destory_cb;
	list->get_track_type = _mp_artist_detail_list_get_track_type;
	list->get_group_type = _mp_artist_detail_list_get_group_type;
	list->get_count = _mp_artist_detail_list_get_count;
	list->set_edit_default = list->set_edit;
	list->set_edit = _mp_artist_detail_list_set_edit;
	list->edit_mode_sel = _mp_aritst_detail_list_edit_mode_sel;
	list->selected_item_data_get = _mp_artist_detail_list_selected_item_data_get;
	list->set_reorder = NULL;
	list->bottom_counter_text_get_cb = _mp_artist_detail_list_bottom_counter_text_cb;


	return list;
}

void mp_artist_detail_list_set_data(MpArtistDetailList_t *list, ...)
{
	startfunc;
	MP_CHECK(list);

	va_list var_args;
	int field;

	va_start(var_args, list);
	do {
		field = va_arg(var_args, int);
		switch (field) {
		case MP_ARTIST_DETAIL_LIST_TYPE: {
			int val = va_arg((var_args), int);

			list->group_type = val;
			DEBUG_TRACE("list->group_type = %d", list->group_type);
			break;
		}

		case MP_ARTIST_DETAIL_LIST_TYPE_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->type_str);
			list->type_str = g_strdup(val);
			DEBUG_TRACE("list->type_str = %s", list->type_str);

			break;
		}
		case MP_ARTIST_DETAIL_LIST_FILTER_STR: {
			char *val = va_arg((var_args), char *);
			SAFE_FREE(list->filter_str);
			list->filter_str = g_strdup(val);
			DEBUG_TRACE("list->filter_str = %s", list->filter_str);

			break;
		}

		default:
			DEBUG_TRACE("Invalid arguments");
		}

	} while (field >= 0);

	va_end(var_args);
}

void mp_artist_detail_list_copy_data(MpArtistDetailList_t *src, MpArtistDetailList_t *dest)
{
	MP_CHECK(src);
	MP_CHECK(dest);

	dest->group_type = src->group_type;
	SAFE_FREE(dest->type_str);
	dest->type_str = g_strdup(src->type_str);
	SAFE_FREE(dest->filter_str);
	dest->filter_str = g_strdup(src->filter_str);
}

