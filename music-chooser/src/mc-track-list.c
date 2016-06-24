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
#include "mc-list-play.h"

static int state_index = -1; //selected radio index
static Evas_Object* g_radio_main = NULL;

extern list_item_data_t *previous_item_data;
extern list_item_data_t *pre_item_data;
extern list_item_data_t *cur_item_data;
extern int g_position;

typedef struct {
	struct app_data *ad;

	Evas_Object *no_content;
	Evas_Object *genlist;
	Elm_Object_Item *btn_done;
	Evas_Object *btn_cancel;
	Evas_Object *btn_set;

	Elm_Genlist_Item_Class itc;
	Elm_Genlist_Item_Class itc_select_all;

	mp_track_type_e t_type;
	char *type_str;
	int playlist_id;
	bool multiple;
	bool single;

	mp_media_list_h track_list;

	Ecore_Timer *destroy_timer;
	Elm_Object_Item *win_navi_it;
} track_list_data_t;


#define GET_LIST_DATA(obj)	evas_object_data_get(obj, "list_data")

static Evas_Object *
_mc_create_genlist(Evas_Object *parent)
{
	Evas_Object *genlist = NULL;
	MP_CHECK_NULL(parent);

	genlist = elm_genlist_add(parent);
	elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);

	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	g_radio_main = elm_radio_add(genlist);
	elm_radio_state_value_set(g_radio_main, -1);
	elm_radio_value_set(g_radio_main, -1);
	evas_object_data_set(genlist, "radio_main", g_radio_main);

	return genlist;
}

static void
_mc_track_play_btn_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	startfunc;
	MP_CHECK(emission);

	list_item_data_t *itemData = data;
	MP_CHECK(itemData);

	mc_pre_play_control_play_music_item(itemData);

	return ;
}

static void _check_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	list_item_data_t *it_data = data;
	MP_CHECK(it_data);

	it_data->checked = !it_data->checked;
	it_data->checkbox_cb = EINA_TRUE;
}

static Evas_Object *_gl_select_all_content_get(void *data, Evas_Object *obj, const char *part)
{
	if (!strcmp(part, "elm.swallow.end")) {
		Evas_Object *content = elm_check_add(obj);
		elm_object_style_set(content, "default/genlist");
		evas_object_smart_callback_add(content, "changed", _mc_track_list_select_all_selected_item_data_get, data);
		evas_object_repeat_events_set(content, EINA_FALSE);
		evas_object_propagate_events_set(content, EINA_FALSE);
		return content;
	}
	return NULL;
}

void mc_post_notification_indicator(list_item_data_t *it_data, player_state_e state)
{
	startfunc;
	MP_CHECK(it_data);
	struct app_data *ad = it_data->ad;
	int ret = NOTIFICATION_ERROR_NONE;

	notification_image_type_e img_type = NOTIFICATION_IMAGE_TYPE_ICON_FOR_INDICATOR;
	char *path = app_get_shared_resource_path();

	//DEBUG_TRACE("Shared Resource Path is %s", path);
	char icon_path[1024] = {0};

	if (state == PLAYER_STATE_PLAYING || state == PLAYER_STATE_IDLE) {
		snprintf(icon_path, 1024, "%sshared_images/T02_control_circle_icon_play.png", path);
	} else {
		snprintf(icon_path, 1024, "%sshared_images/T02_control_circle_icon_pause.png", path);
	}
	free(path);

	if (ad->noti) {
		ret = notification_set_image(ad->noti, img_type, icon_path);
		if (ret != NOTIFICATION_ERROR_NONE) {
			DEBUG_TRACE("Cannot set the notification image");
		}
		notification_update(ad->noti);
	}
	DEBUG_TRACE("Icon Path is: %s", icon_path);
	endfunc;
}

static Evas_Object *_gl_content_get(void *data, Evas_Object *obj, const char *part)
{
	char *thumbpath = NULL;
	char *prev_uri = NULL;
	char *cur_uri = NULL;
	Evas_Object *content = NULL;
	Evas_Object *icon = NULL;
	Evas_Object *check = NULL;
	Evas_Object *prev_part_content;
	Evas_Object *part_content;

	list_item_data_t *it_data = data;
	MP_CHECK_NULL(it_data);

	mp_media_info_h media = it_data->media;
	it_data->checkbox_cb = EINA_FALSE;

	if (previous_item_data) {
		mp_media_info_get_file_path((mp_media_info_h)(previous_item_data->media), &prev_uri);
	}

	if (previous_item_data && (g_strcmp0(prev_uri, cur_uri) != 0)) {
		prev_part_content = elm_object_item_part_content_get(previous_item_data->it, "elm.swallow.icon");
		DEBUG_TRACE("Previous URI: %s", prev_uri);
		if (prev_part_content) {
			elm_object_signal_emit(prev_part_content, "show_default", "*");
			//elm_object_item_signal_emit(previous_item_data->it, "hide_color", "*");
		}
	}

	if (cur_item_data) {
		mp_media_info_get_file_path((mp_media_info_h)(cur_item_data->media), &cur_uri);
		DEBUG_TRACE("Current URI is: %s", cur_uri);
		part_content = elm_object_item_part_content_get(cur_item_data->it, "elm.swallow.icon");
		player_state_e state = mc_pre_play_get_player_state();

		if (part_content) {
			if (state == PLAYER_STATE_PLAYING || state == PLAYER_STATE_IDLE) {
				elm_object_signal_emit(part_content, "show_play", "*");
			} else {
				elm_object_signal_emit(part_content, "show_pause", "*");
			}
			mc_post_notification_indicator(it_data, state);

			//elm_object_item_signal_emit(cur_item_data->it, "show_color" ,"*");
		}
	}

	if (!strcmp(part, "elm.swallow.icon")) {
		content = elm_layout_add(obj);
		mp_media_info_get_thumbnail_path(media, &thumbpath);
		icon = elm_image_add(obj);

		if (mc_check_image_valid(evas_object_evas_get(obj), thumbpath)) {
			elm_image_file_set(icon, thumbpath, NULL);
		} else {
			char default_thumbnail[1024] = {0};
			char *shared_path = app_get_shared_resource_path();
			DEBUG_TRACE("Chooser Shared Path : %s", shared_path);
			snprintf(default_thumbnail, 1024, "%s%s/%s", shared_path, "shared_images", DEFAULT_THUMBNAIL);
			free(shared_path);
			elm_image_file_set(icon, default_thumbnail, NULL);
		}

		elm_layout_theme_set(content, "layout", "list/B/type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		return content;
	}

	if ((!strcmp(part, "elm.swallow.end") && (it_data->ad->select_type == MC_SELECT_SINGLE_RINGTONE || it_data->ad->select_type == MC_SELECT_SINGLE))) {	// swallow checkbox or radio button
		if (it_data->ad->select_uri && it_data->ad->select_type == MC_SELECT_SINGLE_RINGTONE) {
			char *filepath = NULL;
			mp_media_info_get_file_path(media, &filepath);
			if (mc_check_file_exist(filepath) && !strcmp(it_data->ad->select_uri, filepath)) {
				state_index = it_data->index;
				it_data->ad->select_uri = NULL;
				//elm_genlist_item_bring_in(it_data->it, ELM_GENLIST_ITEM_SCROLLTO_IN);
			}
		}
		content = elm_layout_add(obj);
		check = elm_radio_add(obj);
		elm_radio_state_value_set(check, it_data->index);
		elm_radio_group_add(check, g_radio_main);
		evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(check, EINA_TRUE);
		elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
		elm_layout_content_set(content, "elm.swallow.content", check);
	} else if ((!strcmp(part, "elm.swallow.end") && (it_data->ad->select_type == MC_SELECT_MULTI))) {
		content = elm_check_add(obj);
		elm_check_state_pointer_set(content, &it_data->checked);
		elm_object_style_set(content, "default/genlist");
		evas_object_smart_callback_add(content, "changed", _check_changed_cb, it_data);
		evas_object_repeat_events_set(content, EINA_TRUE);
		evas_object_propagate_events_set(content, EINA_FALSE);
	}

	if (!strcmp(part, "elm.edit.icon.2")) {
		if (it_data->ad->select_type == MC_SELECT_SINGLE_RINGTONE) {
			if (it_data->index == state_index) {
				Evas_Object *music_button = NULL;

				music_button = elm_button_add(obj);

				player_state_e state = mc_pre_play_get_player_state();
				if (state == PLAYER_STATE_PLAYING) {
					elm_object_style_set(music_button, "music/ug_control_play");
				} else if (state == PLAYER_STATE_PAUSED) {
					elm_object_style_set(music_button, "music/ug_control_pause");
				} else {
					return NULL;
				}
				evas_object_propagate_events_set(music_button, EINA_FALSE);
				elm_object_signal_callback_add(music_button, "mouse,clicked,1", "*", _mc_track_play_btn_cb, data);

				return music_button;
			}
		}
	}
	return content;
}

char *mc_create_selectioninfo_text_with_count(int count)
{
	startfunc;
	char *name = NULL;
	if (count > 0) {
		name =  g_strdup_printf(GET_STR(STR_MP_SELECT_ITEMS), count);
	} else {
		name = g_strdup_printf(GET_STR(MC_TEXT_SELECT));
	}
	return name;
}

static char *_gl_select_all_text_get(void *data, Evas_Object *obj, const char *part)
{
	startfunc;
	if (!strcmp(part, "elm.text")) {
		char *name = GET_SYS_STR("IDS_MUSIC_BODY_SELECT_ALL");
		return g_strdup(name);
	}
	return NULL;
}

static char *_gl_text_get(void *data, Evas_Object *obj, const char *part)
{
	char *title = NULL;
	char *cur_uri = NULL;
	char *uri = NULL;
	bool match = false;

	list_item_data_t *it_data = data;
	MP_CHECK_NULL(it_data);

	mp_media_info_h media = it_data->media;

	mp_media_info_get_file_path(media, &uri);
	mp_retv_if(!uri, NULL);

	int ret = 0;
	if ((!strcmp(part, "elm.text")) || (!strcmp(part, "elm.text.sub"))) {
		if (!strcmp(part, "elm.text")) {
			ret = mp_media_info_get_title(media,  &title);
			MP_CHECK_NULL(ret == 0);
		} else {
			ret = mp_media_info_get_artist(media, &title);
			MP_CHECK_NULL(ret == 0);
		}

		if (cur_item_data) {
			mp_media_info_get_file_path((mp_media_info_h)(cur_item_data->media), &cur_uri);

			if (cur_item_data && (g_strcmp0(cur_uri, uri) == 0)) {
				match = true;
			}
		}

		char *markup = NULL;
		static char result[DEF_STR_LEN + 1] = { 0, };

		if (match) {
			char *markup_title = elm_entry_utf8_to_markup(title);

			int r, g, b, a;
			//Apply RGB equivalent of color
			r = 21;
			g = 108;
			b = 148;
			a = 255;
			memset(result, 0x00, DEF_STR_LEN + 1);
			snprintf(result, DEF_STR_LEN,
			         "<color=#%02x%02x%02x%02x>%s</color>", r, g, b, a, markup_title);
			IF_FREE(markup_title);

			return g_strdup(result);
		} else {
			markup = elm_entry_utf8_to_markup(title);
		}
		return markup;
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
	track_list_data_t *ld  = data;
	MP_CHECK_FALSE(ld);
	ld->destroy_timer = NULL;
	elm_exit();
	return EINA_FALSE;
}

static long long int
_get_total_size(void *data)
{
	startfunc;
	long long int item_size = 0;
	struct stat mpFileInfo;
	char *path = NULL;

	track_list_data_t *ld  = data;
	Elm_Object_Item *item;
	MP_CHECK_VAL(ld, 0);
	MP_CHECK_VAL(ld->genlist, 0);

	item = elm_genlist_first_item_get(ld->genlist);
	item = elm_genlist_item_next_get(item);
	while (item) {
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (!it_data) {
			continue;
		}
		mp_media_info_h media = it_data->media;
		MP_CHECK_VAL(media, 0);
		mp_media_info_get_file_path(media, &path);
		stat(path, &mpFileInfo);
		item_size = item_size + mpFileInfo.st_size;
		item = elm_genlist_item_next_get(item);
	}
	return item_size;

	endfunc;
}

static unsigned int
_get_media_list_count(void *data)
{
	startfunc;
	unsigned int count = 0;

	track_list_data_t *ld  = data;
	Elm_Object_Item *item;
	MP_CHECK_VAL(ld, 0);
	MP_CHECK_VAL(ld->genlist, 0);

	item = elm_genlist_first_item_get(ld->genlist);
	while (item) {
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (!it_data) {
			continue;
		}
		item = elm_genlist_item_next_get(item);
		if (it_data) {
			count++;
		}
	}
	if (count > 0) {
		count = count - 1;
		return count;
	} else {
		return -1;
	}

	endfunc;
}

unsigned int
_get_select_count(void *data)//(Evas_Object *genlist)
{
	startfunc;
	unsigned int count = 0;

	track_list_data_t *ld  = data;
	Elm_Object_Item *item;
	MP_CHECK_VAL(ld, 0);
	MP_CHECK_VAL(ld->genlist, 0);

	item = elm_genlist_first_item_get(ld->genlist);
	if (ld->ad->select_type == MC_SELECT_MULTI) {
		item = elm_genlist_item_next_get(item);
	}
	while (item) {
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (!it_data) {
			continue;
		}
		item = elm_genlist_item_next_get(item);
		if (it_data && it_data->checked) {
			count++;
		}
	}
	return count;
}

static Elm_Object_Item  *_get_select_radio(void *data)
{
	startfunc;

	track_list_data_t *ld  = data;

	Elm_Object_Item *item = NULL;
	item = elm_genlist_first_item_get(ld->genlist);

	while (item) {
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (!it_data) {
			continue;
		}
		item = elm_genlist_item_next_get(item);

		int index = elm_radio_value_get(g_radio_main);
		DEBUG_TRACE("index: %d", index);
		if (it_data->index == index) {
			return item;
		}
	}
	return item;

}

static void _gl_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	track_list_data_t *ld  = data;
	static long long int item_size = 0;
	struct stat mpFileInfo;
	char *path = NULL;
	bool size_exceeded = false;

	MP_CHECK(ld);
	MP_CHECK(!ld->destroy_timer);

	if (cur_item_data) {
		elm_genlist_item_update(cur_item_data->it);
	}

	elm_genlist_item_update(event_info);
	elm_genlist_item_selected_set(event_info, EINA_FALSE);

	list_item_data_t *it_data = elm_object_item_data_get(event_info);
	MP_CHECK(it_data);
	Elm_Object_Item *item = it_data->it;
	int index = 0;

	mp_media_info_h media = it_data->media;
	MP_CHECK(media);
	mp_media_info_get_file_path(media, &path);
	DEBUG_TRACE("path: %s", path);
	stat(path, &mpFileInfo);

	if (it_data->ad->select_type == MC_SELECT_SINGLE_RINGTONE) {
		mc_pre_play_control_play_no_pause_music_item((void*)it_data);
		g_position = 0;
		if (item) {
			index = it_data->index;
			Evas_Object *radio = elm_object_item_part_content_get(item, "elm.swallow.end");
			MP_CHECK(radio);

			state_index = index;
			elm_radio_value_set(g_radio_main, state_index);
			if (index != -1) {
				if (ld->btn_set) {
					elm_object_disabled_set(ld->btn_set, false);
				}
				if (ld->btn_cancel) {
					elm_object_disabled_set(ld->btn_cancel, false);
				}
			}
//			elm_genlist_realized_items_update(ld->genlist);
		}
	} else if (it_data->ad->select_type == MC_SELECT_SINGLE) {
		if (item) {
			index = it_data->index;
			elm_radio_value_set(g_radio_main, index);
		}

		if (index != -1) {
			if (ld->btn_set) {
				elm_object_disabled_set(ld->btn_set, false);
			}
		}

		return;
	} else {
		it_data->checked = !it_data->checked;
	}

	if (ld->ad->select_type == MC_SELECT_MULTI && !it_data->checkbox_cb) {
		Evas_Object *check = elm_object_item_part_content_get(item, "elm.swallow.end");
		Eina_Bool check_state = elm_check_state_get(check);
		elm_check_state_set(check, !check_state);
	}

	elm_genlist_item_fields_update(event_info, "elm.swallow.end", ELM_GENLIST_ITEM_FIELD_CONTENT);

	if ((ld->ad->select_type == MC_SELECT_MULTI) && ((ld->ad->limitsize > 0) && (item_size + mpFileInfo.st_size > ld->ad->limitsize)) && it_data->checked) {
		WARN_TRACE("Exceeded max size by caller");
		size_exceeded = true;
		it_data->checked = !it_data->checked;
		if (!it_data->checkbox_cb) {
			Evas_Object *check = elm_object_item_part_content_get(item, "elm.swallow.end");
			Eina_Bool check_state = elm_check_state_get(check);
			elm_check_state_set(check, !check_state);
		}
		char *name = g_strdup(GET_STR(STR_MC_MAX_SIZE_EXCEEDED));
		mc_post_status_message(name);
		IF_FREE(name);
	}

	if ((ld->ad->select_type == MC_SELECT_MULTI) && ((ld->ad->max_count > 0) && _get_select_count(ld) > ld->ad->max_count)) {
		WARN_TRACE("Exceeded max count by caller");
		it_data->checked = !it_data->checked;
		if (!it_data->checkbox_cb) {
			Evas_Object *check = elm_object_item_part_content_get(item, "elm.swallow.end");
			Eina_Bool check_state = elm_check_state_get(check);
			elm_check_state_set(check, !check_state);
		}
		char *name = g_strdup_printf(GET_STR(STR_MC_MAX_COUNT_EXCEEDED), ld->ad->max_count);
		mc_post_status_message(name);
		IF_FREE(name);
	}
	mc_common_obj_domain_text_translate(ld->ad->navi_bar, mc_create_selectioninfo_text_with_count(_get_select_count(ld)));

	if (ld->ad->select_type == MC_SELECT_MULTI) {
		Elm_Object_Item *selected_item = elm_genlist_first_item_get(ld->genlist);
		Evas_Object *check = elm_object_item_part_content_get(selected_item, "elm.swallow.end");
		if (!size_exceeded) {
			if (it_data->checked) {
				item_size = item_size + mpFileInfo.st_size;
			} else {
				item_size = item_size - mpFileInfo.st_size;
			}
		}
		if ((_get_media_list_count(ld) - _get_select_count(ld)) == 0) {
			elm_check_state_set(check, EINA_TRUE);
		} else {
			elm_check_state_set(check, EINA_FALSE);
		}
	}


	if (ld->btn_done && ld->btn_cancel) {
		if (_get_select_count(ld)) {
			elm_object_disabled_set(ld->btn_done, false);
			elm_object_disabled_set(ld->btn_cancel, false);
		} else {
			elm_object_disabled_set(ld->btn_done, true);
//			elm_object_disabled_set(ld->btn_cancel, true);
		}
		DEBUG_TRACE("Selected Count = %d", _get_select_count(ld));
		return;
	}
}

void mc_track_list_set_uri_selected(void *thiz, const char *uri)
{
	startfunc;

	struct app_data *ad = thiz;
	MP_CHECK(ad);
	MP_CHECK(ad->track_list);

	track_list_data_t *ld = evas_object_data_get(ad->track_list, "list_data");
	MP_CHECK(ld);

	Elm_Object_Item *item;
	item = elm_genlist_first_item_get(ld->genlist);

	while (item) {
		char *path = NULL;
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (!it_data) {
			continue;
		}
		mp_media_info_h media = it_data->media;
		MP_CHECK(media);
		mp_media_info_get_file_path(media, &path);

		if (!strcmp(path, uri)) {
			elm_genlist_item_bring_in(it_data->it, ELM_GENLIST_ITEM_SCROLLTO_TOP);
			elm_radio_value_set(g_radio_main, it_data->index);

			elm_genlist_item_selected_set(item, EINA_TRUE);
		}
		item = elm_genlist_item_next_get(item);
	}

//	elm_genlist_realized_items_update(ld->genlist);
}

static void
_layout_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	state_index = -1;
	pre_item_data = NULL;
	cur_item_data = NULL;

	track_list_data_t *ld  = data;
	MP_CHECK(ld);

	IF_G_FREE(ld->type_str);

	mc_pre_play_mgr_destroy_play();
	mc_pre_play_control_clear_pre_item_data();

	free(ld);
}

static void
_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	track_list_data_t *ld  = (track_list_data_t *)data;
	MP_CHECK(ld);

	app_control_h service = NULL;
	app_control_create(&service);
	app_control_reply_to_launch_request(service, ld->ad->service, APP_CONTROL_RESULT_SUCCEEDED);
	if (ld->t_type == MP_TRACK_ALL) {
		mp_ecore_timer_del(ld->destroy_timer);
		ld->destroy_timer = ecore_timer_add(0.1, _destroy_timer_cb, ld);
	} else {
		_mc_popup_view(ld);
	}

	app_control_destroy(service);
	ld->destroy_timer = ecore_timer_add(0.1, _destroy_timer_cb, ld);
}

static void
_set_cb(void *data, Evas_Object *obj, void *event_info)
{
	track_list_data_t *ld  = (track_list_data_t *)data;
	MP_CHECK(ld);

	char *fmt = ";%s";
	char **path_array = NULL;
	int count = 0;
	GString *path = NULL;

	Elm_Object_Item *item = elm_genlist_first_item_get(ld->genlist);
	Elm_Object_Item *selected_item = _get_select_radio(ld);

	while (item) {
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (it_data == NULL) {
			IF_FREE(path_array);
			return;
		}
		item = elm_genlist_item_next_get(item);
		if (selected_item == item) {
			char *tmp = NULL;
			mp_media_info_h media = it_data->media;
			if (!media) {
				continue;
			}
			mp_media_info_get_file_path(media, &tmp);
			if (path == NULL) {
				path = g_string_new(tmp);
			} else {
				g_string_append_printf(path, fmt, tmp);
			}

			count++;
			path_array = realloc(path_array, sizeof(char *) * count);
			if (path_array != NULL) {
				path_array[count - 1] = tmp;
			}
		}
	}

	if (!path) {
		IF_FREE(path_array);
		return;
	}

	DEBUG_TRACE("Done: Return uri: %s", path->str);
	DEBUG_TRACE("Done: Return position: %d", g_position);
	app_control_h service = NULL;
	app_control_create(&service);
	app_control_add_extra_data_array(service, APP_CONTROL_DATA_PATH,
	                                 (const char **)path_array,
	                                 count);
	app_control_add_extra_data_array(service, APP_CONTROL_DATA_SELECTED, (const char **)path_array, count);
	app_control_reply_to_launch_request(service, ld->ad->service, APP_CONTROL_RESULT_SUCCEEDED);

	mp_ecore_timer_del(ld->destroy_timer);
	ld->destroy_timer = ecore_timer_add(0.1, _destroy_timer_cb, ld);

	app_control_destroy(service);

	IF_FREE(path_array);
	if (path) {
		g_string_free(path, TRUE);
		path = NULL;
	}

}

#if 0
static Eina_Bool
_back_cb(void *data, Elm_Object_Item *it)
{
	startfunc;
	track_list_data_t *ld  = (track_list_data_t *)data;
	MP_CHECK_FALSE(ld);

	if (ld->t_type == MP_TRACK_ALL) {

		Elm_Object_Item *selected_item = _get_select_radio(ld);

		if (selected_item != NULL) {
			_set_cb(ld, NULL, NULL);
		} else {
			_cancel_cb(ld, NULL, NULL);
		}
	} else {
		_mc_popup_view(ld);
	}

	return EINA_FALSE;
}
#endif

void _mc_popup_view(void *data)
{
	startfunc;
	track_list_data_t *ld  = data;
	MP_CHECK(ld);
	MP_CHECK(ld->win_navi_it);
	elm_object_item_del(ld->win_navi_it);
	ld->win_navi_it = NULL;
}

static void
_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	char *fmt = ";%s";
	GString *path = NULL;
	track_list_data_t *ld = data;
	Elm_Object_Item *item = NULL;
	char **path_array = NULL;
	int count = 0;
	MP_CHECK(ld);

	item = elm_genlist_first_item_get(ld->genlist);
	DEBUG_TRACE("In done_cb -> Select_type is: %d", ld->ad->select_type);
	if (ld->ad->select_type == MC_SELECT_MULTI) {
		item = elm_genlist_item_next_get(item);
	}
	while (item) {
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (it_data == NULL) {
			IF_FREE(path_array);
			return;
		}
		item = elm_genlist_item_next_get(item);
		if (it_data && it_data->checked) {
			char *tmp = NULL;
			mp_media_info_h media = it_data->media;
			if (!media) {
				break;
			}

			mp_media_info_get_file_path(media, &tmp);
			if (path == NULL) {
				path = g_string_new(tmp);
				MP_CHECK(path);
			} else {
				g_string_append_printf(path, fmt, tmp);
			}

			count++;
			path_array = realloc(path_array, sizeof(char *) * count);
			if (path_array != NULL) {
				path_array[count - 1] = tmp;
			}
		}
	}

	MP_CHECK(path);

	DEBUG_TRACE("Done: return uri: %s", path->str);
	DEBUG_TRACE("Done: return position: %d", g_position);
	if (path) {
		app_control_h service = NULL;
		app_control_create(&service);
		app_control_add_extra_data_array(service, APP_CONTROL_DATA_PATH,
		                                 (const char **)path_array,
		                                 count);
		app_control_add_extra_data_array(service,
		                                 APP_CONTROL_DATA_SELECTED,
		                                 (const char **)path_array,
		                                 count);

		app_control_reply_to_launch_request(service, ld->ad->service,
		                                    APP_CONTROL_RESULT_SUCCEEDED);
		ld->destroy_timer = ecore_timer_add(0.1, _destroy_timer_cb,
		                                    ld);

		app_control_destroy(service);
	}
	IF_FREE(path_array);

	g_string_free(path, TRUE);
}

void _mc_track_list_select_all_selected_item_data_get(void *data, Evas_Object *obj, void *event_info)
{
	track_list_data_t *ld  = data;
	Elm_Object_Item *item = NULL;
	MP_CHECK(ld);

	Eina_Bool all_selected = EINA_FALSE;

	item = elm_genlist_first_item_get(ld->genlist);
	Evas_Object *check = elm_object_item_part_content_get(item, "elm.swallow.end");
	Eina_Bool state = elm_check_state_get(check);
	all_selected = state;

	if ((ld->ad->limitsize > 0) && (_get_total_size(ld) > ld->ad->limitsize)) {
		DEBUG_TRACE("total size: %lld", _get_total_size(ld));
		elm_check_state_set(check, !state);
		char *name = g_strdup(GET_STR(STR_MC_MAX_SIZE_EXCEEDED));
		mc_post_status_message(name);
		IF_FREE(name);
		WARN_TRACE("Exceeded max size by caller");
		return;
	}

	if ((ld->ad->max_count > 0) && (_get_media_list_count(ld) > ld->ad->max_count)) {
		elm_check_state_set(check, !state);
		char *name = g_strdup_printf(GET_STR(STR_MC_MAX_COUNT_EXCEEDED), ld->ad->max_count);
		mc_post_status_message(name);
		IF_FREE(name);
		WARN_TRACE("Exceeded max count by caller");
		return;
	}

	item = elm_genlist_first_item_get(ld->genlist);
	item = elm_genlist_item_next_get(item);
	while (item) {
		list_item_data_t *it_data = elm_object_item_data_get(item);
		if (!it_data) {
			continue;
		}
		it_data->checked = all_selected;
		Evas_Object *chk = elm_object_item_part_content_get(item, "elm.swallow.end");
		if (chk) {
			elm_check_state_set(chk, all_selected);
		}
		item = elm_genlist_item_next_get(item);
	}

	mc_common_obj_domain_text_translate(ld->ad->navi_bar, mc_create_selectioninfo_text_with_count(_get_select_count(ld)));

	if (ld->btn_done) {
		if (_get_select_count(ld) && all_selected) {
			elm_object_disabled_set(ld->btn_done, EINA_FALSE);
		} else {
			elm_object_disabled_set(ld->btn_done, EINA_TRUE);
		}
	}
}

void _mc_track_list_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	track_list_data_t *ld  = data;
	Elm_Object_Item *item = NULL;
	MP_CHECK(ld);

	elm_genlist_item_selected_set(event_info, EINA_FALSE);

	item = elm_genlist_first_item_get(ld->genlist);
	Evas_Object *check = elm_object_item_part_content_get(item, "elm.swallow.end");
	Eina_Bool state = elm_check_state_get(check);
	if (ld->ad->max_count <= 0) {
		elm_check_state_set(check, !state);
	}

	_mc_track_list_select_all_selected_item_data_get(data, obj, event_info);

	endfunc;
}


Evas_Object *mc_track_list_create(Evas_Object *parent, struct app_data *ad)
{
	startfunc;
	Evas_Object *layout ;
	track_list_data_t *ld = NULL;

	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(ad);

	char mc_edj_path[1024] = {0};
	char *path = app_get_resource_path();
	MP_CHECK_NULL(path);
	snprintf(mc_edj_path, 1024, "%s%s", path, MC_EDJ_FILE);
	free(path);
	layout = mc_common_load_edj(parent, mc_edj_path, "list_layout");
	MP_CHECK_NULL(layout);

	ld = calloc(1, sizeof(track_list_data_t));
	MP_CHECK_NULL(ld);

	ld->ad = ad;
	if (ad->select_type == MC_SELECT_MULTI) {
		ld->multiple = true;
	}

	if (ad->select_type == MC_SELECT_SINGLE_RINGTONE) {
		ld->single = true;
	}

	Evas_Object *done_btn = elm_object_item_part_content_unset(ld->win_navi_it, "toolbar");
	if (done_btn) {
		evas_object_del(done_btn);
	}

	evas_object_data_set(layout, "list_data", ld);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, _layout_del_cb, ld);

	ld->itc.item_style = "type1";
	ld->itc.func.content_get = _gl_content_get;
	ld->itc.func.text_get = _gl_text_get;
	ld->itc.func.del = _gl_del;
//	ld->itc.decorate_all_item_style = "edit_default";

	ld->itc_select_all.item_style = "default";
	ld->itc_select_all.func.content_get = _gl_select_all_content_get;
	ld->itc_select_all.func.text_get = _gl_select_all_text_get;

	return layout;
}

static void
_clear_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	Evas_Object *sub_view = data;
	evas_object_del(sub_view);
}

int mc_track_list_update(Evas_Object *list, Elm_Object_Item *navi_it, Evas_Object *sub_view)
{
	startfunc;
	Evas_Object *content;

	int count = 0;
	track_list_data_t *ld  = GET_LIST_DATA(list);
	MP_CHECK_VAL(ld, -1);

	struct app_data *ad  = ld->ad;
	MP_CHECK_VAL(ad, -1);

	ld->win_navi_it = navi_it;

	if (ld->win_navi_it && ad->navi_bar) {
		DEBUG_TRACE("In cancel button");
		ld->btn_cancel = mc_widget_create_navi_left_btn(ad->navi_bar, ld->win_navi_it, _cancel_cb, ld);
	}

	if (ad->select_type == MC_SELECT_MULTI) {
#if 0
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
		evas_object_smart_callback_add(select_btn, "clicked", _mc_track_list_select_cb, ld);
		evas_object_show(select_btn);
		elm_object_item_part_content_set(ld->win_navi_it, "title_right_btn", select_btn);
#endif

		if (ld->win_navi_it && ad->navi_bar) {

			ld->btn_done = mc_widget_create_navi_right_btn(ad->navi_bar, ld->win_navi_it, _done_cb, ld);
		}
	} else if (ld->ad->select_type == MC_SELECT_SINGLE_RINGTONE || ld->ad->select_type == MC_SELECT_SINGLE) {
		if (ld->win_navi_it && ad->navi_bar) {
			ld->btn_set = mc_widget_create_navi_right_btn(ad->navi_bar, ld->win_navi_it, _set_cb, ld);
			elm_object_disabled_set(ld->btn_set, EINA_TRUE);
		}
		/*reset back button callback*/
		/* elm_naviframe_item_pop_cb_set(navi_it, _back_cb, ld);*/
	}

	if (ld->track_list) {
		mp_media_info_list_destroy(ld->track_list);
		ld->track_list = NULL;
	}

	content = elm_layout_content_get(list, "list_content");
	evas_object_del(content);

	mp_media_info_list_count(ld->t_type, ld->type_str, NULL, NULL, ld->playlist_id, &count);
	mc_common_obj_domain_text_translate(ld->ad->navi_bar, mc_create_selectioninfo_text_with_count(0));

	if (count) {
		ld->genlist = content = _mc_create_genlist(list);
		mp_media_info_list_create(&ld->track_list, ld->t_type, ld->type_str, NULL, NULL, ld->playlist_id, 0, count);
		if (ld->ad->select_type == MC_SELECT_MULTI) {
			elm_genlist_item_append(content, &ld->itc_select_all, ld, NULL, ELM_GENLIST_ITEM_NONE, _mc_track_list_select_cb, ld);
		}
		int i = 0;
		for (i = 0; i < count; i++) {
			mp_media_info_h media =  mp_media_info_list_nth_item(ld->track_list, i);
			list_item_data_t *data = calloc(1, sizeof(list_item_data_t));
			if (data) {
				data->media = media;
				data->ad = ad;
				data->index = i;
				if (ld->t_type == MP_TRACK_ALL) {
					data->list_type = MC_TRACK;
				} else if (ld->t_type == MP_TRACK_BY_ARTIST) {
					data->list_type = MC_ARTIST_TRACK;
				} else if (ld->t_type == MP_TRACK_BY_ALBUM) {
					data->list_type = MC_ALBUM_TRACK;
				} else if (ld->t_type == MP_TRACK_BY_FOLDER) {
					data->list_type = MC_FOLDER_TRACK;
				}

				data->it = elm_genlist_item_append(content, &ld->itc, data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_cb, ld);
				elm_object_item_data_set(data->it, data);
			}
		}
	} else {
		content = mc_widget_no_content_add(list, NO_CONTENT_SONG);
	}

	elm_layout_content_set(list, "list_content", content);
	/*add index*/

	if (count > 0) {
		if (ld->ad->select_type == MC_SELECT_MULTI) {
			mc_common_create_fastscroller(list, EINA_TRUE, content);
		} else {
			mc_common_create_fastscroller(list, EINA_FALSE, content);
		}
	} else {
		ERROR_TRACE("No content case");
		elm_object_signal_emit(list, "hide.fastscroll", "*");
	}

	if (sub_view != NULL) {
		evas_object_smart_callback_add(ld->genlist, "realized", _clear_cb, sub_view);
	}

	return 0;
}

int mc_track_list_set_data(Evas_Object *list, int track_type, const char *type_str, int playlist_id)
{
	startfunc;
	track_list_data_t *ld  = GET_LIST_DATA(list);
	MP_CHECK_VAL(ld, -1);

	ld->t_type = track_type;
	IF_G_FREE(ld->type_str);
	ld->type_str = g_strdup(type_str);
	IF_FREE(type_str);
	ld->playlist_id = playlist_id;

	return 0;
}

int mc_track_list_get_radio()
{
	int index = elm_radio_value_get(g_radio_main);
	DEBUG_TRACE("index: %d", index);

	return index;
}

