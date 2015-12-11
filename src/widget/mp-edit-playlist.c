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

#include "mp-edit-playlist.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-edit-callback.h"
#include "mp-add-track-view.h"
#include "mp-common.h"
#include "mp-playlist-detail-view.h"
#include <efl_extension.h>

typedef struct _Item_Data {
	Elm_Object_Item *item;
	int dial;
	int title;
	Mp_Playlist_Data *mp_playlist_data;
} Item_Data;

#define mp_edit_popup_set_popup_data(obj, data) evas_object_data_set((obj), "popup_data", (data))

static void
mp_edit_playlist_rename_done_cb(void *data, Evas_Object * obj, void *event_info);

static void
_mp_edit_playlist_destory(void * thiz)
{
	eventfunc;
	Mp_Playlist_Data *mp_playlist_data = thiz;
	MP_CHECK(mp_playlist_data);

	struct appdata *ad = mp_util_get_appdata();
	ad->del_cb_invoked = 0;
	// TODO: release resource..
	IF_FREE(mp_playlist_data->adding_media_id);

	mp_evas_object_del(mp_playlist_data->popup);
	IF_FREE(mp_playlist_data->name);
	mp_ecore_timer_del(mp_playlist_data->entry_show_timer);
	mp_ecore_idler_del(mp_playlist_data->set_line_end_idler);

	free(mp_playlist_data);
}

static void
_mp_edit_playlist_create_new_done_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Mp_Playlist_Data *mp_playlist_data = data;
	IF_FREE(mp_playlist_data->oldname);

	int plst_uid = -1;

	char *converted_name = NULL;
	//Evas_Object *entry = mp_widget_editfield_entry_get(view->editfiled_new_playlist);
	//const char *name = elm_entry_entry_get(entry);

	const char *name = elm_entry_entry_get(mp_playlist_data->editfiled_entry);

	if (name == NULL || strlen(name) == 0) {
		//name = elm_object_part_text_get(view->editfiled_new_playlist, "elm.guidetext");
		name = elm_object_part_text_get(mp_playlist_data->editfiled_entry, "elm.guide");
	}
	converted_name = elm_entry_markup_to_utf8(name);

	struct appdata *ad = mp_util_get_appdata();
	mp_playlist_h playlist = NULL;
	plst_uid = mp_util_create_playlist(ad, converted_name, &playlist);
	if (plst_uid < 0) {
		mp_media_info_playlist_handle_destroy(playlist);
		IF_FREE(converted_name);
		MpView_t *view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_EDIT);
		if (view) {
			mp_view_update_options(view);
		}
		//_mp_edit_playlist_destory(mp_playlist_data);
		return;
	}

	if (mp_playlist_data->adding_list) {
		WARN_TRACE("adding list = %p", mp_playlist_data->adding_list);
		mp_edit_cb_excute_add_to_playlist(mp_playlist_data->adding_list, plst_uid, converted_name, mp_playlist_data->add_to_selected);
		goto END;
	}

	if (mp_playlist_data->adding_media_id) {
		mp_media_info_h media_info = NULL;
		char *path = NULL;
		mp_media_info_create(&media_info, mp_playlist_data->adding_media_id);
		mp_media_info_get_thumbnail_path(media_info, &path);
		if (mp_media_info_playlist_add_item(playlist, mp_playlist_data->adding_media_id, path) == 0) {
			mp_media_info_playlist_db_update(playlist);
		}

		mp_media_info_destroy(media_info);

	}

	//create playlist view
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();

	MpPlaylistDetailView_t *view_plst_detail = mp_playlist_detail_view_create(view_mgr->navi,
	        MP_TRACK_BY_PLAYLIST, converted_name, plst_uid);
	mp_view_mgr_push_view(view_mgr, (MpView_t *)view_plst_detail, NULL);

	/*post event to update all-view in playlist detail view transition finished
	A.push playlist detail view will give the zoom out effect
	B.update all view will delete the gengrid and create new
	since zoom out will cover the screen little by little,
	there will give a phase of all view is blank which is called blink
	temparory remove back key callback, to avoid quick back which transaction is not done case
	back key callback will be added in playlist detail view transaction finished event handle routine
	*/
	eext_object_event_callback_del(view_mgr->navi, EEXT_CALLBACK_BACK, eext_naviframe_back_cb);

	mp_view_update_options((MpView_t *)view_plst_detail);
	mp_view_set_title((MpView_t *)view_plst_detail, converted_name);

END:

	mp_media_info_playlist_handle_destroy(playlist);
	IF_FREE(converted_name);

	_mp_edit_playlist_destory(mp_playlist_data);
}

static void
_mp_edit_playlist_entry_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source) // When X marked button is clicked, empty entry's contents.
{
	eventfunc;

	Evas_Object *entry = (Evas_Object *)data;
	MP_CHECK(entry);

	elm_entry_entry_set(entry, "");
}

static void
_mp_edit_playlist_entry_eraser_status_set(void *obj, void *data)
{
	eventfunc;
	Mp_Playlist_Data *mp_playlist_data = data;
	MP_CHECK(mp_playlist_data);

	Evas_Object *editfield = obj;
	MP_CHECK(editfield);

	if (elm_object_focus_get(editfield)) {
		if (elm_entry_is_empty(editfield)) {
			elm_object_signal_emit(editfield, "elm,state,eraser,hide", "elm");
			if (mp_playlist_data->btn_ok) {
				elm_object_disabled_set(mp_playlist_data->btn_ok, EINA_TRUE);
			}
		} else {
			elm_object_signal_emit(editfield, "elm,state,eraser,show", "elm");
			if (mp_playlist_data->btn_ok) {
				elm_object_disabled_set(mp_playlist_data->btn_ok, EINA_FALSE);
			}
		}
	}
}

static Eina_Bool _mp_edit_playlist_entry_set_line_end(void *data)
{
	Mp_Playlist_Data *mp_playlist_data = data;
	MP_CHECK_FALSE(mp_playlist_data);
	MP_CHECK_FALSE(mp_playlist_data->editfiled_entry);

	elm_entry_cursor_line_end_set(mp_playlist_data->editfiled_entry);
	mp_playlist_data->set_line_end_idler = NULL;
	mp_playlist_data->set_to_end = false;

	return ECORE_CALLBACK_CANCEL;
}

static bool __mp_rename_ctrl_check_valid_text(const char *text, int *nLen)
{
	if (!text) {
		return FALSE;
	}
	if (text[0] == '.') {
		ERROR_TRACE("Invalid starting dot character");
		return FALSE;
	}

	char invalid_chars[] = { '/', '\\', ':', '*', '?', '"', '<', '>', '|', '\0' };
	char *ptr = invalid_chars;

	while (*ptr != '\0') {
		if (strchr(text, (*ptr)) != NULL) {
			ERROR_TRACE("Invalid text=%s char=%c", text, *ptr);
			return false;
		}
		++ptr;
	}

	return true;
}

static void
_mp_edit_playlist_entry_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();

	Evas_Object *editfield = obj;
	MP_CHECK(editfield);
	Mp_Playlist_Data *mp_playlist_data = data;

	char *text = NULL;
	text = mp_util_isf_get_edited_str(editfield, TRUE);

	if (text) {
		int length = strlen(text);
		if (length > 0) {
			elm_object_signal_emit(mp_playlist_data->layout, "image,enable,1", "*");
		} else {
			elm_object_signal_emit(mp_playlist_data->layout, "image,disable,1", "*");
		}
	}
	_mp_edit_playlist_entry_eraser_status_set(editfield, data);

	char *name = (char *)elm_entry_entry_get(mp_playlist_data->editfiled_entry);
	char *szFileName = elm_entry_markup_to_utf8(name);

	int nDstLen = 0;
	if (!mp_util_is_playlist_name_valid(szFileName)) {
		elm_object_disabled_set(mp_playlist_data->btn_ok, TRUE);
		IF_FREE(text);
		IF_FREE(szFileName);
		return;
	}

	char * popup_txt = "Invalid Character";
	if (!__mp_rename_ctrl_check_valid_text(szFileName, &nDstLen)) {
		elm_object_disabled_set(mp_playlist_data->btn_ok, TRUE);
		if (strlen(szFileName) == 1) {
			elm_entry_entry_set(mp_playlist_data->editfiled_entry, "");
			mp_playlist_data->oldname = "";
			mp_util_post_status_message(ad, popup_txt);
			IF_FREE(szFileName);
			IF_FREE(text);
			return;
		} else {
			int position = elm_entry_cursor_pos_get(mp_playlist_data->editfiled_entry);
			ERROR_TRACE("THE cursor position is %d", position);
			elm_entry_cursor_begin_set(mp_playlist_data->editfiled_entry);
			elm_entry_entry_set(mp_playlist_data->editfiled_entry, elm_entry_utf8_to_markup(mp_playlist_data->oldname));
			elm_entry_cursor_begin_set(mp_playlist_data->editfiled_entry);
			elm_entry_cursor_pos_set(mp_playlist_data->editfiled_entry, position - 1);
			mp_util_post_status_message(ad, popup_txt);
			IF_FREE(text);
			IF_FREE(szFileName);
			return;
		}
	}
	if (!strlen(szFileName) == 1) {
		IF_FREE(mp_playlist_data->oldname);
	}
	mp_playlist_data->oldname = strdup(szFileName);

	if (mp_playlist_data->type == MP_PLST_RENAME) {
		if (mp_playlist_data->editfiled_entry != NULL) {
			IF_FREE(mp_playlist_data->name);
			mp_playlist_data->name = elm_entry_markup_to_utf8(name);
		}
	} else {
		if (mp_playlist_data->editfiled_entry != NULL) {
			IF_FREE(mp_playlist_data->new_playlist_name);
			mp_playlist_data->new_playlist_name = elm_entry_markup_to_utf8(name);
		}
	}

	bool exist = false;
	mp_media_info_playlist_is_exist(text, &exist);
	elm_entry_input_panel_return_key_disabled_set(editfield, exist);

	if (mp_playlist_data->set_to_end) {
		mp_ecore_idler_del(mp_playlist_data->set_line_end_idler);
		mp_playlist_data->set_line_end_idler = ecore_idler_add(_mp_edit_playlist_entry_set_line_end, data);
	}
	IF_FREE(szFileName);
	IF_FREE(text);
}

static char *
_mp_edit_playlist_get_new_playlist_name(void)
{
	char unique_name[MP_PLAYLIST_NAME_SIZE] = "\0";
	int ret = 0;
	ret = mp_media_info_playlist_unique_name(GET_STR(STR_MP_MY_PLAYLIST), unique_name, MP_PLAYLIST_NAME_SIZE);
	if (ret == 0) {
		if (strlen(unique_name) <= 0) {
			ERROR_TRACE("playlist name is NULL");
			return NULL;
		} else {
			return g_strdup(unique_name);
		}
	} else {
		ERROR_TRACE("fail to mp_media_info_playlist_unique_name() : error code [%x] ", ret);
		return NULL;
	}

	return NULL;
}

static void _mp_edit_playlist_entry_focused_cb(void *data, Evas_Object *obj, void *event_info) // Focused callback will show X marked button and hide rename icon.
{
	eventfunc;

	Evas_Object *editfield = obj;
	MP_CHECK(editfield);

	Mp_Playlist_Data *mp_playlist_data = data;
	MP_CHECK(mp_playlist_data);

	_mp_edit_playlist_entry_eraser_status_set(editfield, data);

	if (mp_playlist_data->type == MP_PLST_RENAME) {
		elm_object_signal_emit(editfield, "elm,state,eraser,hide", "elm");
//                char *text = elm_entry_utf8_to_markup(mp_playlist_data->name);
//                elm_entry_entry_set(mp_playlist_data->editfiled_entry,  text);
//                IF_FREE(text);
		elm_entry_cursor_end_set(mp_playlist_data->editfiled_entry);
	}
}

static void _mp_edit_playlist_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info) // Unfocused callback will show rename icon and hide X marked button.
{
	eventfunc;

	Evas_Object *editfield = obj;
	MP_CHECK(editfield);

	elm_object_signal_emit(editfield, "elm,state,eraser,hide", "elm");
	//elm_object_item_signal_emit(id->item, "elm,state,rename,show", "");
}

static void
_mp_create_plst_entry_maxlength_reached_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	mp_popup_max_length(obj, STR_NH_COM_POPUP_CHARACTERS_MAXNUM_REACHED);
}

static void
_mp_search_edit_cancel_button_clicked(void *data, Evas_Object *o, const char *emission, const char *source)
{
	Evas_Object *en = (Evas_Object *) data;
	elm_object_text_set(en, "");
}

Evas_Object *
_mp_edit_playlist_create_editfield(void *thiz)
{
	startfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	Mp_Playlist_Data *mp_playlist_data = (Mp_Playlist_Data *)thiz;
	MP_CHECK_NULL(mp_playlist_data);
	MP_CHECK_NULL(mp_playlist_data->popup);

	mp_playlist_data->set_to_end = true;

	Evas_Object *entry = elm_entry_add(mp_playlist_data->layout);
	MP_CHECK_NULL(entry);
	elm_entry_scrollable_set(entry, EINA_TRUE);
	elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	elm_entry_single_line_set(entry, EINA_TRUE);

	elm_entry_cnp_mode_set(entry, ELM_CNP_MODE_PLAINTEXT);
	elm_entry_input_panel_layout_set(entry, ELM_INPUT_PANEL_LAYOUT_NORMAL);
	elm_entry_editable_set(entry, TRUE);
	elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);
	elm_entry_input_panel_return_key_disabled_set(entry, EINA_FALSE);
	//elm_entry_prediction_allow_set(entry, EINA_FALSE);

	evas_object_smart_callback_add(entry, "changed", _mp_edit_playlist_entry_changed_cb, mp_playlist_data);
	evas_object_smart_callback_add(entry, "preedit,changed", _mp_edit_playlist_entry_changed_cb, mp_playlist_data);
	evas_object_smart_callback_add(entry, "focused", _mp_edit_playlist_entry_focused_cb, mp_playlist_data);
	evas_object_smart_callback_add(entry, "unfocused", _mp_edit_playlist_entry_unfocused_cb, mp_playlist_data);

	static Elm_Entry_Filter_Limit_Size limit_filter_data;
	limit_filter_data.max_char_count = MP_PLAYLIST_NAME_SIZE;
	limit_filter_data.max_byte_count = 0;
	elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
	evas_object_smart_callback_add(entry, "maxlength,reached", _mp_create_plst_entry_maxlength_reached_cb, mp_playlist_data);
	edje_object_signal_callback_add(_EDJ(mp_playlist_data->layout), "elm,action,click", "cancel_image",  _mp_search_edit_cancel_button_clicked, entry);
	//elm_entry_cursor_end_set (entry);
	evas_object_show(entry);

	if (mp_playlist_data->new_playlist_name && (mp_playlist_data->type != MP_PLST_RENAME)) {
		//elm_object_part_text_set(entry, "elm.guide", mp_playlist_data->new_playlist_name);
		char *text = elm_entry_utf8_to_markup(mp_playlist_data->new_playlist_name);
		elm_entry_entry_set(entry, text);
		IF_FREE(text);
		elm_entry_cursor_end_set(entry);
	}

	if (mp_playlist_data->type == MP_PLST_RENAME) {
		evas_object_smart_callback_add(entry, "activated", mp_edit_playlist_rename_done_cb, mp_playlist_data);
		char *name = NULL;
		mp_media_info_group_get_main_info(mp_playlist_data->playlist_handle, &name);

		IF_FREE(mp_playlist_data->name);
		mp_playlist_data->name = g_strdup(name);
		if (mp_playlist_data->name) {
			char *text = elm_entry_utf8_to_markup(mp_playlist_data->name);
			elm_entry_entry_set(entry, text);
			IF_FREE(text);
			elm_entry_cursor_end_set(entry);
		}
	} else {
		evas_object_smart_callback_add(entry, "activated", _mp_edit_playlist_create_new_done_cb, mp_playlist_data);
	}

	elm_object_signal_callback_add(entry, "elm,eraser,clicked", "elm", _mp_edit_playlist_entry_eraser_clicked_cb, entry);

	return entry;
}

static void _mp_edit_playlist_cancel_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	ad->del_cb_invoked = 0;

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_POPUP_CANCEL);

	Mp_Playlist_Data *mp_playlist_data = (Mp_Playlist_Data *)data;
	MP_CHECK(mp_playlist_data);
	IF_FREE(mp_playlist_data->oldname);

	_mp_edit_playlist_destory(mp_playlist_data);
}

void mp_edit_playlist_add_to_selected_mode(void *data, bool selected)
{
	eventfunc;

	Mp_Playlist_Data *mp_playlist_data = (Mp_Playlist_Data *)data;
	MP_CHECK(mp_playlist_data);

	mp_playlist_data->add_to_selected = selected;
}

static Eina_Bool
_entry_focus_timer_cb(void *data)
{
	Mp_Playlist_Data *mp_playlist_data = data;
	elm_object_focus_set(mp_playlist_data->editfiled_entry, EINA_TRUE);
	mp_playlist_data->entry_show_timer = NULL;
	return false;
}

void
mp_edit_playlist_content_create(void *thiz)
{
	startfunc;
	char *new_playlist_name = NULL;
	Mp_Playlist_Data *mp_playlist_data = (Mp_Playlist_Data *)thiz;
	MP_CHECK(mp_playlist_data);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *layout = NULL;
	char *btn_str = NULL;

	mp_playlist_data->add_to_selected = true;

	layout = elm_layout_add(mp_playlist_data->popup);
	char edje_path[1024] ={0};
	char * path = app_get_resource_path();

	MP_CHECK(path);
	snprintf(edje_path, 1024, "%s%s/%s", path, "edje", PLAY_VIEW_EDJ_NAME);

	MP_CHECK(edje_path);
	elm_layout_file_set(layout, edje_path, "popup_entryview");
	free(path);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_focus_set(layout, EINA_FALSE);

	mp_playlist_data->layout = layout;

	mp_playlist_data->new_playlist_name = _mp_edit_playlist_get_new_playlist_name();
	mp_playlist_data->editfiled_entry = _mp_edit_playlist_create_editfield(mp_playlist_data);
	IF_FREE(new_playlist_name);

	mp_popup_response_callback_set(mp_playlist_data->popup, _mp_edit_playlist_cancel_cb, mp_playlist_data);
	Evas_Object *btn1 = mp_widget_create_button(mp_playlist_data->popup, "popup", STR_MP_CANCEL, NULL,
	                    _mp_edit_playlist_cancel_cb, mp_playlist_data);

	if (mp_playlist_data->type == MP_PLST_CREATE || mp_playlist_data->type == MP_PLST_CREATE_TO_ADD_TRACK) {
		btn_str = STR_MP_CREATE;
	} else if (mp_playlist_data->type == MP_PLST_RENAME) {
		btn_str = STR_MP_RENAME;
	} else {
		btn_str = STR_MP_OK;
	}

	Evas_Object *btn2 = mp_widget_create_button(mp_playlist_data->popup, "popup", btn_str, NULL, NULL, NULL);

	mp_playlist_data->btn_ok = btn2;
	elm_object_part_content_set(mp_playlist_data->popup, "button1", btn1);
	elm_object_part_content_set(mp_playlist_data->popup, "button2", btn2);

	elm_object_part_content_set(layout, "elm.swallow.content", mp_playlist_data->editfiled_entry);
	elm_object_content_set(mp_playlist_data->popup, layout);

	if (mp_playlist_data->type == MP_PLST_RENAME) {
		evas_object_smart_callback_add(btn2, "clicked", mp_edit_playlist_rename_done_cb, mp_playlist_data);
	} else {
		evas_object_smart_callback_add(btn2, "clicked", _mp_edit_playlist_create_new_done_cb, mp_playlist_data);
	}

	if (mp_playlist_data->editfiled_entry && !ad->popup[MP_POPUP_NOTIFY]) {
		mp_playlist_data->entry_show_timer =  ecore_timer_add(0.1, _entry_focus_timer_cb, mp_playlist_data);
	}

	return;
}

static void
_mp_edit_playlist_popup_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	mp_evas_object_del(obj);
	Evas_Object *editfiled_entry = (Evas_Object *)data;
	MP_CHECK(editfiled_entry);
	elm_entry_cursor_end_set(editfiled_entry);
	elm_object_focus_set(editfiled_entry, EINA_TRUE);
	elm_entry_input_panel_show(editfiled_entry);
}


static void
mp_edit_playlist_rename_done_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	Mp_Playlist_Data *mp_playlist_data = data;
	MP_CHECK(mp_playlist_data);

	Evas_Object *editfiled_entry = mp_playlist_data->editfiled_entry;
	MP_CHECK(editfiled_entry);

	char *text = NULL;
	int ret = -1;
	mp_media_info_h playlist = mp_playlist_data->playlist_handle;
	MP_CHECK(playlist);

	struct appdata *ad = mp_util_get_appdata();
	text = mp_util_isf_get_edited_str(editfiled_entry, TRUE);

	if (!mp_util_is_playlist_name_valid((char *)text)) {
		//_mp_edit_playlist_destory(mp_playlist_data);
		mp_widget_notify_cb_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_RENAME_PLAYLIST"), _mp_edit_playlist_popup_timeout_cb, (void*)editfiled_entry);
		IF_FREE(text);
		return ;
	} else {
		bool exist = false;
		ret = mp_media_info_playlist_is_exist(text, &exist);
		if (ret != 0) {
			ERROR_TRACE("Fail to get playlist count by name: %d", ret);
			mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_RENAME_PLAYLIST"));
		} else if (exist) {
			_mp_edit_playlist_destory(mp_playlist_data);
			IF_FREE(text);
			//mp_widget_text_popup(ad, GET_STR(STR_MP_POP_EXISTS));
			Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, NULL, NULL, ad);
			/*set text*/
			mp_util_domain_translatable_text_set(popup, STR_MP_POP_EXISTS);
			mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_OK, MP_POPUP_YES);
			evas_object_show(popup);
			return;
		} else {
			ret = mp_media_info_playlist_rename(playlist, text);
			if (ret == 0) {
				mp_debug("mp_media_info_playlist_rename().. OK");
				MpView_t *create_playlist_detail_view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYLIST_DETAIL);
				if (create_playlist_detail_view != NULL) {
					mp_view_set_title(create_playlist_detail_view, text);
					((MpPlaylistDetailView_t *)create_playlist_detail_view)->content_set(create_playlist_detail_view);
				}

				mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_RENAMED);
			}
		}
	}

	_mp_edit_playlist_destory(mp_playlist_data);

	IF_FREE(text);
}

/*
#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_edit_playlist_rotate_cb(void *thiz, int randscape)
{
	DEBUG_TRACE("create_plst view rotated");
	Mp_Playlist_Data *mp_playlist_data = thiz;
	MP_CHECK(mp_playlist_data);

        struct appdata *ad = mp_util_get_appdata();
        MP_CHECK(ad);

        if (mp_playlist_data->editfiled_entry && !ad->popup[MP_POPUP_NOTIFY])
        {
                elm_object_focus_set(mp_playlist_data->editfiled_entry, EINA_TRUE);
        }
}
#endif
*/

void *mp_edit_playlist_create(mp_plst_operation_type type)
{
	eventfunc;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	Evas_Object *popup = NULL;

	Mp_Playlist_Data *mp_playlist_data = (Mp_Playlist_Data *)calloc(1, sizeof(Mp_Playlist_Data));
	MP_CHECK_NULL(mp_playlist_data);
	mp_playlist_data->type = type;

	char *title = NULL;
	if (type == MP_PLST_CREATE || type == MP_PLST_CREATE_TO_ADD_TRACK) {
		title = STR_MP_CREATE_PLAYLIST;
	} else if (type == MP_PLST_RENAME) {
		title = STR_MP_RENAME;
	} else if (type == MP_PLST_SAVE_AS) {
		title = STR_MP_TITLE_SAVE_AS_PLAYLIST;
	}

	popup = mp_entry_popup_create(title);

	if (!popup) {
		IF_FREE(mp_playlist_data);
		ERROR_TRACE("mp_entry_popup_create fail");
		return NULL;
	}
	mp_playlist_data->popup = popup;

	return mp_playlist_data;
}

int mp_edit_playlist_set_edit_list(Mp_Playlist_Data *mp_playlist_data, MpList_t *adding_list)
{
	startfunc;
	MP_CHECK_VAL(mp_playlist_data, -1);
	mp_playlist_data->adding_list = adding_list;
	return 0;
}

int mp_edit_playlist_set_media_id(Mp_Playlist_Data *mp_playlist_data, const char *adding_media_id)
{
	startfunc;
	MP_CHECK_VAL(mp_playlist_data, -1);
	mp_playlist_data->adding_media_id = g_strdup(adding_media_id);
	return 0;
}

int mp_edit_playlist_set_create_type(Mp_Playlist_Data *mp_playlist_data, mp_plst_create_type_e type)
{
	MP_CHECK_VAL(mp_playlist_data, -1);
	mp_playlist_data->creation_type = type;
	return 0;
}

