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

#include "mp-now-playing-list.h"
#include "mp-util.h"
#include "mp-play.h"
#include "mp-volume.h"
#include "mp-player-view.h"
#include "mp-playlist-mgr.h"
#include "mp-widget.h"
#include "mp-common.h"
#include "mp-file-util.h"
#include "mp-player-mgr.h"

#include <player.h>

static char *
_mp_now_playing_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);

	MpNowPlayingList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	mp_media_info_h track = (mp_media_info_h)(item->handle);
	mp_retvm_if(!track, NULL, "data is null");

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	mp_plst_item * current = mp_playlist_mgr_get_current(ad->playlist_mgr);

	if (!strcmp(part, "elm.text") || !strcmp(part, "elm.text.sub")) {
		char *title = NULL;
		if (!strcmp(part, "elm.text")) {
			mp_media_info_get_title(track,  &title);
		} else {
			mp_media_info_get_artist(track, &title);
		}
		mp_retv_if(!title, NULL);

		char *markup = NULL;
		static char result[DEF_STR_LEN + 1] = { 0, };
		if (list->highlight_current && current == item->plst_item) {
			char *info = elm_entry_utf8_to_markup(title);

			int r, g, b, a;
			//Apply RGB equivalent of color
			r = 21;
			g = 108;
			b = 148;
			a = 255;
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
	return NULL;
}

static Evas_Object *
_mp_now_playing_list_content_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);

	mp_media_info_h track = item->handle;
	mp_retvm_if(!track, NULL, "data is null");

	Evas_Object *content = NULL;
	content = elm_layout_add(obj);

	Evas_Object *icon = NULL;

	//get player status
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	mp_plst_item * current = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_NULL(current);
	char *uri = NULL;
	mp_media_info_get_file_path(track, &uri);
	mp_retv_if(!uri, NULL);

	Evas_Object *part_content = elm_object_item_part_content_get(item->it, "elm.icon.1");

	if (part_content) {
		elm_object_signal_emit(part_content, "show_default", "*");
	}

	bool match = false;
	if (current && (current == item->plst_item)) {
		match = true;
	}

	if (match && part_content) {
		ERROR_TRACE("set state: %d", (int)mp_player_mgr_get_state());
		if ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PLAYING) {
			elm_object_signal_emit(part_content, "show_play", "*");
		} else if (((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PAUSED) || ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_READY)) {
			elm_object_signal_emit(part_content, "show_pause", "*");
		}
	}

	if (!g_strcmp0(part, "elm.swallow.icon")) {
		char *thumbpath = NULL;

		mp_media_info_get_thumbnail_path(track, &thumbpath);
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);

		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		if (match && content) {
			if ((int)mp_player_mgr_get_state() == (int)PLAYER_STATE_PLAYING) {
				elm_object_signal_emit(content, "show_play", "*");
			} else {
				elm_object_signal_emit(content, "show_pause", "*");
			}
		}
	} else if (!g_strcmp0(part, "elm.edit.icon.1")) {
		// swallow checkbox or radio button
		content = elm_check_add(obj);
		elm_object_style_set(content, "default/genlist");
		elm_check_state_pointer_set(content, &item->checked);
		//evas_object_smart_callback_add(content, "changed", mp_common_genlist_checkbox_sel_cb, item);

	}

	return content;
}

static void
_mp_now_playing_list_item_del_cb(void *data, Evas_Object *obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);

	if (item_data->handle) {
		mp_media_info_destroy(item_data->handle);

		item_data->handle = NULL;
	}

	free(item_data);
}

static Eina_Bool
_mp_now_playing_genlist_sel_timer_cb(void *data)
{
	MpNowPlayingList_t *list = data;
	MP_CHECK_FALSE(list);
	MP_CHECK_FALSE(list->genlist);

	list->sel_idler = NULL;

	Elm_Object_Item *gl_item = elm_genlist_first_item_get(list->genlist);
	while (gl_item) {
		elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
		gl_item = elm_genlist_item_next_get(gl_item);
	}
	return ECORE_CALLBACK_DONE;
}


static void
_mp_now_playing_genlist_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);

	elm_genlist_item_selected_set(gli, EINA_FALSE);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);
	MP_CHECK(ad->win_main);

	mp_list_item_data_t *item_data = elm_object_item_data_get(gli);
	MP_CHECK(item_data);

	MpNowPlayingList_t *list = data;
	MP_CHECK(list);
	if (list->edit_mode) {
		mp_list_edit_mode_sel((MpList_t *)list, item_data);
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		mp_view_update_options_edit(view);
		return;
	}

	mp_plst_item *select_plst_item = item_data->plst_item;
	mp_plst_item *current_plst_item = mp_playlist_mgr_get_current(ad->playlist_mgr);

	if (select_plst_item != current_plst_item) {
		mp_playlist_mgr_set_current(ad->playlist_mgr, select_plst_item);
		mp_play_destory(ad);
		ad->paused_by_user = FALSE;
		int ret = mp_play_new_file(ad, TRUE);
		if (ret) {
			ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
			if (ret == MP_PLAY_ERROR_NETWORK) {
				mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
			}
#endif
			return;
		}
	} else {
		/*if click the current track, it should chang the playing status.
		If at the beginning, should play the song*/
		if (ad->player_state == PLAY_STATE_PLAYING) {
			mp_play_control_play_pause(ad, false);
		} else if (ad->player_state == PLAY_STATE_PAUSED) {
			mp_play_control_play_pause(ad, true);
		} else {
			ad->paused_by_user = FALSE;
			mp_play_new_file(ad, TRUE);
		}
	}

	MpPlayerView_t *player_view = (MpPlayerView_t *)GET_PLAYER_VIEW;
	if (player_view) {
		mp_player_view_refresh(player_view);
	}

	Elm_Object_Item *gl_item = elm_genlist_first_item_get(list->genlist);
	while (gl_item) {
		elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		gl_item = elm_genlist_item_next_get(gl_item);
	}
	mp_ecore_idler_del(list->sel_idler);
	list->sel_idler = ecore_idler_add(_mp_now_playing_genlist_sel_timer_cb, list);

	return;
}

static void
_mp_now_playing_list_load_list(void *thiz, int count)
{
	MpNowPlayingList_t *list = thiz;
	MP_CHECK(list);

	/*clear genlist*/
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	if (item) {
		elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
		elm_genlist_clear(list->genlist);
	}

	/*get data from playlist mgr*/
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	count = mp_playlist_mgr_count(ad->playlist_mgr);
	mp_plst_item *plst_item = NULL;

	int res = 0;
	int index = 0;
	for (index = 0; index < count; index++) {
		plst_item = mp_playlist_mgr_normal_list_get_nth(ad->playlist_mgr, index);
		if (!plst_item) {
			continue;
		}

		mp_list_item_data_t *item_data = calloc(1, sizeof(mp_list_item_data_t));
		mp_assert(item_data);

		item_data->index = index;
		item_data->plst_item = plst_item;

		mp_media_info_h handle = NULL;
		res = mp_media_info_create_by_path(&handle, plst_item->uri);
		if (res != 0) {
			mp_error("mp_media_info_create()... [0x%x]", res);
			mp_media_info_destroy(handle);
			IF_FREE(item_data);
			continue;
		}
		item_data->handle = handle;

		item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
		                                        ELM_GENLIST_ITEM_NONE, _mp_now_playing_genlist_sel_cb, list);
	}

	endfunc;
}

static void
_mp_now_playing_list_item_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpNowPlayingList_t *list = data;
	MP_CHECK(list);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	int index = -1;
	Elm_Object_Item *temp = elm_genlist_first_item_get(obj);
	while (temp) {
		++index;
		if (temp == item) {
			break;
		}
		temp = elm_genlist_item_next_get(temp);
	}
	mp_debug("reordered index = %d", index);
	MP_CHECK(index >= 0);

	mp_list_item_data_t *item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	char *uid = NULL;
	mp_media_info_h media = item_data->handle;
	if (media) {
		mp_media_info_get_media_id(media, &uid);
	}

	mp_plst_item *plst_item = mp_playlist_mgr_get_item_by_uid(ad->playlist_mgr, uid);
	if (plst_item) {
		mp_playlist_mgr_item_reorder(ad->playlist_mgr, plst_item, index);

		mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MODIFIED);
	}
}

static void
_mp_now_playing_list_destory_cb(void *thiz)
{
	startfunc;
	MpNowPlayingList_t *list = thiz;
	MP_CHECK(list);

	if (list->itc) {
		elm_genlist_item_class_free(list->itc);
		list->itc = NULL;
	}

	mp_ecore_timer_del(list->loading_timer);
	mp_ecore_idler_del(list->sel_idler);

	free(list);
}

static void
_mp_now_playing_list_update(void *thiz)
{
	startfunc;
	MpNowPlayingList_t *list = thiz;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	mp_ecore_timer_del(list->loading_timer);
	mp_evas_object_del(list->loading_progress);

	mp_evas_object_del(list->no_content);
	mp_evas_object_del(list->genlist);

	int count = mp_playlist_mgr_count(ad->playlist_mgr);
	if (count) {
		if (!list->genlist) {
			/*create new genlist*/
			list->genlist = mp_widget_genlist_create(list->box);
			MP_CHECK(list->genlist);
			elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
			evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_genlist_homogeneous_set(list->genlist, EINA_TRUE);
			elm_genlist_mode_set(list->genlist, ELM_LIST_COMPRESS);
			evas_object_show(list->genlist);

			evas_object_data_set(list->genlist, "list_data", list);

			/*packet genlist to box*/
			elm_box_pack_end(list->box, list->genlist);

			list->itc = elm_genlist_item_class_new();
			MP_CHECK(list->itc);
			list->itc->item_style = "type1";
			list->itc->func.text_get = _mp_now_playing_list_label_get;
			list->itc->func.content_get = _mp_now_playing_list_content_get;
			list->itc->func.del = _mp_now_playing_list_item_del_cb;

			evas_object_smart_callback_add(list->genlist, "moved", _mp_now_playing_list_item_moved_cb, list);

			//evas_object_smart_callback_add(list->genlist, "drag,start,left", list->flick_left_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,right", list->flick_right_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,stop", list->flick_stop_cb, NULL);

			//evas_object_smart_callback_add(list->genlist, "drag,start,right", list->mode_right_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,left", list->mode_left_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,up", list->mode_cancel_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,down", list->mode_cancel_cb, NULL);
		}

		/* load list */
		_mp_now_playing_list_load_list(thiz, count);

	} else {
		DEBUG_TRACE("count is 0");
		list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_NORMAL, NULL, NULL);
		elm_box_pack_end(list->box, list->no_content);
	}

}

static Eina_Bool
_mp_now_playing_list_loading_timer_cb(void *data)
{
	MpNowPlayingList_t *list = data;
	MP_CHECK_FALSE(list);

	list->loading_timer = NULL;

	mp_evas_object_del(list->loading_progress);
	list->loading_progress = mp_widget_loading_icon_add(list->box, MP_LOADING_ICON_SIZE_XLARGE);
	elm_box_pack_end(list->box, list->loading_progress);

	return ECORE_CALLBACK_DONE;
}

MpNowPlayingList_t *
mp_now_playing_list_create(Evas_Object *parent)
{
	startfunc;
	MP_CHECK_NULL(parent);

	MpNowPlayingList_t *list = calloc(1, sizeof(MpNowPlayingList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_TRACK);

	list->update = _mp_now_playing_list_update;
	list->destory_cb = _mp_now_playing_list_destory_cb;

	list->reorderable = TRUE;

	list->loading_timer = ecore_timer_add(0.1, _mp_now_playing_list_loading_timer_cb, list);

	return list;
}

void
mp_now_playing_list_set_data(MpNowPlayingList_t *list, ...)
{
	MP_CHECK(list);

	va_list var_args;
	int field;

	va_start(var_args, list);
	do {
		field = va_arg(var_args, int);
		DEBUG_TRACE("field is %d", field);

		switch (field) {
		case MP_NOW_PLAYING_LIST_ATTR_HIGHLIGHT_CURRENT: {
			int val = va_arg((var_args), int);

			list->highlight_current = val;
			DEBUG_TRACE("list->highlight_current = %d", list->highlight_current);
			break;
		}
		default:
			DEBUG_TRACE("Invalid arguments");
		}

	} while (field >= 0);

	va_end(var_args);
}

void
mp_now_playing_list_remove_selected_item(MpNowPlayingList_t *list)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->genlist);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	bool now_playing_changed = false;
	mp_plst_item *current_plst = mp_playlist_mgr_get_current(ad->playlist_mgr);
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	while (item) {
		mp_list_item_data_t *item_data = elm_object_item_data_get(item);
		item = elm_genlist_item_next_get(item);

		if (item_data && item_data->checked && item_data->handle) {
			char *uid = NULL;
			mp_media_info_get_media_id(item_data->handle, &uid);
			mp_plst_item *remove_item = mp_playlist_mgr_get_item_by_uid(ad->playlist_mgr, uid);
			if (remove_item == current_plst) {
				WARN_TRACE("remove current play list item");
				current_plst = mp_playlist_mgr_get_next(ad->playlist_mgr, EINA_FALSE, false);
				now_playing_changed = true;
			}
			mp_playlist_mgr_item_remove_item(ad->playlist_mgr, remove_item);
			elm_object_item_del(item_data->it);
		}
	}

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MGR_ITEM_CHANGED);

	if (now_playing_changed) {
		mp_play_destory(ad);
		ad->paused_by_user = FALSE;
		mp_playlist_mgr_set_current(ad->playlist_mgr, current_plst);
		int ret = mp_play_new_file(ad, true);
		if (ret) {
			ERROR_TRACE("Fail to play new file");
#ifdef MP_FEATURE_CLOUD
			if (ret == MP_PLAY_ERROR_NETWORK) {
				mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
			}
			return;
#endif
		}
	}
}

void
mp_now_playing_list_refresh(MpNowPlayingList_t *list)
{
	MP_CHECK(list);
	MP_CHECK(list->genlist);

	bool mmc_removed = mp_util_is_mmc_removed();
	bool itemDeleted = false;

	Elm_Object_Item *it = NULL, *next = NULL;
	next = it = elm_genlist_first_item_get(list->genlist);;
	while (next) {
		next = elm_genlist_item_next_get(it);
		mp_list_item_data_t *item_data = elm_object_item_data_get(it);
		if (item_data) {
			mp_plst_item *plst_item = item_data->plst_item;
			if (plst_item && plst_item->uri && plst_item->track_type == MP_TRACK_URI && !mp_util_is_streaming(plst_item->uri)) {
				if (!mp_file_exists(plst_item->uri) || (mmc_removed && strstr(plst_item->uri, MP_MMC_ROOT_PATH) == plst_item->uri)) {
					WARN_TRACE("removed uri %s", plst_item->uri);
					elm_object_item_del(it);
					itemDeleted = true;
				}
			}
		}

		if (!itemDeleted) {
			elm_genlist_item_update(it);
		}
		itemDeleted = false;
		it = next;
	}
	Eina_List *realized_items = elm_genlist_realized_items_get(list->genlist);
	Eina_List *l = NULL;
	Elm_Object_Item *data_tmp = NULL;

	EINA_LIST_FOREACH(realized_items, l, data_tmp)
	elm_genlist_item_fields_update(data_tmp, "elm.text.*", ELM_GENLIST_ITEM_FIELD_TEXT);
	mp_now_playing_list_current_item_show(list);

	if (realized_items) {
		eina_list_free(realized_items);
	}
}

void
mp_now_playing_list_current_item_show(MpNowPlayingList_t *list)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->genlist);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	mp_plst_item *current_plst = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(current_plst);
	MP_CHECK(current_plst->uid);

	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	while (item) {
		mp_list_item_data_t *item_data = elm_object_item_data_get(item);
		if (item_data && item_data->handle) {
			char *uid = NULL;
			mp_media_info_get_media_id(item_data->handle, &uid);

			if (!g_strcmp0(uid, current_plst->uid)) {
				elm_genlist_item_show(item, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
				break;
			}
		}

		item = elm_genlist_item_next_get(item);
	}

	endfunc;
}

