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


#include "mc-list-play.h"
#include "mc-common.h"

static mc_player_cbs *g_player_cbs = NULL;
static Ecore_Pipe *g_player_pipe = NULL;
static player_h mc_player = NULL;
list_item_data_t *previous_item_data = NULL;
list_item_data_t *pre_item_data = NULL;
list_item_data_t *cur_item_data = NULL;

sound_type_e g_init_current_type;
int g_init_volume = -1;

int g_position = 0;

static void _mc_pre_play_mgr_play_new_uri(void *data);
static void _mc_pre_play_mgr_realize_player_mgr(void *data);
static void _mc_pre_play_mgr_create_player_mgr(const char *uri);
static void _mc_pre_play_mgr_callback_pipe_handler(void *data, void *buffer, unsigned int nbyte);
static void _mc_pre_play_mgr_create_player(player_h *player);
static void _mc_pre_play_mgr_prepare_cb(void *userdata);
static void _mc_pre_play_mgr_update_play_button_status(list_item_data_t *item_data);
static void _mc_pre_play_mgr_player_start();
static void _mc_pre_play_mgr_player_pause();
static void _mc_pre_play_mgr_player_stop();
//static void _mc_pre_play_mgr_player_set_position(void *data, player_seek_completed_cb callback, void *cb_data);
static void _mc_pre_play_mgr_player_unprepare();

Evas_Object *g_popup;

sound_type_e mc_get_sound_type()
{
	startfunc;
	sound_type_e type = SOUND_TYPE_SYSTEM;
	int ret = 0;
	ret = sound_manager_get_current_sound_type(&type);
	DEBUG_TRACE("ret is [%d]", ret);
	endfunc;
	return type;
}

int mc_get_volume(sound_type_e type)
{
	startfunc;
	int volume = 0;
	sound_manager_get_volume(type, &volume);
	endfunc;
	return volume;
}

void mc_vol_type_set(sound_type_e type)
{
	startfunc;
	sound_type_e current_type;
	int volume = 0;
	current_type = mc_get_sound_type();
	volume = mc_get_volume(current_type);
	if (g_init_volume == -1) {
		g_init_current_type = current_type;
		g_init_volume = volume;
	}
	sound_manager_set_current_sound_type(type);
}

void mc_vol_reset_default_value(void* data)
{
	startfunc;
	MP_CHECK(data);
	struct app_data *ad = data;

	sound_type_e current_type;
	current_type = mc_get_sound_type();
	DEBUG_TRACE("current type is %d", current_type);
	if (ad->select_type == MC_SELECT_SINGLE_RINGTONE) {
		if (g_init_current_type != 	current_type) {
			mc_vol_type_set(g_init_current_type);
		}
	}
}


bool mc_player_mgr_is_active(void)
{
	return (mc_player != NULL) ? TRUE : FALSE;
}

player_state_e mc_pre_play_get_player_state(void)
{
	player_state_e state = PLAYER_STATE_NONE;

	player_get_state(mc_player, &state);
	return state;
}

void mc_pre_play_control_clear_pre_item_data()
{
	pre_item_data = NULL;
	previous_item_data = NULL;
}

player_h mc_pre_play_mgr_get_player_handle(void)
{
	player_h player_handle = NULL;

	if (mc_player_mgr_is_active()) {
		return mc_player;
	} else {
		_mc_pre_play_mgr_create_player(&player_handle);
		mc_player = player_handle;
		return player_handle;
	}
}

void mc_pre_play_mgr_reset_song(void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK(item_data);

	_mc_pre_play_mgr_player_stop();
	DEBUG_TRACE("update reset song item_data =%x, item_data", item_data);
	item_data->state = PLAYER_STATE_NONE;
	elm_genlist_item_fields_update(item_data->it, MC_PRE_PLAY_BUTTON_PART_NAME, ELM_GENLIST_ITEM_FIELD_CONTENT);
	DEBUG_TRACE("update reset song item_data =%x, state=%d", item_data, item_data->state);
	_mc_pre_play_mgr_player_unprepare();

	return ;
}

void mc_pre_play_mgr_reset_song_without_stop(void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK(item_data);

	if (mc_player) {
		int error_code = player_destroy(mc_player);
		if (error_code) {
			DEBUG_TRACE("player destroy error %d", error_code);
		}
		mc_player = NULL;
	}

	if (item_data) {
		item_data->state = PLAYER_STATE_NONE;
		_mc_pre_play_mgr_update_play_button_status(item_data);
	}
}

void mc_pre_play_mgr_destroy_play(void)
{
	MP_CHECK(mc_player);
	player_state_e state = PLAYER_STATE_NONE;
	int error_code = 0;

	player_get_state(mc_player, &state);
	if (PLAYER_STATE_NONE != state) {
		player_stop(mc_player);
		error_code = player_destroy(mc_player);
		if (error_code) {
			DEBUG_TRACE("player destroy error %d", error_code);
		}
	}
	mc_player = NULL;

	MP_CHECK(cur_item_data);
	if (cur_item_data) {
		_mc_pre_play_mgr_update_play_button_status(cur_item_data);
	}

	return;
}

void mc_pre_play_mgr_play_control(void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t*)data;
	player_state_e state = PLAYER_STATE_NONE;

	MP_CHECK(mc_player);
	player_get_state(mc_player, &state);
	DEBUG_TRACE("mh_pre_play_mgr_play_control state = %d", state);

	if (PLAYER_STATE_PLAYING == state) {
		_mc_pre_play_mgr_player_pause();
	}

	if (PLAYER_STATE_PAUSED == state) {
		_mc_pre_play_mgr_player_start();
	}

	//after complete click again
	if (PLAYER_STATE_IDLE == state) {
		mc_pre_play_mgr_play_song(item_data);
	}
	_mc_pre_play_mgr_update_play_button_status(item_data);

	return ;
}

void mc_pre_play_mgr_play_song(void *data)
{
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK(item_data);

	DEBUG_TRACE("item_data = %x", item_data);
	_mc_pre_play_mgr_play_new_uri(item_data);

	return ;
}

void mc_player_pause(void)
{
	startfunc;
	MP_CHECK(mc_player);
	MP_CHECK(cur_item_data);

	player_state_e play_status = mc_pre_play_get_player_state();

	if (PLAYER_STATE_PLAYING == play_status) {
		_mc_pre_play_mgr_player_pause();
	}
	_mc_pre_play_mgr_update_play_button_status(cur_item_data);
}

void mc_player_play(void)
{
	startfunc;
	MP_CHECK(mc_player);
	MP_CHECK(cur_item_data);

	player_state_e play_status = mc_pre_play_get_player_state();

	if (PLAYER_STATE_PLAYING == play_status) {
		_mc_pre_play_mgr_player_start();
	}
	_mc_pre_play_mgr_update_play_button_status(cur_item_data);
}

static void _mc_pre_play_error(int ret)
{
	const char *message = NULL;

	if (ret == PLAYER_ERROR_SOUND_POLICY) {
		if (mc_is_call_connected()) {
			message = STR_MP_UNABLE_TO_PLAY_DURING_CALL;
		}
	}
	if (message) {
		mc_post_status_message(GET_STR(message));
	}
}

static void _mc_pre_play_mgr_player_start()
{
	startfunc;
	int error_code =  PLAYER_ERROR_NONE;
	MP_CHECK(mc_player);

	error_code = player_start(mc_player);
	if (error_code) {
		DEBUG_TRACE("play start error %d", error_code);
		_mc_pre_play_error(error_code);
	}

	return;
}

static void _mc_pre_play_mgr_player_pause()
{
	startfunc;
	int error_code =  PLAYER_ERROR_NONE;
	MP_CHECK(mc_player);

	error_code = player_pause(mc_player);
	if (error_code) {
		DEBUG_TRACE("play pause error %d", error_code);
	}
	return ;
}

static void _mc_pre_play_mgr_player_stop()
{
	startfunc;
	int error_code =  PLAYER_ERROR_NONE;
	MP_CHECK(mc_player);

	error_code = player_stop(mc_player);
	if (error_code) {
		DEBUG_TRACE("play stop error %d", error_code);
	}
	return ;
}

static void _mc_pre_play_mgr_player_unprepare()
{
	startfunc;
	int error_code =  PLAYER_ERROR_NONE;
	MP_CHECK(mc_player);

	error_code = player_unprepare(mc_player);
	if (error_code) {
		DEBUG_TRACE("player_unprepare %d", error_code);
	}

	return ;
}

static void _mc_pre_play_mgr_update_play_button_status(list_item_data_t *item_data)
{
	//player_get_state(mc_player, &item_data->state);
	MP_CHECK(item_data);
	DEBUG_TRACE("get state = %d", item_data->state);
	elm_genlist_item_fields_update(item_data->it, MC_PRE_PLAY_BUTTON_PART_NAME, ELM_GENLIST_ITEM_FIELD_CONTENT);

	return ;
}

static void _mc_pre_play_mgr_play_stop()
{
	player_state_e state = PLAYER_STATE_NONE;
	int error_code = PLAYER_ERROR_NONE;

	MP_CHECK(mc_player);
	error_code = player_get_state(mc_player, &state);
	if (PLAYER_ERROR_NONE == error_code) {
		if ((PLAYER_STATE_PLAYING == state) || (PLAYER_STATE_PAUSED == state)) {
			_mc_pre_play_mgr_player_stop();
		} else {
			DEBUG_TRACE("player state error %d", state);
		}
	} else {
		DEBUG_TRACE("_mc_pre_play_mgr_play_stop get state error %d", error_code);
	}

	return ;
}

static void _mc_pre_play_complete_cb(void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t*)data;

	_mc_pre_play_mgr_play_stop();

	Evas_Object *part_content = elm_object_item_part_content_get(item_data->it, "elm.icon.1");
	if (part_content) {
		elm_object_signal_emit(part_content, "show_default", "*");
		elm_object_item_signal_emit(item_data->it, "hide_color", "*");
	}

	player_get_state(mc_player, &item_data->state);

	_mc_pre_play_mgr_player_unprepare();

	if (mc_player) {
		mc_pre_play_mgr_destroy_play();
	}

	return ;
}

static void _mc_pre_play_buffering_cb(int percent, void *userdata)
{
	DEBUG_TRACE("buffering percent = %d\%", percent);
	list_item_data_t *item_data = (list_item_data_t*)userdata;
	MP_CHECK(item_data);

	if (100 == percent) {
		_mc_pre_play_mgr_update_play_button_status(item_data);
	}

	return ;
}

static void _mc_pre_play_interrupt_cb(player_interrupted_code_e code, void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t*)data;
	player_state_e state = PLAYER_STATE_NONE;

	switch (code) {
	case PLAYER_INTERRUPTED_BY_MEDIA:
		DEBUG_TRACE("Interrupt :: PLAYER_INTERRUPTED_BY_MEDIA");
		break;

	case PLAYER_INTERRUPTED_BY_CALL:
		DEBUG_TRACE("Interrupt :: PLAYER_INTERRUPTED_BY_CALL_START");
		break;

	case PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT:
		DEBUG_TRACE("Interrupt :: PLAYER_INTERRUPTED_BY_RESOURCE_CONFLICT");
		break;

	case PLAYER_INTERRUPTED_BY_ALARM:
		DEBUG_TRACE("Interrupt :: PLAYER_INTERRUPTED_BY_ALARM_START");
		break;

	case PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG:
		DEBUG_TRACE("Interrupt :: PLAYER_INTERRUPTED_BY_EARJACK_UNPLUG");
		break;

	case PLAYER_INTERRUPTED_COMPLETED:
		DEBUG_TRACE("PLAYER_INTERRUPTED_COMPLETED");
		break;
	default:
		break;
	}

	player_get_state(mc_player, &state);
	DEBUG_TRACE("mc_pre_play_mgr_play_control state = %d", state);
	item_data->state = state;

	_mc_pre_play_mgr_update_play_button_status(item_data);

	//mc_pre_play_mgr_play_control(item_data);

	return ;
}

static void _mc_pre_play_prepare_cb(void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t *)data;
	MP_CHECK(item_data);

	player_get_state(mc_player, &item_data->state);
	DEBUG_TRACE("player status = %d", item_data->state);

	_mc_pre_play_mgr_player_start();
	_mc_pre_play_mgr_update_play_button_status(item_data);

	return ;
}

static void _mc_pre_play_error_cb(int error_code, void *userdata)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t *)userdata;
	MP_CHECK(item_data);
	DEBUG_TRACE("error code %d", error_code);

	switch (error_code) {
	case PLAYER_ERROR_OUT_OF_MEMORY:
		DEBUG_TRACE("PLAYER_ERROR_OUT_OF_MEMORY");
		break;
	case PLAYER_ERROR_INVALID_PARAMETER:
		DEBUG_TRACE("PLAYER_ERROR_INVALID_PARAMETER");
		break;
	case PLAYER_ERROR_NOT_SUPPORTED_FILE:
		DEBUG_TRACE("receive MM_ERROR_PLAYER_CODEC_NOT_FOUND\n");
		break;
	case PLAYER_ERROR_CONNECTION_FAILED:
		DEBUG_TRACE("MM_ERROR_PLAYER_STREAMING_CONNECTION_FAIL");
		break;
	default:
		DEBUG_TRACE("default: error_code: %d", error_code);
	}

	_mc_pre_play_mgr_play_stop();
	_mc_pre_play_mgr_update_play_button_status(item_data);

	return ;
}


void mc_pre_play_mgr_set_completed_cb(player_completed_cb  callback, void *user_data)
{
	startfunc;
	MP_CHECK(g_player_cbs);

	g_player_cbs->completed_cb = callback;
	g_player_cbs->user_data[MC_PLAYER_CB_TYPE_COMPLETED] = user_data;
}

void mc_pre_play_mgr_set_interrupt_cb(player_interrupted_cb  callback, void *user_data)
{
	startfunc;
	MP_CHECK(g_player_cbs);

	g_player_cbs->interrupted_cb = callback;
	g_player_cbs->user_data[MC_PLAYER_CB_TYPE_INTURRUPTED] = user_data;
}

void mc_pre_play_mgr_set_error_cb(player_error_cb  callback, void *user_data)
{
	startfunc;
	MP_CHECK(g_player_cbs);

	g_player_cbs->error_cb = callback;
	g_player_cbs->user_data[MC_PLAYER_CB_TYPE_ERROR] = user_data;
}

void mc_pre_play_mgr_set_buffering_cb(player_error_cb  callback, void *user_data)
{
	startfunc;
	MP_CHECK(g_player_cbs);

	g_player_cbs->buffering_cb = callback;
	g_player_cbs->user_data[MC_PLAYER_CB_TYPE_BUFFERING] = user_data;
}

void mc_pre_play_mgr_set_prepare_cb(player_prepared_cb  callback, void *user_data)
{
	startfunc;
	MP_CHECK(g_player_cbs);

	g_player_cbs->prepare_cb = callback;
	g_player_cbs->user_data[MC_PLAYER_CB_TYPE_PREPARE] = user_data;
}

static void _mc_pre_play_mgr_play_new_uri(void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t*)data;
	MP_CHECK(item_data);
	char *uri = NULL;
	mp_media_info_h media_handle = NULL;

	media_handle = (mp_media_info_h)item_data->media;
	mp_media_info_get_file_path(media_handle, &uri);
	DEBUG_TRACE("uri = %s", uri);

	if (NULL == uri) {
		DEBUG_TRACE("play uri fail, get null uri");
		return ;
	} else {
		DEBUG_TRACE("play uri, get uri = %s", uri);

		_mc_pre_play_mgr_create_player_mgr(uri);
		// TEMP_BLOCK
		//player_set_safety_volume(mc_player);

		//mc_player_mgr_set_started_cb(__mf_mc_list_play_start_cb, itemData);
		//mc_pre_play_mgr_set_paused_cb(_mc_list_play_paused_cb, item_data);
		mc_pre_play_mgr_set_completed_cb(_mc_pre_play_complete_cb, item_data);
		mc_pre_play_mgr_set_interrupt_cb(_mc_pre_play_interrupt_cb, item_data);
		mc_pre_play_mgr_set_prepare_cb(_mc_pre_play_prepare_cb, item_data);
		mc_pre_play_mgr_set_error_cb(_mc_pre_play_error_cb, item_data);
		mc_pre_play_mgr_set_buffering_cb(_mc_pre_play_buffering_cb, item_data);

		_mc_pre_play_mgr_realize_player_mgr(item_data);
	}
	_mc_pre_play_mgr_update_play_button_status(item_data);

	return ;
}

static void _mc_pre_play_mgr_realize_player_mgr(void *data)
{
	startfunc;
	list_item_data_t *item_data = (list_item_data_t*)data;
	player_state_e state = PLAYER_STATE_NONE;
	int error = PLAYER_ERROR_NONE;

	if (mc_player != NULL) {
		error = player_get_state(mc_player, &state);
		item_data->state = state;
		if ((PLAYER_ERROR_NONE == error) && (PLAYER_STATE_IDLE == state)) {
			DEBUG_TRACE("prepare async");
			if (player_prepare_async(mc_player, _mc_pre_play_mgr_prepare_cb, item_data)) {
				DEBUG_TRACE("prepare err");
			}
		}
	}

	return ;
}

static void _mc_pre_play_mgr_prepare_cb(void *userdata)
{
	startfunc;
	MP_CHECK(g_player_pipe);

	mc_player_cb_extra_data extra_data;
	memset(&extra_data, 0x00, sizeof(mc_player_cb_extra_data));
	extra_data.cb_type = MC_PLAYER_CB_TYPE_PREPARE;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mc_player_cb_extra_data));
	return ;
}

static void _mc_pre_play_mgr_completed_cb(void *userdata)
{
	startfunc;
	MP_CHECK(g_player_pipe);

	mc_player_cb_extra_data extra_data;
	memset(&extra_data, 0x00, sizeof(mc_player_cb_extra_data));
	extra_data.cb_type = MC_PLAYER_CB_TYPE_COMPLETED;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mc_player_cb_extra_data));
}

static void _mc_pre_play_mgr_interrupt_cb(player_interrupted_code_e code, void *userdata)
{
	startfunc;
	MP_CHECK(g_player_pipe);

	mc_player_cb_extra_data extra_data;
	memset(&extra_data, 0x00, sizeof(mc_player_cb_extra_data));
	extra_data.cb_type = MC_PLAYER_CB_TYPE_INTURRUPTED;
	extra_data.param.interrupted_code = code;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mc_player_cb_extra_data));
}

static void _mc_pre_play_mgr_error_cb(int error_code, void *userdata)
{
	startfunc;
	MP_CHECK(g_player_pipe);

	mc_player_cb_extra_data extra_data;
	memset(&extra_data, 0x00, sizeof(mc_player_cb_extra_data));
	extra_data.cb_type = MC_PLAYER_CB_TYPE_ERROR;
	extra_data.param.error_code = error_code;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mc_player_cb_extra_data));
}

static void _mc_pre_play_mgr_buffer_cb(int percent, void *user_data)
{
	startfunc;
	MP_CHECK(g_player_pipe);

	mc_player_cb_extra_data extra_data;
	memset(&extra_data, 0x00, sizeof(mc_player_cb_extra_data));
	extra_data.cb_type = MC_PLAYER_CB_TYPE_BUFFERING;
	extra_data.param.percent = percent;

	ecore_pipe_write(g_player_pipe, &extra_data, sizeof(mc_player_cb_extra_data));

	return ;
}

static void _mc_pre_play_mgr_create_player_mgr(const char *uri)
{
	startfunc;
	MP_CHECK(uri);

	if (NULL != uri) {
		_mc_pre_play_mgr_create_player(&mc_player);

		DEBUG_TRACE("player = %x", mc_player);

		player_set_uri(mc_player, uri);


		if (NULL == g_player_pipe) {
			g_player_pipe = ecore_pipe_add(_mc_pre_play_mgr_callback_pipe_handler, NULL);
			/*ecore_pipe_del(g_player_pipe);
			g_player_pipe = NULL;*/
		}
		if (NULL == g_player_cbs) {
			g_player_cbs = calloc(1, sizeof(mc_player_cbs));
		}

		player_set_completed_cb(mc_player, _mc_pre_play_mgr_completed_cb, NULL);
		player_set_interrupted_cb(mc_player, _mc_pre_play_mgr_interrupt_cb, NULL);
		player_set_error_cb(mc_player, _mc_pre_play_mgr_error_cb, NULL);
		player_set_buffering_cb(mc_player, _mc_pre_play_mgr_buffer_cb, NULL);

	}
	return ;
}



static void _mc_pre_play_mgr_callback_pipe_handler(void *data, void *buffer, unsigned int nbyte)
{
	startfunc;
	mc_player_cb_extra_data *extra_data = buffer;
	MP_CHECK(extra_data);
	MP_CHECK(g_player_cbs);

	switch (extra_data->cb_type) {
		/*note: start callback and paused callback for player have been removed*/
		/*case MC_PLAYER_CB_TYPE_STARTED:
			if (g_player_cbs->started_cb)
				g_player_cbs->started_cb(g_player_cbs->user_data[MF_PLAYER_CB_TYPE_STARTED]);
			break;

		case MC_PLAYER_CB_TYPE_PAUSED:
			if (g_player_cbs->paused_cb)
				g_player_cbs->paused_cb(g_player_cbs->user_data[MC_PLAYER_CB_TYPE_PAUSED]);
			break; */

	case MC_PLAYER_CB_TYPE_COMPLETED: {
		if (g_player_cbs->completed_cb) {
			g_player_cbs->completed_cb(g_player_cbs->user_data[MC_PLAYER_CB_TYPE_COMPLETED]);
		}
	}
	break;

	case MC_PLAYER_CB_TYPE_INTURRUPTED: {
		if (g_player_cbs->interrupted_cb) {
			g_player_cbs->interrupted_cb(extra_data->param.interrupted_code, g_player_cbs->user_data[MC_PLAYER_CB_TYPE_INTURRUPTED]);
		}
	}
	break;

	case MC_PLAYER_CB_TYPE_ERROR: {
		if (g_player_cbs->error_cb) {
			g_player_cbs->error_cb(extra_data->param.error_code, g_player_cbs->user_data[MC_PLAYER_CB_TYPE_ERROR]);
		}
	}
	break;

	case MC_PLAYER_CB_TYPE_BUFFERING: {
		if (g_player_cbs->buffering_cb) {
			g_player_cbs->buffering_cb(extra_data->param.percent , g_player_cbs->user_data[MC_PLAYER_CB_TYPE_BUFFERING]);
		}
	}
	break;

	case MC_PLAYER_CB_TYPE_PREPARE: {
		if (g_player_cbs->prepare_cb) {
			g_player_cbs->prepare_cb(g_player_cbs->user_data[MC_PLAYER_CB_TYPE_PREPARE]);
		}
	}
	break;

	default:
		DEBUG_TRACE("Not suppoted callback type [%d]", extra_data->cb_type);
	}

	return ;
}

static void _mc_pre_play_mgr_create_player(player_h *player)
{
	startfunc;
	int ret = 0;

	if (NULL != *player) {
		mc_pre_play_mgr_destroy_play();
	}

	ret = player_create(player);
	if (PLAYER_ERROR_NONE != ret) {
		DEBUG_TRACE("create player error %s", ret);
		return ;
	}

	return ;
}

void mc_pre_play_control_play_no_pause_music_item(list_item_data_t *item_data)
{
	startfunc;
	MP_CHECK(item_data);
	char *pre_uri = NULL;
	char *cur_uri = NULL;
	mp_media_info_h media_handle = NULL;

	media_handle = (mp_media_info_h)item_data->media;
	mp_media_info_get_file_path(media_handle, &cur_uri);
	cur_item_data = item_data;
	if (pre_item_data) {
		mp_media_info_get_file_path((mp_media_info_h)(pre_item_data->media), &pre_uri);
	}

	if (NULL == pre_uri || NULL == mc_player) {
		DEBUG_TRACE("pre listen play new song");
		mc_pre_play_mgr_play_song(item_data);
	} else {
		if (g_strcmp0(pre_uri, cur_uri) != 0) {
			//playing song changed update genlist
			DEBUG_TRACE("pre listen change listen song");
			//mc_pre_play_mgr_reset_song(pre_item_data);
			mc_pre_play_mgr_reset_song_without_stop(pre_item_data);
			mc_pre_play_mgr_play_song(item_data);
		} else {
			// play control in pre song
			DEBUG_TRACE("pre listen play control");
			mc_pre_play_mgr_play_control(item_data);
		}
	}

	previous_item_data = pre_item_data;
	pre_item_data = item_data;
	return ;
}

void mc_pre_play_control_play_music_item(list_item_data_t *item_data)
{
	startfunc;
	MP_CHECK(item_data);
	char *pre_uri = NULL;
	char *cur_uri = NULL;
	mp_media_info_h media_handle = NULL;

	media_handle = (mp_media_info_h)item_data->media;
	mp_media_info_get_file_path(media_handle, &cur_uri);
	cur_item_data = item_data;
	if (pre_item_data) {
		mp_media_info_get_file_path((mp_media_info_h)(pre_item_data->media), &pre_uri);
	}
	DEBUG_TRACE("pre uri = %s", pre_uri);

	if (NULL == pre_uri || NULL == mc_player) {
		//play new song
		DEBUG_TRACE("pre listen play new song");
		mc_pre_play_mgr_play_song(item_data);
	} else {
		if (g_strcmp0(pre_uri, cur_uri) != 0) {
			//playing song changed update genlist
			DEBUG_TRACE("pre listen change listen song");
			mc_pre_play_mgr_reset_song(pre_item_data);
			mc_pre_play_mgr_play_song(item_data);
		} else {
			// play control in pre song
			DEBUG_TRACE("pre listen play control");
			mc_pre_play_mgr_play_control(item_data);
		}
	}

	pre_item_data = item_data;
	return ;
}


