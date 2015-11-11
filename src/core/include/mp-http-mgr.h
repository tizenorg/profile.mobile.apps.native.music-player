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


#ifndef __MP_HTTP_MGR_H__
#define __MP_HTTP_MGR_H__

#include "mp-define.h"

#define HTTP_ADDR_LEN_MAX		64
typedef enum {
	MP_HTTP_SVC_DEFAULT,
	MP_HTTP_SVC_SHAZAM,
	MP_HTTP_SVC_STREAMING,
	MP_HTTP_SVC_MAX,
} mp_http_svc_type;

typedef enum {
	MP_HTTP_RESPONSE_NORMAL,
	MP_HTTP_RESPONSE_DISCONNECT,
} mp_http_response_type;

typedef enum {
	MP_HTTP_RESP_FAIL,
	MP_HTTP_RESP_SUCCESS,
} MpHttpRespResultType_t;

typedef void (*MpHttpOpenCb)(gpointer user_data);
typedef bool (*MpHttpRespExcuteCb)(gpointer user_data, mp_http_response_type response_type);
typedef bool(*MpHttpOpenExcuteCb)(gpointer user_data);

typedef enum {
	MP_HTTP_STATE_NONE = 0,
	MP_HTTP_STATE_OFF = 0,
	MP_HTTP_STATE_CELLULAR,
	MP_HTTP_STATE_WIFI,
} MpHttpState_t;

typedef struct mp_http_mgr_t {
	struct appdata *ad;
	MpHttpState_t http_state;		//the state of the http
} mp_http_mgr_t;

bool mp_http_mgr_create(void *data);
bool mp_http_mgr_destory(void *data);
MpHttpState_t mp_http_mgr_get_state(void *data);
inline bool mp_http_mgr_is_connected(void *data);

#define TOKEN "025B58C0"
#endif //__MP_HTTP_MGR_H__
