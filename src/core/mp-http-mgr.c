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


#include "mp-player-debug.h"
#include "music.h"
#include "mp-http-mgr.h"
#include <runtime_info.h>
#include <wifi.h>

static MpHttpState_t _mp_http_mgr_get_network_status();
static void _mp_http_mgr_refresh_network_info(mp_http_mgr_t *http_mgr);
static bool _mp_http_mgr_register_runtime_info_change_cb(mp_http_mgr_t *http_mgr);
static void _mp_http_mgr_ignore_runtime_info_change_cb();
static void _mp_http_mgr_network_config_changed_cb(runtime_info_key_e key, void *user_data);
static void _mp_wifi_network_config_changed_cb(wifi_connection_state_e state, wifi_ap_h ap, void *user_data);

bool mp_http_mgr_create(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	MP_CHECK_FALSE(ad);
	MP_CHECK_FALSE((!ad->http_mgr));

	ad->http_mgr = calloc(1, sizeof(mp_http_mgr_t));
	MP_CHECK_FALSE(ad->http_mgr);
	ad->http_mgr->ad = ad;

	if (!_mp_http_mgr_register_runtime_info_change_cb(ad->http_mgr))
		goto mp_exception;

	_mp_http_mgr_refresh_network_info(ad->http_mgr);

	return true;

mp_exception:
	mp_http_mgr_destory(ad);
	return false;
}

bool mp_http_mgr_destory(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	MP_CHECK_FALSE(ad);

	if (!ad->http_mgr)
		mp_http_mgr_create(ad);
	MP_CHECK_FALSE(ad->http_mgr);

	_mp_http_mgr_ignore_runtime_info_change_cb();

	IF_FREE(ad->http_mgr);

	return TRUE;
}

static MpHttpState_t _mp_http_mgr_get_network_status()
{
	startfunc;
	MpHttpState_t state = MP_HTTP_STATE_OFF;
	wifi_connection_state_e state_wifi;
	int err = 0;
	bool bwifi_on_off = true;
	bool b3g_on_off = true;

	err = wifi_get_connection_state(&state_wifi);
	if (err != WIFI_ERROR_NONE) {
		    WARN_TRACE("wifi_is_activated error. err is [%d]", err);
		    bwifi_on_off = false;
	}

	if (state_wifi == WIFI_CONNECTION_STATE_FAILURE
		    || state_wifi == WIFI_CONNECTION_STATE_DISCONNECTED) {
		    WARN_TRACE("WIFI_CONNECTION_STATE DISABLED");
		    bwifi_on_off = false;
	}

	err = runtime_info_get_value_bool(RUNTIME_INFO_KEY_PACKET_DATA_ENABLED, &b3g_on_off);
	if (err != RUNTIME_INFO_ERROR_NONE) {
		    WARN_TRACE("runtime_info_get_value_bool error. err is [%d]", err);
		    b3g_on_off = false;
	}

	DEBUG_TRACE("3g flag is %d", b3g_on_off);
	DEBUG_TRACE("wifi flag is %d", bwifi_on_off);
	/*decide status*/
	if (bwifi_on_off == true)
		state = MP_HTTP_STATE_WIFI;
#if 0	/* 3g has issue when getting runtime_info despite SIM is not inserted, it also returns 1 */
	else if (b3g_on_off == true)
		state = MP_HTTP_STATE_CELLULAR;
#endif
	else
		state = MP_HTTP_STATE_OFF;

	mp_debug("network state is %d", state);
	return state;
}

static bool _mp_http_mgr_register_runtime_info_change_cb(mp_http_mgr_t *http_mgr)
{
	startfunc;
	MP_CHECK_FALSE(http_mgr);

	if (runtime_info_set_changed_cb(RUNTIME_INFO_KEY_PACKET_DATA_ENABLED,
		_mp_http_mgr_network_config_changed_cb, http_mgr) != 0) {
		mp_error("runtime_info_set_changed_cb() fail");
		return FALSE;
	}

	if (wifi_set_connection_state_changed_cb(_mp_wifi_network_config_changed_cb, http_mgr)) {
		mp_error("wifi_set_connection_state_changed_cb() fail");
		return FALSE;
	}

	return TRUE;
}

static void _mp_http_mgr_ignore_runtime_info_change_cb()
{
	startfunc;

	if (runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_PACKET_DATA_ENABLED) != 0) {
		mp_error("runtime_info_unset_changed_cb(RUNTIME_INFO_KEY_PACKET_DATA_ENABLED) fail");
	}

	if (wifi_unset_connection_state_changed_cb() != 0) {
		mp_error("wifi_unset_connection_state_changed_cb fail");
	}
}

static void _mp_http_mgr_refresh_network_info(mp_http_mgr_t *http_mgr)
{
	startfunc;
	MP_CHECK(http_mgr);

	http_mgr->http_state = _mp_http_mgr_get_network_status();
}

static void _mp_http_mgr_network_config_changed_cb(runtime_info_key_e key, void *user_data)
{
	startfunc;
	mp_http_mgr_t *http_mgr = (mp_http_mgr_t *)user_data;
	MP_CHECK(http_mgr);

	_mp_http_mgr_refresh_network_info(http_mgr);

	/*post network changed event to each view*/
	mp_view_mgr_post_event(mp_view_mgr_get_view_manager(), MP_NETWORK_STATE_CHANGED);
}

static void _mp_wifi_network_config_changed_cb(wifi_connection_state_e state, wifi_ap_h ap, void *user_data)
{
	startfunc;
	mp_http_mgr_t *http_mgr = (mp_http_mgr_t *)user_data;
	MP_CHECK(http_mgr);

	_mp_http_mgr_refresh_network_info(http_mgr);

	/*post network changed event to each view*/
	mp_view_mgr_post_event(mp_view_mgr_get_view_manager(), MP_NETWORK_STATE_CHANGED);
}

MpHttpState_t
mp_http_mgr_get_state(void *data)
{
	struct appdata *ad = (struct appdata *)data;

	MP_CHECK_FALSE(ad);
	if (!ad->http_mgr)
		mp_http_mgr_create(ad);

	mp_http_mgr_t *http_mgr = ad->http_mgr;

	return http_mgr->http_state;
}

inline bool mp_http_mgr_is_connected(void *data)
{
	struct appdata *ad = data;
	MP_CHECK_FALSE(ad);
	if (!ad->http_mgr)
		mp_http_mgr_create(ad);

	MpHttpState_t state = mp_http_mgr_get_state(data);
	return (state > MP_HTTP_STATE_OFF) ? true : false;
}
