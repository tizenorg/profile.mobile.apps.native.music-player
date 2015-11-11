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

#include "mc-common.h"
#include "mc-library-view.h"
#include "mp-media-info.h"
#include "mc-track-list.h"
#include "mc-list-play.h"
#include "mc-index.h"
#include <telephony.h>
#include <notification.h>
#include <efl_extension.h>
#include <mp-file-util.h>
#include <storage.h>

#define MC_FILE_PREFIX "file://"

static int externalStorageId = -1;
bool detail_view = false;

static Eina_Bool
_back_cb(void *data, Elm_Object_Item *it)
{
	startfunc;
	struct app_data *ad = data;
	elm_naviframe_item_pop(ad->navi_bar);
	detail_view = false;

	return EINA_FALSE;
}

void mc_common_push_track_view_by_group_name(void *data, int track_type, const char *name, int playlist_id, const char *folder_name)
{
	startfunc;
	struct app_data *ad = data;
	MP_CHECK(ad);
	MP_CHECK(name);
	Elm_Object_Item *it = NULL;
	Elm_Object_Item *g_navi_it = NULL;
	Evas_Object *sub_view = NULL;
	Evas_Object *track_list = NULL;
	Evas_Object *tabbar = NULL;

	ad->track_type = track_type;

	Evas_Object *navi_layout = mc_common_load_edj(ad->navi_bar, MC_EDJ_FILE, "view_layout_tabbar");
	g_navi_it = elm_naviframe_top_item_get(ad->navi_bar);
//	g_ly = elm_object_item_part_content_get(g_navi_it, "elm.swallow.content");
	tabbar = _create_tabbar(ad->navi_bar, ad);

	elm_object_part_content_set(navi_layout, "tabbar", tabbar);
//	sub_view = elm_object_part_content_unset(g_ly, "list-content");

	track_list = mc_track_list_create(ad->navi_bar, ad);
	mc_track_list_set_data(track_list, track_type, name, playlist_id);

	elm_object_part_content_set(navi_layout, "list-content", track_list);

	detail_view = true;
	it = elm_naviframe_item_push(ad->navi_bar, NULL, NULL, NULL, navi_layout, NULL);
	mc_track_list_update(track_list, it, sub_view);

	if (track_type == MP_TRACK_BY_ALBUM) {
		mc_common_obj_domain_text_translate(ad->navi_bar, MC_TEXT_SELECT_ALBUM);
	} else if (track_type == MP_TRACK_BY_ARTIST) {
		mc_common_obj_domain_text_translate(ad->navi_bar, MC_TEXT_SELECT_ARTIST);
	} else if (track_type == MP_TRACK_BY_PLAYLIST) {
		mc_common_obj_domain_text_translate(ad->navi_bar, MC_TEXT_SELECT_PLAYLIST);
	} else {
		mc_common_obj_domain_text_translate(ad->navi_bar, MC_TEXT_SELECT);
	}

#ifdef MC_AUTO_RECOMMENDED
	if (ad->auto_recommended_show) {
		elm_object_signal_emit(layout, "show.recommended", "*");
		Evas_Object *recommended_area = mc_common_load_edj(ad->navi_bar, MC_EDJ_FILE, "recommended_area");
		elm_object_part_content_set(layout, "recommended", recommended_area);
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

	/*reset back button callback*/
	elm_naviframe_item_pop_cb_set(g_navi_it, _back_cb, ad);
	endfunc;
}

Elm_Object_Item *mc_common_toolbar_item_append(Evas_Object *obj, const char *icon,
        const char *label, Evas_Smart_Cb func,
        const void *data)
{
	Elm_Object_Item *item = elm_toolbar_item_append(obj, icon, label, func, data);
	MP_CHECK_NULL(item);

	elm_object_item_domain_text_translatable_set(item, DOMAIN_NAME, EINA_TRUE);

	return item;
}

void mc_common_obj_domain_text_translate(Evas_Object *obj, const char *label)
{
	MP_CHECK(obj);
	elm_object_domain_translatable_text_set(obj, DOMAIN_NAME, (const char *)label);
}

void mc_common_obj_domain_translatable_part_text_set(Evas_Object *obj, const char* part, const char* label)
{
	MP_CHECK(obj);
	elm_object_domain_translatable_part_text_set(obj, part, DOMAIN_NAME, label);
}

Evas_Object *mc_widget_genlist_create(Evas_Object * parent)
{
	Evas_Object *list = NULL;

	list = elm_genlist_add(parent);
	MP_CHECK_NULL(list);

	elm_scroller_bounce_set(list, EINA_FALSE, EINA_TRUE);
	return list;
}

void mc_post_status_message(const char *text)
{
	int ret = notification_status_message_post(text);
	if (ret != 0) {
		ERROR_TRACE("notification_status_message_post()... [0x%x]", ret);
	} else {
		DEBUG_TRACE("message: [%s]", text);
	}
}

bool mc_is_call_connected(void)
{
	telephony_call_state_e state;
	telephony_handle_list_s tel_list;
	int tel_valid = telephony_init(&tel_list);
	if (tel_valid != 0) {
		ERROR_TRACE("telephony is not initialized. ERROR Code is %d", tel_valid);
		return false;
	}

	telephony_h *newhandle = tel_list.handle;

	int error = telephony_call_get_voice_call_state(*newhandle , &state);

	telephony_deinit(&tel_list);

	if (error == TELEPHONY_ERROR_NONE) {
		if (state == TELEPHONY_CALL_STATE_IDLE) {
			return false;   /*There exists no calls*/
		}
		/* There exists at least one call that is dialing, alerting or incoming*/
		return true;
	} else {
		ERROR_TRACE("ERROR: state error is %d", error);
	}
	return false;
}

bool mc_get_supported_storages_callback(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	if (type == STORAGE_TYPE_EXTERNAL) {
		externalStorageId = storageId;
		return false;
	}
	return true;
}

bool mc_is_mmc_removed(void)
{
	int error = storage_foreach_device_supported(mc_get_supported_storages_callback, NULL);
	if (error == STORAGE_ERROR_NONE) {
		storage_state_e state;
		storage_get_state(externalStorageId, &state);
		if (state == STORAGE_STATE_REMOVED) {
			return true;
		}
	}

	return false;
}

bool mc_check_file_exist(const char *path)
{
	if (path == NULL || strlen(path) == 0) {
		return FALSE;
	}

	bool mmc_removed = mc_is_mmc_removed();

	if (mmc_removed && strstr(path, MP_MMC_ROOT_PATH) == path) {
		return false;
	}

	if (strstr(path, MC_FILE_PREFIX)) {
		if (!g_file_test(path + strlen(MC_FILE_PREFIX), G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
			ERROR_TRACE("file not exist: %s", path);
			return FALSE;
		}
		return TRUE;
	} else {
		if (!g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR)) {
			ERROR_TRACE("file not exist: %s", path);
			return FALSE;
		}
		return TRUE;
	}
	return TRUE;
}

bool mc_check_image_valid(Evas *evas, const char *path)
{
	if (!path) {
		return false;
	}
	MP_CHECK_FALSE(evas);

	if (!mp_file_exists(path)) {
		ERROR_TRACE("file not exists");
		return false;
	} else if (!strcmp(BROKEN_ALBUMART_IMAGE_PATH, path)) {
		return false;
	}

	Evas_Object *image = NULL;
	int width = 0;
	int height = 0;

	image = evas_object_image_add(evas);
	MP_CHECK_FALSE(image);
	evas_object_image_file_set(image, path, NULL);
	evas_object_image_size_get(image, &width, &height);
	evas_object_del(image);

	if (width <= 0 || height <= 0) {
		DEBUG_TRACE("Cannot load file : %s", path);
		return false;
	}

	return true;
}

char *mc_artist_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *text = NULL;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK_NULL(item_data);
	mp_media_info_h handle = item_data->media;

	if (strcmp(part, "elm.text.main.left.top") == 0) {
		mp_media_info_group_get_main_info(handle, &text);
		return g_strdup(text);
	} else if (strcmp(part, "elm.text.sub.left.bottom") == 0) {
		char **album_thumbs = NULL;
		int song_count;
		int album_count = 0;

		mp_media_info_group_get_album_thumnail_paths(handle, &album_thumbs, &album_count);
		mp_media_info_group_get_main_info(handle, &text);
		mp_media_info_list_count(MP_TRACK_BY_ARTIST, text, NULL, NULL, 0, &song_count);
		if (album_count == 1 && song_count == 1) {
			text = g_strdup(GET_STR(STR_MP_1_ALBUM_1_SONG));
		} else if (album_count == 1 && song_count > 1) {
			text = g_strdup_printf(GET_STR(STR_MP_1_ALBUM_PD_SONGS), song_count);
		} else {
			text = g_strdup_printf(GET_STR(STR_MP_PD_ALBUMS_PD_SONGS), album_count, song_count);
		}
		return text;
	}
	return NULL;
}

char *mc_album_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *text = NULL;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK_NULL(item_data);
	mp_media_info_h handle = item_data->media;

	int ret = 0;
	if (strcmp(part, "elm.text.main.left.top") == 0) {
		ret = mp_media_info_group_get_main_info(handle, &text);
		MP_CHECK_NULL(ret == 0);
		return g_strdup(text);
	} else if (strcmp(part, "elm.text.sub.left.bottom") == 0) {
		ret = mp_media_info_group_get_sub_info(handle, &text);
		MP_CHECK_NULL(ret == 0);
		return g_strdup(text);
	} else if (strcmp(part, "elm.text.3") == 0) {
		int count;
		ret = mp_media_info_group_get_main_info(handle, &text);
		MP_CHECK_NULL(ret == 0);
		ret = mp_media_info_list_count(MP_TRACK_BY_ALBUM, text, NULL, NULL, 0, &count);
		MP_CHECK_NULL(ret == 0);
		text = g_strdup_printf("(%d)", count);
		return text;
	}
	return NULL;
}

char * mc_playlist_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *text = NULL;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK_NULL(item_data);
	mp_media_info_h handle = item_data->media;

	if (strcmp(part, "elm.text.1") == 0) {
		mp_media_info_group_get_main_info(handle, &text);
		return g_strdup(GET_STR(text));
	} else if (strcmp(part, "elm.text") == 0) {
		mp_media_info_group_get_main_info(handle, &text);
		return g_strdup(GET_STR(text));
	} else if (strcmp(part, "elm.text.2") == 0) {
		int id = 0;
		int count = 0;
		mp_media_info_group_get_playlist_id(handle, &id);
		if (id == MP_SYS_PLST_MOST_PLAYED) {
			mp_media_info_list_count(MP_TRACK_BY_PLAYED_COUNT, NULL, NULL, NULL, 0, &count);
		} else if (id == MP_SYS_PLST_RECENTELY_ADDED) {
			mp_media_info_list_count(MP_TRACK_BY_ADDED_TIME, NULL, NULL, NULL, 0, &count);
		} else if (id == MP_SYS_PLST_RECENTELY_PLAYED) {
			mp_media_info_list_count(MP_TRACK_BY_PLAYED_TIME, NULL, NULL, NULL, 0, &count);
		} else if (id == MP_SYS_PLST_QUICK_LIST) {
			mp_media_info_list_count(MP_TRACK_BY_FAVORITE, NULL, NULL, NULL, 0, &count);
		} else {
			mp_media_info_list_count(MP_TRACK_BY_PLAYLIST, NULL, NULL, NULL, id, &count);
		}
		text = g_strdup_printf("(%d)", count);
		return text;
	}
	return NULL;
}

char *mc_folder_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *name = NULL;
	int ret = 0;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK_NULL(item_data);
	mp_media_info_h svc_item = item_data->media;
	MP_CHECK_NULL(svc_item);

	if (!strcmp(part, "elm.text.main.left.top") || !strcmp(part, "elm.slide.text.1")) {
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}

		if (!strcmp(part, "elm.text.main.left.top")) {
			return elm_entry_utf8_to_markup(name);
		} else {
			return g_strdup(name);
		}

	} else if (!strcmp(part, "elm.text.sub.left.bottom")) {
		ret = mp_media_info_group_get_sub_info(svc_item, &name);
		mp_retvm_if((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name)) {
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		}
		return g_strdup(name);
	} else if (!strcmp(part, "elm.text.3")) {
		int count = 0;
		mp_media_info_group_get_track_count(svc_item, &count);
		return g_strdup_printf("(%d)", count);
	}
	return NULL;
}


Evas_Object * mc_group_content_get(void *data, Evas_Object *obj, const char *part)
{
	Evas_Object *content = NULL;
	Evas_Object *icon = NULL;
	char *thumbpath = NULL;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK_NULL(item_data);
	mp_media_info_h handle = item_data->media;

	if (!strcmp(part, "elm.icon.1")) {
		content = elm_layout_add(obj);
		mp_media_info_group_get_thumbnail_path(handle, &thumbpath);
		icon = elm_image_add(obj);
		if (mc_check_image_valid(evas_object_evas_get(obj), thumbpath)) {
			elm_image_file_set(icon, thumbpath, NULL);
		} else {
			elm_image_file_set(icon, DEFAULT_THUMBNAIL, NULL);
		}

		elm_layout_theme_set(content, "layout", "list/B/type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		evas_object_repeat_events_set(content, EINA_TRUE);
		evas_object_propagate_events_set(content, EINA_FALSE);
		return content;
	}
	return content;
}

void
mc_eext_quit_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	elm_exit();
}

Eina_Bool mc_quit_cb(void *data, Elm_Object_Item *it)
{
	DEBUG_TRACE("");
	elm_exit();

	return EINA_FALSE;
}

#ifdef MC_AUTO_RECOMMENDED
void mc_auto_recommended_check_cb(void *data, Evas_Object *obj, void *event_info)
{
	//elm_object_signal_emit(g_ly, "hide.recommended", "*");
	startfunc;
	struct app_data *ad = data;
	MP_CHECK(ad);

	Eina_Bool state = elm_check_state_get(obj);
	ad->auto_recommended_on = state;

	/*find the selected item*/
	int index = mc_track_list_get_radio();
	DEBUG_TRACE("pre play item is %d", index);

	Evas_Object *genlist = elm_layout_content_get(ad->track_list, "list_content");
	if (genlist) {
		ERROR_TRACE("genlist is NULL");
	}
	Elm_Object_Item *pre_play_item = elm_genlist_nth_item_get(genlist, index);

	list_item_data_t *pre_play_item_data = elm_object_item_data_get(pre_play_item);
	if (pre_play_item_data) {
		ERROR_TRACE("item data is NULL");
	}
	mc_pre_play_mgr_reset_song(pre_play_item_data);
	mc_pre_play_mgr_play_song(pre_play_item_data);
}
#endif

void
mc_quit_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	elm_exit();
}

Evas_Object *mc_common_load_edj(Evas_Object * parent, const char *file, const char *group)
{
	Evas_Object *eo = NULL;
	int r = -1;

	eo = elm_layout_add(parent);
	if (eo) {
		r = elm_layout_file_set(eo, file, group);
		if (!r) {
			evas_object_del(eo);
			return NULL;
		}
		evas_object_size_hint_weight_set(eo, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(eo);
	}
	evas_object_name_set(eo, group);

	return eo;
}

Evas_Object *
mc_widget_navigation_new(Evas_Object * parent)
{
	Evas_Object *navi_bar;
	mp_retv_if(parent == NULL, NULL);
	navi_bar = elm_naviframe_add(parent);
	mp_retvm_if(navi_bar == NULL, NULL, "Fail to create navigation bar");
	elm_naviframe_event_enabled_set(navi_bar, EINA_FALSE);

	evas_object_show(navi_bar);
	return navi_bar;
}

void mc_common_create_fastscroller(Evas_Object *parent, Eina_Bool multiple, Evas_Object *genlist)
{
	startfunc;
	Evas_Object *index;
	elm_object_signal_emit(parent, "show.fastscroll", "*");
	if (multiple) {
		index = mc_index_create(parent, 1, genlist);
	} else {
		index = mc_index_create(parent, 0, genlist);
	}
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_OFF);
	elm_layout_content_set(parent, "elm.swallow.content.index", index);
	mc_index_append_item(index, genlist);

	return ;
}

Evas_Object *mc_common_create_processing_popup(void *data)
{
	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;

	struct app_data *ad = data;
	MP_CHECK_NULL(ad);

	popup = elm_popup_add(ad->base_layout);
	//eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, eext_popup_back_cb, NULL);
	Evas_Object *layout = elm_layout_add(popup);
	elm_layout_file_set(layout, MC_EDJ_FILE, "popup_processingview_1button");
	/*create circle progressbar*/

	progressbar = elm_progressbar_add(popup);
	elm_object_style_set(progressbar, "process_large");

	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, 0.5);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(progressbar);
	elm_progressbar_pulse(progressbar, EINA_TRUE);

	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	mc_common_obj_domain_translatable_part_text_set(layout, "elm.text", "IDS_COM_BODY_LOADING");
	elm_object_content_set(popup, layout);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(popup);

	return popup;
}

#define DEF_BUF_LEN            (512)
const char *mc_common_search_markup_keyword(const char *string, char *searchword, bool *result)
{
	char pstr[DEF_BUF_LEN + 1] = {0,};
	static char return_string[DEF_BUF_LEN + 1] = { 0, };
	int word_len = 0;
	int search_len = 0;
	int i = 0;
	bool found = false;
	gchar* markup_text_start = NULL;
	gchar* markup_text_end = NULL;
	gchar* markup_text = NULL;

	MP_CHECK_NULL(string && strlen(string));
	MP_CHECK_NULL(searchword && strlen(searchword));
	MP_CHECK_NULL(result);

	if (g_utf8_validate(string, -1, NULL)) {

		word_len = strlen(string);
		if (word_len > DEF_BUF_LEN) {
			char *temp = (char*)calloc((word_len + 1), sizeof(char));
			MP_CHECK_NULL(temp);
			if (strlen(string) <= DEF_BUF_LEN) {
				strncpy(temp, string , strlen(string));
			}
			i = 0;
			while (word_len > DEF_BUF_LEN) {
				/*truncate uft8 to byte_size DEF_BUF_LEN*/
				gchar *pre_ch = g_utf8_find_prev_char(temp, (temp + DEF_BUF_LEN - 1 - i * 3));
				if (!pre_ch) {
					break;
				}
				gchar *next_ch = g_utf8_find_next_char(pre_ch, NULL);
				if (!next_ch) {
					break;
				}
				/*truncate position*/
				*next_ch = '\0';
				word_len = strlen(temp);
				i++;
			}
			if (strlen(temp) <= DEF_BUF_LEN) {
				strncpy(pstr, temp, strlen(temp));
			}
			IF_FREE(temp);
		} else {
			if (strlen(string) <= DEF_BUF_LEN) {
				strncpy(pstr, string, strlen(string));
			}
		}

		word_len = strlen(pstr);
		search_len = strlen(searchword);

		for (i = 0; i < word_len; i++) {
			if (!strncasecmp(searchword, &pstr[i], search_len)) {
				found = true;
				break;
			}
		}

		*result = found;
		memset(return_string, 0x00, DEF_BUF_LEN + 1);

		if (found) {
			if (i == 0) {
				markup_text = g_markup_escape_text(&pstr[0], search_len);
				markup_text_end = g_markup_escape_text(&pstr[search_len], word_len - search_len);
				MP_CHECK_NULL(markup_text && markup_text_end);
				snprintf(return_string,
				         DEF_BUF_LEN,
				         "<color=#FE5400>%s</color>%s",
				         markup_text,
				         (char*)markup_text_end);
				IF_FREE(markup_text);
				IF_FREE(markup_text_end);
			} else {
				markup_text_start = g_markup_escape_text(&pstr[0], i);
				markup_text = g_markup_escape_text(&pstr[i], search_len);
				markup_text_end =  g_markup_escape_text(&pstr[i + search_len], word_len - (i + search_len));
				MP_CHECK_NULL(markup_text_start && markup_text && markup_text_end);
				snprintf(return_string,
				         DEF_BUF_LEN,
				         "%s<color=#FE5400>%s</color>%s",
				         (char*)markup_text_start,
				         markup_text,
				         (char*)markup_text_end);
				IF_FREE(markup_text);
				IF_FREE(markup_text_start);
				IF_FREE(markup_text_end);
			}
		} else {
			snprintf(return_string, DEF_BUF_LEN, "%s", pstr);
		}
	}

	return return_string;
}

Evas_Object *mc_common_create_thumb_icon(Evas_Object * obj, const char *path, int w, int h)
{
	Evas_Object *thumbnail = elm_image_add(obj);
	if (w == h) {
		elm_image_prescale_set(thumbnail, w);
		elm_image_fill_outside_set(thumbnail, true);
	}

	if ((!path) || !g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) || !strcmp(BROKEN_ALBUMART_IMAGE_PATH, path)) {
		path = DEFAULT_THUMBNAIL;
	}
	elm_image_file_set(thumbnail, path, NULL);

	evas_object_size_hint_align_set(thumbnail, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(thumbnail);
	//endfunc;

	return thumbnail;
}

