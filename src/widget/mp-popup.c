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

#include <bundle.h>
#include <stdio.h>
#include "music.h"
#include "mp-menu.h"
#include "mp-popup.h"
#include "mp-item.h"
#include "mp-player-debug.h"
#include "mp-playlist-mgr.h"
#include "mp-common.h"
#include <sound_manager.h>
#include "mp-util.h"
#include "mp-widget.h"
#include <efl_extension.h>

static Elm_Genlist_Item_Class itc;

typedef struct {
	struct appdata *ad;
	mp_popup_type type;

	Evas_Smart_Cb response_cb;
	void *cb_data;
} Popup_Data;

typedef struct _AllShareListItem {
	Elm_Object_Item *pItem;
	char *deviceId;
	int nIndex;
	char *szName;
	char *thumb;
	void *data;

} AllShareListItem;

typedef struct {
	void* drm_info;
	void* wifidirect_info;
	Ecore_Timer *timer;
	GList	*pItemList;
	Evas_Object * parent;
	bool bStopScan;
	Elm_Object_Item *pLoadingItem;
} Allshare_Data;


#define mp_popup_set_popup_data(obj, data) evas_object_data_set((obj), "popup_data", (data))
#define mp_popup_get_popup_data(obj) evas_object_data_get((obj), "popup_data")
#define MP_POPUP_ALLSHARE_WIFI_ID_START 800
#define MP_POPUP_ALLSHARE_MYDEVICE_ID   -1
#define MP_POPUP_ALLSHARE_LOAD_ID   -2

static void __mp_popup_timeout_cb(void *data, Evas_Object *obj, void *event_info);
static void __mp_popup_block_cb(void *data, Evas_Object *obj, void *event_info);

static char *
_mp_popup_gl_label_get2(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.text")) {
		MP_CHECK_NULL(data);
		Popup_genlist_item *gli_data = (Popup_genlist_item *)data;
		DEBUG_TRACE("gli_data->lable is %s", gli_data->label);
		DEBUG_TRACE("gli_data->content is %s", gli_data->content);
		char *label = NULL;
		if (gli_data->content != NULL) {
			label = g_strconcat(GET_STR(gli_data->label), " : ", gli_data->content, NULL);
		} else {
			label = g_strdup(GET_STR(gli_data->label));
		}

		return label;
	}
	return NULL;
}

static char *
_mp_popup_gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	if (!strcmp(part, "elm.text.main.left")) {
		return g_strdup(data);
	}
	return NULL;
}

static void
_mp_popup_gl_del(void *data, Evas_Object * obj)
{
	Popup_genlist_item *gli_data = (Popup_genlist_item *)data;
	MP_CHECK(gli_data);
	IF_FREE(gli_data->label);
	IF_FREE(gli_data->content);
	IF_FREE(gli_data);
}

static Evas_Object *
_mp_popup_gl_icon_get(void *data, Evas_Object * obj, const char *part)
{
	MP_CHECK_NULL(data);
	Popup_genlist_item *gli_data = (Popup_genlist_item *)data;

	struct appdata *ad = evas_object_data_get(obj, "ad");
	MP_CHECK_NULL(ad);

	if (!g_strcmp0(part, "elm.icon.2")) {
		Evas_Object *layout = NULL;
		layout = elm_layout_add(obj);
		Evas_Object *radio = elm_radio_add(layout);
		elm_layout_theme_set(layout, "layout", "list/C/type.2", "default");
		int index = 0;
		index = (int)gli_data->item_data;
		DEBUG_TRACE("index=%d,radio_group=%p,snd_path=%d", index, ad->radio_group, ad->snd_path);

		elm_radio_state_value_set(radio, index);
		elm_radio_group_add(radio, ad->radio_group);
		if (index == ad->snd_path) {
			elm_radio_value_set(ad->radio_group, index);
		}
		evas_object_size_hint_weight_set(radio, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(radio, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_data_set(radio, "idx", (void *)(index));
		elm_object_part_content_set(layout, "elm.swallow.content", radio);
		evas_object_show(radio);
		evas_object_show(layout);

		return layout;
	}

	return NULL;
}

static void
mp_popup_set_min_size(Evas_Object *box, int cnt)
{
	int min_h = 0;
	MP_CHECK(box);

	if (mp_util_is_landscape()) {
		if (MP_POPUP_GENLIST_ITEM_H*cnt > MP_POPUP_GENLIST_ITEM_H_MAX_LD || cnt == 0) {
			min_h = MP_POPUP_GENLIST_ITEM_H_MAX_LD;
		} else {
			min_h = MP_POPUP_GENLIST_ITEM_H * cnt;
		}
	} else {
		if (MP_POPUP_GENLIST_ITEM_H*cnt > MP_POPUP_GENLIST_ITEM_H_MAX || cnt == 0) {
			min_h = MP_POPUP_GENLIST_ITEM_H_MAX;
		} else {
			min_h = MP_POPUP_GENLIST_ITEM_H * cnt;
		}
	}

	ERROR_TRACE("wishjox cnt: %d,  min_h: %d", cnt, min_h);
	evas_object_size_hint_min_set(box,
	                              -1, min_h);

}

static void
_mp_popup_cancel_button_cb(void *data, Evas_Object *obj, void *event_info)
{
	evas_object_del(obj);
}

static void _mp_popup_genlist_gl_lang_changed(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	elm_genlist_realized_items_update(obj);
}

static void _mp_popup_gl_realized(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	if (!event_info || !obj) {
		ERROR_TRACE("event or obj get error");
		return;
	}

	Elm_Object_Item *current_item = event_info;
	//genlist last item
	Elm_Object_Item *last_item = elm_genlist_last_item_get(obj);
	MP_CHECK(last_item);

	if (current_item == last_item) {
		elm_object_item_signal_emit(last_item, "elm,state,bottomline,hide", ""); //send this signal
	}
}
Evas_Object *
elm_popup_win_get(Evas_Object *popup)
{
	return evas_object_top_get(evas_object_evas_get(popup));
}

static Evas_Object *
_mp_popup_create_min_style_popup(Evas_Object * parent, char *title, int cnt,
                                 void *user_data, Evas_Smart_Cb cb, struct appdata *ad)
{
	Evas_Object *genlist = NULL;
	Evas_Object *box = NULL;
	Evas_Object *popup = mp_popup_create(parent, MP_POPUP_GENLIST, title, user_data, cb, ad);
	MP_CHECK_NULL(popup);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	//layout = elm_layout_add(popup);
	//elm_layout_theme_set(layout, "layout", "content", "min_menustyle");
	//evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	box = elm_box_add(popup);
	MP_CHECK_NULL(box);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	genlist = elm_genlist_add(box);
	MP_CHECK_NULL(genlist);
	//elm_object_style_set(genlist, "popup");
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);

	evas_object_smart_callback_add(genlist, "language,changed", _mp_popup_genlist_gl_lang_changed, NULL);
	evas_object_smart_callback_add(genlist, "realized", _mp_popup_gl_realized, NULL);
	evas_object_data_set(popup, "genlist", genlist);

	elm_box_pack_end(box, genlist);
	evas_object_show(genlist);
	mp_popup_set_min_size(box, cnt);

	elm_object_content_set(popup, box);
	evas_object_show(popup);

	return popup;
}

static Evas_Object *
_mp_popup_list_share_create(Evas_Object * parent, void *user_data, struct appdata *ad)
{
	DEBUG_TRACE("");
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	Evas_Object *genlist;
	int option_count = 3;
#ifdef MP_FEATURE_WIFI_SHARE
	++option_count;
#endif

	popup = _mp_popup_create_min_style_popup(parent, GET_SYS_STR("IDS_COM_BUTTON_SHARE"), option_count, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	itc.item_style = "1text";
	itc.func.text_get = _mp_popup_gl_label_get;
	itc.func.content_get = NULL;
	itc.func.state_get = NULL;
	itc.func.del = NULL;

	genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK_NULL(genlist);

	elm_genlist_item_append(genlist, &itc, BLUETOOTH_SYS, NULL,
	                        ELM_GENLIST_ITEM_NONE, mp_menu_genlist_popup_list_share_select_cb, user_data);
	elm_genlist_item_append(genlist, &itc, EMAIL_SYS, NULL,
	                        ELM_GENLIST_ITEM_NONE, mp_menu_genlist_popup_list_share_select_cb, user_data);
#ifndef MP_FEATURE_DISABLE_MMS
	elm_genlist_item_append(genlist, &itc, MESSAGE_SYS, NULL,
	                        ELM_GENLIST_ITEM_NONE, mp_menu_genlist_popup_list_share_select_cb, user_data);
#endif
#ifdef MP_FEATURE_WIFI_SHARE
	elm_genlist_item_append(genlist, &itc, WIFI_SYS, NULL,
	                        ELM_GENLIST_ITEM_NONE, mp_menu_genlist_popup_list_share_select_cb, user_data);
#endif

	return popup;
}

static Evas_Object *
_mp_popup_sound_path_create(Evas_Object * parent, void *data, struct appdata *ad)
{
	//Replaced for _prod dependency start
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	int cnt = 1;
	sound_device_h device;
	sound_device_type_e type;
	sound_device_list_h g_device_list = NULL;
	sound_device_mask_e g_device_mask = SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
	WARN_TRACE("Enter sound_manager_get_active_device");
	int ret;
	if ((ret = sound_manager_get_current_device_list(g_device_mask, &g_device_list))) {
		ERROR_TRACE("sound_manager_get_active_device()... [0x%x]", ret);
	}

	while (!(ret = sound_manager_get_next_device(g_device_list, &device))) {
		ERROR_TRACE("success to get next device\n");
		if ((ret = sound_manager_get_device_type(device, &type))) {
			ERROR_TRACE("failed to get device type, ret[0x%x]\n", ret);
		}
		switch (type) {
		case SOUND_DEVICE_BLUETOOTH:
		case SOUND_DEVICE_HDMI:
		case SOUND_DEVICE_MIRRORING:
		case SOUND_DEVICE_USB_AUDIO:
			cnt++;
			break;
		default:
			break;
		}
	}

	popup = _mp_popup_create_min_style_popup(parent, GET_SYS_STR("IDS_COM_HEADER_AUDIO_DEVICE_ABB"), cnt, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	return popup;
	//Replaced for _prod dependency end
}

static Evas_Object *
_mp_popup_tracklist_more_info(Evas_Object * parent, void *data, struct appdata *ad)
{

	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	int count = 5;

	popup = _mp_popup_create_min_style_popup(parent, STR_MP_POPUP_MORE_INFO, count, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);
	mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_OK, MP_POPUP_YES);

	return popup;
}


static Evas_Object *
_mp_popup_tracklist_longpressed_create(Evas_Object * parent, void *data, struct appdata *ad)
{

	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	//int count = *(int *)data;
	int count = 0;

	popup = _mp_popup_create_min_style_popup(parent, GET_STR("IDS_MUSIC_BODY_ADD_TO_PLAYLIST"), count, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	return popup;
}
static Evas_Object *
_mp_popup_setting_playlist_create(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	int count = 4;

	popup = _mp_popup_create_min_style_popup(parent, GET_STR(STR_MP_PLAYLISTS), count, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);
	mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_POP_CANCEL", MP_POPUP_NO);
	mp_popup_button_set(popup, MP_POPUP_BTN_2, "Done", MP_POPUP_YES);

	return popup;
}

static void
_mp_popup_button_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	Evas_Object *popup = data;
	MP_CHECK(popup);

	int response = mp_evas_object_response_get(obj);
	mp_popup_response(popup, response);
}

bool
mp_popup_button_set_add_to_palylist(Evas_Object *popup, const char *text, int response)
{
	MP_CHECK_FALSE(popup);
	MP_CHECK_FALSE(text);

	bool ret = FALSE;
	Evas_Object *button = elm_button_add(popup);

	elm_object_style_set(button, "popup");
	mp_util_domain_translatable_text_set(button, text);
	evas_object_smart_callback_add(button, "clicked", _mp_popup_button_clicked_cb, popup);
	if (button) {
		elm_object_part_content_set(popup, "button2", button);
		mp_evas_object_response_set(button, response);
		ret = TRUE;
	}
	int playlistcount = mp_media_playlist_get_playlist_count_from_db();
	if (playlistcount < 100) {
		elm_object_disabled_set(button, FALSE);
	} else {
		elm_object_disabled_set(button, TRUE);
	}
	return ret;
}

static Evas_Object *
_mp_popup_add_to_playlist_create(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	int count = 0;

	mp_media_info_group_list_count(MP_GROUP_BY_PLAYLIST, NULL, NULL, &count);
	if (count <= 0) {
		DEBUG_TRACE("temp playlist");
		count = 1;
	}
	DEBUG_TRACE("count,%d", count);

	popup = _mp_popup_create_min_style_popup(parent, GET_STR("IDS_MUSIC_BODY_ADD_TO_PLAYLIST"), count, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);
	mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_CANCEL, MP_POPUP_NO);
	mp_popup_button_set_add_to_palylist(popup, STR_MP_CREATE, MP_POPUP_YES);

	return popup;
}

static Evas_Object *
_mp_popup_delete_track_create(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	int count = -1;

	popup = _mp_popup_create_min_style_popup(parent, GET_STR("IDS_MUSIC_POP_THIS_TRACK_WILL_BE_DELETED"), count, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);
	mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_POP_CANCEL", MP_POPUP_NO);
	mp_popup_button_set(popup, MP_POPUP_BTN_2, STR_MP_DELETE, MP_POPUP_YES);

	return popup;
}

static Evas_Object *
_mp_popup_info_list_create(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	int count = *(int *)data;

	popup = _mp_popup_create_min_style_popup(parent, GET_SYS_STR("IDS_COM_SK_SELECT"), count, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	return popup;
}

static Evas_Object *
_mp_popup_edit_image_create(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	popup = _mp_popup_create_min_style_popup(parent, GET_STR(STR_MP_EDIT_IMAGE), 2, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	return popup;
}

static Evas_Object *
_mp_popup_search_create(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);
	Evas_Object *popup = NULL;
	popup = _mp_popup_create_min_style_popup(parent, GET_STR(STR_MP_SEARCH_BY), 2, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	return popup;
}

static Evas_Object *
_mp_popup_change_list_display_mode(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);

	Evas_Object *popup = NULL;

	popup = _mp_popup_create_min_style_popup(parent, GET_STR(STR_MP_VIEW_AS), 2, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	return popup;
}

#ifdef MP_FEATURE_CLOUD
static Evas_Object *
_mp_popup_cloud_view_mode(Evas_Object * parent, void *data, struct appdata *ad)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_NULL(ad);

	Evas_Object *popup = NULL;

	popup = _mp_popup_create_min_style_popup(parent, GET_STR(STR_MP_VIEW), 3, NULL, _mp_popup_cancel_button_cb, ad);
	MP_CHECK_NULL(popup);

	return popup;
}
#endif

static void
_mp_popup_del_cb(void *data, Evas * e, Evas_Object * eo, void *event_info)
{
	DEBUG_TRACE("");
	struct appdata *ad = (struct appdata *)data;
	int type = (int)evas_object_data_get(eo, "type");
	DEBUG_TRACE("type: %d", type);
	if (type >= MP_POPUP_MAX) {
		ERROR_TRACE("Never should be here!!!");
		return;
	}
	ad->popup[type] = NULL;
}

static bool
_mp_popup_popup_exist(struct appdata *ad, mp_popup_t type)
{
	MP_CHECK_FALSE(ad);
	if (ad->popup[type]) {
		return TRUE;
	}
	return FALSE;
}

Elm_Object_Item *
mp_genlist_popup_item_append(Evas_Object * popup, char *label, char *content, Evas_Object * icon, void *item_data, void *cb, void *data)
{
	MP_CHECK_NULL(popup);
	MP_CHECK_NULL(label);

	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK_NULL(genlist);

	Popup_genlist_item *gli_data = calloc(1, sizeof(Popup_genlist_item));
	MP_CHECK_NULL(gli_data);
	gli_data->label = g_strdup(label);
	gli_data->content = g_strdup(content);
	gli_data->item_data = item_data;

	Elm_Object_Item *item = NULL;

	if (!icon) {
		itc.item_style = "type1";//"default";
		itc.func.text_get = _mp_popup_gl_label_get2;
		itc.func.content_get = NULL;
		itc.func.state_get = NULL;
		itc.func.del = _mp_popup_gl_del;
	} else {
		//itc.item_style = "1text.1icon.3";
		itc.item_style = "type1";//"default";
		itc.func.text_get = _mp_popup_gl_label_get2;
		itc.func.content_get = _mp_popup_gl_icon_get;
		itc.func.state_get = NULL;
		itc.func.del = _mp_popup_gl_del;
	}

	item = elm_genlist_item_append(genlist, &itc, gli_data, NULL, ELM_GENLIST_ITEM_NONE, cb, data);

	Evas_Object *box = elm_object_part_content_get(popup, NULL);
	if (box) {
		int count = elm_genlist_items_count(genlist);
		mp_popup_set_min_size(box, count);
	}

	return item;

}

Evas_Object *
mp_genlist_popup_create(Evas_Object * parent, mp_popup_t type, void *user_data, struct appdata * ad)
{
	mp_retvm_if(parent == NULL, NULL, "parent is NULL");
	ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	if (_mp_popup_popup_exist(ad, MP_POPUP_GENLIST)) {
		DEBUG_TRACE("popup already exist...");
		return NULL;
	}

	Evas_Object *popup = NULL;

	switch (type) {
	case MP_POPUP_LIST_SHARE:
		popup = _mp_popup_list_share_create(parent, user_data, ad);
		break;
	case MP_POPUP_ADD_TO_PLST:
		popup = _mp_popup_add_to_playlist_create(parent, user_data, ad);
		break;
	case MP_POPUP_SETTING_PLAYLIST:
		popup = _mp_popup_setting_playlist_create(parent, user_data, ad);
		break;
	case MP_POPUP_LIST_LONGPRESSED:
		popup = _mp_popup_tracklist_longpressed_create(parent, user_data, ad);
		break;
	case MP_POPUP_SOUND_PATH:
		popup = _mp_popup_sound_path_create(parent, user_data, ad);
		evas_object_data_set(popup, "sound_path", (char *)1);
		break;

#ifdef MP_FEATURE_CLOUD
	case MP_POPUP_CLOUD_VIEW:
		popup = _mp_popup_cloud_view_mode(parent, user_data, ad);
		break;
#endif
	case MP_POPUP_CHANGE_LIST_DISPLAY_MODE:
		popup = _mp_popup_change_list_display_mode(parent, user_data, ad);
		break;
	case MP_POPUP_EDIT_IMAGE:
		popup = _mp_popup_edit_image_create(parent, user_data, ad);
		break;
	case MP_POPUP_INFO_LIST:
	case MP_POPUP_UPDATE_ALBUM_ART:
		popup = _mp_popup_info_list_create(parent, user_data, ad);
		break;
	case MP_POPUP_SEARCH:
		popup = _mp_popup_search_create(parent, user_data, ad);
		break;
	case MP_POPUP_MORE_INFO:
		popup = _mp_popup_tracklist_more_info(parent, user_data, ad);
		break;
	case MP_POPUP_DELETE_TRACK:
		popup = _mp_popup_delete_track_create(parent, user_data, ad);
		break;
	default:
		break;
	}

	if (popup) {
		evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_popup_del_cb, ad);
		evas_object_data_set(popup, "type", (void *)MP_POPUP_GENLIST);
		ad->popup[MP_POPUP_GENLIST] = popup;
	}

	return popup;
}

static void
_mp_popup_genlist_popup_del_idler(void *data)
{
	Evas_Object *genlist_popup = data;
	MP_CHECK(genlist_popup);
	evas_object_del(genlist_popup);
}


/*This is only for music pasue case*/
static void
_mp_popup_genlist_pause_del_idler(void *data)
{
	Evas_Object *genlist_popup = data;
	MP_CHECK(genlist_popup);
	mp_popup_response(genlist_popup, MP_POPUP_NO);
}

EXPORT_API void
mp_popup_destroy(struct appdata *ad)
{
	MP_CHECK(ad);
	int i = 0;
	for (i = 0; i < MP_POPUP_MAX; i++) {
		if (i == MP_POPUP_ENTRY) {
			ERROR_TRACE("Not deleting Entry popup on language change");
			continue;
		} else if (ad->popup[i] && i != MP_POPUP_NOTIFY) {
			if (i == MP_POPUP_GENLIST) {
				/* do NOT destroy genlist in genlst select callback function */
				evas_object_hide(ad->popup[i]);
				ecore_job_add(_mp_popup_genlist_popup_del_idler, ad->popup[i]);
			} else {
				mp_evas_object_del(ad->popup[i]);
			}
			ad->popup[i] = NULL;
		}
	}
}

EXPORT_API void
mp_longpress_popup_destroy(struct appdata *ad)
{
	MP_CHECK(ad);

	if (ad->popup[MP_POPUP_GENLIST]) {
		/* do NOT destroy genlist in genlst select callback function */
		evas_object_hide(ad->popup[MP_POPUP_GENLIST]);
		ecore_job_add(_mp_popup_genlist_pause_del_idler, ad->popup[MP_POPUP_GENLIST]);
		ad->popup[MP_POPUP_GENLIST] = NULL;
	}
}


static void _mouseup_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	EVENT_TRACE("");
	Evas_Event_Mouse_Up *ev = event_info;
	if (ev->button == 3) { // if mouse right button is up
		mp_popup_response(data, MP_POPUP_NO); // you can call evas_object_del(obj); to remove popup if you want

	}
}
void
mp_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	mp_popup_response(obj, MP_POPUP_NO);
}

static void
_mp_popup_cancel_cb(void *data, Evas_Object *obj, void *event_info)
{
	Evas_Object *popup = (Evas_Object*)data;
	mp_popup_response(popup, MP_POPUP_NO);
}

static void _mp_popup_rotate_cb(void *data, Evas_Object *obj, void *ei)
{
	eventfunc;

	Evas_Object *box = NULL;
	Evas_Object *genlist = NULL;
	int cnt = 0;
	int min_h = 0;
	Evas_Object *popup = (Evas_Object*)data;
	MP_CHECK(popup);
	box = elm_object_content_get(popup);
	genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);
	cnt = elm_genlist_items_count(genlist);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	int angle = elm_win_rotation_get(ad->win_main);
	DEBUG_TRACE("angle = %d", angle);
	if (angle == 90 || angle == 270) {
		/*4 items here the landscape logic is reversed*/
		if (cnt > 4) {
			min_h = MP_POPUP_GENLIST_ITEM_H_MAX_LD;
		} else {
			min_h = MP_POPUP_GENLIST_ITEM_H * cnt + cnt - 1;
		}
	} else {
		/*6 items*/
		if (cnt > 6) {
			min_h = MP_POPUP_GENLIST_ITEM_H_MAX;
		} else {
			min_h = MP_POPUP_GENLIST_ITEM_H * cnt + cnt - 1;
		}
	}

	evas_object_size_hint_min_set(box,
	                              MP_POPUP_GENLIST_ITEM_W * elm_config_scale_get(), min_h * elm_config_scale_get());
	evas_object_size_hint_max_set(box,
	                              0, min_h * elm_config_scale_get());

	return ;
}

static void
_mp_popup_destroy_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	Evas_Object *top_widget = (Evas_Object *)evas_object_data_get(obj, "top_widget");
	if (top_widget) {
		evas_object_smart_callback_del(top_widget, "rotation,changed", _mp_popup_rotate_cb);
	}

	Popup_Data *popup_data = data;
	MP_CHECK(popup_data);

	if (popup_data->type < MP_POPUP_MAX && popup_data->ad) {
		popup_data->ad->popup[popup_data->type] = NULL;
	}

	SAFE_FREE(popup_data);
}


Evas_Object *
mp_popup_message_create(Evas_Object * parent, mp_popup_type type, char *title, char *message, void *user_data, Evas_Smart_Cb response_cb,
                        void *ad)
{
	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;
	struct appdata *p_ad = mp_util_get_appdata();
	MP_CHECK_NULL(p_ad);

	if (_mp_popup_popup_exist(p_ad, type)) {
		DEBUG_TRACE("popup already exist...");
		return NULL;
	}

#ifndef MP_FEATURE_MULTIWINDOW
	popup = elm_popup_add(p_ad->win_main);
#else
	popup = mp_popup_multi_window_center_add(p_ad->win_main);
#endif

	MP_CHECK_NULL(popup);

	Evas_Object *top_widget = elm_object_top_widget_get(popup);
	if (top_widget) {
		evas_object_smart_callback_add(top_widget, "rotation,changed", _mp_popup_rotate_cb, popup);
		evas_object_data_set(popup, "top_widget", (void *)top_widget);
	}

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Popup_Data *popup_data = (Popup_Data *)calloc(1, sizeof(Popup_Data));
	mp_assert(popup_data);
	mp_popup_set_popup_data(popup, popup_data);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb, popup);

	popup_data->ad = p_ad;
	popup_data->type = type;
	mp_popup_response_callback_set(popup, response_cb, user_data);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_popup_destroy_cb, popup_data);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, mp_popup_back_cb, popup_data);

	switch (type) {
	case MP_POPUP_PROGRESS:
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		Evas_Object *layout = NULL;
		layout = elm_layout_add(popup);
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", PLAY_VIEW_EDJ_NAME);

		elm_layout_file_set(layout, edje_path, "popup_processingview_string");
		free(path);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		progressbar = mp_widget_loading_icon_add(popup, MP_LOADING_ICON_SIZE_SMALL);
		elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
		elm_object_part_text_set(layout, "elm.text", GET_SYS_STR(message));
		elm_object_content_set(popup, layout);
		break;

	default:
		DEBUG_TRACE("Unsupported type: %d", type);
	}

	if (title) {
		mp_util_domain_translatable_part_text_set(popup, "title,text", title);
	}

	p_ad->popup[type] = popup;
	return popup;

}

Evas_Object *
mp_popup_create(Evas_Object * parent, mp_popup_type type, char *title, void *user_data, Evas_Smart_Cb response_cb,
                void *ad)
{
	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;
	struct appdata *p_ad = mp_util_get_appdata();
	MP_CHECK_NULL(p_ad);

	if (_mp_popup_popup_exist(p_ad, type)) {
		DEBUG_TRACE("popup already exist...");
		return NULL;
	}

#ifndef MP_FEATURE_MULTIWINDOW
	popup = elm_popup_add(p_ad->win_main);
#else
	popup = mp_popup_multi_window_center_add(p_ad->win_main);
#endif
	elm_popup_align_set(popup, ELM_NOTIFY_ALIGN_FILL, 0.5);

	MP_CHECK_NULL(popup);

	Evas_Object *top_widget = elm_object_top_widget_get(popup);
	if (top_widget) {
		evas_object_smart_callback_add(top_widget, "rotation,changed", _mp_popup_rotate_cb, popup);
		evas_object_data_set(popup, "top_widget", (void *)top_widget);
	}

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	Popup_Data *popup_data = (Popup_Data *)calloc(1, sizeof(Popup_Data));
	mp_assert(popup_data);
	mp_popup_set_popup_data(popup, popup_data);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb, popup);
	//evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, _keydown_cb, popup);

	popup_data->ad = p_ad;
	popup_data->type = type;
	mp_popup_response_callback_set(popup, response_cb, user_data);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_popup_destroy_cb, popup_data);
	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, mp_popup_back_cb, popup_data);

	switch (type) {
	case MP_POPUP_NORMAL:
		DEBUG_TRACE("MP_POPUP_NORMAL");
		evas_object_smart_callback_add(popup, "block,clicked", __mp_popup_block_cb, (void *)MP_POPUP_NO);
		break;

	case MP_POPUP_GENLIST:
		DEBUG_TRACE("MP_POPUP_GENLIST");
		//elm_object_style_set(popup, "min_menustyle");
		//to destory popup if outside of popup clicked.
		evas_object_smart_callback_add(popup, "block,clicked", __mp_popup_block_cb, (void *)MP_POPUP_NO);
		break;

	case MP_POPUP_PROGRESS:
		evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		Evas_Object *layout = NULL;
		layout = elm_layout_add(popup);
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", PLAY_VIEW_EDJ_NAME);

		elm_layout_file_set(layout, edje_path, "popup_processingview_string");
		free(path);
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		progressbar = mp_widget_loading_icon_add(popup, MP_LOADING_ICON_SIZE_SMALL);
		elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
		elm_object_part_text_set(layout, "elm.text", GET_SYS_STR(MP_POPUP_LOADING));
		elm_object_content_set(popup, layout);
		break;

	case  MP_POPUP_PROGRESS_WITH_CANCEL: {
		Evas_Object *layout;
		Evas_Object *btn1;

		layout = elm_layout_add(popup);
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024,"%s%s/%s",path, "edje", PLAY_VIEW_EDJ_NAME);

		elm_layout_file_set(layout, edje_path, "popup_processingview_1button");
		free(path);

		progressbar = mp_widget_loading_icon_add(popup, MP_LOADING_ICON_SIZE_LARGE);

		elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
		elm_object_part_text_set(layout, "elm.text", title);

		elm_object_content_set(popup, layout);
		btn1 = elm_button_add(popup);
		elm_object_style_set(btn1, "popup");
		elm_object_text_set(btn1, GET_SYS_STR(STR_MP_CANCEL));
		elm_object_part_content_set(popup, "button1", btn1);
		evas_object_smart_callback_add(btn1, "clicked", _mp_popup_cancel_cb, popup);
		p_ad->popup[type] = popup;

		return popup;
	}
	break;
	case  MP_POPUP_PROGRESS_WITHOUT_CANCEL: {
		Evas_Object *layout;

		layout = elm_layout_add(popup);
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", PLAY_VIEW_EDJ_NAME);

		elm_layout_file_set(layout, edje_path, "popup_processingview_1button");
		free(path);
		progressbar = mp_widget_loading_icon_add(popup, MP_LOADING_ICON_SIZE_LARGE);
		elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
		elm_object_part_text_set(layout, "elm.text", title);
		elm_object_content_set(popup, layout);
		p_ad->popup[type] = popup;
		return popup;
	}
	break;
	case	MP_POPUP_OPERATION_PROGRESS: {
		Evas_Object *layout;
		Evas_Object *btn1;

		layout = elm_layout_add(popup);
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024,"%s%s/%s",path, "edje", MP_EDJ_NAME);

		elm_layout_file_set(layout, edje_path, "popup_center_progressview");
		free(path);

		progressbar = elm_progressbar_add(popup);
		elm_object_style_set(progressbar, "list_progress");
		elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
		evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		elm_progressbar_value_set(progressbar, 0.0);

		elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
		elm_object_part_text_set(layout, "elm.text", title);

		elm_object_content_set(popup, layout);
		btn1 = elm_button_add(popup);
		elm_object_style_set(btn1, "popup");
		elm_object_text_set(btn1, GET_SYS_STR(STR_MP_CANCEL));
		elm_object_part_content_set(popup, "button1", btn1);
		evas_object_smart_callback_add(btn1, "clicked", _mp_popup_cancel_cb, popup);
		p_ad->popup[type] = popup;
	}
	break;

	case MP_POPUP_PROGRESS_NO_BUTTON:
		progressbar = mp_widget_loading_icon_add(popup, MP_LOADING_ICON_SIZE_LARGE);
		elm_object_content_set(popup, progressbar);
		break;

	case MP_POPUP_NOTIFY:
		DEBUG_TRACE("MP_POPUP_NOTIFY");
		break;

	case MP_POPUP_CONTENT_EXPAND:
		elm_object_style_set(popup, "content_expand");
		break;
#ifdef MP_FEATURE_PERSONAL_PAGE
	case MP_POPUP_CHECK_INFO_PERSONAL: {
		Evas_Object *layout;
		Evas_Object *label;

		layout = elm_layout_add(popup);
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", MP_EDJ_NAME);

		int ret = elm_layout_file_set(layout, edje_path, "popup_checkview_personal");
		free(path);
		if (!ret) {
			ERROR_TRACE("Set layout style failed");
		}
		evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

		label = elm_label_add(popup);
		elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
		evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
		evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(label);

		Evas_Object *checkbox = elm_check_add(popup);
		elm_object_style_set(checkbox, "multiline");
		elm_check_state_set(checkbox, FALSE);
		mp_util_domain_translatable_text_set(checkbox, MP_PERSONAL_DONT_ASK_AGAIN);
		evas_object_size_hint_align_set(checkbox, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_size_hint_weight_set(checkbox, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_show(checkbox);

		elm_object_part_content_set(layout, "elm.swallow.content", label);
		elm_object_part_content_set(layout, "elm.swallow.end", checkbox);

		elm_object_content_set(popup, layout);
	}
	break;
#endif
	default:
		DEBUG_TRACE("Unsupported type: %d", type);
	}

	if (title) {
		//elm_object_part_text_set(popup, "title,text", title);
		mp_util_domain_translatable_part_text_set(popup, "title,text", title);
	}

	p_ad->popup[type] = popup;

	//evas_object_show(popup);
	return popup;

}

static void
_popup_show_cb(void *data)
{
	evas_object_show(data);
}


Evas_Object *
mp_entry_popup_create(char *title)
{
	Evas_Object *popup = NULL;
	mp_popup_type type = MP_POPUP_ENTRY;
	struct appdata *p_ad = mp_util_get_appdata();
	MP_CHECK_NULL(p_ad);

	if (_mp_popup_popup_exist(p_ad, type)) {
		DEBUG_TRACE("popup already exist...");
		return NULL;
	}

	popup = elm_popup_add(GET_NAVIFRAME);

	MP_CHECK_NULL(popup);
	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	//elm_object_style_set(popup, "no_effect");
	elm_object_focus_set(popup, EINA_FALSE);

	Popup_Data *popup_data = (Popup_Data *)calloc(1, sizeof(Popup_Data));
	mp_assert(popup_data);
	mp_popup_set_popup_data(popup, popup_data);

	eext_object_event_callback_add(popup, EEXT_CALLBACK_BACK, mp_popup_back_cb, popup_data);

	evas_object_event_callback_add(popup, EVAS_CALLBACK_MOUSE_UP, _mouseup_cb, popup);
	//evas_object_event_callback_add(popup, EVAS_CALLBACK_KEY_DOWN, _keydown_cb, popup);

	popup_data->ad = p_ad;
	popup_data->type = type;

	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_popup_destroy_cb, popup_data);

	if (title) {
		mp_util_domain_translatable_part_text_set(popup, "title,text", title);
	}

	p_ad->popup[type] = popup;
	//evas_object_show(popup);
	ecore_job_add(_popup_show_cb, popup);
	return popup;

}

void
mp_popup_response_callback_set(Evas_Object *popup, Evas_Smart_Cb cb, void *user_data)
{
	MP_CHECK(popup);

	Popup_Data *popup_data = mp_popup_get_popup_data(popup);
	MP_CHECK(popup_data);

	popup_data->response_cb = cb;
	popup_data->cb_data = user_data;
}

void
mp_popup_response(Evas_Object *popup, int response)
{
	startfunc;
	MP_CHECK(popup);

	Popup_Data *popup_data = mp_popup_get_popup_data(popup);
	MP_CHECK(popup_data);

	if (popup_data->response_cb) {
		popup_data->response_cb(popup_data->cb_data, popup, (void *)response);
	} else {
		mp_evas_object_del(popup);
	}
}

bool
mp_popup_button_set(Evas_Object *popup, popup_button_t btn_index, const char *text, int response)
{
	MP_CHECK_FALSE(popup);
	MP_CHECK_FALSE(text);
	if (btn_index == MP_POPUP_BTN_MAX) {
		mp_error("invalid button type");
		return FALSE;
	}

	bool ret = FALSE;

	static char *part[MP_POPUP_BTN_MAX] = {
		"button1",
		"button2",
		"button3",
	};

	Evas_Object *button = elm_button_add(popup);

	elm_object_style_set(button, "popup");
	mp_util_domain_translatable_text_set(button, text);
	//elm_object_text_set(button, text);
	evas_object_smart_callback_add(button, "clicked", _mp_popup_button_clicked_cb, popup);

	if (button) {
		elm_object_part_content_set(popup, part[btn_index], button);
		mp_evas_object_response_set(button, response);
		ret = TRUE;
	}

	/*
		if (!g_strcmp0(text, GET_SYS_STR(STR_MP_DELETE)) )//|| !g_strcmp0(text, GET_SYS_STR(STR_MP_REMOVE)))
		{
			elm_object_style_set(button, "style1/delete");
		}
	*/
	return ret;
}


static void
__mp_popup_timeout_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	int response = (int)data;
	mp_popup_response(obj, response);
}

static void
__mp_popup_block_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;

	int i;
	static char *part[MP_POPUP_BTN_MAX] = {
		"button1",
		"button2",
		"button3",
	};

	for (i = 0; i < MP_POPUP_BTN_MAX; i++) {
		Evas_Object *button_part = elm_object_part_content_get(obj, part[i]);
		if (button_part) {
			DEBUG_TRACE("exist button in popup");
			return;
		}
	}
	int response = (int)data;
	mp_popup_response(obj, response);

}

void
mp_popup_timeout_set(Evas_Object *popup, double timeout)
{
	startfunc;
	MP_CHECK(popup);

	elm_popup_timeout_set(popup, timeout);
	evas_object_smart_callback_add(popup, "timeout", __mp_popup_timeout_cb, (void *)MP_POPUP_NO);
	//evas_object_smart_callback_add(popup, "block,clicked", __mp_popup_timeout_cb, (void *)MP_POPUP_NO);
}

void _mp_popup_max_length_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	if (data) {
		elm_object_focus_set(data, EINA_TRUE);
	}
	evas_object_del(obj);
}

bool
mp_popup_max_length(Evas_Object *entry, const char *text)
{
	MP_CHECK_FALSE(entry);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);
	//elm_object_focus_set(entry, EINA_FALSE); //do not hide isf on max length

	char *message = GET_STR(text);
	mp_util_post_status_message(ad, message);

	return true;
}

Evas_Object *mp_popup_multi_window_center_add(Evas_Object *parent)
{
	MP_CHECK_NULL(parent);

	Evas_Object *popup = elm_popup_add(parent);
	MP_CHECK_NULL(popup);

	Evas_Object *p_window = elm_popup_win_get(popup);
	if (p_window) {
		char edje_path[1024] ={0};
		char * path = app_get_resource_path();

		MP_CHECK_NULL(path);
		snprintf(edje_path, 1024, "%s%s/%s", path, "edje", THEME_NAME);
		elm_theme_extension_add(NULL, edje_path);
		free(path);
	}

	return popup;
}

