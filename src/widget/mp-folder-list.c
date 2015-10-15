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

#include "mp-folder-list.h"
#include "mp-folder-detail-view.h"
#include "mp-player-view.h"
#include "mp-select-track-view.h"
#include "mp-create-playlist-view.h"
#include "mp-ctxpopup.h"
#include "mp-popup.h"
#include "mp-util.h"
#include "mp-menu.h"
#include "mp-common.h"
#include "mp-widget.h"
#include "mp-play.h"

static const char *_get_label(void *thiz, void *event_info)
{
	MpFolderList_t *list = thiz;
	MP_CHECK_NULL(list);
	char *title = NULL;

	mp_list_item_data_t *folder =  elm_object_item_data_get(event_info);
	MP_CHECK_NULL(folder);

	mp_media_info_group_get_main_info(folder->handle, &title);
	//DEBUG_TRACE("Title is %s", title);
	return title;
}

static void
_mp_folder_list_playall_button_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();

	mp_media_info_h handle = data;
	MP_CHECK(handle);

	int count = 0;
	char *type_str = NULL;
	int ret = 0;

        mp_popup_destroy(ad);

	ret = mp_media_info_group_get_folder_id(handle, &type_str);

	MP_CHECK(type_str);

	/* get playlist data by name */
	mp_media_list_h svc_handle = NULL;

	mp_media_info_list_count(MP_TRACK_BY_FOLDER, type_str, NULL, NULL, 0, &count);
	mp_media_info_list_create(&svc_handle,
		MP_TRACK_BY_FOLDER, type_str, NULL, NULL, 0, 0, count);

	if (!ad->playlist_mgr)
		mp_common_create_playlist_mgr();

	mp_playlist_mgr_clear(ad->playlist_mgr);
	mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, svc_handle, count, 0, NULL);

        ad->paused_by_user = FALSE;
	ret = mp_play_new_file(ad, TRUE);
	if (ret)
	{
		ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK)
			mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
#endif
		IF_FREE(type_str);
		return;
	}

	if (svc_handle)
	{
		mp_media_info_list_destroy(svc_handle);
	}

	IF_FREE(type_str);
	endfunc;
}

static char *
_mp_folder_list_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *name = NULL;
	int ret = 0;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);

	mp_retv_if (svc_item == NULL, NULL);

	if (!strcmp(part, "elm.text.main.left.top") || !strcmp(part, "elm.slide.text.1"))
	{
		ret = mp_media_info_group_get_main_info(svc_item, &name);
		mp_retvm_if ((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name))
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");

		if (!strcmp(part, "elm.text.1"))
			return elm_entry_utf8_to_markup(name);
		else
			return g_strdup(name);

	}
	else if (!strcmp(part, "elm.text.sub.left.bottom"))
	{
		ret = mp_media_info_group_get_sub_info(svc_item, &name);
		mp_retvm_if ((ret != 0), NULL, "Fail to get value");
		if (!name || !strlen(name))
			name = GET_SYS_STR("IDS_COM_BODY_UNKNOWN");
		return g_strdup(name);
	}
	else if (!strcmp(part, "elm.text.3"))
	{
		int count = 0;
		mp_media_info_group_get_track_count(svc_item, &count);
		return g_strdup_printf("(%d)", count);
	}
	return NULL;
}



Evas_Object *
_mp_folder_list_icon_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *icon = NULL;
	Evas_Object *storage_icon = NULL;

	mp_list_item_data_t *item = (mp_list_item_data_t *) data;
	MP_CHECK_NULL(item);
	mp_media_info_h svc_item = (item->handle);

	Evas_Object *content = NULL;
	content = elm_layout_add(obj);

	const char *slide_part_play_all = "";
	if (item->group_type == MP_GROUP_BY_FOLDER)
		slide_part_play_all = "elm.slide.swallow.2";

	if (!strcmp(part, "elm.icon.1"))
	{
		char *thumb_name = NULL;
		mp_media_info_group_get_thumbnail_path(svc_item, &thumb_name);
#ifdef MP_FEATURE_PERSONAL_PAGE
		char *folderpath = NULL;
		mp_media_info_group_get_sub_info(svc_item, &folderpath);
		if (folderpath != NULL && g_str_has_prefix(folderpath, MP_PERSONAL_PAGE_DIR))
			icon = mp_widget_lock_icon_create(obj, (const char *)thumb_name);
		else
			icon = mp_util_create_lazy_update_thumb_icon(obj, thumb_name, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
#else
		icon = mp_util_create_lazy_update_thumb_icon(obj, thumb_name, MP_LIST_ICON_SIZE, MP_LIST_ICON_SIZE);
#endif
		elm_layout_theme_set(content, "layout", "list/B/music.type.1", "default");
		elm_layout_content_set(content, "elm.swallow.content", icon);

		char *folder = NULL;
		storage_icon = NULL;
		int ret = mp_media_info_group_get_sub_info(svc_item, &folder);
		mp_retvm_if ((ret != 0), NULL, "Fail to get value");
		if (folder) {
			const char *icon_path = NULL;
			if (g_strstr_len(folder, strlen(MP_PHONE_ROOT_PATH), MP_PHONE_ROOT_PATH))
				icon_path = MP_ICON_STORAGE_PHONE;
			else if (g_strstr_len(folder, strlen(MP_MMC_ROOT_PATH), MP_MMC_ROOT_PATH))
				icon_path = MP_ICON_STORAGE_MEMORY;
			else
				icon_path = MP_ICON_STORAGE_EXTERNAL;

			storage_icon = elm_icon_add(obj);
			MP_CHECK_NULL(storage_icon);
			elm_image_file_set(storage_icon, IMAGE_EDJ_NAME, icon_path);
			elm_layout_content_set(content, "elm.swallow.storage", storage_icon);
		}
	}

	Evas_Object *button = NULL;
	if (!strcmp(part, "elm.slide.swallow.1"))
	{
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		//elm_object_text_set(button, GET_STR(STR_MP_ADD_TO));
		//mp_language_mgr_register_object(button, OBJ_TYPE_ELM_OBJECT, NULL, STR_MP_ADD_TO);
		mp_util_domain_translatable_text_set(button, STR_MP_ADD_TO);
		evas_object_smart_callback_add(button, "clicked", mp_common_button_add_to_playlist_cb, evas_object_data_get(obj, "list_data"));
		return button;
	}
	else if (!strcmp(part, slide_part_play_all))
	{
		button = elm_button_add(obj);
		elm_object_style_set(button, "sweep");
		//elm_object_text_set(button, GET_STR(STR_MP_PLAY_ALL));
		//mp_language_mgr_register_object(button, OBJ_TYPE_ELM_OBJECT, NULL, STR_MP_PLAY_ALL);
		mp_util_domain_translatable_text_set(button, STR_MP_PLAY_ALL);
		evas_object_smart_callback_add(button, "clicked", _mp_folder_list_playall_button_cb, item->handle);
		return button;
	}
	else if (!g_strcmp0(part, "elm.icon.storage"))
	{
		char *folder = NULL;
		icon = NULL;
		int ret = mp_media_info_group_get_sub_info(svc_item, &folder);
		mp_retvm_if ((ret != 0), NULL, "Fail to get value");
		if (folder) {
			const char *icon_path = NULL;
			if (g_strstr_len(folder, strlen(MP_PHONE_ROOT_PATH), MP_PHONE_ROOT_PATH))
				icon_path = MP_ICON_STORAGE_PHONE;
			else if (g_strstr_len(folder, strlen(MP_MMC_ROOT_PATH), MP_MMC_ROOT_PATH))
				icon_path = MP_ICON_STORAGE_MEMORY;
			else
				icon_path = MP_ICON_STORAGE_EXTERNAL;

			icon = elm_icon_add(obj);
			MP_CHECK_NULL(icon);
			elm_image_file_set(icon, IMAGE_EDJ_NAME, icon_path);
			evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
			elm_layout_theme_set(content, "layout", "list/B/music.type.2", "default");
			elm_layout_content_set(content, "elm.icon.center", icon);

			return content;
		}
	}

	Evas_Object *check = NULL;

	if (elm_genlist_decorate_mode_get(obj)) {			// if edit mode
		if (!strcmp(part, "elm.icon.2")) {		// swallow checkbox or radio button
			check = elm_check_add(obj);
			elm_object_style_set(check, "genlist");
			evas_object_propagate_events_set(check, EINA_FALSE);
			evas_object_smart_callback_add(check, "changed", mp_common_view_check_changed_cb, NULL);
			elm_check_state_pointer_set(check, &item->checked);
			elm_layout_theme_set(content, "layout", "list/C/type.2", "default");
			elm_layout_content_set(content, "elm.swallow.content", check);

			return content;
		}
	}
	return content;
}

static void
_mp_folder_list_item_del_cb(void *data, Evas_Object * obj)
{
	mp_list_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static void
_mp_folder_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	MpFolderList_t *list = (MpFolderList_t *)data;
	MP_CHECK(list);
	eventfunc;
	int ret = 0;
	int index = 0;//(int)data;
	char *name = NULL;
	char *folder = NULL;
	char *title = NULL;

	MP_LIST_ITEM_IGNORE_SELECT(obj);

	Elm_Object_Item *gli = (Elm_Object_Item *) event_info;
	MP_CHECK(gli);
	elm_genlist_item_selected_set(gli, FALSE);

	DEBUG_TRACE("");
	mp_list_item_data_t *gli_data = elm_object_item_data_get(gli);
	MP_CHECK(gli_data);

	index = gli_data->index;
	if (list->edit_mode)
	{
		mp_list_edit_mode_sel((MpList_t *)list, gli_data);

		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MpView_t *view = mp_view_mgr_get_top_view(view_mgr);
		ERROR_TRACE("update options of edit view");
		mp_view_update_options_edit(view);
		ERROR_TRACE("set selected count");
		IF_FREE(folder);
		return;
	}

	DEBUG_TRACE("index is %d", index);
	if (index >=0)
	{
		DEBUG_TRACE("");
		ret = mp_media_info_group_get_main_info(gli_data->handle, &name);
		DEBUG_TRACE("");

		mp_retm_if (ret != 0, "Fail to get value");
		mp_retm_if (name == NULL, "Fail to get value");

		title = name;

		mp_media_info_group_get_folder_id(gli_data->handle, &folder);
		mp_retm_if (ret != 0, "Fail to get value");
		mp_retm_if (folder == NULL, "Fail to get value");
	}


	if (list->function_type == MP_LIST_FUNC_ADD_TRACK)
	{
		MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
		if (view_manager == NULL) {
			IF_FREE(folder);
			return;
		}
		MpSelectTrackView_t *view_select_track = mp_select_track_view_create(view_manager->navi);
		if (view_select_track == NULL) {
			IF_FREE(folder);
			return;
		}
		mp_view_mgr_push_view(view_manager, (MpView_t *)view_select_track, NULL);

		mp_view_set_title((MpView_t *)view_select_track, STR_MP_TILTE_SELECT_ITEM);
		mp_track_list_set_data((MpTrackList_t *)view_select_track->content_to_show, MP_TRACK_LIST_TYPE, MP_TRACK_BY_FOLDER, MP_TRACK_LIST_TYPE_STR, folder, -1);
		mp_list_update(view_select_track->content_to_show);
		mp_view_update_options((MpView_t *)view_select_track);
		mp_list_set_edit(view_select_track->content_to_show, TRUE);
		mp_list_view_set_cancel_btn((MpListView_t*)view_select_track, true);
		mp_list_view_set_done_btn((MpListView_t*)view_select_track, true,MP_DONE_SELECT_ADD_TRACK_TYPE);
		mp_list_view_set_select_all((MpListView_t *)view_select_track, TRUE);
		IF_FREE(folder);
		return;
	}

	/* create the view of folder detail */

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	if (view_manager == NULL) {
		IF_FREE(folder);
		return;
	}
	MpFolderDetailView_t *view_folder_detail = mp_folder_detail_view_create(view_manager->navi, folder);
	if (view_folder_detail == NULL) {
		IF_FREE(folder);
		return;
	}
	mp_view_mgr_push_view(view_manager, (MpView_t *)view_folder_detail, NULL);

	mp_view_update_options((MpView_t *)view_folder_detail);
	mp_view_set_title((MpView_t *)view_folder_detail, title);
	IF_FREE(folder);
	//mp_util_reset_genlist_mode_item(layout_data->genlist);

	//_mp_group_view_push_item_content(view_data, view_layout, title);

}


static void _mp_folder_list_load_list(void *thiz, int count)
{
	MpFolderList_t *list = thiz;
	MP_CHECK(list);

	/*media-svc related*/
	mp_media_list_h svc_handle;

	/*clear genlist*/
	Elm_Object_Item *item = elm_genlist_first_item_get(list->genlist);
	if (item)
	{
		elm_genlist_item_bring_in(item, ELM_GENLIST_ITEM_SCROLLTO_IN);
		elm_genlist_clear(list->genlist);
	}

	gint index = 0;
	int ret = 0;

	DEBUG_TRACE("count: %d", count);

	if (count < 0)
		goto END;

	if (list->folder_list)
	{
		mp_media_info_group_list_destroy(list->folder_list);
	}

	ret = mp_media_info_group_list_create(&list->folder_list, MP_GROUP_BY_FOLDER, list->type_str, list->filter_str, 0, count);

	if (ret != 0)
	{
		DEBUG_TRACE("Fail to get items");
		goto END;
	}


	svc_handle = list->folder_list ;

	for (index = 0; index < count; index++)
	{
		mp_media_info_h item = NULL;
		char *title = NULL;
		mp_list_item_data_t *item_data = NULL;

		item = mp_media_info_group_list_nth_item(svc_handle, index);
		if (!item)
		{
			DEBUG_TRACE("Fail to mp_media_info_group_list_nth_item, ret[%d], index[%d]", ret, index);
			goto END;
		}
		mp_media_info_group_get_main_info(item, &title);
#ifdef MP_FEATURE_PERSONAL_PAGE
		char *path = NULL;
		mp_media_info_group_get_sub_info(item, &path);
		if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_NONE)
			goto append_folder_items;

		if (mp_util_is_in_personal_page((const char *)path))
		{
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_ADD)
				continue;
		}
		else
		{
			if (list->personal_page_type == MP_LIST_PERSONAL_PAGE_REMOVE)
				continue;
		}
append_folder_items:
#endif
		item_data = calloc(1, sizeof(mp_list_item_data_t));
		MP_CHECK(item_data);
		item_data->handle = item;
		item_data->group_type = list->group_type;
		item_data->index = index;
                item_data->checked = mp_list_is_in_checked_path_list(list->checked_path_list, title);

		Elm_Object_Item *parent_group = NULL;
		item_data->it = elm_genlist_item_append(list->genlist, list->itc, item_data, parent_group,
								    ELM_GENLIST_ITEM_NONE, _mp_folder_select_cb, (void *)list);
		elm_object_item_data_set(item_data->it, item_data);
	}

/*
	if (count > load_count && layout_data->view_mode != MP_VIEW_MODE_SEARCH)
	{
		if (!layout_data->load_item_idler)
			layout_data->load_item_idler = ecore_idler_add(_mp_view_layout_load_item_idler_cb, layout_data);
	}

	if (layout_data->edit_mode
	    || (layout_data->ad->b_add_tracks && layout_data->view_data->view_type == MP_VIEW_TYPE_SONGS))
	{
		if (!layout_data->select_all_layout)
		{
			_mp_view_layout_create_select_all(layout_data);
		}
		else
		{
			if (layout_data->select_all_checkbox)
				elm_check_state_set(layout_data->select_all_checkbox, false);
		}

		elm_genlist_decorate_mode_set(layout_data->genlist, EINA_TRUE);
		elm_genlist_select_mode_set(layout_data->genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	}
	else
	{
		if (layout_data->select_all_layout)
		{
			evas_object_del(layout_data->select_all_layout);
			layout_data->select_all_layout = NULL;
		}
		elm_genlist_decorate_mode_set(layout_data->genlist, EINA_FALSE);
		elm_genlist_select_mode_set(layout_data->genlist, ELM_OBJECT_SELECT_MODE_DEFAULT);
	}

	if (layout_data->reorder && layout_data->playlist_id > 0)	// reordering of favorite list is not allowed..
		elm_genlist_reorder_mode_set(layout_data->genlist, EINA_TRUE);
	else
		elm_genlist_reorder_mode_set(layout_data->genlist, EINA_FALSE);
*/
END:
	endfunc;
}

void _mp_folder_list_destory_cb(void *thiz)
{
	eventfunc;
	MpFolderList_t *list = thiz;
	MP_CHECK(list);

	if (list->folder_list)
		mp_media_info_group_list_destroy(list->folder_list);

	if (list->itc) {
		elm_genlist_item_class_free(list->itc);
		list->itc = NULL;
	}

	IF_FREE(list->type_str);
	IF_FREE(list->filter_str);
        mp_list_free_checked_path_list(list->checked_path_list);

	IF_FREE(list);
}

static mp_group_type_e _mp_folder_list_get_group_type(void *thiz)
{
	MpFolderList_t *list = thiz;
	MP_CHECK_VAL(list, MP_GROUP_NONE);
	return MP_GROUP_BY_FOLDER;
}

/*static void
_mp_floder_list_item_longpressed_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	MpFolderList_t *list = (MpFolderList_t*)data;
	MP_CHECK(list);

        struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Elm_Object_Item *item = event_info;
	MP_CHECK(item);

	char *title = NULL;
	char *path = NULL;
	int pop_item_count = 3;
	Elm_Object_Item *temp = NULL;
	Evas_Object *popup = NULL;
	mp_list_item_data_t *item_data = NULL;

        if (list->scroll_drag_status || list->edit_mode == 1)
                return;

        temp = elm_genlist_first_item_get(list->genlist);
	while (temp) {
                item_data = elm_object_item_data_get(temp);
                item_data->checked = false;
		temp = elm_genlist_item_next_get(temp);
	}

	item_data = elm_object_item_data_get(item);
	MP_CHECK(item_data);

	item_data->checked = true;

	mp_media_info_group_get_main_info(item_data->handle, &title);
	mp_media_info_group_get_sub_info(item_data->handle, &path);

	popup = mp_genlist_popup_create(obj, MP_POPUP_LIST_LONGPRESSED, &pop_item_count, ad);
	MP_CHECK(popup);

        char *up_title = g_strdup(title);

        elm_object_part_text_set(popup, "title,text", up_title);
        IF_FREE(up_title);

        mp_genlist_popup_item_append(popup, STR_MP_PLAY_ALL, NULL, NULL, NULL,
                                     _mp_folder_list_playall_button_cb, item_data->handle);
        mp_genlist_popup_item_append(popup, STR_MP_ADD_TO_PLAYLIST, NULL, NULL, NULL,
                                     mp_common_list_add_to_playlist_cb, list);
        mp_genlist_popup_item_append(popup, STR_MP_DELETE, NULL, NULL, NULL,
                                     mp_common_list_delete_cb, list);

#ifdef MP_FEATURE_PERSONAL_PAGE
	if (mp_util_is_personal_page_on())
	{
		if (mp_util_is_in_personal_page((const char *)path))
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

	MP_GENLIST_ITEM_LONG_PRESSED(obj, popup, event_info);
}*/



void _mp_folder_list_update(void *thiz)
{
	startfunc;
	int count = 0, res = 0;
	MpFolderList_t *list = thiz;
	MP_CHECK(list);

	res = mp_media_info_group_list_count(MP_GROUP_BY_FOLDER, NULL, list->filter_str, &count);
	MP_CHECK(res == 0);

        mp_list_free_checked_path_list(list->checked_path_list);
        list->checked_path_list = mp_list_get_checked_path_list((MpList_t *)list);

	mp_evas_object_del(list->genlist);
	mp_evas_object_del(list->no_content);

	if (count)
	{
		/*create new genlist*/
		list->genlist = mp_widget_genlist_create(list->box);
		elm_scroller_policy_set(list->genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
		evas_object_size_hint_weight_set(list->genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(list->genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(list->genlist);
		/*packet genlist to box*/
		elm_box_pack_end(list->box, list->genlist);

		evas_object_data_set(list->genlist, "list_data", list);

#ifdef MP_FEATURE_ADD_TO_HOME
		const char *group_slide_style = "mode/slide";
		group_slide_style = "mode/slide2";
#endif
		if (!list->itc)
		{
			list->itc = elm_genlist_item_class_new();
			if (list->itc) {
				list->itc->item_style = "2line.top";
				//list->itc->decorate_all_item_style = "musiclist/edit_default";
				//list->itc->decorate_item_style = folder_slide_style;
				list->itc->func.text_get = _mp_folder_list_label_get;
				list->itc->func.content_get = _mp_folder_list_icon_get;
				list->itc->func.del = _mp_folder_list_item_del_cb;
			}
		}

		evas_object_smart_callback_add(list->genlist, "drag,start,left", list->flick_left_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,right", list->flick_right_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,stop", list->flick_stop_cb, NULL);

		evas_object_smart_callback_add(list->genlist, "drag,start,right", list->mode_right_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,left", list->mode_left_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,up", list->mode_cancel_cb, NULL);
		evas_object_smart_callback_add(list->genlist, "drag,start,down", list->mode_cancel_cb, NULL);
		//evas_object_smart_callback_add(list->genlist, "longpressed", _mp_floder_list_item_longpressed_cb, list);
		evas_object_smart_callback_add(list->genlist, "scroll,drag,start", list->drag_start_cb, list);
		evas_object_smart_callback_add(list->genlist, "scroll,drag,stop", list->drag_stop_cb, list);
		/* load list */
		_mp_folder_list_load_list(thiz, count);
		list->show_fastscroll(list);

		//mp_list_bottom_counter_item_append((MpList_t *)list);
	}
	else
	{
		DEBUG_TRACE("count is 0");
		list->no_content = mp_widget_no_contents_default_add(list->box, STR_MP_NO_FOLDERS, STR_MP_AFTER_YOU_DOWNLOAD_TRACKS_FOLDER_WILL_BE_SHOWN);
		list->hide_fastscroll(list);
		elm_box_pack_end(list->box, list->no_content);
	}

}

static char *_mp_folder_list_bottom_counter_text_cb(void *thiz)
{
	MpFolderList_t *list = thiz;
	MP_CHECK_NULL(list);

	unsigned int count = mp_list_get_editable_count((MpList_t *)list, mp_list_get_edit_type((MpList_t *)list));

	char *text = NULL;
	if (count == 1)
		text = g_strdup(GET_STR(STR_MP_1_FOLDER));
	else
		text = g_strdup_printf(GET_STR(STR_MP_PD_FOLDERS), count);

	return text;
}

MpFolderList_t * mp_folder_list_create(Evas_Object *parent)
{
	eventfunc;
	MP_CHECK_NULL(parent);

	MpFolderList_t *list = calloc(1, sizeof(MpFolderList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_GROUP);

	list->update = _mp_folder_list_update;
	list->destory_cb = _mp_folder_list_destory_cb;
	list->get_group_type = _mp_folder_list_get_group_type;
	list->get_label = _get_label;
	list->bottom_counter_text_get_cb = _mp_folder_list_bottom_counter_text_cb;
	return list;
}

void mp_folder_list_set_data(MpFolderList_t *list, ...)
{
	startfunc;
	MP_CHECK(list);

	va_list var_args;
	int field;

	va_start(var_args, list);
	do
	{
		field = va_arg(var_args, int);
		switch (field)
		{
		case MP_FOLDER_LIST_TYPE:
			{
				int val = va_arg((var_args), int);

				list->group_type = val;
				DEBUG_TRACE("list->group_type = %d", list->group_type);
				break;
			}
		case MP_FOLDER_LIST_FUNC:
			{
				int val = va_arg((var_args), int);

				list->function_type = val;
				DEBUG_TRACE("list->function_type = %d", list->function_type);
				break;
			}
		case MP_FOLDER_LIST_TYPE_STR:
			{
				char *val = va_arg((var_args), char *);
				SAFE_FREE(list->type_str);
				list->type_str = g_strdup(val);
				DEBUG_TRACE("list->type_str = %s", list->type_str);

				break;
			}
		case MP_FOLDER_LIST_FILTER_STR:
			{
				char *val = va_arg((var_args), char *);
				SAFE_FREE(list->filter_str);
				list->filter_str = g_strdup(val);
				DEBUG_TRACE("list->filter_str = %s", list->filter_str);

				break;
			}

		default:
			DEBUG_TRACE("Invalid arguments");
		}

	}
	while (field >= 0);

	va_end(var_args);
}

void mp_folder_list_copy_data(MpFolderList_t*src, MpFolderList_t *dest)
{
	dest->group_type = src->group_type;
	SAFE_FREE(dest->type_str);
	dest->type_str = g_strdup(src->type_str);
	dest->filter_str = g_strdup(src->filter_str);
}

