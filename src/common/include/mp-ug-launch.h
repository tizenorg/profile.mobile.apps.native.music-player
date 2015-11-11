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

#ifndef __MP_ug_launch_H_
#define __MP_ug_launch_H_
#include <app_control.h>
#include <app_manager.h>

int mp_ug_email_attatch_file(const char *filepath, void *user_data);
int mp_ug_bt_attatch_file(const char *filepath, int count, void *user_data);
//int mp_ug_wifi_setting();
#ifndef MP_SOUND_PLAYER
int mp_ug_camera_take_picture(void *data);
int mp_ug_gallery_get_picture(void *data);
#endif
int mp_ug_message_attatch_file(const char *filepath, void *user_data);
int mp_ug_contact_user_sel(const char *filepath, void *user_data);
int mp_ug_set_as_alarm_tone(const char *filepath, int position);
int mp_ug_show_info(struct appdata *ad);
void mp_ug_send_message(struct appdata *ad, mp_ug_message_t message);
#ifdef MP_FEATURE_WIFI_SHARE
int mp_ug_wifi_attatch_file(const char *filepath, int count, void *user_data);
#endif
#ifdef MP_FEATURE_CLOUD
int mp_ug_dropbox_attatch_file(const char *filepath, int count, void *user_data);
#endif
void mp_ug_destory_current(struct appdata *ad);

bool mp_send_via_appcontrol(struct appdata *ad, mp_send_type_e send_type, const char *files);
bool mp_setting_privacy_launch(void);
bool mp_setting_wifi_launch(void);
bool mp_setting_network_launch(void);

#endif // __MP_ug_launch_H_
