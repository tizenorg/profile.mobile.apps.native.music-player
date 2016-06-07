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

#include <Elementary.h>
#include <tizen.h>
#include <Eet.h>
#include <app.h>
#include <stdlib.h>
#include <bundle.h>
#include <telephony.h>
#include <message_port.h>
#include <linux/inotify.h>
#include <notification.h>
#include <dirent.h>
#include <unistd.h>
#include "mp-common-defs.h"
#include "mp-resource.h"
#include "mp-define.h"
#include "mp_widget.h"
#include "mp_widget_debug.h"

#define WIDGET_HEIGHT 500
#define WIDGET_WIDTH 712
#define EDJE_FILE "music_widget.edj"
#define APP_ID "org.tizen.music-player"
#define MP_LB_EVENT_KEY "LiveboxEvent"
#define MP_LB_EVENT_PLAY_CLICKED "OnLBPlayClicked"
#define MP_LB_EVENT_PAUSE_CLICKED "OnLBPauseClicked"
#define MP_LB_EVENT_NEXT_RELEASED "OnLBNextRelease"
#define MP_LB_EVENT_PREV_RELEASED "OnLBPreviousRelease"
#define EVENT_SIZE (sizeof(struct inotify_event))
#define EVENT_BUF_LEN (1024 * (EVENT_SIZE + 16))
#define domain_name "music-player"

#define BROKEN_ALBUMART_IMAGE_PATH		"/opt/usr/share/media/.thumb/thumb_default.png"
#define DEFAULT_ALBUM_ART_ICON SHAREDDIR"/res/shared_images/default_albumart.png"
#define NOW_PLAYING_INI_PATH "NowPlayingStatus"

static Eina_Bool is_play = EINA_FALSE;
Eina_List *widget_list = NULL;

void mp_widget_win_del_cb(void *data, Evas *evas, Evas_Object *obj,
                          void *event_info)
{
	ecore_timer_del(data);
}

void mp_widget_key_down_cb(void *data, Evas *evas, Evas_Object *obj,
                           void *event_info)
{
	elm_exit();
}

static void mp_widget_read_ini_file_ecore(void *data, char *path)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return;
	}
	if (!path) {
		DEBUG_TRACE("Invalid path information");
		return;
	}
	WidgetData *wgtdata = (WidgetData *)data;
	Evas_Object *layout = (Evas_Object *)wgtdata->layout;
	if (!layout) {
		DEBUG_TRACE("Invalid layout");
		return;
	}

	Evas_Object *image_object = NULL;
	int line_count = 0;
	char str[1000] = {0,};
	char *image_path = NULL;
	FILE *file = fopen(path, "r");

	if (!file) {
		ERROR_TRACE("Failed to open file(%s)", path);
		elm_object_signal_emit(layout, "no_music", "elm");
		return;
	}

	char *sptr = NULL;
	while (fgets(str, sizeof(str), file)) {
		char *key = NULL;
		char *value = NULL;
		key = strtok_r(str, "=", &sptr);
		value = strtok_r(NULL, "=", &sptr);
		DEBUG_TRACE("key is: %s and value is: %s", key, value);
		if (value != NULL) {
			value[strlen(value) - 1] = '\0';
		} else {
			DEBUG_TRACE("value is NULL");
			continue;
		}

		if (key == NULL) {
			continue;
		}

		if (!strcmp(key, " ")) {
			elm_object_signal_emit(layout, "no_music", "elm");
			fclose(file);
			return;
		}
		if (!strcmp(key, "status")) {
			if (!strcmp(value, "playing")) {
				elm_object_signal_emit(layout, "music_present", "elm");
				elm_object_signal_emit(layout, "play_music", "elm");
				is_play = EINA_TRUE;
			} else {
				elm_object_signal_emit(layout, "music_present", "elm");
				elm_object_signal_emit(layout, "pause_music", "elm");
				is_play = EINA_FALSE;
			}
		}
		if ((char *)wgtdata->file_path) {
			free((char *)wgtdata->file_path);
			wgtdata->file_path = NULL;
		}
		if (!strcmp(key, "uri")) {
			wgtdata->file_path = (char *)malloc((strlen(value) + 1) * sizeof(char));
			if (wgtdata->file_path) {
				strncpy(wgtdata->file_path, value, strlen(value));
				wgtdata->file_path[strlen(value)] = '\0';
			}
		}
		if (!strcmp(key, "title")) {
			elm_object_part_text_set(layout, "track_title", value);
		}
		if (!strcmp(key, "artist")) {
			elm_object_part_text_set(layout, "track_artist_title",
			                         value);
		}
		if (!strcmp(key, "thumbnail")) {
			if (value) {
				image_path = (char*)malloc((strlen(value) + 1) * sizeof(char));

				if (image_path != NULL) {
					strncpy(image_path, value, strlen(value));
					image_path[strlen(value)] = '\0';

					if (!strcmp(BROKEN_ALBUMART_IMAGE_PATH, image_path)) {
						free(image_path);
						image_path = NULL;
						image_path = (char*)malloc((strlen(DEFAULT_ALBUM_ART_ICON) + 1) * sizeof(char));
						if (image_path != NULL) {
							strncpy(image_path, DEFAULT_ALBUM_ART_ICON, strlen(DEFAULT_ALBUM_ART_ICON));
							image_path[strlen(DEFAULT_ALBUM_ART_ICON)] = '\0';
						}
					}
				}
			}
		}
		line_count++;
	}

	if (line_count <= 1) {
		elm_object_signal_emit(layout, "no_music", "elm");
		is_play = EINA_FALSE;
	}

	if (image_path != NULL) {
		image_object = elm_image_add(layout);

		if (!elm_image_file_set(image_object, image_path, NULL)) {
			free(image_path);
			image_path = (char*)malloc((strlen(DEFAULT_ALBUM_ART_ICON) + 1) * sizeof(char));
			if (image_path != NULL) {
				strncpy(image_path, DEFAULT_ALBUM_ART_ICON, strlen(DEFAULT_ALBUM_ART_ICON));
				image_path[strlen(DEFAULT_ALBUM_ART_ICON)] = '\0';
				elm_image_file_set(image_object, image_path, NULL);
			}
		}

		elm_image_aspect_fixed_set(image_object, EINA_FALSE);
		elm_object_part_content_set(layout, "track_image", image_object);
		if (image_path) {
			free(image_path);
		}
	}

	fclose(file);
}

void __create_read_ini_file(void)
{
	char *path = app_get_data_path();
	char playing_status[1024] = {0};
	if (path == NULL) {
		return;
	}
	snprintf(playing_status, 1024, "%s%s", path, NOW_PLAYING_INI_PATH);
	free(path);

	FILE *fp = fopen(playing_status, "w");	/* make new file. */

	if (fp == NULL) {
		ERROR_TRACE("Failed to open ini files. : %s", playing_status);
		return;
	}
	fprintf(fp, " \n");

	fclose(fp);
}

static void mp_widget_read_ini_file(char *path, void *data)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return;
	}
	if (!path) {
		DEBUG_TRACE("Invalid path information");
		return;
	}
	WidgetData *wgtdata = (WidgetData *)data;
	Evas_Object *layout = (Evas_Object *)wgtdata->layout;
	if (!layout) {
		DEBUG_TRACE("Invalid layout");
		return;
	}

	char buffer[1000 + 1] = {'\0'};
	Evas_Object *image_object = NULL;
	int line_count = 0;
	char str[1000] = {0,};
	char *image_path = NULL;
	FILE *file = fopen(path, "r");

	if (!file) {
		__create_read_ini_file();
		ERROR_TRACE("ERROR VAlUE is (%d)", strerror_r(errno, buffer, 1000));
		ERROR_TRACE("Failed to open %s file", path);
		elm_object_signal_emit(layout, "no_music", "elm");
		return;
	}

	char *sptr = NULL;
	while (fgets(str, sizeof(str), file)) {
		char *key = NULL;
		char *value = NULL;
		key = strtok_r(str, "=", &sptr);
		value = strtok_r(NULL, "=", &sptr);
		DEBUG_TRACE("key is: %s and value is: %s", key, value);
		if (value != NULL) {
			value[strlen(value) - 1] = '\0';
		} else {
			DEBUG_TRACE("value is NULL");
			continue;
		}
		if (!key) {
			fclose(file);
			if (image_path) {
				free(image_path);
			}
			return;
		}

		if (!strcmp(key, " ")) {
			elm_object_signal_emit(layout, "no_music", "elm");
			fclose(file);
			if (image_path) {
				free(image_path);
			}
			return;
		}
		if (!strcmp(key, "status")) {
			if (!strcmp(value, "playing")) {
				elm_object_signal_emit(layout, "music_present", "elm");
				elm_object_signal_emit(layout, "play_music", "elm");
				is_play = EINA_TRUE;
			} else {
				elm_object_signal_emit(layout, "music_present", "elm");
				elm_object_signal_emit(layout, "pause_music", "elm");
				is_play = EINA_FALSE;
			}
		}
		if ((char *)wgtdata->file_path) {
			free((char *)wgtdata->file_path);
			wgtdata->file_path = NULL;
		}
		if (!strcmp(key, "uri")) {
			wgtdata->file_path = (char *)malloc((strlen(value) + 1) * sizeof(char));

			if (wgtdata->file_path) {
				strncpy(wgtdata->file_path, value, strlen(value));
				wgtdata->file_path[strlen(value)] = '\0';
			}
		}
		if (!strcmp(key, "title")) {
			elm_object_part_text_set(layout, "track_title", value);
		}
		if (!strcmp(key, "artist")) {
			elm_object_part_text_set(layout, "track_artist_title",
			                         value);
		}
		if (!strcmp(key, "thumbnail")) {
			if (value) {
				image_path = (char*)malloc((strlen(value) + 1) * sizeof(char));
				if (image_path) {
					strncpy(image_path, value, strlen(value));
					image_path[strlen(value)] = '\0';

					if (!strcmp(BROKEN_ALBUMART_IMAGE_PATH, image_path)) {
						free(image_path);
						image_path = NULL;
					}
				}
			}
		}
		line_count++;
	}

	if (line_count <= 1) {
		elm_object_signal_emit(layout, "no_music", "elm");
		is_play = EINA_FALSE;
	}

	image_object = elm_image_add(layout);
	if (!elm_image_file_set(image_object, image_path, NULL)) {
		image_path = (char*)malloc((strlen(DEFAULT_ALBUM_ART_ICON) + 1) * sizeof(char));
		if (image_path) {
			strncpy(image_path, DEFAULT_ALBUM_ART_ICON, strlen(DEFAULT_ALBUM_ART_ICON));
			image_path[strlen(DEFAULT_ALBUM_ART_ICON)] = '\0';
			elm_image_file_set(image_object, image_path, NULL);
		}
	}

	elm_image_aspect_fixed_set(image_object, EINA_FALSE);
	elm_object_part_content_set(layout, "track_image", image_object);
	if (image_path) {
		free(image_path);
	}
	fclose(file);
}

static void mp_widget_music_player_result_callback(app_control_h request, app_control_h reply, app_control_result_e result, void *user_data)
{
	if (!user_data) {
		return;
	}
}


static void mp_widget_music_player_result(void *user_data)
{
	if (!user_data) {
		return;
	}
}

static int message_port_send_event_message(const char *event)
{
	int ret;
	bundle *b = bundle_create();
	if (b == NULL) {
		ERROR_TRACE("Unable to add data to bundle");
		return -1;
	}
	bundle_add_str(b, MP_LB_EVENT_KEY, event);
	ret = message_port_send_message(APP_ID, MP_MESSAGE_PORT_LIVEBOX, b);
	if (ret != MESSAGE_PORT_ERROR_NONE) {
		ERROR_TRACE("Message remote port error: %d", ret);
	}
	bundle_free(b);
	return ret;
}

bool check_remote_message_port(void)
{
	int ret = -1;
	bool found = false;

	ret = message_port_check_remote_port(APP_ID, MP_MESSAGE_PORT_LIVEBOX, &found);
	if (ret != MESSAGE_PORT_ERROR_NONE) {
		ERROR_TRACE("message_port_check_remote_port error : %d", ret);
	}
	return found;
}

static int message_port_init(const char *event)
{
	int ret = -1;
	check_remote_message_port();
	ret = message_port_send_event_message(event);
	return ret;
}

static int mp_widget_music_player_launch(void *data, char **extra_data_keys,
        char **extra_data_values, int extra_data_length, Eina_Bool show_player, char *filepath)
{
	Evas_Object *layout = (Evas_Object *)data;
	if (layout == NULL) {
		return -1;
	}
	app_control_h service = NULL;
	int ret = -1;
	int i = 0;

	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("Failed to create appcontrol");
	} else {
		app_control_set_app_id(service, "org.tizen.music-player");
		app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);

		for (i = 0; i < extra_data_length; ++i) {
			app_control_add_extra_data(service, extra_data_keys[i],
			                           extra_data_values[i]);
		}

		if (!show_player) {
			app_control_add_extra_data(service, "request_type", "livebox");
		} else {
			app_control_add_extra_data(service, "host_type", "hide");
		}
		app_control_add_extra_data(service, "uri", filepath);

		ret = app_control_send_launch_request(service,
		                                      mp_widget_music_player_result_callback, layout);
		if (ret != APP_CONTROL_ERROR_NONE) {
			ERROR_TRACE("Failed to send launch request");
		}

		app_control_destroy(service);
	}
	return ret;
}

static void mp_widget_click_on_add_tracks_cb(void *data, Evas_Object *obj,
        const char *emission, const char *source)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return;
	}
	WidgetData *wgtdata = (WidgetData *)data;
	Evas_Object *layout = (Evas_Object *)wgtdata->layout;
	if (!layout) {
		DEBUG_TRACE("Invalid layout");
		return;
	}
	int extra_data_len = 1;
	int i = 0;
	char **extra_data_keys = NULL;
	char **extra_data_values = NULL;

	extra_data_keys = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_keys) {
		return;
	}

	extra_data_values = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_values) {
		free(extra_data_keys);
		return;
	}

	for (i = 0; i < extra_data_len; ++i) {
		extra_data_keys[i] = (char *)malloc(50);
		extra_data_values[i] = (char *)malloc(50);
	}

	mp_widget_music_player_launch(layout, extra_data_keys,
	                              extra_data_values, extra_data_len, EINA_TRUE, NULL);
	mp_widget_music_player_result(layout);

	for (i = 0; i < extra_data_len; ++i) {
		free(extra_data_keys[i]);
		free(extra_data_values[i]);
	}

	free(extra_data_keys);
	free(extra_data_values);
}

static void mp_widget_click_on_track_image_cb(void *data, Evas_Object *obj,
        const char *emission, const char *source)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return;
	}
	WidgetData *wgtdata = (WidgetData *)data;
	Evas_Object *layout = (Evas_Object *)wgtdata->layout;
	if (!layout) {
		DEBUG_TRACE("Invalid layout");
		return;
	}
	int extra_data_len = 1;
	int i = 0;
	char **extra_data_keys = NULL;
	char **extra_data_values = NULL;

	extra_data_keys = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_keys) {
		return;
	}

	extra_data_values = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_values) {
		free(extra_data_keys);
		return;
	}

	for (i = 0; i < extra_data_len; ++i) {
		extra_data_keys[i] = (char *)malloc(50);
		extra_data_values[i] = (char *)malloc(50);
	}

	mp_widget_music_player_launch(layout, extra_data_keys,
	                              extra_data_values, extra_data_len, EINA_TRUE, wgtdata->file_path);
	mp_widget_music_player_result(layout);

	for (i = 0; i < extra_data_len; ++i) {
		free(extra_data_keys[i]);
		free(extra_data_values[i]);
	}

	free(extra_data_keys);
	free(extra_data_values);
}

static bool telephony_is_call_connected(void)
{
	DEBUG_TRACE("start");
	telephony_call_h *call_list_sim1, *call_list_sim2;
	unsigned int count_sim1, count_sim2;
	telephony_handle_list_s tel_list;
	telephony_error_e ret_sim1, ret_sim2;

	int tel_valid = telephony_init(&tel_list);
	if (tel_valid != 0) {
		ERROR_TRACE("telephony is not initialized. ERROR Code is %d", tel_valid);
		return false;
	}

	ret_sim1 = telephony_call_get_call_list(tel_list.handle[0], &count_sim1, &call_list_sim1);
	if (ret_sim1 != TELEPHONY_ERROR_NONE) {
		ERROR_TRACE("Cannot get call list information for primary sim");
	}

	ret_sim2 = telephony_call_get_call_list(tel_list.handle[1], &count_sim2, &call_list_sim2);
	if (ret_sim2 != TELEPHONY_ERROR_NONE) {
		ERROR_TRACE("Cannot get call list information for secondey sim");
	}

	telephony_call_release_call_list(count_sim1, &call_list_sim1);
	telephony_call_release_call_list(count_sim2, &call_list_sim2);
	telephony_deinit(&tel_list);

	if (count_sim1 == 0 && count_sim2 == 0) {
		return false;
	} else {
		return true;
	}

	return false;
}

static void mp_widget_click_on_play_cb(void *data, Evas_Object *obj,
                                       const char *emission, const char *source)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return;
	}

	bool call_state = telephony_is_call_connected();
	const char *message = NULL;
	if (call_state) {
		message = STR_MP_UNABLE_TO_PLAY_DURING_CALL;
		WARN_TRACE("receive PLAYER_INTERRUPTED_BY_CALL");
		if (message) {
			int ret = notification_status_message_post(dgettext(domain_name, message));
			if (ret != 0) {
				ERROR_TRACE("notification_status_message_post()... [0x%x]", ret);
			} else {
				DEBUG_TRACE("message: [%s]", message);
			}
		}
		return;
	}

	WidgetData *wgtdata = (WidgetData *)data;
	Evas_Object *layout = (Evas_Object *)wgtdata->layout;
	if (!layout) {
		DEBUG_TRACE("Invalid layout");
		return;
	}

	int extra_data_len = 1;
	int i = 0;
	char **extra_data_keys = NULL;
	char **extra_data_values = NULL;

	extra_data_keys = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_keys) {
		return;
	}

	extra_data_values = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_values) {
		free(extra_data_keys);
		return;
	}

	for (i = 0; i < extra_data_len; ++i) {
		extra_data_keys[i] = (char *)malloc(50);
		extra_data_values[i] = (char *)malloc(50);
	}

	strncpy(extra_data_keys[0], MP_LB_EVENT_KEY, strlen(MP_LB_EVENT_KEY));
	extra_data_keys[0][strlen(MP_LB_EVENT_KEY)] = '\0';

	if (!is_play) {
		strncpy(extra_data_values[0], MP_LB_EVENT_PLAY_CLICKED, strlen(MP_LB_EVENT_PLAY_CLICKED));
		extra_data_values[0][strlen(MP_LB_EVENT_PLAY_CLICKED)] = '\0';
	} else {
		strncpy(extra_data_values[0], MP_LB_EVENT_PAUSE_CLICKED, strlen(MP_LB_EVENT_PAUSE_CLICKED));
		extra_data_values[0][strlen(MP_LB_EVENT_PAUSE_CLICKED)] = '\0';
	}

	if (message_port_init(extra_data_values[0]) != MESSAGE_PORT_ERROR_NONE) {
		mp_widget_music_player_launch(layout, extra_data_keys,
		                              extra_data_values, extra_data_len, EINA_FALSE, wgtdata->file_path);
		mp_widget_music_player_result(layout);
	}
	for (i = 0; i < extra_data_len; ++i) {
		free(extra_data_keys[i]);
		free(extra_data_values[i]);
	}

	free(extra_data_keys);
	free(extra_data_values);
}

static void mp_widget_click_on_previous_cb(void *data, Evas_Object *obj,
        const char *emission, const char *source)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return;
	}

	bool call_state = telephony_is_call_connected();
	if (call_state) {
		WARN_TRACE("receive PLAYER_INTERRUPTED_BY_CALL");
		return;
	}

	WidgetData *wgtdata = (WidgetData *)data;
	Evas_Object *layout = (Evas_Object *)wgtdata->layout;
	if (!layout) {
		DEBUG_TRACE("Invalid layout");
		return;
	}

	int extra_data_len = 1;
	int i = 0;
	char **extra_data_keys = NULL;
	char **extra_data_values = NULL;

	extra_data_keys = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_keys) {
		return;
	}

	extra_data_values = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_values) {
		free(extra_data_keys);
		return;
	}

	for (i = 0; i < extra_data_len; ++i) {
		extra_data_keys[i] = (char *)malloc(50);
		extra_data_values[i] = (char *)malloc(50);
	}

	strncpy(extra_data_keys[0], MP_LB_EVENT_KEY, strlen(MP_LB_EVENT_KEY));
	extra_data_keys[0][strlen(MP_LB_EVENT_KEY)] = '\0';

	strncpy(extra_data_values[0], MP_LB_EVENT_PREV_RELEASED, strlen(MP_LB_EVENT_PREV_RELEASED));
	extra_data_values[0][strlen(MP_LB_EVENT_PREV_RELEASED)] = '\0';

	if (message_port_init(extra_data_values[0]) != MESSAGE_PORT_ERROR_NONE) {
		mp_widget_music_player_launch(layout, extra_data_keys,
		                              extra_data_values, extra_data_len, EINA_FALSE, wgtdata->file_path);
		mp_widget_music_player_result(layout);
	}

	for (i = 0; i < extra_data_len; ++i) {
		free(extra_data_keys[i]);
		free(extra_data_values[i]);
	}

	free(extra_data_keys);
	free(extra_data_values);
}

static void mp_widget_click_on_next_cb(void *data, Evas_Object *obj,
                                       const char *emission, const char *source)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return;
	}

	bool call_state = telephony_is_call_connected();
	if (call_state) {
		WARN_TRACE("receive PLAYER_INTERRUPTED_BY_CALL");
		return;
	}

	WidgetData *wgtdata = (WidgetData *)data;
	Evas_Object *layout = (Evas_Object *)wgtdata->layout;
	if (!layout) {
		DEBUG_TRACE("Invalid layout");
		return;
	}

	int extra_data_len = 1;
	int i = 0;
	char **extra_data_keys = NULL;
	char **extra_data_values = NULL;

	extra_data_keys = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_keys) {
		return;
	}

	extra_data_values = (char **)malloc(extra_data_len * sizeof(char *));
	if (!extra_data_values) {
		free(extra_data_keys);
		return;
	}

	for (i = 0; i < extra_data_len; ++i) {
		extra_data_keys[i] = (char *)malloc(50);
		extra_data_values[i] = (char *)malloc(50);
	}

	strncpy(extra_data_keys[0], MP_LB_EVENT_KEY, strlen(MP_LB_EVENT_KEY));
	extra_data_keys[0][strlen(MP_LB_EVENT_KEY)] = '\0';

	strncpy(extra_data_values[0], MP_LB_EVENT_NEXT_RELEASED, strlen(MP_LB_EVENT_NEXT_RELEASED));
	extra_data_values[0][strlen(MP_LB_EVENT_NEXT_RELEASED)] = '\0';

	if (message_port_init(extra_data_values[0]) != MESSAGE_PORT_ERROR_NONE) {
		mp_widget_music_player_launch(layout, extra_data_keys,
		                              extra_data_values, extra_data_len, EINA_FALSE, wgtdata->file_path);
		mp_widget_music_player_result(layout);
	}

	for (i = 0; i < extra_data_len; ++i) {
		free(extra_data_keys[i]);
		free(extra_data_values[i]);
	}

	free(extra_data_keys);
	free(extra_data_values);
}

static void __mp_change_multiple_widgets(void *data, Ecore_File_Monitor *em, Ecore_File_Event event, const char *path)
{
	Eina_List *temp_list = NULL;
	WidgetData* wgtdata = NULL;
	if (event == ECORE_FILE_EVENT_MODIFIED || event == ECORE_FILE_EVENT_CREATED_FILE) {
		DEBUG_TRACE("The monitored file path is: %s", ecore_file_monitor_path_get(em));
		EINA_LIST_FOREACH(widget_list, temp_list, wgtdata) {
			char *path = app_get_data_path();
			char playing_status[1024] = {0};
			if (path == NULL) {
				return;
			}
			snprintf(playing_status, 1024, "%s%s", path, NOW_PLAYING_INI_PATH);
			free(path);
			mp_widget_read_ini_file_ecore(wgtdata, playing_status);
		}
	}
	free(temp_list);
}

int mp_widget_create(WidgetData* data, int w, int h)
{
	if (!data) {
		DEBUG_TRACE("Invalid data");
		return -1;
	}
	char edj_path[PATH_MAX] = {0,};
	Evas_Object *layout = NULL;

	layout = elm_layout_add(data->win);
	if (!layout) {
		DEBUG_TRACE("Failed to add layout");
		return -1;
	}
	data->layout = layout;

	char *res_path = app_get_resource_path();
	DEBUG_TRACE("Resource Path is: %s", res_path);
	if (res_path == NULL) {
		return -1;
	}
	snprintf(edj_path, PATH_MAX, "%s%s", res_path, EDJE_FILE);
	free(res_path);

	Eina_Bool fileSet = elm_layout_file_set(layout, edj_path, "mp_widget_main");
	DEBUG_TRACE("Widget Layout File Set: %d", fileSet);
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	if ((elm_config_scale_get() - 1.7) < 0.0001) {
		h = 304;
	} else if ((elm_config_scale_get() - 1.8) < 0.0001) {
		h = 253;
	} else if ((elm_config_scale_get() - 2.4) < 0.0001) {
		h = 405;
	} else if ((elm_config_scale_get() - 2.6) < 0.0001) {
		h = 405;
	} else if ((elm_config_scale_get() - 2.8) < 0.0001) {
		h = 405;
	}

	evas_object_resize(layout, w, h);
	evas_object_show(layout);

	widget_list = eina_list_append(widget_list, data);
	DEBUG_TRACE("Number of widgets: %d", eina_list_count(widget_list));

	elm_object_domain_translatable_part_text_set(layout, "noitems_title", "music-player", "IDS_MUSIC_BODY_MUSIC");
	elm_object_domain_translatable_part_text_set(layout, "noitems_subtitle", "music-player", "IDS_MUSIC_SK3_ADD_TRACKS");
	char *path = app_get_data_path();
	DEBUG_TRACE("Path is: %s", path);
	char playing_status[1024] = {0};
	if (path == NULL) {
		return -1;
	}
	snprintf(playing_status, 1024, "%s%s", path, NOW_PLAYING_INI_PATH);
	free(path);
	mp_widget_read_ini_file(playing_status, data);

	if (data->em == NULL) {
		data->em = ecore_file_monitor_add(playing_status, __mp_change_multiple_widgets, NULL);
	}

	elm_object_signal_callback_add(layout, "mouse,down,1",
	                               "noitems_subtitle", mp_widget_click_on_add_tracks_cb,
	                               (void *)data);
	elm_object_signal_callback_add(layout, "mouse,down,1",
	                               "track_image", mp_widget_click_on_track_image_cb,
	                               (void *)data);
	elm_object_signal_callback_add(layout, "mouse,down,1",
	                               "track_play_image", mp_widget_click_on_play_cb,
	                               (void *)data);
	elm_object_signal_callback_add(layout, "mouse,down,1",
	                               "track_prev_image", mp_widget_click_on_previous_cb,
	                               (void *)data);
	elm_object_signal_callback_add(layout, "mouse,down,1",
	                               "track_next_image", mp_widget_click_on_next_cb,
	                               (void *)data);


	return 0;
}

/* End of a file */
