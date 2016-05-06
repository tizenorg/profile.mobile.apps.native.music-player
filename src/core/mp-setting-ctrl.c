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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sound_manager.h>
#include "mp-media-info.h"

#include "music.h"

#include "mp-setting-ctrl.h"
#include "mp-player-debug.h"
#include "mp-file-tag-info.h"
#include "mp-player-mgr.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-vconf-private-keys.h"
#include "mp-player-view.h"
#include "mp-minicontroller.h"
#include <system_settings.h>
#include <mp-file-util.h>

#ifdef MP_FEATURE_AVRCP_13
#include "mp-avrcp.h"
#endif

enum _mp_menu_item {
	MP_MENU_ALBUMS,
	MP_MENU_ARTISTS,
	MP_MENU_GENRES,
	MP_MEMU_COMPOSER,
	MP_MENU_YEARS,
	MP_MENU_FOLDERS,
	MP_MENU_NUMS,
};

typedef struct _mp_setting_t {
#ifdef MP_FEATURE_AUTO_OFF
	MpSettingAutoOff_Cb auto_off_cb;
	void *auto_off_udata;
#endif
#ifdef MP_FEATURE_PLAY_SPEED
	MpSettingPlaySpeed_Cb play_speed_cb;
	void *play_speed_udata;
#endif

	int side_sync_status;

} mp_setting_t;

static mp_setting_t *g_setting = NULL;


static Eina_Bool _mp_setting_init_idler_cb(void *data);

static void
_mp_setting_playlist_changed_cb(const char *key, void *user_data)
{
	startfunc;

	mp_retm_if(key == NULL, "keymode is NULL");
	mp_retm_if(user_data == NULL, "user_date is NULL");

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_SETTING_PLAYLIST_CHANGED);

	return;
}

#ifdef MP_FEATURE_AUTO_OFF
static void
_mp_setting_auto_off_changed_cb(const char *key, void *user_data)
{
	mp_setting_t *sd = NULL;
	mp_retm_if(user_data == NULL, "user_date is NULL");
	sd = (mp_setting_t *) user_data;

	int min = 0;
	if (preference_get_int(KEY_MUSIC_AUTO_OFF_TIME_VAL, &min)) {
		ERROR_TRACE("Fail to get %s", KEY_MUSIC_AUTO_OFF_TIME_VAL);
		return;
	}

	mp_debug("auto off time changed [%d] miniute", min);
	if (sd->auto_off_cb) {
		sd->auto_off_cb(min, sd->auto_off_udata);
	}

	return;
}
#endif

#ifdef MP_FEATURE_PLAY_SPEED
static void
_mp_setting_play_speed_changed_cb(const char *key, void *user_data)
{
	mp_setting_t *sd = NULL;
	mp_retm_if(user_data == NULL, "user_date is NULL");
	sd = (mp_setting_t *) user_data;

	double speed = 0;
	if (preference_get_double(PREFKEY_MUSIC_PLAY_SPEED, &speed)) {
		ERROR_TRACE("Fail to get %s", PREFKEY_MUSIC_PLAY_SPEED);
		return;
	}

	mp_debug("play speed changed [%f]", speed);
	if (sd->play_speed_cb) {
		sd->play_speed_cb(speed, sd->play_speed_udata);
	}

	return;
}
#endif

static void
_mp_setting_lyric_changed_cb(const char *key, void *user_data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	bool show_lyrics = (bool)(ad->b_show_lyric);
	if (preference_get_boolean(KEY_MUSIC_LYRICS, &show_lyrics)) {
		ERROR_TRACE("Fail to get %s", KEY_MUSIC_LYRICS);
	}
	ad->b_show_lyric = (int)(ad->b_show_lyric);
}

/*static void
_mp_setting_side_sync_changed_cb(keynode_t * node, void *user_data)
{

	g_setting->side_sync_status = vconf_keynode_get_int(node);
	EVENT_TRACE("Side sync activated[%d]", g_setting->side_sync_status);

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_SIDE_SYNC_STATUS_CHANGED);
}*/

static void
_mp_setting_shuffle_changed_cb(const char *key, void *user_data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);
	int val = 0;
	mp_setting_get_shuffle_state(&val);
	mp_playlist_mgr_set_shuffle(ad->playlist_mgr, val);

	mp_player_view_update_state(GET_PLAYER_VIEW);
	mp_minicontroller_update_shuffle_and_repeat_btn(ad);
}

static void
_mp_setting_repeat_changed_cb(const char *key, void *user_data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);
	int val = 0;
	mp_setting_get_repeat_state(&val);
	mp_playlist_mgr_set_repeat(ad->playlist_mgr, val);

	mp_player_view_update_state(GET_PLAYER_VIEW);
	mp_minicontroller_update_shuffle_and_repeat_btn(ad);
}

static int
mp_setting_key_cb_init(void)
{
	int ret = 0;

	mp_retvm_if(g_setting == NULL, -1, "setting data is not initialized, init first!!!!!");

	if (preference_set_changed_cb(MP_PREFKEY_PLAYLIST_VAL_INT, _mp_setting_playlist_changed_cb, g_setting) < 0) {
		ERROR_TRACE("Fail to register MP_PREFKEY_PLAYLIST_VAL_INT key callback");
		ret = -1;
	}

	if (preference_set_changed_cb(MP_KEY_MUSIC_SHUFFLE, _mp_setting_shuffle_changed_cb, g_setting) < 0) {
		ERROR_TRACE("Fail to register MP_KEY_MUSIC_SHUFFLE key callback");
		ret = -1;
	}

	if (preference_set_changed_cb(MP_KEY_MUSIC_REPEAT, _mp_setting_repeat_changed_cb, g_setting) < 0) {
		ERROR_TRACE("Fail to register MP_KEY_MUSIC_REPEAT key callback");
		ret = -1;
	}

#ifdef MP_FEATURE_AUTO_OFF
	if (preference_set_changed_cb(KEY_MUSIC_AUTO_OFF_TIME_VAL, _mp_setting_auto_off_changed_cb, g_setting) < 0) {
		ERROR_TRACE("Fail to register KEY_MUSIC_AUTO_OFF_TIME_VAL key callback");
		ret = -1;
	}
#endif

#ifdef MP_FEATURE_PLAY_SPEED
	if (preference_set_changed_cb(PREFKEY_MUSIC_PLAY_SPEED, _mp_setting_play_speed_changed_cb, g_setting) < 0) {
		ERROR_TRACE("Fail to register PREFKEY_MUSIC_PLAY_SPEED key callback");
		ret = -1;
	}
#endif
	if (preference_set_changed_cb(KEY_MUSIC_LYRICS, _mp_setting_lyric_changed_cb, g_setting) < 0) {
		ERROR_TRACE("Fail to register KEY_MUSIC_LYRICS key callback");
		ret = -1;
	}

	return ret;
}

static void
mp_setting_key_cb_deinit(void)
{
	preference_unset_changed_cb(MP_PREFKEY_PLAYLIST_VAL_INT);
#ifdef MP_FEATURE_AUTO_OFF
	preference_unset_changed_cb(KEY_MUSIC_AUTO_OFF_TIME_VAL);
#endif

#ifdef MP_FEATURE_PLAY_SPEED
	int retcode = preference_unset_changed_cb(PREFKEY_MUSIC_PLAY_SPEED);
	if (retcode != PREFERENCE_ERROR_NONE) {
		ERROR_TRACE("Unable to unset key assigned for speed play in Music Player [%d]", retcode);
	}
#endif
	return;
}

static void _mp_setting_init_preference_key(void)
{
	startfunc;

	bool exist;
	preference_is_existing(KEY_MP_PERSONAL_PAGE, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MP_PERSONAL_PAGE);
		preference_set_boolean(KEY_MP_PERSONAL_PAGE, false);
	}
	preference_is_existing(KEY_MUSIC_SE_CHANGE, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_SE_CHANGE);
		preference_set_boolean(KEY_MUSIC_SE_CHANGE, true);
	}
	preference_is_existing(KEY_MUSIC_SA_USER_CHANGE, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_SA_USER_CHANGE);
		preference_set_boolean(KEY_MUSIC_SA_USER_CHANGE, false);
	}
	preference_is_existing(KEY_MUSIC_MENU_CHANGE, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_MENU_CHANGE);
		preference_set_boolean(KEY_MUSIC_MENU_CHANGE, true);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_1, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_1);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_1, 0.5);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_2, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_2);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_2, 0.5);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_3, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_3);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_3, 0.5);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_4, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_4);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_4, 0.5);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_5, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_5);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_5, 0.5);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_6, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_6);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_6, 0.5);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_7, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_7);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_7, 0.5);
	}
	preference_is_existing(KEY_MUSIC_EQUALISER_CUSTOM_8, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_EQUALISER_CUSTOM_8);
		preference_set_double(KEY_MUSIC_EQUALISER_CUSTOM_8, 0.5);
	}
	preference_is_existing(KEY_MUSIC_USER_AUDIO_EFFECT_3D, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_USER_AUDIO_EFFECT_3D);
		preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_3D, 0.0);
	}
	preference_is_existing(KEY_MUSIC_USER_AUDIO_EFFECT_BASS, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_USER_AUDIO_EFFECT_BASS);
		preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_BASS, 0.0);
	}
	preference_is_existing(KEY_MUSIC_USER_AUDIO_EFFECT_ROOM, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_USER_AUDIO_EFFECT_ROOM);
		preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_ROOM, 0.0);
	}
	preference_is_existing(KEY_MUSIC_USER_AUDIO_EFFECT_REVERB, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_USER_AUDIO_EFFECT_REVERB);
		preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_REVERB, 0.0);
	}
	preference_is_existing(KEY_MUSIC_USER_AUDIO_EFFECT_CLARITY, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_USER_AUDIO_EFFECT_CLARITY);
		preference_set_double(KEY_MUSIC_USER_AUDIO_EFFECT_CLARITY, 0.0);
	}
	preference_is_existing(KEY_MUSIC_AUTO_OFF_TIME_VAL, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_AUTO_OFF_TIME_VAL);
		preference_set_int(KEY_MUSIC_AUTO_OFF_TIME_VAL, 0.0);
	}
	preference_is_existing(KEY_MUSIC_AUTO_OFF_TYPE_VAL, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_AUTO_OFF_TYPE_VAL);
		preference_set_double(KEY_MUSIC_AUTO_OFF_TYPE_VAL, 0.0);
	}
	preference_is_existing(KEY_MUSIC_AUTO_OFF_CUSTOM_TIME, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_AUTO_OFF_CUSTOM_TIME);
		preference_set_int(KEY_MUSIC_AUTO_OFF_CUSTOM_TIME, 0.0);
	}
	preference_is_existing(MP_KEY_MUSIC_SQUARE_AXIS_VAL, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", MP_KEY_MUSIC_SQUARE_AXIS_VAL);
		preference_set_int(MP_KEY_MUSIC_SQUARE_AXIS_VAL, 0.0);
	}
	preference_is_existing(MP_PREFKEY_PLAYLIST_VAL_INT, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", MP_PREFKEY_PLAYLIST_VAL_INT);
		preference_set_int(MP_PREFKEY_PLAYLIST_VAL_INT, 15);
	}
	preference_is_existing(MP_PREFKEY_PLAYLIST_VAL_STR, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", MP_PREFKEY_PLAYLIST_VAL_STR);
		preference_set_string(MP_PREFKEY_PLAYLIST_VAL_STR, "1342");
	}
	preference_is_existing(MP_PREFKEY_TABS_VAL_STR, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", MP_PREFKEY_TABS_VAL_STR);
		preference_set_string(MP_PREFKEY_TABS_VAL_STR, "1234567");
	}
	preference_is_existing(MP_PREFKEY_TABS_VAL_INT, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", MP_PREFKEY_TABS_VAL_INT);
		preference_set_int(MP_PREFKEY_TABS_VAL_INT, 63);
	}
	preference_is_existing(PREFKEY_MUSIC_PLAY_SPEED, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", PREFKEY_MUSIC_PLAY_SPEED);
		preference_set_double(PREFKEY_MUSIC_PLAY_SPEED, 1.0);
	}
	preference_is_existing(KEY_MUSIC_MOTION_ASKED, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_MOTION_ASKED);
		preference_set_boolean(KEY_MUSIC_MOTION_ASKED, false);
	}
	preference_is_existing(KEY_MUSIC_SQUARE_ASKED, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_SQUARE_ASKED);
		preference_set_boolean(KEY_MUSIC_SQUARE_ASKED, false);
	}
	preference_is_existing(KEY_MUSIC_SMART_VOLUME, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_SMART_VOLUME);
		preference_set_boolean(KEY_MUSIC_SMART_VOLUME, false);
	}
	preference_is_existing(KEY_MUSIC_LYRICS, &exist);
	if (!exist) {
		WARN_TRACE("[%s] not exist", KEY_MUSIC_LYRICS);
		preference_set_boolean(KEY_MUSIC_LYRICS, true);
	}
}


static Eina_Bool
_mp_setting_init_idler_cb(void *data)
{
	startfunc;
	TIMER_TRACE();
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK_FALSE(ad);

	bool show_lyric = (bool)(ad->b_show_lyric);
	if (preference_get_boolean(KEY_MUSIC_LYRICS, &show_lyric)) {
		WARN_TRACE("Fail to get %s", KEY_MUSIC_LYRICS);
	}
	ad->b_show_lyric = (int)show_lyric;

	_mp_setting_init_preference_key();
	mp_setting_key_cb_init();

	ad->setting_idler = NULL;

	int shuffle;
	mp_setting_get_shuffle_state(&shuffle);
	mp_playlist_mgr_set_shuffle(ad->playlist_mgr, shuffle);

#ifdef MP_FEATURE_AUTO_OFF
	/* reset auto off in music player only */
	/* mp_setting_reset_auto_off_time(); */
#endif

	return EINA_FALSE;
}

int
mp_setting_init(struct appdata *ad)
{
	int ret = 0;
	MP_CHECK_VAL(!g_setting, -1);
	g_setting = malloc(sizeof(mp_setting_t));
	if (!g_setting) {
		ERROR_TRACE("Fail to alloc memory");
		return -1;
	}
	memset(g_setting, 0x00, sizeof(mp_setting_t));

	/*if (!ad->setting_idler)
		ad->setting_idler =	ecore_idler_add(_mp_setting_init_idler_cb, ad); */

	_mp_setting_init_idler_cb(ad);

	return ret;
}


int
mp_setting_deinit(struct appdata *ad)
{
	mp_ecore_idler_del(ad->setting_idler);
	mp_setting_key_cb_deinit();

	if (g_setting) {
		free(g_setting);
		g_setting = NULL;
	}

	return 0;
}

void
mp_setting_set_nowplaying_id(int val)
{
	startfunc;

	char *path = app_get_data_path();
	char playing_id[1024] = {0};
	if (path == NULL) {
		return;
	}
	snprintf(playing_id, 1024, "%s%s", path, MP_NOW_PLAYING_ID_INI);
	free(path);
	FILE *fp = fopen(playing_id, "w");	/* make new file. */

	if (fp == NULL) {
		SECURE_ERROR("Failed to open ini files. : %s", playing_id);
		return;
	}

	fprintf(fp, "#Nowplaying\n");
	fprintf(fp, "%d\n", val);

	fsync((int)fp);
	fclose(fp);

	return;
}

int
mp_setting_get_nowplaying_id(void)
{
	startfunc;

	FILE *fp = NULL;
	char line[MAX_NAM_LEN + 1];
	int pid = -1;
	char *path = app_get_data_path();
	char playing_id[1024] = {0};
	if (path == NULL) {
		return 0;
	}
	snprintf(playing_id, 1024, "%s%s", path, MP_NOW_PLAYING_ID_INI);
	free(path);
	if ((fp = fopen(playing_id, "r")) == NULL) {
		ERROR_TRACE("unable to open ini file: %s", playing_id);
		return -1;
	}
	if (fgets(line, MAX_NAM_LEN, fp)) { /* #Nowplaying */
		/* skip */
	}
	if (fgets(line, MAX_NAM_LEN, fp)) { /* pid */
		line[MAX_NAM_LEN] = 0;
		line[strlen(line) - 1] = 0;
		pid = atoi(line);
	}
	fclose(fp);

	return pid;
}

void
mp_setting_set_player_state(int val)
{
	startfunc;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	DEBUG_TRACE("%d", ad->freeze_indicator_icon);
	if (val == MP_PLAY_STATE_PLAYING) {
		ad->freeze_indicator_icon = false;
	}

	if (ad->freeze_indicator_icon) {
		WARN_TRACE("icon freezed.. skip state changes [%d]", val);
		return;
	}

	char *path = app_get_data_path();
	char play_state_file[1024] = {0};
	if (path == NULL) {
		return;
	}
	snprintf(play_state_file, 1024, "%s%s", path, MP_PLAY_STATE);
	free(path);
	FILE *fp = fopen(play_state_file, "w");	/* make new file. */

	if (fp == NULL) {
		SECURE_ERROR("Failed to open ini files. : %s", play_state_file);
		return;
	}

	char *state = "off";

	if (val == MP_PLAY_STATE_NONE) {
		state = "off";
	} else if (val == MP_PLAY_STATE_PLAYING) {
		state = "play";
	} else if (val == MP_PLAY_STATE_PAUSED) {
		state = "pause";
	} else if (val == MP_PLAY_STATE_STOP) {
		state = "stop";
	}
	fprintf(fp, "%s\n", state);

	fsync((int)fp);
	fclose(fp);

	return;
}

int
mp_setting_set_shuffle_state(int b_val)
{
	if (preference_set_boolean(MP_KEY_MUSIC_SHUFFLE, b_val)) {
		WARN_TRACE("Fail to set MP_KEY_MUSIC_SHUFFLE");
		return -1;
	}

	return 0;
}

int
mp_setting_get_shuffle_state(int *b_val)
{
	if (preference_get_boolean(MP_KEY_MUSIC_SHUFFLE, (bool *)b_val)) {
		WARN_TRACE("Fail to get MP_KEY_MUSIC_SHUFFLE");

		if (preference_set_boolean(MP_KEY_MUSIC_SHUFFLE, FALSE)) {
			ERROR_TRACE("Fail to set MP_KEY_MUSIC_SHUFFLE");
			return -1;
		}
		*b_val = FALSE;
	}
	return 0;
}

int
mp_setting_set_repeat_state(int val)
{
	if (preference_set_int(MP_KEY_MUSIC_REPEAT, val)) {
		ERROR_TRACE("Fail to set MP_KEY_MUSIC_REPEAT");
		return -1;
	}

	return 0;
}

int
mp_setting_get_repeat_state(int *val)
{
	if (preference_get_int(MP_KEY_MUSIC_REPEAT, val)) {
		WARN_TRACE("Fail to get MP_KEY_MUSIC_REPEAT");
		if (preference_set_int(MP_KEY_MUSIC_REPEAT, MP_SETTING_REP_NON)) {
			ERROR_TRACE("Fail to set MP_KEY_MUSIC_REPEAT");
			return -1;
		}
		*val = MP_SETTING_REP_NON;
	}

	return 0;
}

int mp_setting_playlist_get_state(int *state)
{
	int res = preference_get_int(MP_PREFKEY_PLAYLIST_VAL_INT, state);
	return res;
}

/* This fuction save current track data for homescreen. */
void
mp_setting_save_playing_info(void *data)
{
	struct appdata *ad = data;
	FILE *fp = NULL;
	mp_plst_item *item = NULL;

	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);
	MP_CHECK(ad->current_track_info);

	char *path = app_get_data_path();
	char playing_ini_file_name[1024] = {0};
	if (path == NULL) {
		return;
	}
#ifndef MP_SOUND_PLAYER
	snprintf(playing_ini_file_name, 1024, "%s%s", path, MP_PLAYING_INI_FILE_NAME_MUSIC);
	fp = fopen(playing_ini_file_name, "w");        /* make new file. */
#else
	snprintf(playing_ini_file_name, 1024, "%s%s", path, MP_PLAYING_INI_FILE_NAME_SOUND);
	fp = fopen(playing_ini_file_name, "w");        /* make new file. */
#endif
	if(path)
		free(path);

	if (fp == NULL) {
#ifndef MP_SOUND_PLAYER
		ERROR_TRACE("Failed to open ini files. : %s", playing_ini_file_name);
#else
		ERROR_TRACE("Failed to open ini files. : %s", playing_ini_file_name);
#endif
		return;
	}


	char total_time[16] = { 0, };
	char position[16] = {0,};
	char *playing = "false";

	if (ad->music_length > 3600.) {
		snprintf(total_time, sizeof(total_time), "%" MUSIC_TIME_FORMAT,
		         MUSIC_TIME_ARGS(ad->music_length + 0.5));
		snprintf(position, sizeof(position), "%" MUSIC_TIME_FORMAT,
		         MUSIC_TIME_ARGS(ad->music_pos + 0.5));
	} else {
		snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
		         PLAY_TIME_ARGS(ad->music_length + 0.5));
		snprintf(position, sizeof(position), "%" PLAY_TIME_FORMAT,
		         PLAY_TIME_ARGS(ad->music_pos + 0.5));
	}

	if (mp_player_mgr_get_state() == PLAYER_STATE_PLAYING) {
		playing = "true";
	}

	fprintf(fp, "%s\n", ad->current_track_info->title);
	fprintf(fp, "%s\n", ad->current_track_info->artist);
	fprintf(fp, "%s\n", ad->current_track_info->thumbnail_path);
	fprintf(fp, "%s\n", total_time);
	fprintf(fp, "%s\n", position);
	fprintf(fp, "%s\n", playing);
	fprintf(fp, "\n");

	fclose(fp);

}

void
mp_setting_remove_now_playing_shared_status(void)
{
	startfunc;
	char *path = app_get_data_path();
	char playing_status[1024] = {0};
	if (path == NULL) {
		return;
	}
	snprintf(playing_status, 1024, "%s%s", path, MP_SHARED_PLAYING_STATUS_INI);
	free(path);

	FILE *fp = fopen(playing_status, "w");	/* make new file. */

	if (fp == NULL) {
		ERROR_TRACE("Failed to open ini files. : %s", playing_status);
		return;
	}
	fprintf(fp, " \n");

	fclose(fp);

	endfunc;
}

void
mp_setting_remove_now_playing(void)
{
	startfunc;
	FILE *fp = NULL;

	char *path = app_get_data_path();
	char playing_info[1024] = {0};
	if (path == NULL) {
		return;
	}
	snprintf(playing_info, 1024, "%s%s", path, MP_NOWPLAYING_INI_FILE_NAME);
	free(path);
	fp = fopen(playing_info, "w");	/* make new file. */

	if (fp == NULL) {
		SECURE_ERROR("Failed to open ini files. : %s", playing_info);
		return;
	}
	fprintf(fp, " \n");

	fclose(fp);

	endfunc;
}

void
mp_setting_get_now_playing_path_from_file(char **path)
{
	MP_CHECK(path);

	char line[MAX_NAM_LEN + 1];
	FILE *fp = NULL;

	char *data_path = app_get_data_path();
	char playing_info[1024] = {0};
	if (data_path == NULL) {
		return;
	}
	snprintf(playing_info, 1024, "%s%s", data_path, MP_NOWPLAYING_INI_FILE_NAME);
	free(data_path);
	if(access(playing_info, F_OK) != -1)
	{
		fp = fopen(playing_info, "r");
		if (!fp) {
			SECURE_ERROR("unable to open %s...", playing_info);
			return;
		}
		if (fgets(line, MAX_NAM_LEN, fp)) { /* audio id */
			/* skip */
		}
		if (fgets(line, MAX_NAM_LEN, fp)) { /* uri */
			line[MAX_NAM_LEN] = 0;
			line[strlen(line) - 1] = 0;
			*path = g_strdup(line);
		}
		fclose(fp);
	}
}

#ifndef MP_SOUND_PLAYER
void
mp_setting_save_now_playing(void *data)
{
	startfunc;
	struct appdata *ad = data;
	FILE *fp = NULL;
	mp_plst_item *item = NULL;

	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);

	item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK(item);
	MP_CHECK(ad->current_track_info);

	char *path = app_get_data_path();
	char playing_info[1024] = {0};
	if (path == NULL) {
		return;
	}
	snprintf(playing_info, 1024, "%s%s", path, MP_NOWPLAYING_INI_FILE_NAME);
	free(path);
	fp = fopen(playing_info, "w");	/* make new file. */

	if (fp == NULL) {
		SECURE_ERROR("Failed to open ini files. : %s", playing_info);
		return;
	}

	char total_time[16] = { 0, };
	char music_length[16] = { 0, };
	if (ad->music_length > 3600.) {
		snprintf(total_time, sizeof(total_time), "%" MUSIC_TIME_FORMAT,
		         MUSIC_TIME_ARGS(ad->music_length));
	} else {
		snprintf(total_time, sizeof(total_time), "%" PLAY_TIME_FORMAT,
		         PLAY_TIME_ARGS(ad->music_length));
	}
	snprintf(music_length, sizeof(music_length), "%f", ad->music_length);

	fprintf(fp, "%s\n", item->uid);
	fprintf(fp, "%s\n", item->uri);
	fprintf(fp, "%s\n", ad->current_track_info->title);
	fprintf(fp, "%s\n", ad->current_track_info->artist);
	fprintf(fp, "%s\n", ad->current_track_info->album);
	fprintf(fp, "%s\n", ad->current_track_info->thumbnail_path);
	fprintf(fp, "%s\n", total_time);
	fprintf(fp, "%s\n", music_length);
	fprintf(fp, "\n");

	fclose(fp);

	endfunc;
}

#define MP_SHORTCUT_COUNT 4

void
mp_setting_save_shortcut(char *shortcut_title, char *artist, char *shortcut_description, char *shortcut_image_path)
{
	startfunc;
	FILE *fp = NULL;
	int ret = 0;

	char *path = app_get_data_path();
	char shortcut_path_0[1024] = {0};
	char shortcut_path_1[1024] = {0};
	char shortcut_path_2[1024] = {0};
	char shortcut_path_3[1024] = {0};
	if (path == NULL) {
		return;
	}

	snprintf(shortcut_path_0, 1024, "%s%s", path, MP_SHORTCUT_INI_FILE_NAME_0);
	snprintf(shortcut_path_1, 1024, "%s%s", path, MP_SHORTCUT_INI_FILE_NAME_1);
	snprintf(shortcut_path_2, 1024, "%s%s", path, MP_SHORTCUT_INI_FILE_NAME_2);
	snprintf(shortcut_path_3, 1024, "%s%s", path, MP_SHORTCUT_INI_FILE_NAME_3);

	free(path);
	if (mp_file_exists(shortcut_path_2)) {
		ret = rename(shortcut_path_2, shortcut_path_3);
		if (ret != 0) {
			ERROR_TRACE("Failed to rename file:error=%d", ret);
			return;
		}
	}
	if (mp_file_exists(shortcut_path_1)) {
		rename(shortcut_path_1, shortcut_path_2);
		if (ret != 0) {
			ERROR_TRACE("Failed to rename file:error=%d", ret);
			return;
		}
	}
	if (mp_file_exists(shortcut_path_0)) {
		rename(shortcut_path_0, shortcut_path_1);
		if (ret != 0) {
			ERROR_TRACE("Failed to rename file:error=%d", ret);
			return;
		}
	}

	fp = fopen(shortcut_path_0, "w");	/* make new file. */

	if (fp == NULL) {
		SECURE_ERROR("Failed to open ini files. : %s", shortcut_path_0);
		return;
	}

	fprintf(fp, "[ShortCut]\n");
	fprintf(fp, "title=%s\n", shortcut_title);
	if (artist) {
		fprintf(fp, "artist=%s\n", artist);
	}
	fprintf(fp, "desc=%s\n", shortcut_description);
	fprintf(fp, "artwork=%s\n", shortcut_image_path);
	fprintf(fp, "\n");

	fclose(fp);

	endfunc;
}
#endif

int
mp_setting_read_playing_status(char *uri, char *status)
{
	startfunc;
	char str[1000] = {0,};
	int valid_uri = 0;
	int valid_status = 0;
	char *path = app_get_data_path();
	char playing_status[1024] = {0};
	if (path == NULL) {
		return -1;
	}
	snprintf(playing_status, 1024, "%s%s", path, MP_SHARED_PLAYING_STATUS_INI);
	free(path);

	FILE *fp = fopen(playing_status, "r");	/* read MP_SHARED_PLAYING_STATUS_INI file.  */

	if (fp == NULL) {
		ERROR_TRACE("Failed to open ini files. : %s", playing_status);
		return -1;
	}
	char *sptr = NULL;
	while (fgets(str, sizeof(str), fp)) {
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
		if (key != NULL) {
			if (!strcmp(key, " ")) {
				fclose(fp);
				return 0;
			}

			if (!strcmp(key, "status")) {
				DEBUG_TRACE("status: %s", status);
				if (!strcmp(value, status)) {
					valid_status = 1;
				}
			}

			if (!strcmp(key, "uri")) {
				DEBUG_TRACE("uri: %s", uri);
				if (!strcmp(value, uri)) {
					valid_uri = 1;
				}
			}
		}
	}
	if ((valid_uri == 1) && (valid_status == 1)) {
		if (fp) {
			fclose(fp);
		}
		return 1;
	}
	if (fp) {
		fclose(fp);
	}
	return 0;
}

void
mp_setting_write_playing_status(char *uri, char *status)
{
	startfunc;
	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	if (count <= 0) {
		mp_setting_remove_now_playing_shared_status();
	} else {
		char *path = app_get_data_path();
		DEBUG_TRACE("Path is: %s", path);
		char playing_status[1024] = {0};
		if (path == NULL) {
			return;
		}
		snprintf(playing_status, 1024, "%s%s", path, MP_SHARED_PLAYING_STATUS_INI);
		free(path);

		FILE *fp = fopen(playing_status, "w");	/* make new file. */

		if (fp == NULL) {
			ERROR_TRACE("Failed to open ini files. : %s", playing_status);
			return;
		}

		fprintf(fp, "#Nowplaying\n");
		fprintf(fp, "uri=%s\n", uri);
		fprintf(fp, "status=%s\n", status);
#ifndef MP_SOUND_PLAYER
		struct appdata *ad = mp_util_get_appdata();
		if (ad && ad->current_track_info) {
			fprintf(fp, "title=%s\n", ad->current_track_info->title);
			fprintf(fp, "artist=%s\n", ad->current_track_info->artist);
			fprintf(fp, "album=%s\n", ad->current_track_info->album);
			if (ad->current_track_info->thumbnail_path) {
				fprintf(fp, "thumbnail=%s\n", ad->current_track_info->thumbnail_path);
			} else {
				fprintf(fp, "thumbnail=%s/res/shared_images/default_albumart.png\n", SHAREDDIR);
			}
		}
#endif

		fprintf(fp, "changePlayer=%s\n", "false");
		fsync((int)fp);
		fclose(fp);
	}

	endfunc;
}

#ifdef MP_FEATURE_AUTO_OFF
int
mp_setting_auto_off_set_callback(MpSettingAutoOff_Cb func, void *data)
{
	mp_retvm_if(g_setting == NULL, -1, "setting data is not initialized, init first!!!!!");

	g_setting->auto_off_cb = func;
	g_setting->auto_off_udata = data;

	return 0;
}

void
mp_setting_reset_auto_off_time()
{
	int ret = preference_set_int(KEY_MUSIC_AUTO_OFF_TIME_VAL, 0);
	if (ret) {
		mp_error("preference_set_int().. err[%d]", ret);
	}
	ret = preference_set_int(KEY_MUSIC_AUTO_OFF_TYPE_VAL, 0);
	if (ret) {
		mp_error("preference_set_int().. err[%d]", ret);
	}
}

int
mp_setting_get_auto_off_time()
{
	int min = 0;

	if (preference_get_int(KEY_MUSIC_AUTO_OFF_TIME_VAL, &min)) {
		mp_error("preference_get_int()");
		min = 0;
	}

	return min;
}
#endif

#ifdef MP_FEATURE_PLAY_SPEED
int mp_setting_set_play_speed_change_callback(MpSettingPlaySpeed_Cb func, void *data)
{
	mp_retvm_if(g_setting == NULL, -1, "setting data is not initialized, init first!!!!!");

	g_setting->play_speed_cb = func;
	g_setting->play_speed_udata = data;

	return 0;
}
int mp_setting_reset_play_speed(void)
{
	int ret = preference_set_double(PREFKEY_MUSIC_PLAY_SPEED, 1.0);
	if (ret) {
		mp_error("preference_set_dbl().. err[%d]", ret);
	}
	return ret;
}

double mp_setting_get_play_speed(void)
{
	double speed = 1.0;
	if (preference_get_double(PREFKEY_MUSIC_PLAY_SPEED, &speed)) {
		mp_error("preference_get_int()");
	}

	if (speed > (double)2.0) {
		ERROR_TRACE("inavlid speed: %f", speed);
		speed = 2.0;
	} else if (speed < (double)0.5) {
		ERROR_TRACE("inavlid speed: %f", speed);
		speed = 0.5;
	}

	return speed;
}

#endif

void
mp_setting_update_active_device()
{
}

int mp_setting_get_side_sync_status(void)
{
	if (!g_setting) {
		mp_setting_init(mp_util_get_appdata());
	}
	return g_setting->side_sync_status;

}

#ifdef MP_FEATURE_PERSONAL_PAGE
bool mp_setting_set_personal_dont_ask_again(bool bAsked)
{
	int err = 0;
	err = preference_set_boolean(KEY_PERSONAL_NO_ASK_AGAIN, bAsked);
	if (err != 0) {
		ERROR_TRACE("SET KEY_PERSONAL_NO_ASK_AGAIN is fail [0x%x]", err);
		return FALSE;
	}

	return TRUE;
}

bool mp_setting_get_personal_dont_ask_again(bool *bAsked)
{
	int err = 0;
	bool val = FALSE;

	err = preference_get_boolean(KEY_PERSONAL_NO_ASK_AGAIN, &val);
	if (err != 0) {
		ERROR_TRACE("GET KEY_PERSONAL_NO_ASK_AGAIN is fail [0x%x]", err);
		return FALSE;
	}

	*bAsked = (val == FALSE) ? FALSE : TRUE;

	return TRUE;
}

#endif
