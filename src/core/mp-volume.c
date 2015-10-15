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

#include <Ecore.h>
#include <Ecore_Evas.h>
#include <sound_manager.h>
#include <efl_extension.h>
#include "mp-define.h"
#include "mp-volume.h"

typedef struct {
#if 0
	Ecore_X_Window xwin;
#else
	void *xwin;
#endif
	Elm_Win *win;
	bool condition[MP_VOLUME_KEY_GRAB_COND_MAX];
	bool grabbed;

	/* key event callback */
	Mp_Volume_Key_Event_Cb key_event_cb;
	void *key_event_user_data;

	/* volume change callback */
	Mp_Volume_Change_Cb volume_change_cb;
	void *user_data;

	Ecore_Timer *pressed_timer;
} MpVolumeKeyMgr_t;

static MpVolumeKeyMgr_t g_volume_key_mgr;

#define MP_VOLUME_KEY_LONG_PRESS_TRIGGER_TIME	(0.2) 	/* sec */
#define MP_VOLUME_KEY_LONG_PRESS_INTEVAL_TIME	(0.07)

void _mp_volume_changed_cb(sound_type_e type, unsigned int volume, void *user_data);

#if 0
void
mp_volume_init(Ecore_X_Window xwin, Elm_Win *win)
#else
mp_volume_init(void *xwin, Elm_Win *win)
#endif
{
	startfunc;
	g_volume_key_mgr.xwin = xwin;
	g_volume_key_mgr.win = win;

#if 0
	int res;
	res = sound_manager_set_volume_changed_cb(_mp_volume_changed_cb, &g_volume_key_mgr);
	if (res != SOUND_MANAGER_ERROR_NONE) {
		ERROR_TRACE("Error: sound_manager_set_volume_changed_cb");
	}
#endif
	endfunc;
}

void mp_volume_finalize(void)
{
	startfunc;
	if (!g_volume_key_mgr.xwin)
		return;

/* 	sound_manager_unset_volume_changed_cb(); */
	endfunc;
}

static void
_mp_volume_key_grab_check_condition()
{
	bool start = true;
	int condition = 0;
	while (condition < MP_VOLUME_KEY_GRAB_COND_MAX) {
		if (!g_volume_key_mgr.condition[condition]) {
			/* do NOT start */
			start = false;
			break;
		}
		condition++;
	}

	/* start key grab */
	if (start)
		mp_volume_key_grab_start();
	else
		mp_volume_key_grab_end();
}

void
mp_volume_key_grab_condition_set(mp_volume_key_grab_condition_e condition, bool enabled)
{
	MP_CHECK(condition < MP_VOLUME_KEY_GRAB_COND_MAX);

	/* set condition */
	g_volume_key_mgr.condition[condition] = enabled;
	WARN_TRACE("VOL key grab condition(%d) changed => [%d]", condition, enabled);

	_mp_volume_key_grab_check_condition();
}

bool
mp_volume_key_grab_start()
{
	MP_CHECK_FALSE(g_volume_key_mgr.xwin);

	Eina_Bool error = EINA_FALSE;

	error = eext_win_keygrab_set(g_volume_key_mgr.win, "XF86AudioRaiseVolume");
	if (error != EINA_TRUE) {
		mp_error("(KEY_VOLUMEUP)... [%d]", error);
		return false;
	}

	error = eext_win_keygrab_set(g_volume_key_mgr.win, "XF86AudioLowerVolume");
	if (error != EINA_TRUE) {
		mp_error("(KEY_VOLUMEDOWN)... [%d]", error);
		eext_win_keygrab_unset(g_volume_key_mgr.win, "XF86AudioRaiseVolume");
		return false;
	}

	error = eext_win_keygrab_set(g_volume_key_mgr.win, "XF86AudioMute");
	if (error != EINA_TRUE) {
		mp_error("(KEY_MUTE)... [0x%d]", error);
		eext_win_keygrab_unset(g_volume_key_mgr.win, "XF86AudioRaiseVolume");
		eext_win_keygrab_unset(g_volume_key_mgr.win, "XF86AudioLowerVolume");
		return false;
	}

	WARN_TRACE("START_volume_key_grab");
	g_volume_key_mgr.grabbed = true;
	return true;
}

void
mp_volume_key_grab_end()
{
	eext_win_keygrab_unset(g_volume_key_mgr.win, "XF86AudioRaiseVolume");
	eext_win_keygrab_unset(g_volume_key_mgr.win, "XF86AudioLowerVolume");
	eext_win_keygrab_unset(g_volume_key_mgr.win, "XF86AudioMute");

	if (g_volume_key_mgr.pressed_timer && g_volume_key_mgr.key_event_cb) {
		g_volume_key_mgr.key_event_cb(g_volume_key_mgr.key_event_user_data, MP_VOLUME_KEY_DOWN, true);
	}
	mp_ecore_timer_del(g_volume_key_mgr.pressed_timer);

	WARN_TRACE("STOP_volume_key_grab");
	g_volume_key_mgr.grabbed = false;
}

bool
mp_volume_key_is_grabed()
{
	return g_volume_key_mgr.grabbed;
}

static Eina_Bool
_mp_volume_key_pressed_timer(void *data)
{
	TIMER_TRACE();
	mp_volume_key_e type = (int)data;

	if (g_volume_key_mgr.key_event_cb) {
		g_volume_key_mgr.key_event_cb(g_volume_key_mgr.key_event_user_data, type, false);
	}

	MP_CHECK_VAL(g_volume_key_mgr.pressed_timer, ECORE_CALLBACK_CANCEL);
	ecore_timer_interval_set(g_volume_key_mgr.pressed_timer, MP_VOLUME_KEY_LONG_PRESS_INTEVAL_TIME);

	return ECORE_CALLBACK_RENEW;
}

#if 0
void _mp_volume_changed_cb(sound_type_e type, unsigned int volume, void *user_data)
{
	EVENT_TRACE("type: %d, volume changed: %d", type, volume);
	if (type == SOUND_TYPE_MEDIA) {
		if (g_volume_key_mgr.volume_change_cb)
			g_volume_key_mgr.volume_change_cb(volume, g_volume_key_mgr.user_data);
	}
}
#endif

void _mp_volume_handle_change(unsigned int volume)
{
	if (g_volume_key_mgr.volume_change_cb) {
		g_volume_key_mgr.volume_change_cb(volume, g_volume_key_mgr.user_data);
	}
}


void
mp_volume_key_event_send(mp_volume_key_e type, bool released)
{
	WARN_TRACE("volume key[%d], released[%d]", type, released);

	if (!g_volume_key_mgr.grabbed) {
		WARN_TRACE("already ungrabbed.. ignore this event");
		return;
	}

	if (!released && g_volume_key_mgr.pressed_timer) {
		/* long press timer is working*/
		return;
	}

	mp_ecore_timer_del(g_volume_key_mgr.pressed_timer);

	if (g_volume_key_mgr.key_event_cb) {
		/* send callback */
		g_volume_key_mgr.key_event_cb(g_volume_key_mgr.key_event_user_data, type, released);
	}

	if (!released)
		g_volume_key_mgr.pressed_timer = ecore_timer_add(MP_VOLUME_KEY_LONG_PRESS_TRIGGER_TIME, _mp_volume_key_pressed_timer, (void *)type);
}

void
mp_volume_key_event_callback_add(Mp_Volume_Key_Event_Cb event_cb, void *user_data)
{
	g_volume_key_mgr.key_event_cb = event_cb;
	g_volume_key_mgr.key_event_user_data = user_data;
}

void
mp_volume_key_event_callback_del()
{
	g_volume_key_mgr.key_event_cb = NULL;
	g_volume_key_mgr.key_event_user_data = NULL;
	mp_ecore_timer_del(g_volume_key_mgr.pressed_timer);
}

void
mp_volume_key_event_timer_del()
{
	mp_ecore_timer_del(g_volume_key_mgr.pressed_timer);
	g_volume_key_mgr.pressed_timer = NULL;
}

void
mp_volume_add_change_cb(Mp_Volume_Change_Cb cb, void *user_data)
{
	/* if (g_volume_key_mgr.volume_change_cb) return; */

	if (cb)
		EVENT_TRACE("Add volume change callback");
	else
		EVENT_TRACE("Del volume change callback");
	/* int res = SOUND_MANAGER_ERROR_NONE; */
	g_volume_key_mgr.volume_change_cb = cb;
	g_volume_key_mgr.user_data = user_data;
}


