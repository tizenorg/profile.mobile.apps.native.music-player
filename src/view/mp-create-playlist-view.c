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

#include "mp-create-playlist-view.h"
#include "mp-widget.h"
#include "mp-list-view.h"
#include "mp-util.h"
#include "mp-edit-callback.h"
#include "mp-add-track-view.h"
#include "mp-common.h"

typedef struct _Item_Data {
	Elm_Object_Item *item;
	int dial;
	int title;
	MpCreatePlstView_t *view;
} Item_Data;

static void
mp_create_plst_view_rename_done_cb(void *data, Evas_Object * obj, void *event_info);

static void
_mp_create_plst_view_destory_cb(void *thiz)
{
	eventfunc;
	MpCreatePlstView_t *view = thiz;
	MP_CHECK(view);
	mp_view_fini((MpView_t *)view);

	/*TODO: release resource..*/
	IF_FREE(view->adding_media_id);

	free(view);
}

static Eina_Bool
_mp_create_playlist_view_create_new_cancel_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	{
		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MP_CHECK_VAL(view_mgr, EINA_TRUE);
		mp_view_mgr_pop_view(view_mgr, false);
	}
	return EINA_TRUE;
}

static void
_mp_create_playlist_view_create_new_done_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *) data;

	int plst_uid = -1;

	char *converted_name = NULL;
	/*Evas_Object *entry = mp_widget_editfield_entry_get(view->editfiled_new_playlist);
	const char *name = elm_entry_entry_get(entry);*/

	const char *name = elm_entry_entry_get(view->editfiled_entry);

	if (name == NULL || strlen(name) == 0) {
		/*name = elm_object_part_text_get(view->editfiled_new_playlist, "elm.guidetext");*/
		name = elm_object_part_text_get(view->editfiled_entry, "elm.guide");
	}
	converted_name = elm_entry_markup_to_utf8(name);

	struct appdata *ad = mp_util_get_appdata();
	mp_playlist_h playlist = NULL;
	plst_uid = mp_util_create_playlist(ad, converted_name, &playlist);
	if (plst_uid < 0) {
		mp_media_info_playlist_handle_destroy(playlist);
		IF_FREE(converted_name);
		return;
	}

	if (view->adding_list) {
		WARN_TRACE("adding list = %p", view->adding_list);
		mp_edit_cb_excute_add_to_playlist(view->adding_list, plst_uid, converted_name, true);
	}

	IF_FREE(converted_name);

	if (view->adding_media_id) {
		mp_media_info_h media_info = NULL;
		char *path = NULL;
		mp_media_info_create(&media_info, view->adding_media_id);
		mp_media_info_get_thumbnail_path(media_info, &path);
		if (mp_media_info_playlist_add_item(playlist, view->adding_media_id, path) == 0) {
			mp_media_info_playlist_db_update(playlist);
		}

		mp_media_info_destroy(media_info);
	}

	mp_media_info_playlist_handle_destroy(playlist);


	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	elm_naviframe_item_pop(view_mgr->navi);
	mp_view_mgr_post_event(view_mgr, MP_PLAYLIST_CREATED);

	if (view->parent_view == MP_PLST_PARENT_ALL_VIEW) {
		MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
		MpAddTrackView_t *view_addtrack = mp_add_track_view_create(view_manager->navi, plst_uid);
		MP_CHECK(view_addtrack);
		mp_view_mgr_push_view(view_manager, (MpView_t *)view_addtrack, NULL);

		mp_view_update_options((MpView_t *)view_addtrack);
		mp_list_set_edit((MpList_t *)view_addtrack->content_to_show, TRUE);
		mp_list_view_set_select_all((MpListView_t *)view_addtrack, TRUE);
		mp_view_set_title((MpView_t *)view_addtrack, STR_MP_TILTE_SELECT_ITEM);

		mp_add_track_view_select_tab(view_addtrack, MP_ADD_TRACK_VIEW_TAB_SONGS);
	}
}


static void _mp_create_playlist_view_realized_cb(void *data, Evas_Object *obj, void *ei)
{
	eventfunc;
	Item_Data *id = elm_object_item_data_get(ei);
	if (!id) {
		return;
	} else {
		/* if dialogue styles*/
		if (id->dial == 1) {
			elm_object_item_signal_emit(ei, "elm,state,top", "");
		} else if (id->dial == 2) {
			elm_object_item_signal_emit(ei, "elm,state,center", "");
		} else if (id->dial == 3) {
			elm_object_item_signal_emit(ei, "elm,state,bottom", "");
		}
	}
}


static void
_mp_create_playlist_view_entry_eraser_clicked_cb(void *data, Evas_Object *obj, const char *emission, const char *source) /* When X marked button is clicked, empty entry's contents.*/
{
	eventfunc;

	Evas_Object *entry = (Evas_Object *)data;
	MP_CHECK(entry);

	elm_entry_entry_set(entry, "");
}

static void
_mp_create_playlist_view_entry_changed_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;

	/*MpCreatePlstView_t *view = (MpCreatePlstView_t *)data;
	Evas_Object *editfield = view->editfiled_new_playlist;
	MP_CHECK(editfield);

	Evas_Object *entry = obj;
	MP_CHECK(entry);

	Eina_Bool entry_empty = elm_entry_is_empty(entry);
	const char *eraser_signal = NULL;
	const char *guidetext_signal = NULL;
	if (entry_empty) {
		DEBUG_TRACE("NULL");
		eraser_signal =	"elm,state,eraser,hide";
		guidetext_signal = "elm,state,guidetext,show";
	} else {
		DEBUG_TRACE("NULL");
		eraser_signal =	"elm,state,eraser,show";
		guidetext_signal = "elm,state,guidetext,hide";
	}
	elm_object_signal_emit(editfield, eraser_signal, "elm");
	//elm_object_signal_emit(editfield, guidetext_signal, "elm");
	*/

	Item_Data *id = data;
	MP_CHECK(id);
	MP_CHECK(id->item);
	Evas_Object *editfield = elm_object_item_part_content_get(id->item, "elm.icon.entry");
	MP_CHECK(editfield);

	if (elm_object_focus_get(obj)) {
		if (elm_entry_is_empty(obj)) {
			elm_object_signal_emit(editfield, "elm,state,eraser,hide", "elm");
		} else {
			elm_object_signal_emit(editfield, "elm,state,eraser,show", "elm");
		}
	}
}

static char *
_mp_create_plst_view_get_new_playlist_name(void)
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



static void _mp_create_playlist_view_entry_focused_cb(void *data, Evas_Object *obj, void *event_info) /* Focused callback will show X marked button and hide rename icon.*/
{
	eventfunc;
	/*MpCreatePlstView_t *view = (MpCreatePlstView_t *)data;
	Evas_Object *editfield = view->editfiled_new_playlist;
	MP_CHECK(editfield);

	Evas_Object *entry = obj;
	MP_CHECK(entry);

	Eina_Bool entry_empty = elm_entry_is_empty(entry);

	if (!entry_empty)
		elm_object_signal_emit(editfield, "elm,state,eraser,show", "elm");
	elm_object_signal_emit(editfield, "elm,state,rename,hide", "elm");
	*/

	Item_Data *id = data;
	MP_CHECK(id);
	MP_CHECK(id->item);
	Evas_Object *editfield = elm_object_item_part_content_get(id->item, "elm.icon.entry");
	MP_CHECK(editfield);

	if (!elm_entry_is_empty(obj)) {
		elm_object_signal_emit(editfield, "elm,state,eraser,show", "elm");
	}

	elm_object_item_signal_emit(id->item, "elm,state,rename,hide", "");

	if (id->view->parent_view == MP_PLST_PARENT_DETAIL_VIEW) {
		elm_object_signal_emit(editfield, "elm,state,eraser,hide", "elm");
		elm_entry_entry_set(id->view->editfiled_entry,  id->view->name);
		elm_entry_cursor_end_set(id->view->editfiled_entry);
	} else if (id->view->new_playlist_name) {
		elm_entry_entry_set(id->view->editfiled_entry, id->view->new_playlist_name);
		elm_entry_cursor_end_set(id->view->editfiled_entry);
	}
}

static void _mp_create_playlist_view_entry_unfocused_cb(void *data, Evas_Object *obj, void *event_info) /* Unfocused callback will show rename icon and hide X marked button.*/
{
	eventfunc;
	/*MpCreatePlstView_t *view = (MpCreatePlstView_t *)data;
	Evas_Object *editfield = view->editfiled_new_playlist;
	MP_CHECK(editfield);

	Evas_Object *entry = obj;
	MP_CHECK(entry);

	elm_object_signal_emit(editfield, "elm,state,eraser,hide", "elm");
	elm_object_signal_emit(editfield, "elm,state,rename,show", "elm");
	*/

	Item_Data *id = data;
	MP_CHECK(id);
	MP_CHECK(id->item);
	Evas_Object *editfield = elm_object_item_part_content_get(id->item, "elm.icon.entry");
	MP_CHECK(editfield);

	elm_object_signal_emit(editfield, "elm,state,eraser,hide", "elm");
	elm_object_item_signal_emit(id->item, "elm,state,rename,show", "");
}

static void
_mp_create_plst_entry_maxlength_reached_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *)data;
	MP_CHECK(view);

	Evas_Object *editfiled_entry = view->editfiled_entry;
	MP_CHECK(editfiled_entry);
	char *text = NULL;
	text = mp_util_isf_get_edited_str(editfiled_entry, TRUE);
	MP_CHECK(text);
	view->name = text;
	elm_object_focus_set(editfiled_entry, EINA_TRUE);
	elm_entry_entry_set(editfiled_entry, text);

	mp_popup_max_length(obj, STR_NH_COM_POPUP_CHARACTERS_MAXNUM_REACHED);
}


static Evas_Object *
_mp_create_plst_gl_icon_get(void *data, Evas_Object * obj, const char *part)
{
	Item_Data *id = (Item_Data *)data;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *)id->view;
	MP_CHECK_NULL(view);
	MP_CHECK_NULL(obj);

	if (!strcmp(part, "elm.icon.entry")) {/* Add elm_entry to current editfield genlist item.*/
		Evas_Object *entry = elm_entry_add(obj);
		elm_entry_scrollable_set(entry, EINA_TRUE);
		elm_scroller_policy_set(entry, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
		elm_entry_single_line_set(entry, EINA_TRUE);

		evas_object_smart_callback_add(entry, "changed", _mp_create_playlist_view_entry_changed_cb, id);
		evas_object_smart_callback_add(entry, "preedit,changed", _mp_create_playlist_view_entry_changed_cb, id);
		evas_object_smart_callback_add(entry, "focused", _mp_create_playlist_view_entry_focused_cb, id);
		evas_object_smart_callback_add(entry, "unfocused", _mp_create_playlist_view_entry_unfocused_cb, id);

		static Elm_Entry_Filter_Limit_Size limit_filter_data;
		limit_filter_data.max_char_count = 0;
		limit_filter_data.max_byte_count = MP_PLAYLIST_NAME_SIZE;
		elm_entry_markup_filter_append(entry, elm_entry_filter_limit_size, &limit_filter_data);
		evas_object_smart_callback_add(entry, "maxlength,reached", _mp_create_plst_entry_maxlength_reached_cb, view);

		elm_entry_input_panel_return_key_type_set(entry, ELM_INPUT_PANEL_RETURN_KEY_TYPE_DONE);

		if (view->new_playlist_name && (view->parent_view != MP_PLST_PARENT_DETAIL_VIEW)) {
			/*elm_object_part_text_set(entry, "elm.guide", view->new_playlist_name);*/
			elm_entry_entry_set(entry, view->new_playlist_name);
			elm_entry_cursor_end_set(entry);
		}

		view->editfiled_entry = entry;
		if (view->parent_view == MP_PLST_PARENT_DETAIL_VIEW) {
			evas_object_smart_callback_add(entry, "activated", mp_create_plst_view_rename_done_cb, view);

			if (view->name) {
				elm_entry_entry_set(view->editfiled_entry, view->name);
				elm_entry_cursor_end_set(view->editfiled_entry);
			}
		} else {
			evas_object_smart_callback_add(entry, "activated", _mp_create_playlist_view_create_new_done_cb, view);
		}
		elm_object_signal_callback_add(entry, "elm,eraser,clicked", "elm", _mp_create_playlist_view_entry_eraser_clicked_cb, entry);
		return entry;
	}
	return NULL;

}

static void _mp_create_plst_item_del(void *data, Evas_Object *obj)
{
	Item_Data *item_data = (Item_Data *) data;
	MP_CHECK(item_data);
	IF_FREE(item_data);
}

Evas_Object *
_mp_create_plst_view_create_editfield_layout(void *thiz)
{
	startfunc;
	Evas_Object *genlist = NULL;
	Elm_Object_Item *item = NULL;
	static Elm_Genlist_Item_Class itc;
	Item_Data *id = NULL;

	MpCreatePlstView_t *view = (MpCreatePlstView_t *)thiz;
	MP_CHECK_NULL(view);

	view->create_plst_layout = elm_layout_add(view->layout);
	MP_CHECK_NULL(view->create_plst_layout);

	char edje_path[1024] ={0};
	char * path = app_get_resource_path();

	MP_CHECK_NULL(path);
	snprintf(edje_path, 1024, "%s%s/%s", path, "edje", EDJ_NAME);

	MP_CHECK_NULL(edje_path);
	elm_layout_file_set(view->create_plst_layout, edje_path, "create_playlist");
	free(path);

	genlist = mp_widget_genlist_create(view->create_plst_layout);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_smart_callback_add(genlist, "realized", _mp_create_playlist_view_realized_cb, NULL);

	itc.version = ELM_GENGRID_ITEM_CLASS_VERSION;
	itc.refcount = 0;
	itc.delete_me = EINA_FALSE;
	itc.item_style = "editfield";
	itc.func.text_get = NULL;
	itc.func.content_get = _mp_create_plst_gl_icon_get;
	itc.func.state_get = NULL;
	itc.func.del = _mp_create_plst_item_del;

	id = calloc(sizeof(Item_Data), 1);
	MP_CHECK_NULL(id);
	id->view = view;
	item = elm_genlist_item_append(genlist, &itc, id, NULL, ELM_GENLIST_ITEM_NONE, NULL, NULL);/*id replace view*/
	elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_NONE);
	elm_object_scroll_freeze_push(genlist);
	id->item = item;

	elm_object_part_content_set(view->create_plst_layout, "elm.swallow.content", genlist);
	evas_object_show(view->create_plst_layout);

	return view->create_plst_layout;
}

static Evas_Object *
_mp_create_plst_view_content_create(void *thiz)
{
	startfunc;
	char *new_playlist_name = NULL;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *)thiz;
	MP_CHECK_NULL(view);

	Evas_Object *create_plst_layout = NULL;
	view->new_playlist_name = _mp_create_plst_view_get_new_playlist_name();
	create_plst_layout = _mp_create_plst_view_create_editfield_layout(view);
	IF_FREE(new_playlist_name);
	MP_CHECK_NULL(create_plst_layout);

	/*elm_object_item_text_set(it, GET_STR("IDS_MUSIC_BODY_CREATE_PLAYLIST"));
	mp_language_mgr_register_object_item(it, "IDS_MUSIC_BODY_CREATE_PLAYLIST");*/

	evas_object_show(create_plst_layout);
	return create_plst_layout;
}

static void
mp_create_plst_view_rename_done_cb(void *data, Evas_Object * obj, void *event_info)
{
	eventfunc;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *)data;
	MP_CHECK(view);

	Evas_Object *editfiled_entry = view->editfiled_entry;
	MP_CHECK(editfiled_entry);

	char *text = NULL;
	int ret = -1;
	mp_media_info_h playlist = view->playlist_handle;
	MP_CHECK(playlist);

	struct appdata *ad = mp_util_get_appdata();
	text = mp_util_isf_get_edited_str(editfiled_entry, TRUE);

	if (!mp_util_is_playlist_name_valid((char *)text)) {
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_RENAME_PLAYLIST"));
	} else {
		bool exist = false;
		ret = mp_media_info_playlist_is_exist(text, &exist);
		if (ret != 0) {
			ERROR_TRACE("Fail to get playlist count by name: %d", ret);
			mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_RENAME_PLAYLIST"));
		} else if (exist) {
			mp_widget_text_popup(ad, GET_STR(STR_MP_POP_EXISTS));
			return;
		} else {
			ret = mp_media_info_playlist_rename(playlist, text);
			if (ret == 0) {
				mp_debug("mp_media_info_playlist_rename().. OK");
				MpView_t *create_playlist_detail_view = mp_view_mgr_get_view(GET_VIEW_MGR, MP_VIEW_PLAYLIST_DETAIL);
				mp_view_set_title(create_playlist_detail_view, text);

				mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_RENAMED);
			}
		}
	}
	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	elm_naviframe_item_pop(view_mgr->navi);

	IF_FREE(text);
}

static int _mp_create_plst_view_create_title_buttons(void *thiz)
{
	startfunc;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *)thiz;

	Evas_Object *cancel_btn = mp_create_title_text_btn(view->create_plst_layout, STR_MP_CANCEL, mp_common_view_cancel_cb, view);
	Evas_Object *save_btn = mp_create_title_text_btn(view->create_plst_layout, "IDS_COM_SK_SAVE", NULL, view);

	elm_object_item_part_content_set(view->navi_it, "title_left_btn", cancel_btn);
	elm_object_item_part_content_set(view->navi_it, "title_right_btn", save_btn);

	if (view->parent_view == MP_PLST_PARENT_DETAIL_VIEW) {
		mp_media_info_group_get_main_info(view->playlist_handle, &view->name);
		if (view->name) {
			elm_entry_entry_set(view->editfiled_entry,  view->name);
			elm_entry_cursor_end_set(view->editfiled_entry);
		}
		evas_object_smart_callback_add(save_btn, "clicked", mp_create_plst_view_rename_done_cb, view);
	} else {
		evas_object_smart_callback_add(save_btn, "clicked", _mp_create_playlist_view_create_new_done_cb, view);
	}

	return 0;
}

static int _mp_create_plst_view_update_options(void *thiz)
{
	startfunc;
	MpCreatePlstView_t *view = (MpCreatePlstView_t *)thiz;

	/*_mp_create_plst_view_update_option_clear(view);*/
	/* add buttons */
	/*
		Evas_Object *toolbar = mp_widget_create_naviframe_toolbar(view->navi_it);
		Elm_Object_Item *toolbar_item = NULL;

		toolbar_item = mp_widget_create_toolbar_item_btn(toolbar, MP_TOOLBAR_BTN_DEFAULT, GET_SYS_STR("IDS_COM_SK_SAVE"), _mp_create_playlist_view_create_new_done_cb, view);
		view->toolbar_options[MP_OPTION_LEFT] = toolbar_item;
	*/

	_mp_create_plst_view_create_title_buttons(thiz);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_create_playlist_view_create_new_cancel_cb, view);

	evas_object_show(view->editfiled_entry);
	elm_object_focus_set(view->editfiled_entry, EINA_TRUE);

	return 0;
}

static void
_mp_create_plst_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpCreatePlstView_t *view = thiz;
	MP_CHECK(view);

	switch (event) {
	case MP_SIP_STATE_CHANGED:
		/*
		if (view->navi_it) {
			bool title_visible = (mp_util_get_sip_state() && mp_util_is_landscape()) ? false : true;
			mp_view_set_title_visible(view, title_visible);
		}*/
		break;
	case MP_VIEW_TRANSITION_FINISHED:
		elm_object_focus_set(view->editfiled_entry, EINA_TRUE);
		break;
	default:
		break;
	}

}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_create_plst_view_rotate_cb(void *thiz, int randscape)
{
	DEBUG_TRACE("create_plst view rotated");
	MpCreatePlstView_t *view = thiz;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (view->editfiled_entry && !ad->popup[MP_POPUP_NOTIFY]) {
		elm_object_focus_set(view->editfiled_entry, EINA_TRUE);
	}
	if (mp_util_get_sip_state() && (int)mp_view_mgr_get_top_view(GET_VIEW_MGR) == (int)view) {
		_mp_create_plst_view_on_event(view, MP_SIP_STATE_CHANGED);
	}
}
#endif

static int
_mp_create_plst_view_init(Evas_Object *parent, MpCreatePlstView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_view_init(parent, (MpView_t *)view, MP_VIEW_CREATE_PLAYLIT);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = NULL;
	view->update_options = _mp_create_plst_view_update_options;
	view->update_options_edit = NULL;
	view->view_destroy_cb = _mp_create_plst_view_destory_cb;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;
	view->start_playback = NULL;
	view->pause_playback = NULL;
	view->stop_playback = NULL;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_create_plst_view_rotate_cb;
#endif
	view->on_event = _mp_create_plst_view_on_event;

	view->create_plst_layout = _mp_create_plst_view_content_create(view);
	MP_CHECK_VAL(view->create_plst_layout, -1);

	elm_object_part_content_set(view->layout, "list_content", view->create_plst_layout);

	return ret;
}

MpCreatePlstView_t *mp_create_plst_view_create(Evas_Object *parent)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpCreatePlstView_t *view = calloc(1, sizeof(MpCreatePlstView_t));
	MP_CHECK_NULL(view);

	ret = _mp_create_plst_view_init(parent, view);
	elm_object_focus_set(view->editfiled_entry, EINA_TRUE);
	if (ret) {
		goto Error;
	}

	return view;

Error:
	ERROR_TRACE("Error: mp_create_plst_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_create_plst_view_set_edit_list(MpCreatePlstView_t *view, MpList_t *adding_list)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	view->adding_list = adding_list;
	return 0;
}

int mp_create_plst_view_set_media_id(MpCreatePlstView_t *view, const char *adding_media_id)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	view->adding_media_id = g_strdup(adding_media_id);
	return 0;
}

int mp_create_plst_view_set_creation_type(MpCreatePlstView_t *view, mp_plst_creation_type_e type)
{
	MP_CHECK_VAL(view, -1);
	view->creation_type = type;
	return 0;
}

int mp_create_plst_view_destory(MpCreatePlstView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	return 0;
}
