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

#include "mp-setting-view.h"
#include "ms-eq-view.h"
#include "ms-effect-view.h"
#include "music.h"
#include "ms-util.h"
#include "ms-auto-off.h"
#include "mp-player-debug.h"
#include "mp-player-mgr.h"
#include "app_manager.h"
#include "mp-popup.h"
#include "mp-ctxpopup.h"
#include "ms-play-speed.h"
#include "ms-playlist.h"
#include "mp-util.h"
#include "mp-list.h"
#include "mp-common.h"
#include <runtime_info.h>
#include <efl_extension.h>
#include "mp-player-view.h"

#define GL_POPUP_GENLIST_HEIGHT_HD 230
#define GL_POPUP_GENLIST_WIDTH_HD 500
#define GL_POPUP_GENLIST_HEIGHT_QHD 185
#define GL_POPUP_GENLIST_WIDTH_QHD 450
#define GL_POPUP_GENLIST_HEIGHT_WVGA 202
#define GL_POPUP_GENLIST_WIDTH_WVGA 480
#define ELM_SCALE_SIZE(x) (int)(((double)(x) * elm_config_scale_get()) / elm_app_base_scale_get())
#define MP_POPUP_MENUSTYLE_HEIGHT(x) (50*x)

#define GL_STR_SCREEN_WIDTH_HD 720
#define GL_STR_SCREEN_HEIGHT_HD 1280
#define GL_STR_SCREEN_WIDTH_QHD 540
#define GL_STR_SCREEN_HEIGHT_QHD 960
#define GL_STR_SCREEN_WIDTH_WVGA 480
#define GL_STR_SCREEN_HEIGHT_WVGA 800

#define TAB_COUNT 7

static Evas_Object* group_radio;
static char *tab_str[TAB_COUNT] = {STR_MP_PLAYLISTS, STR_MP_TRACKS,
STR_MP_ALBUMS, STR_MP_ARTISTS,
STR_MP_GENRES, STR_MP_FOLDERS, STR_MP_SQUARE};
static int tab_index[TAB_COUNT] = {0};

static Evas_Object *check_boxs[MP_SETTING_TABS_ITEM_MAX];
static int tab_state = 0;
static Evas_Object *_mp_setting_view_tabs_list_create(MpSettingView_t *view,
	Evas_Object *parent);

static int
_ms_se_state_set(MpSettingView_t *data)
{
	MpSettingView_t *view = data;

	mp_retvm_if(view == NULL, -1, "ug_data is NULL");
	/* ms_effect_view_update_radio_button(); */
	return 0;
}

static void
_ms_key_change_cb(const char *key, void *user_data)
{
	MpSettingView_t *view = NULL;
	view = (MpSettingView_t *)user_data;
	if (strcmp(key, KEY_MUSIC_SE_CHANGE) == 0) {
		_ms_se_state_set(view);
	}
	return;
}

static void
_ms_key_change_cb_init(void *data)
{
	MpSettingView_t *view = NULL;
	view = (MpSettingView_t *)data;
	if (preference_set_changed_cb(KEY_MUSIC_SE_CHANGE,
		_ms_key_change_cb, view)) {
		ERROR_TRACE("Error when register callback");
	}
}

static void
_ms_key_change_cb_deinit(void)
{
	preference_unset_changed_cb(KEY_MUSIC_SE_CHANGE);
	runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_AUDIO_JACK_STATUS);
}

static void _lyrics_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{

	Eina_Bool state = elm_check_state_get(obj);

	WARN_TRACE("lyrics state changed [%d]", state);

	preference_set_boolean(KEY_MUSIC_LYRICS, state);
}


static Evas_Object *
_ms_gl_contents_get(void *data, Evas_Object * obj, const char *part)
{
	Evas_Object *content = NULL;

	if (!strcmp(part, "elm.icon.2")) {
		content  = elm_check_add(obj);
		elm_check_state_set(content, EINA_TRUE);
		elm_object_style_set(content, "on&off");
		evas_object_smart_callback_add(content, "changed", _lyrics_check_clicked_cb, NULL);

	bool lyrics_state = FALSE;
	preference_get_boolean(KEY_MUSIC_LYRICS, &lyrics_state);

	elm_check_state_set(content, (int)lyrics_state);
		evas_object_propagate_events_set(content, EINA_FALSE);
	}

	return content;
}

static char *
_ms_gl_lyrics_label_get(void *data, Evas_Object * obj, const char *part)
{
	/* DEBUG_TRACE("part name = [%s]", part); */

	char *txt = NULL;

	if (strcmp(part, "elm.text.main.left") == 0) {
		txt = GET_STR(STR_MP_SHOW_LYRICS);
	}
	if (txt)
		return strdup(txt);
	else
		return NULL;
}

/*static char *
_ms_gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	//DEBUG_TRACE("part name = [%s]", part);

	char *txt = NULL;

	if (strcmp(part, "elm.text.main.left") == 0)
	{
		txt = GET_STR("IDS_MUSIC_BODY_PLAYLISTS");
	}
	if (txt)
		return strdup(txt);
	else
		return NULL;

}*/

/*static void
_ms_gl_expand(void *data, Evas_Object * obj, void *event_info)
{
	int param = (int)data;
	DEBUG_TRACE("data: %d", param);
}

static void _gl_exp(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");
	mp_retm_if (!event_info, "INVALID param");

	Elm_Object_Item *item = (Elm_Object_Item *)event_info;
	int main_menu = (int)elm_object_item_data_get(item);

	_ms_gl_expand((void *)main_menu, obj, event_info);
}

static void _gl_con(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");

	mp_retm_if (!event_info, "INVALID param");

	Elm_Object_Item *item = event_info;
	elm_genlist_item_subitems_clear(item);
	elm_genlist_realized_items_update(obj);
}*/

static void
_ms_setting_playlist_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	mp_evas_object_del(obj);
	int response = (int)event_info;
	int temp_playlist_state, playlist_state;

	if (response == MP_POPUP_NO) {
		DEBUG_TRACE("cancel btn click");

		ms_playlist_check_state_get_val(&playlist_state);
		ms_key_get_playlist_val(&temp_playlist_state);

		if (temp_playlist_state != playlist_state) {
			ms_playlist_check_state_set_val(temp_playlist_state);
		}

		return;
	}

	ms_playlist_check_state_get_val(&playlist_state);
	ms_key_set_playlist_val(playlist_state);

	return;

}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	mp_retm_if(!obj, "INVALID param");
	mp_retm_if(!event_info, "INVALID param");

	Elm_Object_Item *item = event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	ms_main_menu_item_t menu_item = (ms_main_menu_item_t)data;
	if (menu_item == MS_MAIN_MENU_LYRICS) {
		bool lyrics_state = false;
		preference_get_boolean(KEY_MUSIC_LYRICS, &lyrics_state);
		preference_set_boolean(KEY_MUSIC_LYRICS, !lyrics_state);
		elm_genlist_item_update(event_info);
	} else if (menu_item == MS_MAIN_MENU_PLAYLIST) {
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);

		MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
		MP_CHECK(view_mgr);

		Evas_Object *popup = NULL;
		popup = mp_genlist_popup_create(ad->win_main, 
			MP_POPUP_SETTING_PLAYLIST, NULL, ad);
		mp_retm_if(!popup, "popup is NULL !!!");
		mp_popup_response_callback_set(popup, _ms_setting_playlist_cb, NULL);

		Evas_Object *genlist = evas_object_data_get(popup, "genlist");
		_ms_playlist_append_pop_genlist(genlist, view_mgr->navi);

		evas_object_show(popup);

	} else {
		Eina_Bool expanded = EINA_FALSE;
		expanded = elm_genlist_item_expanded_get(event_info);
		elm_genlist_item_expanded_set(event_info, !expanded);
	}
}

static void
_ms_load_genlist_itc(MpSettingView_t *view)
{
	mp_retm_if (!view, "INVALID param");

	/*if (view->itc[MS_ITC_TYPE_1TEXT_NO_EXP] == NULL) {
		view->itc[MS_ITC_TYPE_1TEXT_NO_EXP] = 
		elm_genlist_item_class_new();
		mp_assert(view->itc[MS_ITC_TYPE_1TEXT_NO_EXP]);
		view->itc[MS_ITC_TYPE_1TEXT_NO_EXP]->func.text_get = 
			_ms_gl_label_get;
		view->itc[MS_ITC_TYPE_1TEXT_NO_EXP]->item_style = "1line";
	}*/

	if (view->itc[MS_ITC_TYPE_1TEXT_1ICON] == NULL) {
		view->itc[MS_ITC_TYPE_1TEXT_1ICON] = 
				elm_genlist_item_class_new();
		mp_assert(view->itc[MS_ITC_TYPE_1TEXT_1ICON]);
		view->itc[MS_ITC_TYPE_1TEXT_1ICON]->func.text_get = _ms_gl_lyrics_label_get;
		view->itc[MS_ITC_TYPE_1TEXT_1ICON]->func.content_get = 
				_ms_gl_contents_get;
		view->itc[MS_ITC_TYPE_1TEXT_1ICON]->item_style = "1line";
	}
}


static void
_ms_append_genlist_items(Evas_Object *genlist, MpSettingView_t *view)
{
	int i;
	Elm_Genlist_Item_Class *itc;
	/* Elm_Object_Item *item; */
	Elm_Genlist_Item_Type flag = ELM_GENLIST_ITEM_NONE;
	mp_retm_if(!view, "INVALID param");

	_ms_load_genlist_itc(view);

	for (i = 0; i < MS_MAIN_MENU_ITEM_MAX; i++) {
		if (i == MS_MAIN_MENU_LYRICS) {
			flag = ELM_GENLIST_ITEM_NONE;
			itc = view->itc[MS_ITC_TYPE_1TEXT_1ICON];
		} else if (i == MS_MAIN_MENU_PLAYLIST) {
			flag = ELM_GENLIST_ITEM_NONE;
			itc = view->itc[MS_ITC_TYPE_1TEXT_NO_EXP];
		}

		view->gl_it[i] = elm_genlist_item_append(genlist, itc, 
			(void *)i, NULL, flag, _gl_sel,
						(void *)i);
	}

	/* evas_object_smart_callback_add(genlist, "expanded", 
		_gl_exp, genlist);
	evas_object_smart_callback_add(genlist, "contracted", 
		_gl_con, genlist); */
}

static void
_mp_setting_view_gl_realized_cb(void *data, Evas_Object *obj, void *event_info)
{
	Elm_Object_Item *gl_item = event_info;
	MP_CHECK(gl_item);

	const char *signal = NULL;

	if (gl_item == elm_genlist_first_item_get(obj))
		signal = "elm,state,top";
	else if (gl_item == elm_genlist_last_item_get(obj))
		signal = "elm,state,bottom";
	else
		signal = "elm,state,center";

	elm_object_item_signal_emit(gl_item, signal, "");
}

static void _mp_setting_view_gl_resize_cb(void *data, Evas *e, 
		Evas_Object *obj, void *event_info)
{
	MpSettingView_t *view = (MpSettingView_t *)data;
	MP_CHECK(view);

	Elm_Object_Item *customized_item = (Elm_Object_Item *)evas_object_data_get(
			obj, "customized_item");
	int customized_on = (int)evas_object_data_get(
		view->content, "customized_on");

	if (customized_on)
		elm_genlist_item_show(customized_item, 
				ELM_GENLIST_ITEM_SCROLLTO_IN);
}

static Evas_Object*
_mp_setting_view_create_list(MpSettingView_t *view, Evas_Object *parent)
{
	MP_CHECK_VAL(view, NULL);

	Evas_Object *genlist = mp_widget_genlist_create(parent);
	elm_scroller_policy_set(genlist, 
		ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(genlist, 
		EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, 
		EVAS_HINT_FILL, EVAS_HINT_FILL);
	view->content = genlist;
	elm_object_style_set(genlist, "dialogue");
	evas_object_smart_callback_add(genlist, "realized", 
		_mp_setting_view_gl_realized_cb, view);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_RESIZE, 
		_mp_setting_view_gl_resize_cb, view);

	_ms_append_genlist_items(genlist, view);

	_ms_key_change_cb_init(view);

	evas_object_show(genlist);
	return genlist;
}

static Eina_Bool
_mp_setting_view_pop_cb(void *data, Elm_Object_Item *it)
{
	startfunc;

	MpSettingView_t *view = (MpSettingView_t *)data;
	MP_CHECK_VAL(view, EINA_TRUE);
	mp_view_mgr_pop_view(GET_VIEW_MGR, true);

	endfunc;
	return EINA_TRUE;
}

static int
_mp_setting_view_update(void *thiz)
{
	startfunc;
	MpSettingView_t *view = thiz;
	MP_CHECK_VAL(view, -1);
	MP_CHECK_VAL(view->navi_it, -1);

	if (view->setting_type == MP_SETTING_VIEW_DEFAULT && view->content) {
		elm_genlist_realized_items_update(view->content);
	}
	endfunc;
	return 0;
}

static void _mp_setting_view_refresh(void *thiz)
{
	startfunc;
	MpSettingView_t *view = thiz;
	MP_CHECK(view);
	MP_CHECK(view->navi_it);

	Evas_Object *content = elm_object_part_content_get(
		view->layout, "list_content");
	if (content) {
		mp_evas_object_del(content);
		view->content = NULL;
	}
	if (view->setting_type == MP_SETTING_VIEW_TABS) {
		ms_key_get_tabs_val(&tab_state);
		view->content = _mp_setting_view_tabs_list_create(view, 
			view->parent);
	} else if (view->setting_type == MP_SETTING_VIEW_PLAYLISTS) {
		view->content = ms_playlist_list_create(
		view, view->parent);
	}

		MP_CHECK(view->content);
		elm_object_part_content_set(view->layout, 
			"list_content", view->content);
		/* elm_object_signal_emit(view->layout, 
			"SHOW_INFO_TEXT_PADDING", ""); */
}
static Eina_Bool _mp_setting_view_reorder_back_cb(void *thiz)
{
	startfunc;
	MpSettingView_t *view = thiz;
	MP_CHECK_FALSE(view);

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	elm_naviframe_item_pop(view_mgr->navi);

	view->back_timer = NULL;
	return EINA_FALSE;
}


void _mp_setting_view_tabs_reorder_update_cb(void *data, 
		Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpSettingView_t *view = (MpSettingView_t *)data;
	MP_CHECK(view);
	char str[TAB_COUNT+1] = {0};
	int i = 0;
	Elm_Object_Item *temp = elm_genlist_first_item_get(view->content);
	while (temp) {
		mp_setting_genlist_item_data_t *item_data = 
			elm_object_item_data_get(temp);
		MP_CHECK(item_data);
		str[i++] = (item_data->index+0x30);
		temp = elm_genlist_item_next_get(temp);
	}
	str[TAB_COUNT] = '\0';
	/* preference_set_string(MP_PREFKEY_TABS_VAL_STR,str); */
	ms_key_set_tabs_str(str);

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_TABS_REORDER_DONE);

	view->back_timer = ecore_timer_add(0.1, 
		_mp_setting_view_reorder_back_cb, view);

}

static void _mp_setting_reorder_cb(void *data, Evas_Object *obj,
		void *event_info)
{
	eventfunc;
	MpSettingView_t *view = (MpSettingView_t *) data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	MP_CHECK(view_mgr);
	MpSettingView_t *reorder_view = mp_setting_view_create(view_mgr->navi, 
		MP_SETTING_VIEW_REORDERS, (void *)view->setting_type);
	mp_view_mgr_push_view(view_mgr, (MpView_t *)reorder_view, NULL);
	mp_view_update_options((MpView_t *)reorder_view);
	mp_view_set_title((MpView_t *)reorder_view, STR_MP_REORDER);
}


static void _mp_tab_view_normal_more_btn_cb(void *data, 
		Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpSettingView_t *view = (MpSettingView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_REORDER, MP_PLAYER_MORE_BTN_SET_REORDER,
				_mp_setting_reorder_cb,
								view);


	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);
}

void _mp_setting_view_playlists_reorder_update_cb(void *data, Evas_Object 
		*obj, void *event_info)
{
eventfunc;
	MpSettingView_t *view = (MpSettingView_t *)data;
		MP_CHECK(view);
		/* int index = 0; */
		char str[5] = {0};
	int i = 0;
	Elm_Object_Item *temp = elm_genlist_first_item_get(view->content);
	while (temp) {
		/* index = elm_genlist_item_index_get(temp); */
		mp_setting_genlist_item_data_t *item_data = 
			elm_object_item_data_get(temp);
		MP_CHECK(item_data);
		str[i++] = (item_data->index+0x30);
		temp = elm_genlist_item_next_get(temp);
	}
	str[4] = '\0';
	ms_key_set_playlist_str(str);
	/* preference_set_string(MP_PREFKEY_TABS_VAL_STR,str); */

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLISTS_REORDER_DONE);

	view->back_timer = ecore_timer_add(0.1, 
		_mp_setting_view_reorder_back_cb, view);

}

static Eina_Bool _mp_setting_view_back_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpSettingView_t *view = (MpSettingView_t *) data;
	MP_CHECK_VAL(view, EINA_TRUE);

	MpViewMgr_t *view_mgr = mp_view_mgr_get_view_manager();
	mp_view_mgr_pop_view(view_mgr, false);
	if (view->setting_type == MP_SETTING_VIEW_TABS) {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_TABS_ITEM_CHANGED);
	} else if (view->setting_type == MP_SETTING_VIEW_DEFAULT) {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_LYRIC_UPDATE);
	}

	return EINA_TRUE;
}


static int
_mp_setting_view_update_option_cb(void *thiz)
{
	startfunc;
	MpSettingView_t *view = thiz;
	MP_CHECK_VAL(view, -1);
	MP_CHECK_VAL(view->navi_it, -1);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_setting_view_pop_cb, 
		view);

	Evas_Object *right_btn = elm_object_item_part_content_unset(
		view->navi_it, "title_right_btn");
	mp_evas_object_del(right_btn);

	Evas_Object *left_btn = elm_object_item_part_content_unset(
		view->navi_it, "title_left_btn");
	mp_evas_object_del(left_btn);

	if (view->setting_type == MP_SETTING_VIEW_TABS || view->setting_type 
			== MP_SETTING_VIEW_PLAYLISTS) {
		Evas_Object *btn = NULL;
		btn = mp_widget_create_toolbar_btn(view->layout, 
			MP_TOOLBAR_BTN_MORE, NULL, 
			_mp_tab_view_normal_more_btn_cb, view);
		elm_object_item_part_content_set(view->navi_it, 
			"toolbar_more_btn", btn);

	} else if (view->setting_type ==  MP_SETTING_VIEW_REORDERS) {
		Evas_Object *toolbar = mp_widget_create_naviframe_toolbar(
			view->navi_it);
		Elm_Object_Item *toolbar_item = NULL;

		if (view->reorder_type == MP_SETTING_REORDER_TABS) {
			toolbar_item = mp_widget_create_toolbar_item_btn(toolbar
			, MP_TOOLBAR_BTN_LEFT, STR_MP_DONE,
			_mp_setting_view_tabs_reorder_update_cb, view);
		} else if (view->reorder_type == MP_SETTING_REORDER_PLAYLISTS) {
			toolbar_item = mp_widget_create_toolbar_item_btn(
				toolbar, MP_TOOLBAR_BTN_LEFT, STR_MP_DONE, 
				_mp_setting_view_playlists_reorder_update_cb, 
				view);
		}
		view->toolbar_options[MP_OPTION_LEFT] = toolbar_item;
		if (!view->reorder) {
			elm_object_item_disabled_set(view->toolbar_options[
				MP_OPTION_LEFT], EINA_TRUE);
		}
	}
	elm_naviframe_item_pop_cb_set(view->navi_it, 
		_mp_setting_view_back_cb, view);

	endfunc;
	return 0;
}

static void
_mp_setting_view_destory_cb(void *thiz)
{
	startfunc;
	MpSettingView_t *view = thiz;
	MP_CHECK(view);
	mp_setting_view_destory(view);

	mp_view_fini((MpView_t *)view);

	free(view);
}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_setting_view_rotate(void *thiz, int init_rotate)
{
	DEBUG_TRACE("mp_player rotated %d", init_rotate);
	MpSettingView_t *view = thiz;
	MP_CHECK(view);
}
#endif

static void
_mp_setting_view_on_event(void *thiz, MpViewEvent_e event)
{
	MpSettingView_t *view = thiz;
	DEBUG_TRACE("event is %d", event);
	switch (event) {
		case MP_PLAYLISTS_REORDER_DONE:
		case MP_TABS_REORDER_DONE:
		{
			if ((view->setting_type == MP_SETTING_VIEW_TABS 
				|| view->setting_type ==
				MP_SETTING_VIEW_PLAYLISTS) 
				&& view->content) {
				_mp_setting_view_refresh(view);
			}
		}
			break;
		default:
			break;
	}

}

static void _mp_setting_view_resume(void *thiz)
{
	startfunc;
	MpSettingView_t *view = (MpSettingView_t *)thiz;
	_mp_setting_view_update((void *)view);
}

static void _mp_tabs_reorder_gl_sel(void *data, 
		Evas_Object *obj, void *event_info)
{
	mp_retm_if(!obj, "INVALID param");
	mp_retm_if(!event_info, "INVALID param");

	Elm_Object_Item *item = event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);

}

static void _mp_tabs_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	mp_retm_if(!obj, "INVALID param");
	mp_retm_if(!event_info, "INVALID param");
	mp_setting_genlist_item_data_t *item_data = 
		(mp_setting_genlist_item_data_t *)data;
	MP_CHECK(item_data);

	Elm_Object_Item *item = event_info;
	elm_genlist_item_selected_set(item, EINA_FALSE);

	if (elm_check_state_get(check_boxs[item_data->index-1]))
		elm_check_state_set(check_boxs[item_data->index-1], FALSE);
	else
		elm_check_state_set(check_boxs[item_data->index-1], TRUE);

	evas_object_smart_callback_call(check_boxs[item_data->index-1], 
		"changed", NULL);
}

static char *
_mp_tabs_gl_label_get(void *data, Evas_Object * obj, const char *part)
{

	mp_setting_genlist_item_data_t *item_data = 
		(mp_setting_genlist_item_data_t *)data;
	MP_CHECK_NULL(item_data);

	if (strcmp(part, "elm.text") == 0) {
		return g_strdup(GET_STR(item_data->str));
	}
	return NULL;


}

static void _mp_setting_tabs_check_changed_cb(void *data, 
	Evas_Object *obj, void *event_info)
{
	int index = (int)evas_object_data_get(obj, "index");
	DEBUG_TRACE("index:%d", index);

	if (tab_state & (1 << index))
		tab_state &= ~(1 << index);
	else
		tab_state |= (1 << index);

	DEBUG_TRACE("set to 0x%x", tab_state);
	ms_key_set_tabs_val(tab_state);

	return;
}


static Evas_Object *
_mp_tabs_gl_icon_get(void *data, Evas_Object * obj, const char *part)
{

	mp_setting_genlist_item_data_t *item_data = 
		(mp_setting_genlist_item_data_t *)data;
	MP_CHECK_NULL(item_data);
	int param = item_data->index-1;

	if (param == 0 || param == 1) {
		elm_object_item_disabled_set(item_data->it, EINA_TRUE);
	}

	if (strcmp(part, "elm.icon") == 0) {
		Evas_Object *check_box = elm_check_add(obj);
		elm_object_style_set(check_box, "default/genlist");
		evas_object_data_set(check_box, "index", (void *)param);
		DEBUG_TRACE("%d", param);
		evas_object_repeat_events_set(check_box, EINA_TRUE);
		evas_object_propagate_events_set(check_box, FALSE);
		elm_check_state_set(check_box, tab_state & (1 << param));
		evas_object_smart_callback_add(check_box, "changed", 
			_mp_setting_tabs_check_changed_cb, NULL);

		evas_object_show(check_box);

		check_boxs[param] = check_box;
		return check_box;
	}
	return NULL;
}

void _mp_tabs_sequence_get()
{
	char *get_str = NULL;
	ms_key_get_tabs_str(&get_str);
	int value = atoi(get_str);
	/* int index[TAB_COUNT] = {0}; */
	int j = 0;
	for (j = TAB_COUNT-1; j >= 0 ; j--) {
		tab_index[j] = value%10;
		value = value / 10;
	}

}

static void
_mp_tabs_list_item_del_cb(void *data, Evas_Object *obj)
{
	mp_setting_genlist_item_data_t *item_data = data;
	MP_CHECK(item_data);
	IF_FREE(item_data->str);
	IF_FREE(item_data);
}

EXPORT_API void mp_setting_items_reorder_cb(void *data, 
	Evas_Object *obj, void *event_info)
{
	startfunc;
	MpSettingView_t *view = (MpSettingView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->content);
	Evas_Object *genlist = view->content;

	int cur_sequence = 0;
	Elm_Object_Item *temp = elm_genlist_first_item_get(genlist);
	while (temp) {
		mp_setting_genlist_item_data_t *item_data = (
			mp_setting_genlist_item_data_t *) 
			elm_object_item_data_get(temp);
		MP_CHECK(item_data);
		if (cur_sequence != item_data->seq) {
			elm_object_item_disabled_set(view->toolbar_options[
				MP_OPTION_LEFT], EINA_FALSE);
			view->reorder = TRUE;
			return;
		}
		temp = elm_genlist_item_next_get(temp);
		cur_sequence++;
	}
	elm_object_item_disabled_set(view->toolbar_options[
		MP_OPTION_LEFT], EINA_TRUE);
	view->reorder = FALSE;
}


static void
_mp_tabs_append_genlist_items(Evas_Object *genlist, MpSettingView_t *view)
{
	int i;
	mp_retm_if(!view, "INVALID param");
	MP_CHECK(view);
	Elm_Genlist_Item_Class *itc = NULL;
	if (view->setting_type == MP_SETTING_VIEW_TABS) {
		if (!view->tabs_itc[0]) {
			view->tabs_itc[0] = elm_genlist_item_class_new();
			MP_CHECK(view->tabs_itc[0]);
			view->tabs_itc[0]->func.text_get = 
				_mp_tabs_gl_label_get;
			view->tabs_itc[0]->func.content_get = 
				_mp_tabs_gl_icon_get;
			view->tabs_itc[0]->item_style = 
				"dialogue/1text.1icon/expandable2";
			view->tabs_itc[0]->func.del = _mp_tabs_list_item_del_cb;
		}
		itc = view->tabs_itc[0];
	} else if (view->setting_type == MP_SETTING_VIEW_REORDERS) {
		if (!view->tabs_itc[1]) {
			view->tabs_itc[1] = elm_genlist_item_class_new();
			MP_CHECK(view->tabs_itc[1]);
			view->tabs_itc[1]->func.text_get = 
				_mp_tabs_gl_label_get;
			view->tabs_itc[1]->func.content_get = 
				_mp_tabs_gl_icon_get;
			view->tabs_itc[1]->func.del = _mp_tabs_list_item_del_cb;
			view->tabs_itc[1]->item_style = "dialogue/1text";
	}
		itc = view->tabs_itc[1];
		evas_object_smart_callback_add(genlist, "moved", 
			mp_setting_items_reorder_cb, view);
	}


	/*get tab sequence */
	 _mp_tabs_sequence_get();

	for (i = 0; i < TAB_COUNT; i++) {
	 int m = tab_index[i];
		DEBUG_TRACE("m  %d %s", m, tab_str[m-1]);
		mp_setting_genlist_item_data_t *item_data = calloc(1, 
			sizeof(mp_setting_genlist_item_data_t));
		MP_CHECK(item_data);
		item_data->index = m;
		item_data->seq = i;
		item_data->str = g_strdup(tab_str[m-1]);
		if (view->setting_type == MP_SETTING_VIEW_TABS) {
			item_data->it  = elm_genlist_item_append(genlist, itc,
				(void *)item_data, NULL, ELM_GENLIST_ITEM_NONE, 
				_mp_tabs_gl_sel, (void *)item_data);
		} else if (view->setting_type == MP_SETTING_VIEW_REORDERS) {
			item_data->it  = elm_genlist_item_append(genlist, itc,
				(void *)item_data, NULL, ELM_GENLIST_ITEM_NONE, 
				_mp_tabs_reorder_gl_sel,
						(void *)item_data);
		}
		elm_object_item_data_set(item_data->it, item_data);
	}
}


static Evas_Object*
_mp_setting_view_tabs_list_create(MpSettingView_t *view, Evas_Object *parent)
{
	MP_CHECK_VAL(view, NULL);

	Evas_Object *genlist = mp_widget_genlist_create(parent);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, 
		ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(genlist, 
		EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, 
		EVAS_HINT_FILL, EVAS_HINT_FILL);

	elm_object_style_set(genlist, "dialogue");
	evas_object_smart_callback_add(genlist, "realized", 
		_mp_setting_view_gl_realized_cb, view);
	evas_object_event_callback_add(genlist, EVAS_CALLBACK_RESIZE, 
		_mp_setting_view_gl_resize_cb, view);

	_mp_tabs_append_genlist_items(genlist, view);

	evas_object_show(genlist);
	return genlist;
}

void mp_lyrics_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
	elm_genlist_realized_items_update(obj);
}

static void _lyrics_state_on_cb(void *data, Evas_Object * obj, void *event_info)
{
	mp_setting_lyric_popup *ly_popup = (mp_setting_lyric_popup *)data;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	if (item != NULL) {
		elm_genlist_item_selected_set(item, FALSE);
	}

	if (ad->b_show_lyric == 0) {
		preference_set_boolean(KEY_MUSIC_LYRICS, 1);
		elm_radio_value_set(group_radio, 0);
		ad->b_show_lyric = 1;
	}

	evas_object_del(ly_popup->popup);
	MpPlayerView_t *player_view = (MpPlayerView_t *)GET_PLAYER_VIEW;
	if (player_view) {
		mp_player_view_refresh(player_view);
	}
}

static void _lyrics_state_off_cb(void *data, Evas_Object * obj, void *event_info)
{
	mp_setting_lyric_popup *ly_popup = (mp_setting_lyric_popup *)data;
	Elm_Object_Item *item = (Elm_Object_Item *) event_info;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	if (item != NULL) {
		elm_genlist_item_selected_set(item, FALSE);
	}

	if (ad->b_show_lyric == 1) {
		preference_set_boolean(KEY_MUSIC_LYRICS, 0);
		elm_radio_value_set(group_radio, 1);
		ad->b_show_lyric = 0;
	}

	evas_object_del(ly_popup->popup);
	MpPlayerView_t *player_view = (MpPlayerView_t *)GET_PLAYER_VIEW;
	if (player_view) {
		mp_player_view_refresh(player_view);
	}
}

static char *_lyrics_view_label_get(void *data, Evas_Object * obj, const char *part)
{
	int index = (int)data;
	if (index == 0) {
		return g_strdup(GET_STR(STR_MP_SHOW_LYRICS));
	} else {
		return g_strdup(GET_STR(STR_MP_HIDE_LYRICS));
	}
	return NULL;
}

static Evas_Object *_lyrics_view_content_get(void *data, Evas_Object * obj, const char *part)
{
	int index = (int)data;
	if (!strcmp(part, "elm.swallow.end")) {

		bool lyrics_state;
		preference_get_boolean(KEY_MUSIC_LYRICS, &lyrics_state);

		if (lyrics_state) {
			elm_radio_value_set(group_radio, 0);
		} else {
			elm_radio_value_set(group_radio, 1);
		}

		Evas_Object *radio = elm_radio_add(obj);
		elm_radio_group_add(radio, group_radio);
		elm_radio_state_value_set(radio, index);
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_propagate_events_set(radio, EINA_TRUE);
		evas_object_show(radio);
		return radio;
	}
	return NULL;
}

void mp_music_viewas_pop_cb(void)
{
	startfunc;

	int index = 0;
	Evas_Object *box = NULL;
	Evas_Object *popup = elm_popup_add(GET_WINDOW());
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	Evas_Object *genlist = NULL;
	Elm_Genlist_Item_Class *itc = NULL;
	/* elm_object_style_set(popup, "content/default"); */
	elm_popup_orient_set(popup, ELM_POPUP_ORIENT_CENTER);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND,
		EVAS_HINT_EXPAND);
	mp_util_domain_translatable_part_text_set(popup,"title,text",STR_MP_LYRICS);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK,
		eext_popup_back_cb, NULL);
	evas_object_repeat_events_set(popup, EINA_FALSE);

	genlist = elm_genlist_add(popup);
	elm_object_focus_set(genlist, EINA_FALSE);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_smart_callback_add(genlist, "language,changed", mp_lyrics_lang_changed, NULL);

	box = elm_box_add(popup);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_min_set(box, -1,
						ELM_SCALE_SIZE(MP_POPUP_MENUSTYLE_HEIGHT(2)));
	elm_box_pack_end(box, genlist);
	evas_object_show(popup);
	evas_object_show(genlist);
	elm_object_content_set(popup, box);


	group_radio = elm_radio_add(genlist);
	elm_radio_state_value_set(group_radio, -1);
	elm_radio_value_set(group_radio, -1);

	mp_setting_lyric_popup *ly_popup = (mp_setting_lyric_popup *)malloc(sizeof(mp_setting_lyric_popup));
	MP_CHECK(ly_popup);

	ly_popup->popup = popup;
	ly_popup->group_radio = group_radio;

	itc = elm_genlist_item_class_new();
	if (itc) {
		itc->item_style = "default";
		itc->func.text_get = _lyrics_view_label_get;
		itc->func.content_get = _lyrics_view_content_get;
		itc->func.del = NULL;
	}
	elm_genlist_item_append(genlist, itc, 0, NULL,
			ELM_GENLIST_ITEM_NONE, _lyrics_state_on_cb, (mp_setting_lyric_popup *)ly_popup);
	elm_genlist_item_append(genlist, itc, 1, NULL,
			ELM_GENLIST_ITEM_NONE, _lyrics_state_off_cb, (mp_setting_lyric_popup *)ly_popup);
}

static int
_mp_setting_view_init(Evas_Object *parent, MpSettingView_t *view, 
		MpSettingViewType_e type, void *data)
{
	startfunc;
	int ret = 0;

	ret =  mp_view_init(parent, (MpView_t *)view, MP_VIEW_SETTING);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_setting_view_update;
	view->update_options = _mp_setting_view_update_option_cb;
	view->update_options_edit = NULL;
	view->view_destroy_cb = _mp_setting_view_destory_cb;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;
	view->start_playback = NULL;
	view->pause_playback = NULL;
	view->on_event = _mp_setting_view_on_event;
	view->view_resume = _mp_setting_view_resume;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_setting_view_rotate;
#endif

	view->setting_type = type;

	if (type == MP_SETTING_VIEW_DEFAULT)
		view->content = _mp_setting_view_create_list(view, parent);
	else if (type == MP_SETTING_VIEW_TABS) {
		ms_key_get_tabs_val(&tab_state);
		view->content = _mp_setting_view_tabs_list_create(view, parent);
	} else if (type == MP_SETTING_VIEW_REORDERS) {
	int parent_type = (int)data;
		if (parent_type == MP_SETTING_VIEW_TABS) {
			view->content = _mp_setting_view_tabs_list_create(
				view, parent);
			view->reorder_type = MP_SETTING_REORDER_TABS;
		} else if (parent_type == MP_SETTING_VIEW_PLAYLISTS) {
			view->content = ms_playlist_list_create(view, parent);
			view->reorder_type = MP_SETTING_REORDER_PLAYLISTS;
		}
		mp_list_reorder_mode_set(view->content, EINA_TRUE);
		mp_list_select_mode_set(view->content, 
			ELM_OBJECT_SELECT_MODE_ALWAYS);
	} else if (type == MP_SETTING_VIEW_PLAYLISTS) {
		view->content = ms_playlist_list_create(view, parent);
	} else
		return -1;
	MP_CHECK_VAL(view->content, -1);
	elm_object_part_content_set(view->layout, "list_content", 
		view->content);
	/*elm_object_signal_emit(view->layout, "SHOW_INFO_TEXT_PADDING", "");*/

	return ret;
}

/* param void *data is used to update previous view.. */
EXPORT_API MpSettingView_t *mp_setting_view_create(Evas_Object *parent, 
	MpSettingViewType_e type, void *data)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpSettingView_t *view = calloc(1, sizeof(MpSettingView_t));
	MP_CHECK_NULL(view);
	view->parent = parent;
	ret = _mp_setting_view_init(parent, view, type, data);
	if (ret) goto Error;

	return view;

Error:
	ERROR_TRACE("Error: mp_setting_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_setting_view_destory(MpSettingView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);
	_ms_key_change_cb_deinit();

	/* free item classes */
	int i = 0;
	for (; i < MS_ITC_TYPE_NUM; i++) {
		if (view->itc[i]) {
			elm_genlist_item_class_free(view->itc[i]);
			view->itc[i] = NULL;
		}
	}
	int j = 0;
	for (; j < 2; j++) {
		if (view->tabs_itc[j]) {
			elm_genlist_item_class_free(view->tabs_itc[j]);
			view->tabs_itc[j] = NULL;
		}
	}
	return 0;
}

