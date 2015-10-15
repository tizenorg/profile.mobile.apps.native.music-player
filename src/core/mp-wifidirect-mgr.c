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

#include <wifi-direct.h>

#include "mp-common.h"
#include "mp-wifidirect-mgr.h"

#define ALLSHARE_CAST_APP_NAME		"com.samsung.allshare-cast-popup"

#define MP_DISCOVERY_TIME_OUT		10

static bool			g_IsWifiDirectInit = FALSE;

typedef struct _WifiDirectHandle {
	bool bIsRealize;
	mp_wifi_discovery_cb pDiscoveryCb;
	mp_wifi_state_change_cb pActiveCb;
	void *pUserData;
} WifiDirectHandle;


static void _mp_wifidirect_mgr_destroy_handle(WifiDirectHandle *pWifiDirect);
static void _mp_wifidirect_mgr_print_error_msg(int nErr);

static mp_wifidirect_mgr_device_type_t __mp_wifidirect_mgr_get_device_type(wifi_direct_primary_device_type_e primary_device_type, wifi_direct_secondary_device_type_e secondary_device_type)
{
	mp_wifidirect_mgr_device_type_t nType = MP_WIFI_DIRECT_DEVICE_UNKNOWN;
	switch (primary_device_type) {
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_COMPUTER: 		nType = MP_WIFI_DIRECT_DEVICE_PC; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_INPUT_DEVICE: 	nType = MP_WIFI_DIRECT_DEVICE_UNKNOWN; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_PRINTER:		nType = MP_WIFI_DIRECT_DEVICE_UNKNOWN; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_CAMERA: 		nType = MP_WIFI_DIRECT_DEVICE_CAMERA; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_STORAGE: 		nType = MP_WIFI_DIRECT_DEVICE_UNKNOWN; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_NETWORK_INFRA: 	nType = MP_WIFI_DIRECT_DEVICE_UNKNOWN; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_DISPLAY: 		nType = MP_WIFI_DIRECT_DEVICE_TV; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_MULTIMEDIA_DEVICE:
		if (secondary_device_type == 5)	/* dongle */
			nType = MP_WIFI_DIRECT_DEVICE_DONGLE;
		else if (secondary_device_type == 4)	/* homesync */
			nType = MP_WIFI_DIRECT_DEVICE_STB;
		else if (secondary_device_type == 6)	/* blueray disc */
			nType = MP_WIFI_DIRECT_DEVICE_BD;
		else
			nType = MP_WIFI_DIRECT_DEVICE_PC;
		break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_GAME_DEVICE: 	nType = MP_WIFI_DIRECT_DEVICE_PC; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_TELEPHONE: 	nType = MP_WIFI_DIRECT_DEVICE_MOBILE; 	break;
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_AUDIO: 		nType = MP_WIFI_DIRECT_DEVICE_HOME_THEATER; 	break;/* sub type = 1 */
		case WIFI_DIRECT_PRIMARY_DEVICE_TYPE_OTHER: 		nType = MP_WIFI_DIRECT_DEVICE_UNKNOWN; 	break;
	}
	return nType;
}

/* callback functions */
static bool __mp_wifidirect_mgr_connected_peer_cb(wifi_direct_connected_peer_info_s *peer, void *pUserData)
{
	if (pUserData == NULL) {
		DEBUG_TRACE("pUserData is NULL");
		return FALSE;
	}
	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pUserData;

	if (mp_util_mirroring_is_connected() == false)
		return TRUE;

	if (peer->is_miracast_device) {
		if (pWifiDirect->pDiscoveryCb) {
			mp_wifidirect_mgr_device_type_t nType = __mp_wifidirect_mgr_get_device_type(peer->primary_device_type, peer->secondary_device_type);
			pWifiDirect->pDiscoveryCb(nType, peer->device_name, (void *)peer->mac_address, TRUE, pWifiDirect->pUserData);
		}
	}

	return TRUE;

}

static bool __mp_wifidirect_mgr_discoverd_peer_cb(wifi_direct_discovered_peer_info_s *peer, void *pUserData)
{
	startfunc;
	if (pUserData == NULL) {
		DEBUG_TRACE("pUserData is NULL");
		return FALSE;
	}
	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pUserData;

	if (peer->is_miracast_device) {
		if (pWifiDirect->pDiscoveryCb) {
			mp_wifidirect_mgr_device_type_t nType = __mp_wifidirect_mgr_get_device_type(peer->primary_device_type, peer->secondary_device_type);
			pWifiDirect->pDiscoveryCb(nType, peer->device_name, (void *)peer->mac_address, peer->is_connected, pWifiDirect->pUserData);
		}
	}

	return TRUE;
}

static void __mp_wifidirect_mgr_discovery_cb(int error_code, wifi_direct_discovery_state_e discovery_state, void *pUserData)
{
	if (discovery_state == WIFI_DIRECT_DISCOVERY_FOUND) {
		wifi_direct_foreach_discovered_peers(__mp_wifidirect_mgr_discoverd_peer_cb, (void *)pUserData);
	}
}

static void __mp_wifidirect_mgr_device_state_changed_cb(wifi_direct_error_e error_code, wifi_direct_device_state_e device_state, void *pUserData)
{
	if (pUserData == NULL) {
		DEBUG_TRACE("pUserData is NULL");
		return;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pUserData;
	if (pWifiDirect->pActiveCb == NULL) {
		DEBUG_TRACE("pWifiDirect->pActiveCb is NULL");
		return;
	}
	if (device_state == WIFI_DIRECT_DEVICE_STATE_ACTIVATED) {
		pWifiDirect->pActiveCb(MP_WIFI_DIRECT_DISCOVERY_STATE_ACTIVE, pWifiDirect->pUserData);
	} else {
		pWifiDirect->pActiveCb(MP_WIFI_DIRECT_DISCOVERY_STATE_DEACTIVE, pWifiDirect->pUserData);
	}
}

/* internal functions */
static void _mp_wifidirect_mgr_print_error_msg(int nErr)
{
	switch (nErr) {
	case WIFI_DIRECT_ERROR_OUT_OF_MEMORY:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_OUT_OF_MEMORY");
		break;
	case WIFI_DIRECT_ERROR_NOT_PERMITTED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_NOT_PERMITTED");
		break;
	case WIFI_DIRECT_ERROR_INVALID_PARAMETER:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_INVALID_PARAMETER");
		break;
	case WIFI_DIRECT_ERROR_RESOURCE_BUSY:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_RESOURCE_BUSY");
		break;
	case WIFI_DIRECT_ERROR_CONNECTION_TIME_OUT:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_CONNECTION_TIME_OUT");
		break;
	case WIFI_DIRECT_ERROR_NOT_INITIALIZED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_NOT_INITIALIZED");
		break;
	case WIFI_DIRECT_ERROR_COMMUNICATION_FAILED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_COMMUNICATION_FAILED");
		break;
	case WIFI_DIRECT_ERROR_WIFI_USED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_WIFI_USED");
		break;
	case WIFI_DIRECT_ERROR_MOBILE_AP_USED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_MOBILE_AP_USED");
		break;
	case WIFI_DIRECT_ERROR_CONNECTION_FAILED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_CONNECTION_FAILED");
		break;
	case WIFI_DIRECT_ERROR_AUTH_FAILED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_AUTH_FAILED");
		break;
	case WIFI_DIRECT_ERROR_OPERATION_FAILED:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_OPERATION_FAILED");
		break;
	case WIFI_DIRECT_ERROR_TOO_MANY_CLIENT:
		DEBUG_TRACE("WIFI_DIRECT_ERROR_TOO_MANY_CLIENT");
		break;
	default:
		DEBUG_TRACE("UNKNOWN ERROR Message 0x%x", nErr);
		break;
	}
}

static void _mp_wifidirect_mgr_destroy_handle(WifiDirectHandle *pWifiDirect)
{
	if (pWifiDirect == NULL) {
		DEBUG_TRACE("pWifiDirect is NULL");
		return;
	}

	pWifiDirect->pDiscoveryCb = NULL;
	pWifiDirect->pActiveCb = NULL;

	SAFE_FREE(pWifiDirect);
}



/* external functions */
bool mp_wifidirect_mgr_app_control_init()
{
	if (g_IsWifiDirectInit) {
		DEBUG_TRACE("Already wifi direct initialize");
		return TRUE;
	}

	int nRet = WIFI_DIRECT_ERROR_NONE;
	nRet = wifi_direct_initialize();
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_initialize is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	g_IsWifiDirectInit = TRUE;

	return TRUE;
}


bool mp_wifidirect_mgr_app_control_deinit()
{
	int nRet = WIFI_DIRECT_ERROR_NONE;

	if (g_IsWifiDirectInit == FALSE) {
		DEBUG_TRACE("Not Yet wifi direct initialize");
		return TRUE;
	}

	nRet = wifi_direct_deinitialize();
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_deinitialize is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	g_IsWifiDirectInit = FALSE;

	return TRUE;
}


mp_wifidirect_mgr_h mp_wifidirect_mgr_create(mp_wifi_discovery_cb pDiscoveryCb, mp_wifi_state_change_cb pActiveCb)
{
	WifiDirectHandle *pWifiDirect = calloc(1, sizeof(WifiDirectHandle));

	if (pWifiDirect == NULL) {
		DEBUG_TRACE("pWifiDirect alloc is fail");
		return NULL;
	}

	pWifiDirect->pDiscoveryCb = pDiscoveryCb;
	pWifiDirect->pActiveCb = pActiveCb;

	return pWifiDirect;
}



bool mp_wifidirect_mgr_set_user_data(mp_wifidirect_mgr_h pWifiDirectHandle, void *pUserData)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	pWifiDirect->pUserData = pUserData;

	return TRUE;
}

bool mp_wifidirect_mgr_realize(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;

	nRet = wifi_direct_set_discovery_state_changed_cb(__mp_wifidirect_mgr_discovery_cb, (void *)pWifiDirect);
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_set_discovery_state_changed_cb is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	nRet = wifi_direct_set_device_state_changed_cb(__mp_wifidirect_mgr_device_state_changed_cb, (void *)pWifiDirect);
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_set_device_state_changed_cb is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	pWifiDirect->bIsRealize = TRUE;

	return TRUE;
}

bool mp_wifidirect_mgr_unrealize(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;
	nRet = wifi_direct_unset_discovery_state_changed_cb();
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_unset_discovery_state_changed_cb is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	nRet = wifi_direct_unset_device_state_changed_cb();
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_unset_device_state_changed_cb is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}
	pWifiDirect->bIsRealize = FALSE;

	return TRUE;
}

void mp_wifidirect_mgr_destroy(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	mp_wifidirect_mgr_unrealize((mp_wifidirect_mgr_h)pWifiDirect);

	_mp_wifidirect_mgr_destroy_handle(pWifiDirect);
}

bool mp_wifidirect_mgr_init_miracast(bool enable)
{
	int nRet = WIFI_DIRECT_ERROR_NONE;

	nRet = wifi_direct_init_miracast(enable);
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_init_miracast is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}
	return TRUE;
}

bool mp_wifidirect_mgr_start_scan(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;
	if (pWifiDirect->bIsRealize == FALSE) {
		DEBUG_TRACE("Not yet realize");
		return FALSE;
	}
	nRet = wifi_direct_init_miracast(TRUE);
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_init_miracast is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
	}
	nRet = wifi_direct_start_discovery_specific_channel(false, 0, WIFI_DIRECT_DISCOVERY_FULL_SCAN);
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_start_discovery_specific_channel is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}
	return TRUE;
}

bool mp_wifidirect_mgr_stop_scan(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}


	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;
	if (pWifiDirect->bIsRealize == FALSE) {
		DEBUG_TRACE("Not yet realize");
		return FALSE;
	}
	wifi_direct_state_e nStatus = WIFI_DIRECT_STATE_DEACTIVATED;
	wifi_direct_get_state(&nStatus);

	if (nStatus != WIFI_DIRECT_STATE_DISCOVERING) {
		return TRUE;
	}

	nRet = wifi_direct_cancel_discovery();
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_cancel_discovery is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	return TRUE;
}

bool mp_wifidirect_mgr_connected_device_scan(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;
	int nRet = WIFI_DIRECT_ERROR_NONE;

	nRet = wifi_direct_foreach_connected_peers(__mp_wifidirect_mgr_connected_peer_cb, (void *) pWifiDirect);
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_foreach_connected_peers is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	return TRUE;
}

bool mp_wifidirect_mgr_activate(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;

	if (pWifiDirect->bIsRealize == FALSE) {
		DEBUG_TRACE("Not yet realize");
		return FALSE;
	}

	nRet = wifi_direct_activate();
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_activate is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	return TRUE;
}

bool mp_wifidirect_mgr_deactivate(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;

	if (pWifiDirect->bIsRealize == FALSE) {
		DEBUG_TRACE("Not yet realize");
		return FALSE;
	}
	wifi_direct_state_e nStatus = WIFI_DIRECT_STATE_DEACTIVATED;
	wifi_direct_get_state(&nStatus);

	if (nStatus == WIFI_DIRECT_STATE_ACTIVATING ||
		nStatus == WIFI_DIRECT_STATE_ACTIVATED ||
		nStatus == WIFI_DIRECT_STATE_DISCOVERING) {
		nRet = wifi_direct_init_miracast(FALSE);
		if (nRet != WIFI_DIRECT_ERROR_NONE) {
			DEBUG_TRACE("wifi_direct_init_miracast is fail");
			_mp_wifidirect_mgr_print_error_msg(nRet);
		}

		nRet = wifi_direct_deactivate();
		if (nRet != WIFI_DIRECT_ERROR_NONE) {
			DEBUG_TRACE("wifi_direct_activate is fail");
			_mp_wifidirect_mgr_print_error_msg(nRet);
			return FALSE;
		}

	}

	return TRUE;
}

bool mp_wifidirect_mgr_connect(mp_wifidirect_mgr_h pWifiDirectHandle, char *szMacAddr)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle is NULL");
		return FALSE;
	}

	if (szMacAddr == NULL) {
		DEBUG_TRACE("szMacAddr is NULL");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;

	if (pWifiDirect->bIsRealize == FALSE) {
		DEBUG_TRACE("Not yet realize");
		return FALSE;
	}

	nRet = wifi_direct_connect(szMacAddr);
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_connect is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	return TRUE;

}

bool mp_wifidirect_mgr_disconnect(mp_wifidirect_mgr_h pWifiDirectHandle)
{
	if (pWifiDirectHandle == NULL) {
		DEBUG_TRACE("pWifiDirectHandle alloc is fail");
		return FALSE;
	}

	WifiDirectHandle *pWifiDirect = (WifiDirectHandle *)pWifiDirectHandle;

	int nRet = WIFI_DIRECT_ERROR_NONE;

	if (pWifiDirect->bIsRealize == FALSE) {
		DEBUG_TRACE("Not yet realize");
		return FALSE;
	}

	nRet = wifi_direct_disconnect_all();
	if (nRet != WIFI_DIRECT_ERROR_NONE) {
		DEBUG_TRACE("wifi_direct_disconnect_all is fail");
		_mp_wifidirect_mgr_print_error_msg(nRet);
		return FALSE;
	}

	return TRUE;
}

static void __mp_wifidirect_mgr_ug_reply_cb(app_control_h pRequest, app_control_h pReply, app_control_result_e nResult, void *pUserData)
{

	if (pRequest) {
		char *pkgName;
		app_control_get_app_id (pRequest, &pkgName);
		WARN_TRACE("Request : %s", pkgName);
		SAFE_FREE(pkgName);
	}

	if (pReply) {
		char *pkgName;
		app_control_get_app_id (pReply, &pkgName);
		WARN_TRACE("Reply : %s", pkgName);
		SAFE_FREE(pkgName);
	}

}

bool mp_wifidirect_mgr_launch_allshare_cast(Evas_Object *pWin, const char *szMacAddr, void *pUserData)
{
	app_control_h pService = NULL;

	int nRet = 0;
	nRet  = app_control_create(&pService);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_create is fail [0x%x]", nRet);
		goto Execption;
	}
	MP_CHECK_FALSE(pService);

	ERROR_TRACE("Mac Addr : %s", szMacAddr);

#if 0
	nRet = app_control_set_window(pService, elm_win_xwindow_get(pWin));
	if (nRet != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_set_window is fail [0x%x]", nRet);
		goto Execption;
	}
#endif

	nRet = app_control_add_extra_data(pService, "-t", "connection_req");
	if (nRet != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_add_extra_data is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_add_extra_data(pService, "mac", szMacAddr);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_add_extra_data is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_set_app_id(pService, ALLSHARE_CAST_APP_NAME);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_set_app_id is fail [0x%x]", nRet);
		goto Execption;
	}

	nRet = app_control_send_launch_request(pService, __mp_wifidirect_mgr_ug_reply_cb, pUserData);
	if (nRet != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_send_launch_request is fail [0x%x]", nRet);
		goto Execption;
	}

	app_control_destroy(pService);
	pService = NULL;

	return TRUE;

Execption:
	if (pService) {
		app_control_destroy(pService);
		pService = NULL;
	}
	return FALSE;

}

