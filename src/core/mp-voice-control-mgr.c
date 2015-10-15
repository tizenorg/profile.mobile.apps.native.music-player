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


#include <voice_control_static.h>

#include "mp-define.h"
#include "mp-voice-control-mgr.h"
#include "mp-player-debug.h"
#include "mp-util.h"

typedef struct {
	int x_win_id;
	bool listen;

	Mp_Voice_Ctrl_Action_Cb action_cb;
	void *user_data;
} mp_voice_ctrl_s;

static mp_voice_ctrl_s g_voice_ctrl_data;


static void
_mp_voice_ctrl_mgr_state_changed_cb(vc_static_state_e previous, vc_static_state_e current, void *user_data)
{
	mp_debug("previous = [%d], current = [%d]", previous, current);

	mp_voice_ctrl_s *mgr = user_data;
	MP_CHECK(mgr);

	int ret = 0;
	switch (current) {
	case VC_STATIC_STATE_NONE:
		break;

	case VC_STATIC_STATE_INITIALIZING:
		break;

	case VC_STATIC_STATE_READY:
		if (previous == VC_STATIC_STATE_INITIALIZING) {

			ret = vc_static_set_command_type(VC_STATIC_COMMAND_TYPE_MUSIC);
			if (ret != VC_STATIC_ERROR_NONE) {
				mp_error("vc_static_set_command_type().. [0x%x]", ret);
				return;
			}

			ret = vc_static_start();
			if (ret != VC_STATIC_ERROR_NONE) {
				mp_error("vc_static_start() [0x%x]", ret);
				return;
			}
		}
		break;

	case VC_STATIC_STATE_RECORDING:
		break;

	default:
		WARN_TRACE("state is invalid");
		break;
	}
}

static inline mp_voice_ctrl_action_e
_mp_voice_ctrl_mgr_convert_action(int action_type)
{
	mp_voice_ctrl_action_e mp_action = MP_VOICE_CTRL_ACTION_NONE;
	switch (action_type) {
	case VC_STATIC_MULTIMEDIA_ACTION_NEXT:
		mp_action = MP_VOICE_CTRL_ACTION_NEXT;
		break;
	case VC_STATIC_MULTIMEDIA_ACTION_PREVIOUS:
		mp_action = MP_VOICE_CTRL_ACTION_PREVIOUS;
		break;
	case VC_STATIC_MULTIMEDIA_ACTION_PAUSE:
		mp_action = MP_VOICE_CTRL_ACTION_PAUSE;
		break;
	case VC_STATIC_MULTIMEDIA_ACTION_PLAY:
		mp_action = MP_VOICE_CTRL_ACTION_PLAY;
		break;
	case VC_STATIC_MULTIMEDIA_ACTION_VOLUME_UP:
		mp_action = MP_VOICE_CTRL_ACTION_VOLUME_UP;
		break;
	case VC_STATIC_MULTIMEDIA_ACTION_VOLUME_DOWN:
		mp_action = MP_VOICE_CTRL_ACTION_VOLUME_DOWN;
		break;
	default:
		mp_action = MP_VOICE_CTRL_ACTION_NONE;
	}

	return mp_action;
}

static void
_mp_voice_ctrl_mgr_result_cb(vc_static_result_event_e event, vc_static_command_type_e type, int action_type, void *user_data)
{
	WARN_TRACE("event = [%d], type = [%d], aciton = [%d]", event, type, action_type);
	MP_CHECK(type == VC_STATIC_COMMAND_TYPE_MUSIC);

	mp_voice_ctrl_s *mgr = user_data;
	MP_CHECK(mgr);

	switch (event) {
	case VC_STATIC_RESULT_EVENT_SUCCESS:
		if (mgr->action_cb)
			mgr->action_cb(_mp_voice_ctrl_mgr_convert_action(action_type), mgr->user_data);
		break;

	case VC_STATIC_RESULT_EVENT_REJECTED:
		break;

	case VC_STATIC_RESULT_EVENT_ERROR:
		break;

	default:
		WARN_TRACE("state is invalid");
		break;
	}
}

static void
_mp_voice_ctrl_mgr_error_cb(vc_static_error_e reason, void *user_data)
{
	mp_error("error = %d", reason);

	mp_voice_ctrl_s *mgr = user_data;
	MP_CHECK(mgr);
}


static bool
_mp_voice_ctrl_mgr_init_fw()
{
	startfunc;
	MP_CHECK_FALSE(g_voice_ctrl_data.x_win_id);

	vc_static_state_e state = VC_STATIC_STATE_NONE;
	int ret = vc_static_get_state(&state);
	if (ret != VC_STATIC_ERROR_NONE) {
		mp_error("vc_static_get_state() .. [0x%x]", ret);
		state = VC_STATIC_STATE_NONE;
	}
	mp_debug("state = %d", state);

	if (state == VC_STATIC_STATE_NONE) {
		ret = vc_static_initialize(g_voice_ctrl_data.x_win_id,
					_mp_voice_ctrl_mgr_state_changed_cb,
					_mp_voice_ctrl_mgr_result_cb,
					_mp_voice_ctrl_mgr_error_cb,
					&g_voice_ctrl_data);

		if (ret != VC_STATIC_ERROR_NONE) {
			mp_error("vc_static_initialize().. [0x%x]", ret);
			return false;
		}

	} else if (state == VC_STATIC_STATE_READY) {
		mp_debug("vc_static_initialize");
		ret = vc_static_set_command_type(VC_STATIC_COMMAND_TYPE_MUSIC);
		if (ret != VC_STATIC_ERROR_NONE) {
			mp_error("vc_static_set_command_type().. [0x%x]", ret);
			return false;
		}

		ret = vc_static_start();
		if (ret != VC_STATIC_ERROR_NONE) {
			mp_error("vc_static_start() [0x%x]", ret);
			return false;
		}
	}

	endfunc;
	return true;
}

static void
_mp_voice_ctrl_mgr_deinit_fw()
{
	int ret = 0;
	vc_static_state_e state = VC_STATIC_STATE_NONE;
	ret = vc_static_get_state(&state);
	if (ret != VC_STATIC_ERROR_NONE) {
		mp_error("vc_static_get_state() .. [0x%x]", ret);
		state = VC_STATIC_STATE_NONE;
	}

	if (state != VC_STATIC_STATE_NONE) {
		if (state == VC_STATIC_STATE_RECORDING) {
			ret = vc_static_cancel();
			if (ret != VC_STATIC_ERROR_NONE) {
				mp_error("vc_static_cancel() .. [0x%x]", ret);
			}
		}

		ret = vc_static_deinitialize();
		if (ret != VC_STATIC_ERROR_NONE) {
			mp_error("vc_static_deinitialize().. [0x%x]", ret);
		}
	}
}

static void
_mp_voice_ctrl_mgr_setting_value_changed_cb(keynode_t *node, void *user_data)
{
	bool setting_enabled = _mp_voice_ctrl_mgr_check_setting();
	mp_debug("setting[%d], app listening[%d]", setting_enabled, g_voice_ctrl_data.listen);

	if (setting_enabled && g_voice_ctrl_data.listen) {
		_mp_voice_ctrl_mgr_init_fw();
	} else {
		_mp_voice_ctrl_mgr_deinit_fw();
	}
}

static bool
_mp_vc_supported_command_cb(const char *command, int action_type, void *user_data)
{
	/* DEBUG_TRACE("command: %s, action_type: %d", command, action_type); */
	char **command_txt = user_data;
	MP_CHECK_FALSE(command_txt);

	command_txt[action_type] = (char *)command;
	return true;
}


static void
_mp_voice_ctrl_mgr_visual_queue(void)
{
	startfunc;
	char *text = NULL;
	char *command[VC_STATIC_MULTIMEDIA_ACTION_MAX] = {0,};

	vc_static_language_e language = VC_STATIC_LANGUAGE_EN_US;
	vc_static_get_default_language(&language);

	vc_static_foreach_supported_command(VC_STATIC_COMMAND_TYPE_MUSIC, language,
		_mp_vc_supported_command_cb, &command);

	text = g_strdup_printf(GET_STR(STR_MP_YOU_CAN_CONTROL),
		command[VC_STATIC_MULTIMEDIA_ACTION_NEXT],
		command[VC_STATIC_MULTIMEDIA_ACTION_PREVIOUS],
		command[VC_STATIC_MULTIMEDIA_ACTION_PLAY],
		command[VC_STATIC_MULTIMEDIA_ACTION_PAUSE],
		command[VC_STATIC_MULTIMEDIA_ACTION_VOLUME_UP],
		command[VC_STATIC_MULTIMEDIA_ACTION_VOLUME_DOWN]);

	/* DEBUG_TRACE("%s", text); */
	mp_util_post_status_message(NULL, text);

	IF_FREE(text);

}

void
mp_voice_ctrl_mgr_set_action_callback(Mp_Voice_Ctrl_Action_Cb action_cb, void *user_data)
{
	mp_debug("callback = %p, data = %p", action_cb, user_data);

	g_voice_ctrl_data.action_cb = action_cb;
	g_voice_ctrl_data.user_data = user_data;
}

bool
mp_voice_ctrl_mgr_start_listening()
{
	startfunc;

	MP_CHECK_FALSE(g_voice_ctrl_data.x_win_id);
	MP_CHECK_FALSE(g_voice_ctrl_data.action_cb);

	g_voice_ctrl_data.listen = true;

	bool result = false;
	if (_mp_voice_ctrl_mgr_check_setting())
		 result = _mp_voice_ctrl_mgr_init_fw();

	if (result)
		_mp_voice_ctrl_mgr_visual_queue();

	endfunc;
	return g_voice_ctrl_data.listen;
}

void
mp_voice_ctrl_mgr_stop_listening()
{
	startfunc;

	_mp_voice_ctrl_mgr_deinit_fw();
	g_voice_ctrl_data.listen = false;

	endfunc;
}

