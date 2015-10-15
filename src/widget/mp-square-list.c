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

#include "mp-square-playlist-view.h"
#include "mp-square-list.h"
#include "mp-util.h"
#include "mp-play.h"
#include "mp-volume.h"
#include "mp-player-view.h"
#include "mp-playlist-mgr.h"
#include "mp-widget.h"
#include "mp-common.h"

static char *
_mp_square_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);

	MpSquareList_t *list = evas_object_data_get(obj, "list_data");
	MP_CHECK_NULL(list);

	mp_media_info_h track = (mp_media_info_h) (item->handle);
	mp_retvm_if (!track, NULL, "data is null");

	if (!strcmp(part, "elm.text.1") || !strcmp(part, "elm.slide.text.1"))
	{
		char *title = NULL;

		mp_media_info_get_title(track,  &title);

		mp_retv_if (!title, NULL);
		if (!strcmp(part, "elm.text.1"))
		{
			char *markup = NULL;
			struct appdata *ad = mp_util_get_appdata();
                        mp_track_info_t* current = ad->current_track_info;
                        char *uri = NULL;

                        mp_media_info_get_file_path(track, &uri);
                        mp_retv_if (!uri, NULL);

                        bool match = false;
                        if (current && !g_strcmp0(current->uri, uri) && list->edit_mode == 0)
                                match = true;

                        if (match)
		        {
			        char *markup_title = elm_entry_utf8_to_markup(title);
			        markup = g_strdup_printf("<match>%s</match>", markup_title);
			        IF_FREE(markup_title);
		        }
			else {
				markup = elm_entry_utf8_to_markup(title);
			}
			return markup;
		}
		else
			return g_strdup(title);
	}
	else if (!strcmp(part, "elm.text.2"))
	{
		char *artist = NULL;

		mp_media_info_get_artist(track, &artist);
		mp_retv_if (!artist, NULL);
		return g_strdup(artist);
	}
	else if (!strcmp(part, "elm.text.3") )
	{
		int duration;
		char time[16] = "";

		mp_media_info_get_duration(track, &duration);

		mp_util_format_duration(time, duration);
		time[15] = '\0';
		return g_strdup(time);
	}
	return NULL;
}

static Evas_Object *
_mp_square_list_content_get(void *data, Evas_Object * obj, const char *part)
{
	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);

	mp_media_info_h track = item->handle;
	mp_retvm_if (!track, NULL, "data is null");

	Evas_Object *content = NULL;
	if (!g_strcmp0(part, "elm.icon"))
	{
		char *thumbpath = NULL;

		mp_media_info_get_thumbnail_path(track, &thumbpath);
		content = mp_util_create_lazy_update_thumb_icon(obj, thumbpath, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
	}
	else if (!g_strcmp0(part, "elm.edit.icon.1"))
	{		// swallow checkbox or radio button
		content = elm_check_add(obj);
		elm_object_style_set(content, "default/genlist");
		elm_check_state_pointer_set(content, &item->checked);
		//evas_object_smart_callback_add(content, "changed", mp_common_genlist_checkbox_sel_cb, item);

	}

	return content;
}

static void
_mp_square_list_item_del_cb(void *data, Evas_Object *obj)
{
	mp_list_item_data_t *item_data = data;
	MP_CHECK(item_data);

	if (item_data->handle) {
		mp_media_info_destroy(item_data->handle);

		item_data->handle = NULL;
	}

	free(item_data);
}


static char *
_mp_square_list_shuffle_text_get(void *data, Evas_Object *obj, const char *part)
{
        startfunc;
        char *result = NULL;
        struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);
	MP_CHECK_NULL(ad->playlist_mgr);

	int count = mp_playlist_mgr_count(ad->playlist_mgr);
	if (!strcmp(part, "elm.text"))
	{
		MpSquareList_t *list = evas_object_data_get(obj, "list_data");
		MP_CHECK_NULL(list);
		result = (count == 1) ? g_strdup(GET_STR(STR_MP_SHUFFLE_1_TRACK)) : g_strdup_printf(GET_STR(STR_MP_SHUFFLE_PD_TRACKS), count);
	}
	return result;

}

Evas_Object *
_mp_square_list_shuffle_icon_get(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.icon.2")) {
		Evas_Object *icon;
		icon = mp_util_create_image(obj, IMAGE_EDJ_NAME, MP_ICON_SHUFFLE, MP_LIST_SHUFFLE_ICON_SIZE, MP_LIST_SHUFFLE_ICON_SIZE);
		evas_object_color_set(icon, 122, 122, 122, 255);
		return icon;
	}
	return NULL;
}


static void
_mp_square_list_shuffle_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	elm_genlist_item_selected_set(gli, FALSE);

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	MpList_t *list = data;
	MP_CHECK(list);

	mp_list_item_data_t *item = (mp_list_item_data_t *) elm_object_item_data_get(gli);
	MP_CHECK(item);
	mp_play_control_shuffle_set(NULL, true);
	mp_common_play_track_list(item, obj);

	return;
}

static void
_mp_square_list_shuffle_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

void _mp_square_list_append_shuffle_item(MpSquareList_t *list)
{
	startfunc;
	MP_CHECK(list);

        list->itc_shuffle = elm_genlist_item_class_new();

	list->itc_shuffle->item_style = "music/1text.2icon.3";//"music/3text.1icon.2"
	list->itc_shuffle->func.text_get = _mp_square_list_shuffle_text_get;
        list->itc_shuffle->decorate_all_item_style = NULL;
	list->itc_shuffle->func.content_get = _mp_square_list_shuffle_icon_get;
	list->itc_shuffle->func.del = _mp_square_list_shuffle_item_del_cb;

        mp_list_item_data_t *item_data;
        item_data = calloc(1, sizeof(mp_list_item_data_t));
        MP_CHECK(item_data);
        item_data->item_type = MP_LIST_ITEM_TYPE_SHUFFLE;

        item_data->it = list->shuffle_it =  elm_genlist_item_append(list->genlist, list->itc_shuffle, item_data, NULL,
                                                            ELM_GENLIST_ITEM_NONE, _mp_square_list_shuffle_cb, list);

        endfunc;
}

void mp_square_list_update_genlist(void *thiz)
{
        startfunc;

        MP_CHECK(thiz);
        MpSquareList_t *list = thiz;
        MP_CHECK(list->genlist);

        struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

        int count = mp_playlist_mgr_count(ad->playlist_mgr);

        if (count <= 0)
        {
                mp_list_update(thiz);
        }
        else
        {
                elm_genlist_realized_items_update(list->genlist);
        }
}

static void
_mp_square_del_btn_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpSquareListview_t *view = (MpSquareListview_t *)data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

        mp_popup_destroy(ad);

	mp_square_playlist_view_remove_popup_show(view);
}

static void
_mp_square_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpSquareList_t *list = (MpSquareList_t*)data;
	MP_CHECK(list);

        struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

        MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
        MpView_t *view = mp_view_mgr_get_top_view(view_manager);
        MP_CHECK(view);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

        int pop_item_count = 5;
        char *title = NULL;
        Elm_Object_Item *temp = NULL;
        Evas_Object *popup = NULL;
        mp_list_item_data_t *item_data = NULL;

        if (list->scroll_drag_status || list->shuffle_it == item || list->edit_mode == 1)
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

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

        mp_media_info_get_title(item_data->handle, &title);

        char *up_title = g_strdup(title);

        elm_object_part_text_set(popup, "title,text", up_title);
        IF_FREE(up_title);

        mp_genlist_popup_item_append(popup, GET_STR(STR_MP_SET_AS), NULL, NULL, NULL,
                                     mp_common_list_set_as_cb, list);

        mp_genlist_popup_item_append(popup, GET_STR(STR_MP_ADD_TO_PLAYLIST), NULL, NULL, NULL,
                                     mp_common_list_add_to_playlist_cb, list);

        mp_genlist_popup_item_append(popup, GET_STR(STR_MP_FAVOURITES), NULL, NULL, NULL,
                                     mp_common_list_add_to_favorite_cb, list);

        mp_genlist_popup_item_append(popup, GET_STR(STR_MP_DELETE), NULL, NULL, NULL,
                                     _mp_square_del_btn_cb, view);

        mp_genlist_popup_item_append(popup, GET_STR(STR_MP_POPUP_MORE_INFO), NULL, NULL, NULL,
                                     mp_common_list_more_info_cb, list);

	MP_GENLIST_ITEM_LONG_PRESSED(obj, popup, event_info);
}

static void
_mp_square_genlist_sel_cb(void *data, Evas_Object * obj, void *event_info)
{
	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	elm_genlist_item_selected_set(gli, EINA_FALSE);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);
	MP_CHECK(ad->win_main);

	mp_list_item_data_t *item_data = elm_object_item_data_get(gli);
	MP_CHECK(item_data);

	MpSquareList_t *list = data;
	MP_CHECK(list);
	if (list->edit_mode)
	{
		mp_list_edit_mode_sel((MpList_t *)list, item_data);
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		mp_view_update_options_edit(view);
		return;
	}

	int index = item_data->index;
	mp_plst_item *select_plst_item = mp_playlist_mgr_get_nth(ad->playlist_mgr, index);
	mp_plst_item *current_plst_item = mp_playlist_mgr_get_current(ad->playlist_mgr);

	if (select_plst_item != current_plst_item) {
		mp_playlist_mgr_set_current(ad->playlist_mgr, select_plst_item);
		mp_play_destory(ad);
		ad->paused_by_user = FALSE;
		int ret = mp_play_new_file(ad, TRUE);
		if (ret)
		{
			ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
			if (ret == MP_PLAY_ERROR_NETWORK)
				mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
#endif
			return;
		}
	}

	MpPlayerView_t *player_view = (MpPlayerView_t *)GET_PLAYER_VIEW;
	if (player_view) {
		mp_player_view_refresh(player_view);
	}
	return;
}


static void
_mp_square_list_load_list(void *thiz, int count)
{
	MpSquareList_t *list = thiz;
	MP_CHECK(list);

	/*clear genlist*/
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	if (item)
	{
		elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
		elm_genlist_clear(list->genlist);
	}

	/*get data from playlist mgr*/
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	count = mp_playlist_mgr_count(ad->playlist_mgr);
	mp_plst_item *plst_item = NULL;

        _mp_square_list_append_shuffle_item(thiz);

	int res = 0;
	int index =0;
	for (index = 0; index < count; index++)
	{
		plst_item = mp_playlist_mgr_get_nth(ad->playlist_mgr, index);
		if (!plst_item) continue;

		mp_list_item_data_t *item_data = calloc(1, sizeof(mp_list_item_data_t));
		mp_assert(item_data);

		item_data->index = index;

		mp_media_info_h handle = NULL;
                res = mp_media_info_create_by_path(&handle, plst_item->uri);
		if (res != 0) {
			mp_error("mp_media_info_create()... [0x%x]", res);
			mp_media_info_destroy(handle);
			IF_FREE(item_data);
			continue;
		}
                if (mp_list_get_edit((MpList_t *)list)) {
                        char *file_path = NULL;
                        mp_media_info_get_file_path(handle, &file_path);
                        item_data->checked = mp_list_is_in_checked_path_list(list->checked_path_list, file_path);
                }
		item_data->handle = handle;

		item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, NULL,
									    ELM_GENLIST_ITEM_NONE, _mp_square_genlist_sel_cb, list);
	}

	endfunc;
}

static void
_mp_square_list_item_moved_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpSquareList_t *list = data;
	MP_CHECK(list);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	int index = -1;
	Elm_Object_Item *temp = elm_genlist_first_item_get(obj);
	while (temp) {
		++index;
		if (temp == item) break;
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
	if (media)
		mp_media_info_get_media_id(media, &uid);

	mp_plst_item *plst_item = mp_playlist_mgr_get_item_by_uid(ad->playlist_mgr, uid);
	if (plst_item) {
		mp_playlist_mgr_item_reorder(ad->playlist_mgr, plst_item, index);

		mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MODIFIED);
	}
}

static void
_mp_square_list_destory_cb(void *thiz)
{
	startfunc;
	MpSquareList_t *list = thiz;
	MP_CHECK(list);

	if (list->itc) {
		elm_genlist_item_class_free(list->itc);
		list->itc = NULL;
	}
        mp_list_free_checked_path_list(list->checked_path_list);
	free(list);
}

static void _mp_square_list_set_edit(void *thiz, bool edit)
{
	startfunc;
	MpSquareList_t *list = thiz;
	MP_CHECK(list);

        if (list->shuffle_it)
        {
                elm_object_item_del(list->shuffle_it);
                list->shuffle_it = NULL;
        }

        if (list->set_edit_default)
                list->set_edit_default(list, edit);

}

static void
_mp_square_list_update(void *thiz)
{
	startfunc;
	MpSquareList_t *list = thiz;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

        if (mp_list_get_edit((MpList_t *)list))
        {
                mp_list_free_checked_path_list(list->checked_path_list);
                list->checked_path_list = mp_list_get_checked_path_list((MpList_t *)list);
        }

	mp_evas_object_del(list->no_content);
	mp_evas_object_del(list->genlist);

	int count = mp_playlist_mgr_count(ad->playlist_mgr);
	if (count)
	{
		if (!list->genlist) {
			/*create new genlist*/
			list->genlist = mp_widget_genlist_create(list->box);
			elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
			evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
			elm_genlist_homogeneous_set(list->genlist, EINA_TRUE);
			evas_object_show(list->genlist);

			evas_object_data_set(list->genlist, "list_data", list);

			/*packet genlist to box*/
			elm_box_pack_end(list->box, list->genlist);

			list->itc = elm_genlist_item_class_new();
			list->itc->item_style = "music/2text.1icon.2.tb";
			//list->itc->decorate_item_style = "mode/slide4";
			list->itc->decorate_all_item_style = "musiclist/edit_default";
			list->itc->func.text_get = _mp_square_list_label_get;
			list->itc->func.content_get = _mp_square_list_content_get;
			list->itc->func.del = _mp_square_list_item_del_cb;

			evas_object_smart_callback_add(list->genlist, "moved", _mp_square_list_item_moved_cb, list);
                        evas_object_smart_callback_add(list->genlist, "longpressed", _mp_square_list_item_longpressed_cb, list);

			//evas_object_smart_callback_add(list->genlist, "drag,start,left", list->flick_left_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,right", list->flick_right_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,stop", list->flick_stop_cb, NULL);

			//evas_object_smart_callback_add(list->genlist, "drag,start,right", list->mode_right_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,left", list->mode_left_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,up", list->mode_cancel_cb, NULL);
			//evas_object_smart_callback_add(list->genlist, "drag,start,down", list->mode_cancel_cb, NULL);
		}

		/* load list */
		_mp_square_list_load_list(thiz, count);
                if (mp_list_get_editable_count((MpList_t *)list, MP_LIST_EDIT_TYPE_NORMAL) <= 1)//only shuffle item
                {
                        elm_genlist_clear(list->genlist);
                        mp_evas_object_del(list->genlist);
                        list->shuffle_it = NULL;
                        DEBUG_TRACE("count is 0");
                        list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_NORMAL, NULL, NULL);
                        elm_box_pack_end(list->box, list->no_content);
                }

	}
	else
	{
		DEBUG_TRACE("count is 0");
		list->no_content = mp_widget_create_no_contents(list->box, MP_NOCONTENT_NORMAL, NULL, NULL);
		elm_box_pack_end(list->box, list->no_content);
	}

}

static mp_track_type_e _mp_square_list_get_track_type(void *thiz)
{
	return MP_TRACK_BY_SQUARE;
}

MpSquareList_t *
mp_square_list_create(Evas_Object *parent)
{
	startfunc;
	MP_CHECK_NULL(parent);

	MpSquareList_t *list = calloc(1, sizeof(MpSquareList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_TRACK);

	list->update = _mp_square_list_update;
	list->destory_cb = _mp_square_list_destory_cb;
        list->set_edit_default = list->set_edit;
        list->set_edit = _mp_square_list_set_edit;
	list->get_track_type = _mp_square_list_get_track_type;

	list->reorderable = TRUE;

	return list;
}

void
mp_square_list_set_data(MpSquareList_t *list, ...)
{
	MP_CHECK(list);

	va_list var_args;
	int field;

	va_start(var_args, list);
	do
	{
		field = va_arg(var_args, int);
		DEBUG_TRACE("field is %d", field);

		switch (field)
		{
		case MP_SQUARE_LIST_ATTR_HIGHLIGHT_CURRENT:
			{
				int val = va_arg((var_args), int);

				list->highlight_current = val;
				DEBUG_TRACE("list->highlight_current = %d", list->highlight_current);
				break;
			}
		default:
			DEBUG_TRACE("Invalid arguments");
		}

	}
	while (field >= 0);

	va_end(var_args);
}

void
mp_square_list_remove_selected_item(MpSquareList_t *list)
{
	startfunc;
	MP_CHECK(list);
	MP_CHECK(list->genlist);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	bool square_changed = false;
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
				square_changed = true;
			}
			mp_playlist_mgr_item_remove_item(ad->playlist_mgr, remove_item);
			elm_object_item_del(item_data->it);
		}
	}

	if (square_changed) {
		mp_play_destory(ad);
		ad->paused_by_user = FALSE;
		mp_playlist_mgr_set_current(ad->playlist_mgr, current_plst);
		int ret = mp_play_new_file(ad, true);
                if (ret)
		{
			ERROR_TRACE("Fail to play new file");
#ifdef MP_FEATURE_CLOUD
			if (ret == MP_PLAY_ERROR_NETWORK)
				mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
                        return;
#endif
		}
		mp_view_mgr_post_event(ad->view_manager, MP_UPDATE_NOW_PLAYING);
	}
}

void
mp_square_list_refresh(MpSquareList_t *list)
{
	MP_CHECK(list);
	MP_CHECK(list->genlist);

	elm_genlist_realized_items_update(list->genlist);
}

void
mp_square_list_current_item_bring_in(MpSquareList_t *list)
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
				elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_MIDDLE);
				break;
			}
		}

		item = elm_genlist_item_next_get(item);
	}

	endfunc;
}

