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

#include <stdio.h>
#include <stdlib.h>
#include <glib.h>

#include <sys/time.h>
#include <glib.h>
#include <fcntl.h>
#include <app.h>
#include <media_content.h>

#include "music.h"
#include "mp-item.h"
#include "mp-menu.h"
#include "mp-ug-launch.h"
#include "mp-define.h"

#ifdef IDEBUILD
#include "idebuild.h"
#endif

#include "music.h"
#include "mp-item.h"
#include "mp-player-debug.h"
#include "mp-widget.h"
#include "mp-volume.h"


#ifdef MP_SOUND_PLAYER

#else
#include "mp-common.h"
#include "mp-list.h"
#include "mp-media-info.h"
#endif
#include "mp-util.h"

#define UG_EMAIL_NAME "email-composer-efl"
#define UG_BT_NAME "setting-bluetooth-efl"
#define UG_MSG_NAME "msg-composer-efl"
#define UG_WIFI_NAME "wifi-efl-UG"
#ifdef MP_FEATURE_WIFI_SHARE
#define UG_FTM_NAME "fileshare-efl"
#endif
#ifdef MP_FEATURE_CLOUD
#define DROPBOX_PKG_NAME "dropbox-efl"
#endif

#define MP_UG_INFO_PATH "path"
#define MP_UG_INFO_ALBUMART "albumart"
#define MP_UG_INFO_ARTIST "artist"
#define MP_UG_INFO_ID "id"
#define MP_UG_INFO_DESTROY "destroy"
#define MP_UG_INFO_BACK "back"
#define MP_UG_INFO_LOAD "load"
#define MP_UG_INFO_ALBUMART_CLICKED "albumart_clicked"
#define MP_UG_INFO_MEDIA_SVC_HANDLE	"media_app_control_handle"
#ifdef MP_FEATURE_INNER_SETTINGS
#define MP_UG_MESSAGE_VAL_BACK "back"
#endif

/* for contact ug */
#define CT_UG_REQUEST_SAVE_RINGTONE 42
#define CT_UG_BUNDLE_TYPE "type"
#define CT_UG_BUNDLE_PATH "ct_path"
#define UG_CONTACTS_LIST "contacts-list-efl"

int
mp_ug_email_attatch_file(const char *filepath, void *user_data)
{
	bool ret = mp_send_via_appcontrol(user_data, MP_SEND_TYPE_EMAIL, filepath);
	return (ret) ? 0 : -1;
}

#ifdef MP_FEATURE_CLOUD
int
mp_ug_dropbox_attatch_file(const char *filepath, int count, void *user_data)
{
	startfunc;
	struct appdata *ad = NULL;
	app_control_h service = NULL;

	mp_retvm_if(filepath == NULL, -1, "file path is NULL");
	mp_retvm_if(user_data == NULL, -1, "appdata is NULL");

	ad = user_data;
	int err = 0;

	err = app_control_create(&service);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_create");
		return -1;
	}

	err = app_control_set_operation(service, APP_CONTROL_OPERATION_SEND);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_operation().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_set_uri(service, filepath);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_uri().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_set_app_id(service, DROPBOX_PKG_NAME);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_app_id().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}
	/*
		err = app_control_set_window(service, elm_win_xwindow_get(ad->win_main));
		if (err != APP_CONTROL_ERROR_NONE)
		{
			ERROR_TRACE("Error: app_control_set_window().. [0x%x]", err);
			app_control_destroy(service);
			return -1;
		}
	*/
	err = app_control_send_launch_request(service, NULL, NULL);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_send_launch_request().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_destroy(service);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_operation().. [0x%x]", err);
	}

	return 0;
}
#endif

#ifndef MP_SOUND_PLAYER
static void __mp_ug_gallery_result_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
	startfunc;
	int ret = 0;
	DEBUG_TRACE("result : %d", result);

	MP_CHECK(data);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (result == APP_CONTROL_RESULT_SUCCEEDED) {
		char *result_filename;
		ret = app_control_get_extra_data(reply, "path", &result_filename);
		if (ret != APP_CONTROL_ERROR_NONE) {
			ERROR_TRACE("app_control_get_extra_data() is failed : %d", ret);
			return;
		}

		if (result_filename) {
			/*update thumbnail in db*/
			ret = mp_media_info_playlist_set_thumbnail_path(data, result_filename);
			ERROR_TRACE("ret from set thumbnail is %d", ret);
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_IMAGE_UPDATED);
			char *popup_txt = NULL;
			if (!ret) {
				popup_txt = GET_SYS_STR("IDS_COM_POP_SUCCESS");
			} else {
				popup_txt = GET_SYS_STR("IDS_COM_POP_FAILED");
			}

			mp_widget_text_popup(ad, popup_txt);
		} else {
			ERROR_TRACE("No result");
		}
	}

	endfunc;
}

int
mp_ug_gallery_get_picture(void *data)
{
	startfunc;

	int ret;
	app_control_h svc_handle = NULL;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, -1);

	if (app_control_create(&svc_handle) < 0 || svc_handle == NULL) {
		ERROR_TRACE("app_control_create() is failed !!");
		return -1;
	}
	/*
		app_control_set_window(svc_handle, elm_win_xwindow_get(ad->win_main));
	*/
	int a, b;

	evas_object_geometry_get(ad->win_main, NULL, NULL, &a, &b);
	ERROR_TRACE("main window ----- win_width, win_height: [%d, %d]", a, b);

	app_control_set_operation(svc_handle, APP_CONTROL_OPERATION_PICK);
	app_control_set_app_id(svc_handle, "gallery-efl");
	app_control_add_extra_data(svc_handle, "launch-type", "select-one");
	app_control_add_extra_data(svc_handle, "file-type", "image");

	ret = app_control_send_launch_request(svc_handle, __mp_ug_gallery_result_cb, data);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_send_launch_request is failed ret = %d", ret);
		app_control_destroy(svc_handle);
		return -1;
	}

	app_control_destroy(svc_handle);

	endfunc;
	return 0;
}


static void __mp_ug_camera_result_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
	startfunc;
	int ret = 0;
	DEBUG_TRACE("result : %d", result);
	MP_CHECK(data);

	if (result == APP_CONTROL_RESULT_SUCCEEDED) {
		char *result_filename;
		ret = app_control_get_extra_data(reply, APP_CONTROL_DATA_SELECTED, &result_filename);
		if (ret != APP_CONTROL_ERROR_NONE) {
			ERROR_TRACE("app_control_get_extra_data() is failed : %d", ret);
			return;
		}

		if (result_filename) {
			/*update thumbnail in db*/
			ret = mp_media_info_playlist_set_thumbnail_path(data, result_filename);
			mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_IMAGE_UPDATED);

		} else {
			ERROR_TRACE("No result");
		}
	}

	endfunc;
}

int
mp_ug_camera_take_picture(void *data)
{
	startfunc;

	int ret;
	app_control_h svc_handle = NULL;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, -1);

	if (app_control_create(&svc_handle) < 0 || svc_handle == NULL) {
		ERROR_TRACE("app_control_create() is failed !!");
		return -1;
	}
	/*
		app_control_set_window(svc_handle, elm_win_xwindow_get(ad->win_main));
	*/
	int a, b;

	evas_object_geometry_get(ad->win_main, NULL, NULL, &a, &b);
	ERROR_TRACE("main window ----- win_width, win_height: [%d, %d]", a, b);



	app_control_set_operation(svc_handle, APP_CONTROL_OPERATION_CREATE_CONTENT);
	app_control_set_mime(svc_handle, "image/jpg");
	app_control_add_extra_data(svc_handle, "CALLER", "music-player");
	app_control_add_extra_data(svc_handle, "RESOLUTION", "VGA");

	ret = app_control_send_launch_request(svc_handle, __mp_ug_camera_result_cb, data);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_send_launch_request is failed ret = %d", ret);
		app_control_destroy(svc_handle);
		return -1;
	}

	app_control_destroy(svc_handle);

	endfunc;
	return 0;
}
#endif

static void _mp_ug_contact_reply_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	startfunc;
	char *value = NULL;
	app_control_get_extra_data(reply, "is_success", &value);
	DEBUG_TRACE("is_success: %s", value);

	if (!g_strcmp0(value, "1")) {
		mp_util_post_status_message(NULL, GET_STR(STR_MP_POP_CALLER_RINGTONE_SAVED));
	}

	IF_FREE(value);
}

int
mp_ug_contact_user_sel(const char *filepath, void *user_data)
{
	startfunc;
	//struct appdata *ad = NULL;
	app_control_h service = NULL;
	int err = 0;

	mp_retvm_if(filepath == NULL, -1, "file path is NULL");
	//ad = mp_util_get_appdata();

	if (app_control_create(&service) != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_create");
		return -1;
	}

	char buf[16];
	snprintf(buf, sizeof(buf), "%d", CT_UG_REQUEST_SAVE_RINGTONE);
	app_control_add_extra_data(service, CT_UG_BUNDLE_TYPE, buf);
	app_control_add_extra_data(service, CT_UG_BUNDLE_PATH, filepath);

	err = app_control_add_extra_data(service, "tone", filepath);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_add_extra_data().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_operation().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_set_app_id(service, UG_CONTACTS_LIST);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_app_id().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}
	/*
	        err = app_control_set_window(service, ad->xwin);
	        if (err != APP_CONTROL_ERROR_NONE)
		{
			ERROR_TRACE("Error: app_control_set_window().. [0x%x]", err);
			app_control_destroy(service);
			return -1;
		}
	*/
	err = app_control_send_launch_request(service, _mp_ug_contact_reply_cb, NULL);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_send_launch_request().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_destroy(service);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_operation().. [0x%x]", err);
	}

	return 0;
}

int
mp_ug_set_as_alarm_tone(const char *filepath, int position)
{
	startfunc;
	//struct appdata *ad = NULL;
	app_control_h service = NULL;
	int err = 0;
	char *pos = NULL;

	mp_retvm_if(filepath == NULL, -1, "file path is NULL");
	//ad = mp_util_get_appdata();

	if (app_control_create(&service) != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_create");
		return -1;
	}

	err = app_control_add_extra_data(service, "tone", filepath);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_add_extra_data().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	if (position > 0) {
		pos = g_strdup_printf("%d", position);
		DEBUG_TRACE("pos: %s", pos);
		err = app_control_add_extra_data(service, "position", pos);
		IF_FREE(pos);
		if (err != APP_CONTROL_ERROR_NONE) {
			ERROR_TRACE("Error: app_control_add_extra_data().. [0x%x]", err);
			app_control_destroy(service);
			return -1;
		}
	}

	err = app_control_set_operation(service, APP_CONTROL_OPERATION_SEND);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_operation().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_set_uri(service, filepath);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_uri().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_set_app_id(service, "alarm-efl");
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_app_id().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}
	/*
	        err = app_control_set_window(service, ad->xwin);
	        if (err != APP_CONTROL_ERROR_NONE)
		{
			ERROR_TRACE("Error: app_control_set_window().. [0x%x]", err);
			app_control_destroy(service);
			return -1;
		}
	*/
	err = app_control_send_launch_request(service, NULL, NULL);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_send_launch_request().. [0x%x]", err);
		app_control_destroy(service);
		return -1;
	}

	err = app_control_destroy(service);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Error: app_control_set_operation().. [0x%x]", err);
	}

	return 0;
}

bool
mp_send_via_appcontrol(struct appdata *ad, mp_send_type_e send_type, const char *files)
{
	startfunc;
	MP_CHECK_FALSE(ad);

	bool result = false;
	const char *ug_name = NULL;

	app_control_h service = NULL;
	int ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_create()... [0x%x]", ret);
		goto END;
	}

	ret = app_control_set_operation(service, APP_CONTROL_OPERATION_SEND);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_set_operation()... [0x%x]", ret);
		goto END;
	}

	ret = app_control_set_uri(service, files);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_set_uri()... [0x%x]", ret);
		goto END;
	}

	switch (send_type) {
	case MP_SEND_TYPE_EMAIL:
		ug_name = UG_EMAIL_NAME;
		ret = app_control_add_extra_data(service, "RUN_TYPE", "5");
		if (ret != APP_CONTROL_ERROR_NONE) {
			mp_error("app_control_add_extra_data()... [0x%x]", ret);
			goto END;
		}
		break;

	default:
		WARN_TRACE("Not supported type.. [%d]", send_type);
		goto END;
	}

	/* appcontrol name */
	ret = app_control_set_app_id(service, ug_name);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_set_app_id()... [0x%x]", ret);
		goto END;
	}

	/* set window */
	/*
		ret = app_control_set_window(service, ad->xwin);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mp_error("app_control_set_window()... [0x%x]", ret);
			goto END;
		}
	*/
	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_send_launch_request()... [0x%x]", ret);
		goto END;
	}

	result = true;

END:
	if (service) {
		app_control_destroy(service);
		service = NULL;
	}

	return result;
}

bool
mp_setting_privacy_launch(void)
{
	startfunc;

	//struct appdata *ad = mp_util_get_appdata();

	bool result = false;

	app_control_h service = NULL;
	int ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_create()... [0x%x]", ret);
		goto END;
	}

	ret = app_control_set_operation(service, "http://tizen.org/appcontrol/operation/configure/privacy");
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_set_operation()... [0x%x]", ret);
		goto END;
	}

	/* set window */
	/*
		ret = app_control_set_window(service, ad->xwin);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mp_error("app_control_set_window()... [0x%x]", ret);
			goto END;
		}
	*/
	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_send_launch_request()... [0x%x]", ret);
		goto END;
	}
	result = true;
END:
	if (service) {
		app_control_destroy(service);
		service = NULL;
	}
	return result;
}

static bool
_mp_ug_launch_as_appcontrol(const char *ug_name)
{
	MP_CHECK_FALSE(ug_name);

	//struct appdata *ad = mp_util_get_appdata();
	//MP_CHECK_FALSE(ad);

	bool result = false;

	app_control_h service = NULL;
	int ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_create()... [0x%x]", ret);
		goto END;
	}

	ret = app_control_set_app_id(service, ug_name);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_set_app_id()... [0x%x]", ret);
		goto END;
	}

	/* set window */
	/*
		ret = app_control_set_window(service, ad->xwin);
		if (ret != APP_CONTROL_ERROR_NONE) {
			mp_error("app_control_set_window()... [0x%x]", ret);
			goto END;
		}
	*/
	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_send_launch_request()... [0x%x]", ret);
		goto END;
	}
	result = true;
END:
	if (service) {
		app_control_destroy(service);
		service = NULL;
	}
	return result;
}

bool
mp_setting_wifi_launch(void)
{
	return _mp_ug_launch_as_appcontrol("wifi-efl-ug");
}

bool
mp_setting_network_launch(void)
{
	return _mp_ug_launch_as_appcontrol("setting-network-efl");
}

