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


#include "mp-ctxpopup.h"
#include <bundle.h>
#include <stdio.h>
#include "music.h"
#include "mp-menu.h"
#include "mp-item.h"
#include "mp-player-debug.h"
#include "mp-playlist-mgr.h"
#include "mp-common.h"
#include <sound_manager.h>
#include "mp-util.h"

#include "mp-widget.h"

static void _dismissed_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE("");

	MP_CHECK(data);

	evas_object_del(data);
	evas_object_smart_callback_del(data,"dismissed", _dismissed_cb);
}

static void _move_ctxpopup(Evas_Object *ctxpopup, Evas_Object *btn)
{
	DEBUG_TRACE("");

	Evas_Coord x, y, w , h;
	evas_object_geometry_get(btn, &x, &y, &w, &h);
    evas_object_move(ctxpopup, x + (w / 2), y + (h /2));
}

Evas_Object *
_mp_ctxpopup_pv_share_create(Evas_Object *parent, void *user_data, struct appdata *ad)
{
	DEBUG_TRACE("");

	MP_CHECK_NULL(ad);
	MP_CHECK_NULL(parent);

	Evas_Object *popup = elm_ctxpopup_add(GET_WINDOW());
	evas_object_smart_callback_add(popup,"dismissed", _dismissed_cb, popup);
	elm_ctxpopup_direction_priority_set(popup,
						     ELM_CTXPOPUP_DIRECTION_DOWN,
						     ELM_CTXPOPUP_DIRECTION_LEFT,
						     ELM_CTXPOPUP_DIRECTION_UP,
						     ELM_CTXPOPUP_DIRECTION_RIGHT );

#ifndef MP_FEATURE_DISABLE_MMS
	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_MESSAGE", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
#endif
	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_EMAIL", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_BLUETOOTH", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
#ifdef MP_FEATURE_WIFI_SHARE
	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_WI_FI", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
#endif
#ifdef MP_FEATURE_CLOUD
	char *path = user_data;
	mp_storage_type_e storage_type;
	mp_media_info_h media_info = NULL;
	mp_media_info_create_by_path(&media_info, path);
	mp_media_info_get_storage_type(media_info, &storage_type);
	if (storage_type != MP_STORAGE_CLOUD)
		mp_util_ctxpopup_item_append(popup, STR_MP_DROPBOX, NULL, mp_menu_ctxpopup_share_select_cb, user_data);
#endif
	_move_ctxpopup(popup, parent);
	evas_object_show(popup);

	return popup;
}

Evas_Object *
_mp_ctxpopup_list_share_create(Evas_Object *parent, void *user_data, struct appdata *ad)
{
	DEBUG_TRACE("");

	MP_CHECK_NULL(ad);
	MP_CHECK_NULL(parent);

	Evas_Object *popup = elm_ctxpopup_add(GET_WINDOW());
	evas_object_smart_callback_add(popup,"dismissed", _dismissed_cb, popup);
	elm_ctxpopup_direction_priority_set(popup,
						     ELM_CTXPOPUP_DIRECTION_DOWN,
						     ELM_CTXPOPUP_DIRECTION_LEFT,
						     ELM_CTXPOPUP_DIRECTION_UP,
						     ELM_CTXPOPUP_DIRECTION_RIGHT );

	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_BLUETOOTH", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_EMAIL", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
#ifndef MP_FEATURE_DISABLE_MMS
	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_MESSAGE", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
#endif
#ifdef MP_FEATURE_WIFI_SHARE
	mp_util_ctxpopup_item_append(popup, "IDS_COM_BODY_WI_FI", NULL, mp_menu_ctxpopup_share_select_cb, user_data);
#endif

	_move_ctxpopup(popup, parent);
	evas_object_show(popup);

	return popup;
}

void
mp_ctxpopup_destroy(Evas_Object * popup)
{
	mp_evas_object_del(popup);
}

void
mp_ctxpopup_clear(Evas_Object * popup)
{
	DEBUG_TRACE("");

	MP_CHECK(popup);
	elm_ctxpopup_clear(popup);
}

Evas_Object *
mp_ctxpopup_create(Evas_Object *parent, int type, void *user_data, void *ad)
{
	DEBUG_TRACE("");

	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(ad);

	Evas_Object *popup = NULL;

	switch (type)
	{
	case MP_CTXPOPUP_PV_SHARE:
		popup = _mp_ctxpopup_pv_share_create(parent, user_data, ad);
		break;
	case MP_CTXPOPUP_LIST_SHARE:
		popup = _mp_ctxpopup_list_share_create(parent, user_data, ad);
		break;
	default:
		break;
	}

	return popup;
}
