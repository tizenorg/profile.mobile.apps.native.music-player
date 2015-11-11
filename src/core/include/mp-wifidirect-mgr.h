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


#pragma once


#include <stdbool.h>
#include <glib.h>


typedef	void *mp_wifidirect_mgr_h;

typedef enum {
	MP_WIFI_DIRECT_DEVICE_TV = 0x00,
	MP_WIFI_DIRECT_DEVICE_BD,
	MP_WIFI_DIRECT_DEVICE_LFD,
	MP_WIFI_DIRECT_DEVICE_STB,
	MP_WIFI_DIRECT_DEVICE_MOBILE,
	MP_WIFI_DIRECT_DEVICE_TABLET,
	MP_WIFI_DIRECT_DEVICE_DONGLE,
	MP_WIFI_DIRECT_DEVICE_CAMERA,
	MP_WIFI_DIRECT_DEVICE_CAMCORDER,
	MP_WIFI_DIRECT_DEVICE_REF,
	MP_WIFI_DIRECT_DEVICE_AC_WALL,
	MP_WIFI_DIRECT_DEVICE_WM,
	MP_WIFI_DIRECT_DEVICE_ROBOT_TV,
	MP_WIFI_DIRECT_DEVICE_PC,
	MP_WIFI_DIRECT_DEVICE_DMR,
	MP_WIFI_DIRECT_DEVICE_HOME_THEATER,
	MP_WIFI_DIRECT_DEVICE_UNKNOWN,
} mp_wifidirect_mgr_device_type_t;


typedef enum {
	MP_WIFI_DIRECT_DISCOVERY_STATE_NONE = 0x00,
	MP_WIFI_DIRECT_DISCOVERY_STATE_STARTED,
	MP_WIFI_DIRECT_DISCOVERY_STATE_FOUND,
	MP_WIFI_DIRECT_DISCOVERY_STATE_FINISHED,
} mp_wifidirect_mgr_discovery_status_t;

typedef enum {
	MP_WIFI_DIRECT_DISCOVERY_STATE_ACTIVE = 0x00,
	MP_WIFI_DIRECT_DISCOVERY_STATE_DEACTIVE,
} mp_wifidirect_mgr_active_status_t;



typedef void (*mp_wifi_discovery_cb)(mp_wifidirect_mgr_device_type_t nDeviceType, char *szDeviceName, void *pDeviceInfo, bool bIsConnect, void *pUserData);
typedef void (*mp_wifi_state_change_cb)(mp_wifidirect_mgr_active_status_t nStatus, void *pUserData);



bool mp_wifidirect_mgr_app_control_init();
bool mp_wifidirect_mgr_app_control_deinit();

mp_wifidirect_mgr_h mp_wifidirect_mgr_create(mp_wifi_discovery_cb pDiscoveryCb, mp_wifi_state_change_cb pActiveCb);
void mp_wifidirect_mgr_destroy(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_set_user_data(mp_wifidirect_mgr_h pWifiDirectHandle, void *pUserData);
bool mp_wifidirect_mgr_realize(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_unrealize(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_init_miracast(bool enable);
bool mp_wifidirect_mgr_start_scan(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_stop_scan(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_connected_device_scan(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_activate(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_connect(mp_wifidirect_mgr_h pWifiDirectHandle, char *szMacAddr);
bool mp_wifidirect_mgr_disconnect(mp_wifidirect_mgr_h pWifiDirectHandle);
bool mp_wifidirect_mgr_launch_allshare_cast(Evas_Object *pWin, const char *szMacAddr, void *pUserData);
bool mp_wifidirect_mgr_deactivate(mp_wifidirect_mgr_h pWifiDirectHandle);

