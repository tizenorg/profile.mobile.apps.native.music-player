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
#include <stdbool.h>

#include "ms-key-ctrl.h"
#include "ms-util.h"
#include "mp-player-mgr.h"

#ifndef EXPORT_API
#define EXPORT_API __attribute__ ((visibility("default")))
#endif

#define dgettext_noop(s)	(s)
#define N_(s)			dgettext_noop(s)

#define MP_SETTING_INIT 				"db/setting/music-player/initialization"	//boolean

#define DEFAULT_ALBUMS 		true
#define DEFAULT_ARTISTS		true
#define DEFAULT_GENRES		true
#define DEFAULT_COMPOSERS	false
#define DEFAULT_YEARS		false
#define DEFAULT_FOLDERS		false
#define DEFAULT_SQUARE		false

#define DEFAULT_MENU_VAL 0x7

#define DEFAULT_EQ_CUSTOM_VAL (0)

static char *auto_off_time_text[KEY_MUSIC_AUTO_OFF_TIME_MAX] = {
	N_(STR_MP_NOT_USED),
	N_("IDS_MUSIC_OPT_AFTER_15_MIN_ABB"),
	N_("IDS_MUSIC_BODY_AFTER_30_MIN"),
	N_("IDS_MUSIC_BODY_AFTER_1_HOUR"),
	N_("IDS_MUSIC_POP_AFTER_1_HOUR_30_MIN"),
	N_("IDS_MUSIC_BODY_AFTER_2_HOURS"),
	N_("IDS_MUSIC_BODY_CUSTOM"),
};

EXPORT_API int
ms_key_set_menu_changed(void)
{
	if (preference_set_boolean(KEY_MUSIC_MENU_CHANGE, true)) {
		ERROR_TRACE("Fail to set KEY_MUSIC_MENU_CHANGE");
		return -1;
	}

	return 0;
}

EXPORT_API int
ms_key_set_playlist_val(int b_val)
{
	if (preference_set_int(MP_PREFKEY_PLAYLIST_VAL_INT, b_val)) {
		ERROR_TRACE("Fail to set %s : %d", MP_PREFKEY_PLAYLIST_VAL_INT, b_val);
		return -1;
	}


	return 0;
}

EXPORT_API int
ms_key_get_playlist_val(int *b_val)
{
	if (preference_get_int(MP_PREFKEY_PLAYLIST_VAL_INT, b_val)) {
		ERROR_TRACE("Fail to get %s ", MP_PREFKEY_PLAYLIST_VAL_INT);
		return -1;
	}
	return 0;
}

EXPORT_API int
ms_key_set_playlist_str(char* b_str)
{
	if (preference_set_string(MP_PREFKEY_PLAYLIST_VAL_STR, b_str)) {
		ERROR_TRACE("Fail to set %s : %s", MP_PREFKEY_PLAYLIST_VAL_STR, b_str);
		return -1;
	}


	return 0;
}

EXPORT_API int
ms_key_get_playlist_str(char **b_str)
{
	MP_CHECK_VAL(b_str, -1);
	int ret = 0;
	ret = preference_get_string(MP_PREFKEY_PLAYLIST_VAL_STR, b_str);
	if (*b_str == NULL) {
		ERROR_TRACE("Fail to get %d ", ret);
		return -1;
	}
	return 0;
}


EXPORT_API int ms_key_set_tabs_str(char* b_str)
{
	if (preference_set_string(MP_PREFKEY_TABS_VAL_STR, b_str)) {
		ERROR_TRACE("Fail to set %s : %s", MP_PREFKEY_TABS_VAL_STR, b_str);
		return -1;
	}


	return 0;
}

EXPORT_API int ms_key_get_tabs_str(char **b_str)
{
	MP_CHECK_VAL(b_str, -1);
	int ret = 0;
	ret = preference_get_string(MP_PREFKEY_TABS_VAL_STR, b_str);
	if (*b_str == NULL) {
		ERROR_TRACE("Fail to get %d ", ret);
		return -1;
	}
	return 0;
}

EXPORT_API int
ms_key_set_tabs_val(int b_val)
{
	if (preference_set_int(MP_PREFKEY_TABS_VAL_INT, b_val)) {
		ERROR_TRACE("Fail to set %s : %d", MP_PREFKEY_TABS_VAL_INT, b_val);
		return -1;
	}


	return 0;
}

EXPORT_API int
ms_key_get_tabs_val(int *b_val)
{
	if (preference_get_int(MP_PREFKEY_TABS_VAL_INT, b_val)) {
		ERROR_TRACE("Fail to get %s ", MP_PREFKEY_TABS_VAL_INT);
		return -1;
	}
	return 0;
}

EXPORT_API int
ms_key_set_eq_custom(ms_eq_custom_t custom_val)
{
	int ret = 0;

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_1, custom_val.band_1)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_1");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_2, custom_val.band_2)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_2");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_3, custom_val.band_3)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_3");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_4, custom_val.band_4)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_4");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_5, custom_val.band_5)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_5");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_6, custom_val.band_6)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_6");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_7, custom_val.band_7)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_7");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_8, custom_val.band_8)) {
		ERROR_TRACE("fail to set KEY_MUSIC_EQUALISER_CUSTOM_8");
		ret = -1;
	}

	return ret;
}

EXPORT_API int
ms_key_set_extended_effect(ms_extended_effect_t *extended_val)
{
	mp_retv_if(!extended_val, -1);

	int ret = 0;

	if (preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_3D, extended_val->three_d)) {
		ERROR_TRACE("fail to set KEY_MUSIC_USER_AUDIO_EFFECT_3D");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_BASS, extended_val->bass)) {
		ERROR_TRACE("fail to set KEY_MUSIC_USER_AUDIO_EFFECT_BASS");
		ret = -1;
	}

	if (preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_CLARITY, extended_val->clarity)) {
		ERROR_TRACE("fail to set KEY_MUSIC_USER_AUDIO_EFFECT_CLARITY");
		ret = -1;
	}

	return ret;
}

EXPORT_API void
ms_key_set_user_effect(int value)
{
	DEBUG_TRACE("value = [0x%x]", value);
	if (preference_set_int(KEY_MUSIC_SA_USER_CHANGE, value) != 0) {
		ERROR_TRACE("preference_set_int() failed");
	}
}

EXPORT_API int
ms_key_set_auto_off_time(int min)
{
	int ret = preference_set_int(KEY_MUSIC_AUTO_OFF_TIME_VAL, min);
	if (ret != 0) {
		ERROR_TRACE("preference_set_int() failed");
	}

	return ret;
}

EXPORT_API int
ms_key_get_auto_off_time(void)
{
	int min = 0;
	int ret = preference_get_int(KEY_MUSIC_AUTO_OFF_TIME_VAL, &min);
	if (ret != 0) {
		ERROR_TRACE("preference_set_int() failed");
	}

	return min;
}

EXPORT_API int
ms_key_set_auto_off_custom_time(int min)
{
	int ret = preference_set_int(KEY_MUSIC_AUTO_OFF_CUSTOM_TIME, min);
	if (ret != 0) {
		ERROR_TRACE("preference_set_int() failed");
	}

	return ret;
}

EXPORT_API int
ms_key_get_auto_off_custom_time(void)
{
	int min = 0;
	int ret = preference_get_int(KEY_MUSIC_AUTO_OFF_CUSTOM_TIME, &min);
	if (ret != 0) {
		ERROR_TRACE("preference_set_int() failed");
	}

	return min;
}

EXPORT_API int
ms_key_set_auto_off_val(int type)
{
	int ret = preference_set_int(KEY_MUSIC_AUTO_OFF_TYPE_VAL, type);
	if (ret != 0) {
		ERROR_TRACE("preference_set_int() failed");
	}

	return ret;
}

EXPORT_API int
ms_key_get_auto_off_val(void)
{
	int type = 0;
	if (preference_get_int(KEY_MUSIC_AUTO_OFF_TYPE_VAL, &type) != 0) {
		ERROR_TRACE("preference_get_int() failed");
		return KEY_MUSIC_AUTO_OFF_TIME_OFF;	// 0
	}

	return type;
}

EXPORT_API char*
ms_key_get_auto_off_time_text(int index)
{
	if (index < 0 || index >= KEY_MUSIC_AUTO_OFF_TIME_MAX) {
		ERROR_TRACE("invalid index : %d", index);
		return NULL;
	}

	char *text = NULL;
	if (index == 0) {
		text = GET_SYS_STR(auto_off_time_text[index]);
	} else {
		text = GET_STR(auto_off_time_text[index]);
	}

	return text;
}

EXPORT_API double
ms_key_get_play_speed(void)
{
	double speed = 0.0;
	if (preference_get_double(PREFKEY_MUSIC_PLAY_SPEED, &speed) != 0) {
		ERROR_TRACE("preference_get_dbl failed");
		return 1.0;
	}
	return speed;
}

EXPORT_API void
ms_key_set_play_speed(double speed)
{
	if (preference_set_double(PREFKEY_MUSIC_PLAY_SPEED, speed) != 0) {
		ERROR_TRACE("preference_set_dbl() failed");
	}
}


