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


#include <glib.h>
#include <string.h>
#include "music.h"
#include "mp-player-mgr.h"
#include "mp-player-control.h"
#include "mp-play.h"
/* #include "mp-player-drm.h" */
#include <player.h>

#include "mp-ta.h"
#include "mp-player-debug.h"
#include "mp-widget.h"
#include "mp-streaming-mgr.h"
#include "mp-util.h"
#include "mp-volume.h"
#include "mp-minicontroller.h"
#include "mp-setting-ctrl.h"
#include "mp-lockscreenmini.h"
#include "mp-player-view.h"

#ifdef MP_FEATURE_AVRCP_13
#include "mp-avrcp.h"
#endif

#define MAX_PATH_LEN			1024

static player_h _player = 0;
static mp_player_type_e _player_type = MP_PLAYER_TYPE_MMFW;
static void *_player_buffer;

static bool is_seeking = false;
static bool resume_on_seek_done = false;
static bool pause_on_seek_done = false;
static int g_reserved_seek_pos = -1;
bool reacquire_state;

static Seek_Done_Cb g_requesting_cb = NULL;
static void *g_requesting_cb_data = NULL;

static Seek_Done_Cb g_reserved_cb = NULL;
static void *g_reserved_cb_data = NULL;

static Ecore_Pipe *g_player_pipe = NULL;

#define PLAYER_ENTER_LOG(LABEL) WARN_TRACE("ENTER:"LABEL)
#define PLAYER_LEAVE_LOG(LABEL) WARN_TRACE("LEAVE:"LABEL)

typedef enum {
	MP_PLAYER_CB_TYPE_STARTED,
	MP_PLAYER_CB_TYPE_COMPLETED,
	MP_PLAYER_CB_TYPE_INTURRUPTED,
	MP_PLAYER_CB_TYPE_ERROR,
	MP_PLAYER_CB_TYPE_BUFFERING,
	MP_PLAYER_CB_TYPE_PREPARE,
	MP_PLAYER_CB_TYPE_PAUSED,
	MP_PLAYER_CB_TYPE_DURATION_CHANGED,
	MP_PLAYER_CB_TYPE_PROGRESSIVE_DOWNLOAD_MESSAGE,
	MP_PLAYER_CB_TYPE_NUM,
} mp_player_cb_type;

typedef struct {
	/* player callbacks */
	mp_player_started_cb started_cb;
	player_completed_cb completed_cb;
	player_interrupted_cb interrupted_cb;
	player_error_cb error_cb;
	player_buffering_cb buffering_cb;
	player_prepared_cb prepare_cb;
	mp_player_paused_cb paused_cb;
	mp_player_duration_changed_cb duration_changed_cb;
	player_pd_message_cb pd_message_cb;

	/* callback user data */
	void *user_data[MP_PLAYER_CB_TYPE_NUM];
} mp_player_cbs;

typedef struct {
	mp_player_cb_type cb_type;

	union {
		player_interrupted_code_e interrupted_code;
		int error_code;
		int percent;
		player_pd_message_type_e pd_message_type;
	} param;
} mp_player_cb_extra_data;

typedef struct {
	int (*create)(player_h *);
	int (*destroy)(player_h);
	int (*prepare)(player_h);
	int (*prepare_async)(player_h, player_prepared_cb, void *);
	int (*unprepare)(player_h);
	int (*set_uri)(player_h, const char *);
	int (*get_state)(player_h, player_state_e *);
	int (*set_sound_type)(player_h, sound_stream_info_h);
	int (*set_audio_latency_mode)(player_h, audio_latency_mode_e);
	int (*get_audio_latency_mode)(player_h, audio_latency_mode_e *);
	int (*start)(player_h);
	int (*pause)(player_h);
	int (*stop)(player_h);
	int (*set_started_cb)(player_h, mp_player_started_cb, void *);
	int (*set_completed_cb)(player_h, player_completed_cb, void *);
	int (*set_interrupted_cb)(player_h, player_interrupted_cb, void *);
	int (*set_error_cb)(player_h, player_error_cb, void *);
	int (*set_buffering_cb)(player_h, player_buffering_cb, void *);
	int (*set_paused_cb)(player_h, mp_player_paused_cb, void *);
	int (*set_position)(player_h, int, bool, player_seek_completed_cb, void *);
	int (*set_play_rate)(player_h, float);
	int (*get_position)(player_h, int *);
	int (*get_duration)(player_h, int *);
	int (*set_mute)(player_h, bool);
	int (*get_content_info)(player_h, player_content_info_e , char * *);
} mp_player_api_s;
static mp_player_api_s g_player_apis;
#define CHECK_MMFW_PLAYER()	((_player_type == MP_PLAYER_TYPE_MMFW) ? true : false)

static mp_player_cbs *g_player_cbs = NULL;

player_h mp_player_mgr_get_player(void)
{
	return _player;
}

bool
mp_player_mgr_is_active(void)
{
	return _player ? TRUE : FALSE;
}

void mp_player_mgr_set_started_db(mp_player_started_cb callback, void *user_data)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->started_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_STARTED] = user_data;
}

void mp_player_mgr_set_completed_cb(player_completed_cb  callback, void *user_data)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->completed_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_COMPLETED] = user_data;
}

void mp_player_mgr_set_interrupted_cb(player_interrupted_cb  callback, void *user_data)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->interrupted_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_INTURRUPTED] = user_data;
}

void mp_player_mgr_set_error_cb(player_error_cb  callback, void *user_data)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->error_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_ERROR] = user_data;
}

void mp_player_mgr_set_buffering_cb(player_buffering_cb  callback, void *user_data)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->buffering_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_BUFFERING] = user_data;
}

void mp_player_mgr_set_prepare_cb(player_prepared_cb callback, void *user_data)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->prepare_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_PREPARE] = user_data;
}

void mp_player_mgr_unset_completed_cb(void)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->completed_cb = NULL;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_COMPLETED] = NULL;
}

void mp_player_mgr_unset_interrupted_cb(void)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->interrupted_cb = NULL;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_INTURRUPTED] = NULL;
}

void mp_player_mgr_unset_error_cb(void)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->error_cb = NULL;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_ERROR] = NULL;
}

void mp_player_mgr_unset_buffering_cb(void)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->buffering_cb = NULL;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_BUFFERING] = NULL;
}

void mp_player_mgr_set_paused_cb(mp_player_paused_cb callback, void *user_data)
{
	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->paused_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_PAUSED] = user_data;
}

static Eina_Bool
_mp_player_mgr_duration_timer_cb(void *data)
{
	TIMER_TRACE();
	mp_player_cbs *cb_info = data;
	MP_CHECK_VAL(cb_info, ECORE_CALLBACK_CANCEL);

	if (cb_info->duration_changed_cb) {
		cb_info->duration_changed_cb(cb_info->user_data[MP_PLAYER_CB_TYPE_DURATION_CHANGED]);
	}

	return ECORE_CALLBACK_RENEW;
}
void mp_player_mgr_set_duration_changed_cb(mp_player_duration_changed_cb callback, void *user_data)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (!mp_player_mgr_is_active()) {
		return;
	}

	MP_CHECK(g_player_cbs);

	g_player_cbs->duration_changed_cb = callback;
	g_player_cbs->user_data[MP_PLAYER_CB_TYPE_DURATION_CHANGED] = user_data;

	mp_ecore_timer_del(ad->duration_change_timer);
	ad->duration_change_timer = ecore_timer_add(5.0, _mp_player_mgr_duration_timer_cb, g_player_cbs);

	if (mp_player_mgr_get_state() == PLAYER_STATE_PLAYING && !ad->is_lcd_off) {
		MP_TIMER_THAW(ad->duration_change_timer);
	} else {
		MP_TIMER_FREEZE(ad->duration_change_timer);
	}
}

player_state_e
mp_player_mgr_get_state(void)
{
	player_state_e state_now = PLAYER_STATE_NONE;

	if (!_player) {
		return state_now;
	}

	g_player_apis.get_state(_player, &state_now);
	return state_now;
}

mp_player_type_e
mp_player_mgr_get_player_type(void)
{
	mp_debug("player type = %d", _player_type);
	return _player_type;
}

static void
_mp_player_mgr_callback_pipe_handler(void *data, void *buffer, unsigned int nbyte)
{
	mp_player_cb_extra_data *extra_data = buffer;
	MP_CHECK(extra_data);
	MP_CHECK(g_player_cbs);

	switch (extra_data->cb_type) {
	case MP_PLAYER_CB_TYPE_STARTED:
		if (g_player_cbs->started_cb) {
			g_player_cbs->started_cb(g_player_cbs->user_data[MP_PLAYER_CB_TYPE_STARTED]);
		}
		break;

	case MP_PLAYER_CB_TYPE_COMPLETED:
		if (g_player_cbs->completed_cb) {
			g_player_cbs->completed_cb(g_player_cbs->user_data[MP_PLAYER_CB_TYPE_COMPLETED]);
		}
		break;

	case MP_PLAYER_CB_TYPE_INTURRUPTED:
		if (g_player_cbs->interrupted_cb) {
			g_player_cbs->interrupted_cb(extra_data->param.interrupted_code, g_player_cbs->user_data[MP_PLAYER_CB_TYPE_INTURRUPTED]);
		}
		break;

	case MP_PLAYER_CB_TYPE_ERROR:
		if (g_player_cbs->error_cb) {
			g_player_cbs->error_cb(extra_data->param.error_code, g_player_cbs->user_data[MP_PLAYER_CB_TYPE_ERROR]);
		}
		break;

	case MP_PLAYER_CB_TYPE_BUFFERING:
		if (g_player_cbs->buffering_cb) {
			g_player_cbs->buffering_cb(extra_data->param.percent, g_player_cbs->user_data[MP_PLAYER_CB_TYPE_BUFFERING]);
		}
		break;

	case MP_PLAYER_CB_TYPE_PREPARE:
		if (g_player_cbs->prepare_cb) {
			g_player_cbs->prepare_cb(g_player_cbs->user_data[MP_PLAYER_CB_TYPE_PREPARE]);
		}
		break;

	case MP_PLAYER_CB_TYPE_PAUSED:
		if (g_player_cbs->paused_cb) {
			g_player_cbs->paused_cb(g_player_cbs->user_data[MP_PLAYER_CB_TYPE_PAUSED]);
		}
		break;

	case MP_PLAYER_CB_TYPE_PROGRESSIVE_DOWNLOAD_MESSAGE:
		if (g_player_cbs->pd_message_cb) {
			g_player_cbs->pd_message_cb(extra_data->param.pd_message_type, g_player_cbs->user_data[MP_PLAYER_CB_TYPE_PROGRESSIVE_DOWNLOAD_MESSAGE]);
		}
		break;

	default:
		WARN_TRACE("Not suppoted callback type [%d]", extra_data->cb_type);
	}
}

static void
_mp_player_mgr_started_cb(void *userdata)
{
	MP_CHECK(g_player_pipe);

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_STARTED;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}

static void
_mp_player_mgr_completed_cb(void *userdata)
{
	MP_CHECK(g_player_pipe);

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_COMPLETED;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}

static void
_mp_player_mgr_interrupted_cb(player_interrupted_code_e code, void *userdata)
{
	startfunc;
	MP_CHECK(g_player_pipe);

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_INTURRUPTED;
	extra_data.param.interrupted_code = code;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}


static void
_mp_player_mgr_error_cb(int error_code, void *userdata)
{
	MP_CHECK(g_player_pipe);

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_ERROR;
	extra_data.param.error_code = error_code;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}

static void
_mp_player_mgr_buffering_cb(int percent, void *userdata)
{
	MP_CHECK(g_player_pipe);

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_BUFFERING;
	extra_data.param.percent = percent;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}

static void
_mp_player_mgr_prepare_cb(void *userdata)
{
	MP_CHECK(g_player_pipe);
	struct appdata *ad = (struct appdata *)userdata;
	MP_CHECK(ad);
	ad->player_state = PLAY_STATE_READY;

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_PREPARE;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}

static void
_mp_player_mgr_paused_cb(void *userdata)
{
	MP_CHECK(g_player_pipe);

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_PAUSED;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}

static void
_mp_player_mgr_change_player(mp_player_type_e player_type)
{
	_player_type = player_type;

	WARN_TRACE("player type = [%d]", _player_type);

	memset(&g_player_apis, 0x0, sizeof(mp_player_api_s));

	{	/* MP_PLAYER_TYPE_MMFW */
		g_player_apis.create = player_create;
		g_player_apis.destroy = player_destroy;
		g_player_apis.prepare = player_prepare;
		g_player_apis.prepare_async = player_prepare_async;
		g_player_apis.unprepare = player_unprepare;
		g_player_apis.set_uri = player_set_uri;
		g_player_apis.get_state = player_get_state;
		g_player_apis.set_sound_type = player_set_audio_policy_info;
		g_player_apis.set_audio_latency_mode = player_set_audio_latency_mode;
		g_player_apis.get_audio_latency_mode = player_get_audio_latency_mode;
		g_player_apis.start = player_start;
		g_player_apis.pause = player_pause;
		g_player_apis.stop = player_stop;
		g_player_apis.set_started_cb = NULL;
		g_player_apis.set_completed_cb = player_set_completed_cb;
		g_player_apis.set_interrupted_cb = player_set_interrupted_cb;
		g_player_apis.set_error_cb = player_set_error_cb;
		g_player_apis.set_buffering_cb = player_set_buffering_cb;
		g_player_apis.set_paused_cb = NULL;
		g_player_apis.set_position = player_set_play_position;
		g_player_apis.get_position = player_get_play_position;
		g_player_apis.get_duration = player_get_duration;
		g_player_apis.set_mute = player_set_mute;
		g_player_apis.set_play_rate = player_set_playback_rate;
		g_player_apis.get_content_info = player_get_content_info;
	}
}

int
_mp_player_mgr_create_common(struct appdata *ad, mp_player_type_e type)
{
	MP_CHECK_VAL(ad, -1);
	int ret = PLAYER_ERROR_NONE;

	if (mp_player_mgr_is_active()) {
		WARN_TRACE("Destroy previous player");
		mp_player_mgr_destroy(ad);
	}

	IF_FREE(_player_buffer);

	/* change player for playing in DMR */
	_mp_player_mgr_change_player(type);

	PLAYER_ENTER_LOG("create");
	ret = g_player_apis.create(&_player);
	PLAYER_LEAVE_LOG("create");
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_create");
		return ret;
	}

	if (_player_type == MP_PLAYER_TYPE_MMFW) {

#ifdef MP_SOUND_PLAYER
		if (ad->cookie) {
			player_set_streaming_cookie(_player, ad->cookie, strlen(ad->cookie));
		}
#endif
	}

	if (g_player_apis.set_sound_type) {
		PLAYER_ENTER_LOG("set_sound_type");
		g_player_apis.set_sound_type(_player, ad->stream_info);
		PLAYER_LEAVE_LOG("set_sound_type");
	}
	if (g_player_apis.set_audio_latency_mode) {
		g_player_apis.set_audio_latency_mode(_player, AUDIO_LATENCY_MODE_HIGH);
	}

	is_seeking = false;
	resume_on_seek_done = false;
	g_reserved_seek_pos = -1;

	if (!g_player_cbs) {
		g_player_cbs = calloc(1, sizeof(mp_player_cbs));
		mp_assert(g_player_cbs);
	}

	if (g_player_apis.set_started_cb) {
		g_player_apis.set_started_cb(_player, _mp_player_mgr_started_cb, NULL);
	}
	if (g_player_apis.set_completed_cb) {
		g_player_apis.set_completed_cb(_player, _mp_player_mgr_completed_cb, NULL);
	}
	if (g_player_apis.set_interrupted_cb) {
		g_player_apis.set_interrupted_cb(_player, _mp_player_mgr_interrupted_cb, NULL);
	}
	if (g_player_apis.set_error_cb) {
		g_player_apis.set_error_cb(_player, _mp_player_mgr_error_cb, NULL);
	}
	if (g_player_apis.set_buffering_cb) {
		g_player_apis.set_buffering_cb(_player, _mp_player_mgr_buffering_cb, NULL);
	}
	if (g_player_apis.set_paused_cb) {
		g_player_apis.set_paused_cb(_player, _mp_player_mgr_paused_cb, NULL);
	}

	if (!g_player_pipe) {
		g_player_pipe = ecore_pipe_add(_mp_player_mgr_callback_pipe_handler, ad);
	}

	ad->player_state = PLAY_STATE_CREATED;


	return ret;
}

int
mp_player_mgr_prepare(void *data)
{
	mp_plst_item *item = NULL;
	struct appdata *ad = data;
	item = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_FALSE(item);
	char *uri = NULL;
	uri = item->uri;
	void *extra_data = NULL;
	int error;
	error = mp_player_mgr_create(ad, uri, MP_PLAYER_TYPE_MMFW, extra_data);
	mp_player_mgr_realize(ad);
	return error;
}

int
mp_player_mgr_create(void *data, const char * path, mp_player_type_e type, void *extra_data)
{
	struct appdata *ad = (struct appdata *)data;
	MP_CHECK_VAL(ad, -1);
	MP_CHECK_VAL(path, -1);

	DEBUG_TRACE("path: %s", path);

	int path_len = strlen(path);
	if (path_len <= 0 || path_len > MAX_PATH_LEN) {
		mp_error("invlaid path");
		return -1;
	}

	int ret = _mp_player_mgr_create_common(ad, type);
	MP_CHECK_VAL(ret == 0, -1);

	ret = g_player_apis.set_uri(_player, path);
	if (ret != PLAYER_ERROR_NONE) {
		mp_error("player_set_uri() .. [0x%x]", ret);
		goto exception;
	}

	return 0;

exception:
	mp_player_mgr_destroy(ad);
	return ret;
}

int
mp_player_mgr_create_with_buffer(void *data, void *buffer, int size)
{
	struct appdata *ad = data;
	MP_CHECK_VAL(ad, -1);

	int ret = _mp_player_mgr_create_common(ad, MP_PLAYER_TYPE_MMFW);
	MP_CHECK_VAL(ret == 0, -1);

	IF_FREE(_player_buffer);
	_player_buffer = buffer;
	ret = player_set_memory_buffer(_player, _player_buffer, size);
	if (ret != PLAYER_ERROR_NONE) {
		mp_error("player_set_memory_buffer() .. [0x%x]", ret);
		mp_player_mgr_destroy(ad);
		return ret;
	}

	return 0;
}

bool
mp_player_mgr_destroy(void *data)
{
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);
	int res = true;
	if (ad->current_track_info) {
		if (mp_setting_read_playing_status(ad->current_track_info->uri, "paused") != 1) {
			mp_setting_write_playing_status(ad->current_track_info->uri, "paused");
		}
	}

	if (!mp_player_mgr_is_active()) {
		return FALSE;
	}

	mp_ecore_timer_del(ad->duration_change_timer);

	PLAYER_ENTER_LOG("destroy");
	res = g_player_apis.destroy(_player);
	PLAYER_LEAVE_LOG("destroy");
	if (res != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_destroy");
		res = false;
	}

	IF_FREE(_player_buffer);
	_player = 0;
	ad->player_state = PLAY_STATE_PAUSED;
	if (!ad->freeze_indicator_icon && !mp_util_is_other_player_playing()) {
		preference_set_int(PREF_MUSIC_STATE, PREF_MUSIC_OFF);
		mp_setting_set_player_state(MP_PLAY_STATE_NONE);
	}

	is_seeking = false;
	g_reserved_seek_pos = -1;
	g_reserved_cb = NULL;
	g_reserved_cb_data = NULL;
	g_requesting_cb = NULL;
	g_requesting_cb_data = NULL;

	WARN_TRACE("player handle is destroyed..");
	mp_util_release_cpu();
	return res;
}

int
mp_player_mgr_realize(void *data)
{
	struct appdata *ad = data;
	int error = PLAYER_ERROR_NONE;

	if (!mp_player_mgr_is_active()) {
		return -1;
	}
	PLAYER_ENTER_LOG("prepare_async");
	error = g_player_apis.prepare_async(_player, _mp_player_mgr_prepare_cb, ad);
	PLAYER_LEAVE_LOG("prepare_async");
	if (error != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_realize .. [0x%x]", error);
		return error;
	}

	ad->player_state = PLAY_STATE_PREPARING;
	return error;
}

bool
mp_player_mgr_unrealize(void *data)
{
	if (!mp_player_mgr_is_active()) {
		return FALSE;
	}
	PLAYER_ENTER_LOG("unprepare");
	int res = g_player_apis.unprepare(_player);
	PLAYER_LEAVE_LOG("unprepare");
	if (res != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_unrealize");
		return FALSE;
	}
	return TRUE;
}

int
mp_player_mgr_play(void *data)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);
	int err = -1;
	int error = SOUND_MANAGER_ERROR_NONE;

	MP_CHECK_VAL(mp_player_mgr_is_active(), -1);

	mp_util_lock_cpu();

	if (ad->start_pos > 0) {
		WARN_TRACE("start position = %d", ad->start_pos);
		mp_player_mgr_set_position(ad->start_pos, NULL, NULL);
		ad->start_pos = 0;
	}

	if (is_seeking) {
		resume_on_seek_done = true;
		/* mp_allshare_player_destroy_loading_popup(); */
		return PLAYER_ERROR_INVALID_OPERATION;
	}

	PLAYER_ENTER_LOG("start");
	if (ad->stream_info) {
		error = sound_manager_acquire_focus(ad->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (error != SOUND_MANAGER_ERROR_NONE) {
			ERROR_TRACE("failed to acquire focus [%x]", error);
		}

		sound_manager_get_focus_reacquisition(ad->stream_info, &reacquire_state);
		if (reacquire_state == EINA_FALSE) {
			sound_manager_set_focus_reacquisition(ad->stream_info, EINA_TRUE);
		}
	}
	err = g_player_apis.start(_player);
	PLAYER_LEAVE_LOG("start");

	if (err != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_play. err[%x]", err);
		/* mp_allshare_player_destroy_loading_popup(); */
		return err;
	}

	is_seeking = false;
	resume_on_seek_done = false;
	pause_on_seek_done = false;
	g_reserved_seek_pos = -1;

	if (!g_player_apis.set_started_cb && g_player_cbs->started_cb) {	/* sync */
		g_player_cbs->started_cb(g_player_cbs->user_data[MP_PLAYER_CB_TYPE_STARTED]);
	}

	if (!ad->is_lcd_off) {
		MP_TIMER_THAW(ad->duration_change_timer);
	}

	return err;
}


bool
mp_player_mgr_stop(void *data)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);

	if (!mp_player_mgr_is_active()) {
		return FALSE;
	}
	PLAYER_ENTER_LOG("stop");
	int res = g_player_apis.stop(_player);
	PLAYER_LEAVE_LOG("stop");

	if (res != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_stop");
	}

	is_seeking = false;
	g_reserved_seek_pos = -1;
	resume_on_seek_done = false;
	pause_on_seek_done = false;

	if (ad->duration_change_timer) {
		MP_TIMER_FREEZE(ad->duration_change_timer);
	}

	mp_play_stop(ad);
	return TRUE;
}

int
mp_player_mgr_resume(void *data)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);
	int err = -1,error = -1;
	sound_stream_focus_state_e state_for_playback;
	sound_stream_focus_state_e state_for_recording;
	int ret = -1;

	mp_util_lock_cpu();
	if (!mp_player_mgr_is_active()) {
		DEBUG_TRACE("player is not active");
		err = mp_player_mgr_prepare(ad);
		if (err) {
			DEBUG_TRACE("failed to prepare _player");
			return err;
		}
	}

	if (ad->stream_info) {
		ret = sound_manager_get_focus_state(ad->stream_info, &state_for_playback, &state_for_recording);
		if(ret != SOUND_MANAGER_ERROR_NONE) {
			ERROR_TRACE("failed in sound_manager_get_focus_state");
		}
		if (state_for_playback != SOUND_STREAM_FOCUS_STATE_ACQUIRED) {
			error = sound_manager_acquire_focus(ad->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
			if (error != SOUND_MANAGER_ERROR_NONE) {
				ERROR_TRACE("failed to acquire focus [%x]", error);
				return error;
			}
		}

		sound_manager_get_focus_reacquisition(ad->stream_info, &reacquire_state);
		if (reacquire_state == EINA_FALSE) {
			sound_manager_set_focus_reacquisition(ad->stream_info, EINA_TRUE);
		}
	}

	if (mp_player_mgr_get_state() != PLAYER_STATE_IDLE) {
		DEBUG_TRACE("player state is ready");
		PLAYER_ENTER_LOG("start");
		err = g_player_apis.start(_player);
		PLAYER_LEAVE_LOG("start");

		if (err != PLAYER_ERROR_NONE) {
			ERROR_TRACE("Error when mp_player_mgr_resume. err[%x]", err);
			return err;
		}
	}

	is_seeking = false;
	g_reserved_seek_pos = -1;

	if (!ad->is_lcd_off) {
		MP_TIMER_THAW(ad->duration_change_timer);
	}

	mp_player_view_update_progressbar(GET_PLAYER_VIEW);
	mp_player_view_progress_timer_thaw(GET_PLAYER_VIEW);

	return err;
}

bool
mp_player_mgr_pause(void *data)
{
	startfunc;
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);
	int err = -1;
	int error = SOUND_MANAGER_ERROR_NONE;

	mp_util_release_cpu();

	if (!mp_player_mgr_is_active()) {
		return FALSE;
	}

	PLAYER_ENTER_LOG("pause");
	err = g_player_apis.pause(_player);
	if (ad->stream_info) {
		error = sound_manager_release_focus(ad->stream_info, SOUND_STREAM_FOCUS_FOR_PLAYBACK, NULL);
		if (error != SOUND_MANAGER_ERROR_NONE) {
			ERROR_TRACE("failed to release focus error[%x]", error);
		}
	}
	PLAYER_LEAVE_LOG("pause");

	if (err != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_pause. err[%x]", err);
		if (is_seeking) {
			pause_on_seek_done = true;
		}
		if (ad->win_minicon) {
			mp_minicontroller_update_control(ad);
		}
#ifdef MP_FEATURE_LOCKSCREEN
		if (ad->win_lockmini) {
			mp_lockscreenmini_update_control(ad);
		}
#endif
		return FALSE;
	}

	MP_TIMER_FREEZE(ad->duration_change_timer);

	if (!g_player_apis.set_paused_cb && g_player_cbs->paused_cb) {
		g_player_cbs->paused_cb(g_player_cbs->user_data[MP_PLAYER_CB_TYPE_PAUSED]);
	}

	return TRUE;
}

Eina_Bool
mp_player_mgr_seek_done(void *data)
{
	if (!is_seeking) {
		return ECORE_CALLBACK_DONE;
	}

	is_seeking = false;

#ifdef MP_FEATURE_AVRCP_13
	mp_avrcp_noti_track_position((unsigned int)data);
#endif

	if (g_requesting_cb) {
		/* invoke seek done callback */
		g_requesting_cb(g_requesting_cb_data);

		g_requesting_cb = NULL;
		g_requesting_cb_data = NULL;
	}

	if (g_reserved_seek_pos >= 0) {
		mp_player_mgr_set_position(g_reserved_seek_pos, g_reserved_cb, g_reserved_cb_data);
		g_reserved_seek_pos = -1;
		g_reserved_cb = NULL;
		g_reserved_cb_data = NULL;
	}

	if (resume_on_seek_done) {
		mp_player_mgr_play(mp_util_get_appdata());
		resume_on_seek_done = false;
	} else if (pause_on_seek_done) {
		mp_player_mgr_pause(mp_util_get_appdata());
		pause_on_seek_done = false;
	}

	return ECORE_CALLBACK_DONE;
}

static void
_mp_player_mgr_seek_done_cb(void *data)
{
	if (is_seeking) {
		mp_player_mgr_seek_done(data);
	}
}

bool
mp_player_mgr_is_seeking(void)
{
	return is_seeking;
}

bool
mp_player_mgr_set_position(unsigned int pos, Seek_Done_Cb done_cb, void *data)
{
	if (!mp_player_mgr_is_active()) {
		return FALSE;
	}

	if (is_seeking) {
		g_reserved_seek_pos = pos;
		g_reserved_cb = done_cb;
		g_reserved_cb_data = data;
		return TRUE;
	}

	PLAYER_ENTER_LOG("set_position");
	int err = g_player_apis.set_position(_player, (int)pos, TRUE, _mp_player_mgr_seek_done_cb, (void *)pos);
	PLAYER_LEAVE_LOG("set_position");
	if (err != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error [0x%x] when mp_player_mgr_set_position(%d)", err, pos);
		return FALSE;
	}

	is_seeking = true;
	g_requesting_cb = done_cb;
	g_requesting_cb_data = data;

	return TRUE;
}

void
mp_player_mgr_unset_seek_done_cb()
{
	g_requesting_cb = NULL;
	g_requesting_cb_data = NULL;
	g_reserved_cb = NULL;
	g_reserved_cb_data = NULL;
}

bool
mp_player_mgr_set_play_speed(double rate)
{
	int err = PLAYER_ERROR_NONE;
	if (!mp_player_mgr_is_active()) {
		return FALSE;
	}

	if (g_player_apis.set_play_rate) {
		err = g_player_apis.set_play_rate(_player, rate);
	} else {
		WARN_TRACE("Unsupported function");
	}

	if (err != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error [0x%x] when set_playback_rate(%f)", err, rate);
		return FALSE;
	}
	return TRUE;
}

int
mp_player_mgr_get_position(void)
{
	int pos = 0;

	if (!mp_player_mgr_is_active()) {
		return 0;
	}

	if (g_player_apis.get_position(_player, &pos) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_get_position");
		return 0;
	}

	return pos;
}

int
mp_player_mgr_get_duration(void)
{
	if (!mp_player_mgr_is_active()) {
		return 0;
	}

	int duration = 0;

	if (g_player_apis.get_duration(_player, &duration) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Error when mp_player_mgr_get_position");
		return 0;
	}

	return duration;
}

int
mp_player_mgr_vol_type_set(void)
{
	return sound_manager_set_current_sound_type(SOUND_TYPE_MEDIA);
}

int
mp_player_mgr_vol_type_unset(void)
{
	return sound_manager_unset_current_sound_type();
}

int
mp_player_mgr_safety_volume_set(int foreground)
{
	int set = false;
	player_state_e state = mp_player_mgr_get_state();

	if (foreground) {
		set = true;
	} else {
		if (state == PLAYER_STATE_PLAYING) {
			set = true;
		} else {
			set = false;
		}
	}
	EVENT_TRACE("Foreground[%d], PlayerState[%d], set[%d]", foreground, state, set);

	return 0;
}

void mp_player_focus_callback(sound_stream_info_h stream_info, sound_stream_focus_change_reason_e reason_for_change,
					const char *additional_info, void *user_data)
{
	struct appdata *ad = user_data;

	sound_stream_focus_state_e state_for_playback;
	sound_stream_focus_state_e state_for_recording;
	int ret = -1;
	ret = sound_manager_get_focus_state(ad->stream_info, &state_for_playback,
			&state_for_recording);
	if(ret != SOUND_MANAGER_ERROR_NONE) {
		ERROR_TRACE("failed in sound_manager_get_focus_state");
	}
	if (state_for_playback == SOUND_STREAM_FOCUS_STATE_RELEASED) {
		mp_player_mgr_pause(ad);

		if (reason_for_change != SOUND_STREAM_FOCUS_CHANGED_BY_ALARM &&
				reason_for_change != SOUND_STREAM_FOCUS_CHANGED_BY_NOTIFICATION) {
			sound_manager_get_focus_reacquisition(ad->stream_info, &reacquire_state);
			if (!strcmp(additional_info, "cam_capture")) {
				sound_manager_set_focus_reacquisition(ad->stream_info, EINA_TRUE);
			} else if (reacquire_state == EINA_TRUE) {
				sound_manager_set_focus_reacquisition(ad->stream_info, EINA_FALSE);
			}
		}
	} else {
		mp_play_control_play_pause(ad, true);
	}
}


bool
mp_player_mgr_session_init(void)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);
	int ret = SOUND_MANAGER_ERROR_NONE;
	if (!ad->stream_info) {
		ret = sound_manager_create_stream_information(SOUND_STREAM_TYPE_MEDIA, mp_player_focus_callback, ad, &ad->stream_info);

		if (ret != SOUND_MANAGER_ERROR_NONE) {
			EVENT_TRACE("failed to create_stream_information %x", ret);
			return FALSE;
		}
	}
	return TRUE;
}

bool
mp_player_mgr_session_finish(void)
{
	return TRUE;
}

void
mp_player_mgr_set_mute(bool bMuteEnable)
{

	if (!mp_player_mgr_is_active()) {
		return;
	}

	if (g_player_apis.set_mute(_player, bMuteEnable) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("[ERR] mm_player_set_mute");
	}
}

int
mp_player_mgr_volume_get_max()
{
	int max_vol = 0;

	static int max = -1;
	if (max < 0) {
		PLAYER_ENTER_LOG("sound_manager_get_max_volume");
		int ret = sound_manager_get_max_volume(SOUND_TYPE_MEDIA, &max_vol);
		PLAYER_LEAVE_LOG("sound_manager_get_max_volume");
		if (ret != SOUND_MANAGER_ERROR_NONE) {
			mp_error("sound_manager_get_max_volume().. [0x%x]", ret);
			return -1;
		}
		max = max_vol;
	} else {
		max_vol = max;
	}

	return max_vol;
}

int
mp_player_mgr_volume_get_current()
{
	int current = 0;
	int ret = 0;


	PLAYER_ENTER_LOG("sound_manager_get_volume");
	ret = sound_manager_get_volume(SOUND_TYPE_MEDIA, &current);
	PLAYER_LEAVE_LOG("sound_manager_get_volume");
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		mp_error("sound_manager_get_max_volume().. [0x%x]", ret);
		return -1;
	}

	return current;
}

bool
mp_player_mgr_volume_set(int volume)
{
	int ret = 0;

	PLAYER_ENTER_LOG("sound_manager_set_volume_with_safety");
	ret =  sound_manager_set_volume(SOUND_TYPE_MEDIA, volume);
	PLAYER_LEAVE_LOG("sound_manager_set_volume_with_safety");
	if (ret != SOUND_MANAGER_ERROR_NONE) {
		mp_error("sound_manager_set_volume_with_safety().. [0x%x]", ret);
		mp_volume_key_event_timer_del();
		return false;
	}

	WARN_TRACE("set volue [%d]", volume);
	return true;
}

bool
mp_player_mgr_volume_up()
{
	int current = mp_player_mgr_volume_get_current();
	int max = mp_player_mgr_volume_get_max();

	int step = 1;

	int new_vol = current + step;
	if (new_vol > max) {
		new_vol = max;
	}

	bool ret = true;
	if (current < max) {
		ret = mp_player_mgr_volume_set(new_vol);
		_mp_volume_handle_change(new_vol);
	}

	return ret;
}

bool
mp_player_mgr_volume_down()
{
	int current = mp_player_mgr_volume_get_current();

	int step = 1;

	int new_vol = current - step;
	if (new_vol < 0) {
		new_vol = 0;
	}

	bool ret = true;
	if (current > 0) {
		ret = mp_player_mgr_volume_set(new_vol);
		_mp_volume_handle_change(new_vol);
	}

	return ret;
}

bool
mp_player_mgr_get_content_info(char **title, char **album, char **artist, char **author, char **genre, char **year)
{
	if (!mp_player_mgr_is_active()) {
		return false;
	}

	MP_CHECK_FALSE(g_player_apis.get_content_info);

	if (album && g_player_apis.get_content_info(_player, PLAYER_CONTENT_INFO_ALBUM, album) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("[ERR] mm_player_get_content_info");
	}
	if (artist && g_player_apis.get_content_info(_player, PLAYER_CONTENT_INFO_ARTIST, artist) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("[ERR] mm_player_get_content_info");
	}
	if (author && g_player_apis.get_content_info(_player, PLAYER_CONTENT_INFO_AUTHOR, author) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("[ERR] mm_player_get_content_info");
	}
	if (genre && g_player_apis.get_content_info(_player, PLAYER_CONTENT_INFO_GENRE, genre) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("[ERR] mm_player_get_content_info");
	}
	if (title && g_player_apis.get_content_info(_player, PLAYER_CONTENT_INFO_TITLE, title) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("[ERR] mm_player_get_content_info");
	}
	if (year && g_player_apis.get_content_info(_player, PLAYER_CONTENT_INFO_YEAR, year) != PLAYER_ERROR_NONE) {
		ERROR_TRACE("[ERR] mm_player_get_content_info");
	}
	return true;
}

static void
_mp_player_mgr_pd_message_cb(player_pd_message_type_e type, void *user_data)
{
	MP_CHECK(g_player_pipe);

	mp_player_cb_extra_data extra_data;
	memset(&extra_data, 0, sizeof(mp_player_cb_extra_data));
	extra_data.cb_type = MP_PLAYER_CB_TYPE_PROGRESSIVE_DOWNLOAD_MESSAGE;
	extra_data.param.pd_message_type = type;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mp_player_cb_extra_data));
}

int
mp_player_mgr_set_progressive_download(const char *path, player_pd_message_cb callback, void *user_data)
{
	startfunc;

	if (!mp_player_mgr_is_active()) {
		return -1;
	}

	int err = player_set_progressive_download_path(_player, path);
	if (err != PLAYER_ERROR_NONE) {
		mp_error("player_set_progressive_download_path() .. [0x%x]", err);
		return -1;
	}

	if (callback) {
		MP_CHECK_VAL(g_player_cbs, -1);
		err = player_set_progressive_download_message_cb(_player, _mp_player_mgr_pd_message_cb, NULL);
		if (err != PLAYER_ERROR_NONE) {
			mp_error("player_set_progressive_download_path() .. [0x%x]", err);
		}

		g_player_cbs->pd_message_cb = callback;
		g_player_cbs->user_data[MP_PLAYER_CB_TYPE_PROGRESSIVE_DOWNLOAD_MESSAGE] = user_data;
	}

	return err;
}
