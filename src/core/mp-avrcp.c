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

#include "mp-avrcp.h"
#include "mp-player-debug.h"
#include <bluetooth.h>
#include "mp-define.h"
#include "mp-player-mgr.h"

#ifdef MP_FEATURE_AVRCP_13

typedef struct {
	mp_avrcp_connection_state_changed_cb connection_cb;
	mp_avrcp_shuffle_changed_cb s_cb;
	mp_avrcp_repeat_changed_cb r_cb;
	mp_avrcp_eq_changed_cb e_cb;
	void *user_data;
} MpAvrcpCb_t;

static bool gMpAvrcpInitialized;
static MpAvrcpCb_t *gMpAvrcpCb;

static void _mp_avrcp_equalizer_state_changed_cb (bt_avrcp_equalizer_state_e equalizer, void *user_data)
{
	eventfunc;
	MP_CHECK(gMpAvrcpCb);
	mp_avrcp_eq_state_e mode ;
	if (equalizer == BT_AVRCP_EQUALIZER_STATE_OFF) {
		mode =  MP_AVRCP_EQ_OFF;
	} else {
		mode =  MP_AVRCP_EQ_ON;
	}
	if (gMpAvrcpCb->e_cb)
		gMpAvrcpCb->e_cb(mode, gMpAvrcpCb->user_data);
}

static void _mp_avrcp_shuffle_mode_changed_cb (bt_avrcp_shuffle_mode_e shuffle, void *user_data)
{
	eventfunc;
	MP_CHECK(gMpAvrcpCb);
	mp_avrcp_shuffle_mode_e mode ;
	if (shuffle == BT_AVRCP_SHUFFLE_MODE_OFF) {
		mode =  MP_AVRCP_SHUFFLE_OFF;
	} else {
		mode =  MP_AVRCP_SHUFFLE_ON;
	}

	if (gMpAvrcpCb->s_cb)
		gMpAvrcpCb->s_cb(mode, gMpAvrcpCb->user_data);
}

static void _mp_avrcp_repeat_mode_changed_cb (bt_avrcp_repeat_mode_e repeat, void *user_data)
{
	eventfunc;
	MP_CHECK(gMpAvrcpCb);
	mp_avrcp_repeat_mode_e mode ;
	if (repeat == BT_AVRCP_REPEAT_MODE_OFF) {
		mode =  MP_AVRCP_REPEAT_OFF;
	} else if (repeat == BT_AVRCP_REPEAT_MODE_SINGLE_TRACK) {
		mode =  MP_AVRCP_REPEAT_ONE;
	} else {
		mode =  MP_AVRCP_REPEAT_ALL;
	}

	if (gMpAvrcpCb->r_cb)
		gMpAvrcpCb->r_cb(mode, gMpAvrcpCb->user_data);
}

void _mp_avrcp_connection_state_changed_cb(bool connected, const char *remote_address, void *user_data)
{
	eventfunc;
	MP_CHECK(gMpAvrcpCb);
	if (gMpAvrcpCb->connection_cb)
		gMpAvrcpCb->connection_cb(connected, remote_address, gMpAvrcpCb->user_data);
}

MpAvrcpErr_e mp_avrcp_target_initialize(void)
{
	startfunc;
	int res = BT_ERROR_NONE;

	if (gMpAvrcpInitialized)
		return 0;

	res = bt_initialize();
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	MP_CHECK_VAL(res == BT_ERROR_NONE, res);

	res = bt_avrcp_target_initialize(_mp_avrcp_connection_state_changed_cb, NULL);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	MP_CHECK_VAL(res == BT_ERROR_NONE, res);

	res = bt_avrcp_set_equalizer_state_changed_cb(_mp_avrcp_equalizer_state_changed_cb, NULL);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	res = bt_avrcp_set_shuffle_mode_changed_cb(_mp_avrcp_shuffle_mode_changed_cb, NULL);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	res = bt_avrcp_set_repeat_mode_changed_cb(_mp_avrcp_repeat_mode_changed_cb, NULL);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);

	gMpAvrcpInitialized = true;

	return res;
}

int mp_avrcp_target_finalize(void)
{
	startfunc;
	int res = BT_ERROR_NONE;

	if (!gMpAvrcpInitialized)
		return -1;

	res = bt_avrcp_target_deinitialize();
	MP_CHECK_VAL(res == BT_ERROR_NONE, res);

	res = bt_deinitialize();
	MP_CHECK_VAL(res == BT_ERROR_NONE, res);

	bt_avrcp_unset_shuffle_mode_changed_cb();
	bt_avrcp_unset_equalizer_state_changed_cb();
	bt_avrcp_unset_repeat_mode_changed_cb();

	gMpAvrcpInitialized = false;

	IF_FREE(gMpAvrcpCb);
	return res;
}

MpAvrcpErr_e mp_avrcp_set_mode_change_cb(mp_avrcp_connection_state_changed_cb connection_cb, mp_avrcp_shuffle_changed_cb s_cb,
	mp_avrcp_repeat_changed_cb r_cb, mp_avrcp_eq_changed_cb e_cb, void *user_data)
{
	startfunc;
	gMpAvrcpCb = calloc(1, sizeof(MpAvrcpCb_t));
	MP_CHECK_VAL(gMpAvrcpCb, MP_AVRCP_ERROR);
	gMpAvrcpCb->connection_cb = connection_cb;
	gMpAvrcpCb->s_cb = s_cb;
	gMpAvrcpCb->r_cb = r_cb;
	gMpAvrcpCb->e_cb = e_cb;
	gMpAvrcpCb->user_data = user_data;
	return 0;
}

MpAvrcpErr_e mp_avrcp_noti_player_state(mp_avrcp_player_state_e state)
{
	mp_avrcp_target_initialize();
	MP_CHECK_VAL(gMpAvrcpInitialized, MP_AVRCP_ERROR);

	bt_avrcp_player_state_e player_state = BT_AVRCP_PLAYER_STATE_STOPPED;
	switch (state) {
	case MP_AVRCP_STATE_STOPPED:
		player_state = BT_AVRCP_PLAYER_STATE_STOPPED;
		break;
	case MP_AVRCP_STATE_PLAYING:
		player_state = BT_AVRCP_PLAYER_STATE_PLAYING;
		break;
	case MP_AVRCP_STATE_PAUSED:
		player_state = BT_AVRCP_PLAYER_STATE_PAUSED;
		break;
	case MP_AVRCP_STATE_REW:
		player_state = BT_AVRCP_PLAYER_STATE_REWIND_SEEK;
		break;
	case MP_AVRCP_STATE_FF:
		player_state = BT_AVRCP_PLAYER_STATE_FORWARD_SEEK;
		break;
	default:
		break;
	}
	mp_avrcp_noti_track_position(mp_player_mgr_get_position());

	int res = bt_avrcp_target_notify_player_state(player_state);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);

	return res;
}
MpAvrcpErr_e mp_avrcp_noti_eq_state(mp_avrcp_eq_state_e eq)
{
	mp_avrcp_target_initialize();
	MP_CHECK_VAL(gMpAvrcpInitialized, MP_AVRCP_ERROR);
	bt_avrcp_equalizer_state_e state = BT_AVRCP_EQUALIZER_STATE_OFF;
	if (eq == MP_AVRCP_EQ_ON)
		state = BT_AVRCP_EQUALIZER_STATE_ON;

	int res = bt_avrcp_target_notify_equalizer_state(state);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	return res;
}

MpAvrcpErr_e mp_avrcp_noti_repeat_mode(mp_avrcp_repeat_mode_e repeat)
{
	mp_avrcp_target_initialize();
	MP_CHECK_VAL(gMpAvrcpInitialized, MP_AVRCP_ERROR);
	bt_avrcp_repeat_mode_e state = BT_AVRCP_REPEAT_MODE_OFF;
	switch (repeat) {
	case MP_AVRCP_REPEAT_OFF:
		state = BT_AVRCP_REPEAT_MODE_OFF;
		break;
	case MP_AVRCP_REPEAT_ONE:
		state = BT_AVRCP_REPEAT_MODE_SINGLE_TRACK;
		break;
	case MP_AVRCP_REPEAT_ALL:
		state = BT_AVRCP_REPEAT_MODE_ALL_TRACK;
		break;
	default:
		break;
	}
	int res = bt_avrcp_target_notify_repeat_mode(state);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	return res;
}

MpAvrcpErr_e mp_avrcp_noti_shuffle_mode(mp_avrcp_shuffle_mode_e shuffle)
{
	mp_avrcp_target_initialize();
	MP_CHECK_VAL(gMpAvrcpInitialized, MP_AVRCP_ERROR);
	bt_avrcp_shuffle_mode_e state = BT_AVRCP_SHUFFLE_MODE_OFF;
	switch (shuffle) {
	case MP_AVRCP_SHUFFLE_OFF:
		state = BT_AVRCP_SHUFFLE_MODE_OFF;
		break;
	case MP_AVRCP_SHUFFLE_ON:
		state = BT_AVRCP_SHUFFLE_MODE_ALL_TRACK;
		break;
	default:
		break;
	}

	int res = bt_avrcp_target_notify_shuffle_mode(state);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	return res;

}

MpAvrcpErr_e mp_avrcp_noti_track_position(unsigned int position)
{
	mp_avrcp_target_initialize();
	MP_CHECK_VAL(gMpAvrcpInitialized, MP_AVRCP_ERROR);
	int res = bt_avrcp_target_notify_position(position);
	mp_retv_if (res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	return res;
}

MpAvrcpErr_e mp_avrcp_noti_track(const char *title, const char *artist, const char *album, const char *genre, unsigned int duration)
{
	mp_avrcp_target_initialize();
	MP_CHECK_VAL(gMpAvrcpInitialized, MP_AVRCP_ERROR);
	int res = bt_avrcp_target_notify_track(title, artist, album, genre, 0, 0, duration);
	mp_retv_if(res == BT_ERROR_PERMISSION_DENIED, MP_AVRCP_ERROR_PERMISSION_DENIED);
	return res;
}

#endif

