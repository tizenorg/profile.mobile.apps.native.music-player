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


#include <notification.h>
#include <app_control.h>
#include "mp-define.h"
#include "mp-player-debug.h"
#include "mp-notification.h"
#include "mp-util.h"

typedef struct {
	notification_h handle;
	notification_type_e type;
	int priv_id;
	int count;
	char *extra_data;
} mp_noti_s;

/*static app_control_h
_mp_noti_make_excute_option(const char *key, const char *value)
{
	MP_CHECK_NULL(key);

	app_control_h service = NULL;

	int err = 	app_control_create(&service);
	if (err) {
		mp_error("app_control_create() .. [0x%x]", err);
		return NULL;
	}

	err = app_control_set_app_id(service, "org.tizen.music-player");
	if (err) {
		mp_error("app_control_set_app_id() .. [0x%x]", err);
		app_control_destroy(service);
		service = NULL;
		return NULL;
	}

	err = app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
	if (err) {
		mp_error("app_control_set_operation() .. [0x%x]", err);
		app_control_destroy(service);
		service = NULL;
		return NULL;
	}

	err = app_control_add_extra_data(service, key, value);
	if (err) {
		mp_error("app_control_add_extra_data() .. [0x%x]", err);
		app_control_destroy(service);
		service = NULL;
		return NULL;
	}

	return service;
}*/

void
mp_noti_destroy(mp_noti_h noti)
{
	startfunc;
	mp_noti_s *noti_data = noti;
	MP_CHECK(noti_data);

	if (noti_data->handle) {
		int err = 0;

		err = notification_delete(noti_data->handle);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_delete().. [0x%x]", err);
		}

		err = notification_free(noti_data->handle);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_free().. [0x%x]", err);
		}
	}
	noti_data->handle = NULL;

	IF_FREE(noti_data->extra_data);

	free(noti_data);
}

bool
mp_noti_update_size(mp_noti_h noti, unsigned long long total, unsigned long long byte)
{
	mp_noti_s *noti_data = noti;
	MP_CHECK_FALSE(noti_data);
	MP_CHECK_FALSE(noti_data->handle);

	int err = 0;
	if (total > 0) {
		double progress = (double)byte / total;
		err = notification_set_progress(noti_data->handle, progress);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_set_progress().. [0x%x]", err);
		}
	}
	else {
		err = notification_set_size(noti_data->handle, (double)byte);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_set_size().. [0x%x]", err);
		}
	}

	return (err == NOTIFICATION_ERROR_NONE) ? true : false;
}


/*static notification_h
_mp_noti_box_create_real_notification(const char *extra_data)
{
	int err = 0;

	notification_h handle = NULL;
	handle = notification_create(NOTIFICATION_TYPE_NOTI);
	if (!handle) {
		mp_error("notification_create()");
		goto exception;
	}

	if (extra_data) {
		DEBUG_TRACE("extar_data = %s", extra_data);
		app_control_h service = _mp_noti_make_excute_option(MP_DOWNLOAD_NOTIFICATION, extra_data);
		if (service)
		{
			bundle *b = NULL;
			app_control_to_bundle(service, &b);
			err = notification_set_execute_option(handle, NOTIFICATION_EXECUTE_TYPE_MULTI_LAUNCH, NULL, NULL, b);
			app_control_destroy(service);
			service = NULL;
			if (err != NOTIFICATION_ERROR_NONE) {
				mp_error("notification_set_execute_option().. [0x%x]", err);
				goto exception;
			}
		}
	}

	err = notification_set_layout(handle, NOTIFICATION_LY_NOTI_EVENT_SINGLE);
	if (err != NOTIFICATION_ERROR_NONE) {
		mp_error("notification_set_layout().. [0x%x]", err);
		goto exception;
	}

	err = notification_set_display_applist(handle,  NOTIFICATION_DISPLAY_APP_NOTIFICATION_TRAY);
	if (err != NOTIFICATION_ERROR_NONE) {
		mp_error("notification_set_display_applist().. [0x%x]", err);
		goto exception;
	}

	return handle;

exception:
	if (handle) {
		notification_delete(handle);
		notification_free(handle);
		handle = NULL;
	}

	return NULL;
}

mp_noti_h
mp_noti_box_create(const char *title, const char *extra_data)
{
	MP_CHECK_NULL(title);

	int err = 0;

	notification_h handle = NULL;
	handle = _mp_noti_box_create_real_notification(extra_data);

	err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_TITLE, GET_STR(title), NULL, NOTIFICATION_TEXT_TYPE_NONE);
	if (err != NOTIFICATION_ERROR_NONE) {
		mp_error("notification_set_text().. [0x%x]", err);
		goto exception;
	}

	int priv_id = 0;
	err = notification_insert(handle, &priv_id);
	if (err != NOTIFICATION_ERROR_NONE) {
		mp_error("notification_insert().. [0x%x]", err);
		goto exception;
	}

	mp_noti_s *noti_data = (mp_noti_s *)calloc(1, sizeof(mp_noti_s));
	if (!noti_data)
		goto exception;
	noti_data->handle = handle;
	noti_data->type = NOTIFICATION_TYPE_NOTI;
	noti_data->extra_data = g_strdup(extra_data);

	mp_debug("notification[%s] created", title);
	return noti_data;

exception:
	if (handle) {
		notification_delete(handle);
		notification_free(handle);
		handle = NULL;
	}
	return NULL;
}

void
mp_noti_box_update(mp_noti_h noti, const char *title, const char *info, time_t time)
{
	mp_noti_s *noti_data = noti;
	MP_CHECK(noti_data);
	MP_CHECK(noti_data->type == NOTIFICATION_TYPE_NOTI);
	MP_CHECK(noti_data->handle);

	notification_h handle = notification_load(NULL, noti_data->priv_id);
	if (handle == NULL)
	{
		DEBUG_TRACE("priv id [%d] :: notification is deleted", noti_data->priv_id);
		notification_delete(noti_data->handle);
		notification_free(noti_data->handle);
		noti_data->handle = NULL;
		noti_data->count = 0;
		noti_data->priv_id = 0;
	}

	if (noti_data->handle == NULL)
		noti_data->handle = _mp_noti_box_create_real_notification(noti_data->extra_data);

	int err = 0;
	handle = noti_data->handle;
	MP_CHECK(handle);

	noti_data->count++;
	title = GET_STR(title);
	char *new_title = NULL;
	if (noti_data->count > 1)
	{
		new_title = g_strdup_printf("%s (%d)", title, noti_data->count);
		title = new_title;
	}

	if (title)
	{
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_TITLE, title, NULL, NOTIFICATION_TEXT_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_set_text().. [0x%x]", err);
		}
	}

	DEBUG_TRACE("box [%s][%d] update .. ", title, noti_data->count);
	IF_FREE(new_title);

	if (info)
	{
		err = notification_set_text(handle, NOTIFICATION_TEXT_TYPE_INFO_1, info, NULL, NOTIFICATION_TEXT_TYPE_NONE);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_set_text().. [0x%x]", err);
		}
	}

	if (time)
	{
		err = notification_set_time_to_text(handle, NOTIFICATION_TEXT_TYPE_INFO_2, time);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_set_time_to_text().. [0x%x]", err);
		}
	}

	if (noti_data->priv_id)
	{
		err = notification_update(handle);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_update().. [0x%x]", err);
		}
	}
	else
	{
		err = notification_insert(handle, &noti_data->priv_id);
		if (err != NOTIFICATION_ERROR_NONE) {
			mp_error("notification_insert().. [0x%x]", err);
			notification_delete(handle);
			notification_free(handle);
			noti_data->priv_id = 0;
			noti_data->handle = NULL;
		}
	}
}*/

