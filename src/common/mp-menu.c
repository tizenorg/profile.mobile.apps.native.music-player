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

#include "music.h"
#include <stdio.h>
#include <glib.h>
#include <system_settings.h>

#include "mp-player-debug.h"
#include "mp-menu.h"
#include "mp-util.h"
#include "mp-popup.h"
#include "mp-playlist-mgr.h"
#include "mp-ug-launch.h"
#include "mp-item.h"
//#include "mp-player-drm.h"
#include "mp-widget.h"
#include "mp-ctxpopup.h"

#include "mp-setting-ctrl.h"
#ifdef MP_FEATURE_ADD_TO_HOME
//#include "shortcut.h"
#endif

#ifndef MP_SOUND_PLAYER
#include "mp-common.h"
#endif

enum
{
       MP_SHARE_BLUETOOTH,
       MP_SHARE_EMAIL,
#ifndef MP_FEATURE_DISABLE_MMS
       MP_SHARE_MESSAGE,
#endif
       MP_SHARE_WIFI,
#ifdef MP_FEATURE_CLOUD
       MP_SHARE_DROPBOX,
#endif
};


#include "mp-list.h"
#include "mp-list-view.h"

#define MP_MENU_FID "mp_menu_fid"
#define MP_MENU_PLAY_LIST_FID "mp_menu_playlist_id"
#define MP_MENU_GROUP_ITEM_HANDLER    	"mp_menu_group_item_handler"

mp_track_type_e
mp_menu_get_track_type_by_group(mp_group_type_e group_type)
{
	mp_track_type_e item_type = MP_TRACK_ALL;

	if (group_type == MP_GROUP_BY_ALBUM)
	{
		item_type = MP_TRACK_BY_ALBUM;
	}
	else if (group_type == MP_GROUP_BY_ARTIST)
	{
		item_type = MP_TRACK_BY_ARTIST;
	}
	else if (group_type == MP_GROUP_BY_ARTIST_ALBUM)
	{
		item_type = MP_TRACK_BY_ALBUM;
	}
	else if (group_type == MP_GROUP_BY_GENRE)
	{
		item_type = MP_TRACK_BY_GENRE;
	}
	else if (group_type == MP_GROUP_BY_YEAR)
	{
		item_type = MP_TRACK_BY_YEAR;
	}
	else if (group_type == MP_GROUP_BY_COMPOSER)
	{
		item_type = MP_TRACK_BY_COMPOSER;
	}
	else if (group_type == MP_GROUP_BY_FOLDER)
	{
		item_type = MP_TRACK_BY_FOLDER;
	}

	return item_type;
}

static int
_mp_menu_get_share_type(const char *label)
{
	MP_CHECK_VAL(label, -1);
	int share_type = -1;
	DEBUG_TRACE("%s selected", label);
	if (g_strcmp0(label, BLUETOOTH_SYS) == 0)
		share_type = MP_SHARE_BLUETOOTH;
	else if (g_strcmp0(label, EMAIL_SYS) == 0)
		share_type = MP_SHARE_EMAIL;
#ifndef MP_FEATURE_DISABLE_MMS
	else if (g_strcmp0(label, MESSAGE_SYS) == 0)
		share_type = MP_SHARE_MESSAGE;
#endif
#ifdef MP_FEATURE_WIFI_SHARE
	else if (g_strcmp0(label, WIFI_SYS) == 0)
		share_type = MP_SHARE_WIFI;
#endif
#ifdef MP_FEATURE_CLOUD
	else if (g_strcmp0(label, STR_MP_DROPBOX) == 0)
		share_type = MP_SHARE_DROPBOX;
#endif

	return share_type;
}

void
_mp_menu_share_file(GList *sel_list, const char *label)
{
	startfunc;
	MP_CHECK(sel_list);
	int share_type = 0;
	GString *path = NULL;
	char *fmt = NULL;
	share_type = _mp_menu_get_share_type(label);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (share_type == MP_SHARE_BLUETOOTH)
		fmt = "?%s";
	else if (share_type == MP_SHARE_EMAIL)
		fmt = "\n%s";
#ifndef MP_FEATURE_DISABLE_MMS
	else if (share_type == MP_SHARE_MESSAGE)
		fmt = "\n%s";
#endif
#ifdef MP_FEATURE_WIFI_SHARE
	else if (share_type == MP_SHARE_WIFI)
		fmt = "|%s";
#endif
	else
	{
		fmt = ";%s";
	}

	GList *list = g_list_first(sel_list);
	while (list)
	{
		if (list->data)
		{
			if (path == NULL)
				path = g_string_new(list->data);
			else
				g_string_append_printf(path, fmt, list->data);
		}
		else
			WARN_TRACE("path name is NULL");

		list = g_list_next(list);
	}

	if (path && path->str)
	{
		DEBUG_TRACE("path is [%s]", path->str);

		if (share_type == MP_SHARE_EMAIL)
				mp_ug_email_attatch_file(path->str, ad);
#ifdef MP_FEATURE_WIFI_SHARE
		else if (share_type == MP_SHARE_WIFI)
			mp_ug_wifi_attatch_file(path->str, g_list_length(sel_list), ad);
#endif
#ifdef MP_FEATURE_CLOUD
		else if (share_type == MP_SHARE_DROPBOX)
 			mp_ug_dropbox_attatch_file(path->str, g_list_length(sel_list), ad);
#endif
		else
			WARN_TRACE("unsupported type");
		g_string_free(path, TRUE);
	}
	else
	{
		ERROR_TRACE("path is NULL");
	}
}


//ctx popup single select
void
mp_menu_ctxpopup_share_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	GList *list = g_list_append(NULL, data);
	const char *label = elm_object_item_text_get((Elm_Object_Item *)event_info);
	_mp_menu_share_file(list, label);
	mp_ctxpopup_destroy(obj);
	g_list_free(list);
}

//genlist popup single select
void
mp_menu_genlist_popup_share_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	GList *list = g_list_append(NULL, data);
	const char *label = elm_object_item_data_get((Elm_Object_Item *)event_info);
	_mp_menu_share_file(list, label);
	mp_popup_destroy(mp_util_get_appdata());
	g_list_free(list);
}

//genlist popup single select
void
mp_menu_genlist_popup_list_share_select_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	GList *list = (GList *)data;
	const char *label = elm_object_item_data_get((Elm_Object_Item *)event_info);
	_mp_menu_share_file(list, label);
	mp_popup_destroy(mp_util_get_appdata());
	g_list_free(list);
}

#ifdef MP_FEATURE_ADD_TO_HOME
static int
_mp_menu_shortcut_res_cb(int ret, int pid, void *data)
{
	startfunc;
	struct appdata *ad = NULL;
	char *msg = NULL;

	ad = data;
	MP_CHECK_VAL(ad, -1);

	if (ret == 0)
	{
		msg = GET_STR("IDS_MUSIC_POP_ADDED");
	}
	else
	{
		msg = GET_STR("IDS_MUSIC_POP_UNABLE_TO_ADD");
	}
	mp_util_post_status_message(ad, msg);

	IF_FREE(ad->shortcut_descrition);

	return 0;
}
#endif

#ifdef MP_FEATURE_ADD_TO_HOME
void
mp_menu_add_to_home(int type, void *main_info, void *extra_info1, void *extra_info2)
{
	startfunc;

	if (type >= MP_ADD_TO_HOME_SHORTCUT_TYPE_NUM) {
		mp_assert(1);
	}

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	char *shortcut_descrition = NULL;
	if (type == MP_ADD_TO_HOME_SHORTCUT_TYPE_USER_PLAYLIST)
		shortcut_descrition = g_strdup_printf("%s%d|%d|%d", MP_ADD_TO_HOME_SHORTCUT_PREFIX, type, (int)main_info, 0);
	else
		shortcut_descrition = g_strdup_printf("%s%d|%s|%s|%s", MP_ADD_TO_HOME_SHORTCUT_PREFIX, type, (char *)main_info, (char *)extra_info1, (char *)extra_info2);

	mp_debug("description = %s", shortcut_descrition);
	int ret = add_to_home_livebox(PKGNAME_FOR_SHORTCUT, NULL, LIVEBOX_TYPE_1x1, shortcut_descrition, NULL, -1.0f, true, _mp_menu_shortcut_res_cb, ad);
	if (ret < 0)
	{
		char *msg = NULL;
		ERROR_TRACE("ret: %d", ret);
		msg = GET_STR("IDS_MUSIC_POP_UNABLE_TO_ADD");
		mp_util_post_status_message(ad, msg);
	}
	IF_FREE(shortcut_descrition);

	endfunc;
}
#endif

