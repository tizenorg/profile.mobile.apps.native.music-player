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

#include "mp-cloud.h"
#include "mp-http-mgr.h"
#ifdef MP_FEATURE_CLOUD

#include "mp-player-debug.h"
#include "mp-util.h"
#include "mp-playlist-mgr.h"
#include "mp-streaming-mgr.h"
#include <cloud_content_sync.h>

#define CLOUD_DOMAIN_NAME "Dropbox"

#define MP_CLOUD_TIMEOUT 15

static cloud_content_sync_h g_cloud_handle;
static Ecore_Pipe *g_cloud_pipe;

enum {
	MP_CLOUD_CONTENT_CHANGED,
	MP_CLOUD_STREAMING_URL,
	MP_CLOUD_SYNC_STATUS_CHANGED,
};

typedef struct _cloud_pipe_data {
	int type;
	void *data;
	void *user_data;
} cloud_pipe_data;

static int
_mp_cloud_get_wifi_only()
{
	if (!g_cloud_handle)
		mp_cloud_create();
	int res = 1;
	cloud_content_sync_setting_get_wifi_only_type(g_cloud_handle, CLOUD_CONTENT_SYNC_TYPE_AUDIO, &res);
	DEBUG_TRACE("WIFI only: %d", res);
	return res;
}

static void
_mp_cloud_content_sync_content_changed_cb(cloud_content_sync_content_h content, cloud_content_sync_state_e state, void *user_data)
{
	EVENT_TRACE("Cloud content updated.. state: %d", state);
	cloud_pipe_data data;
	data.type = MP_CLOUD_CONTENT_CHANGED;
	ecore_pipe_write(g_cloud_pipe, &data, sizeof(cloud_pipe_data));
}

static void
_mp_cloud_content_sync_streaming_url_cb(cloud_content_sync_status_e status, char *url, int expiry, int error_code, void *user_data)
{

	cloud_pipe_data data;
	data.type = MP_CLOUD_STREAMING_URL;
	data.data = g_strdup(url);
	data.user_data = user_data;
	ecore_pipe_write(g_cloud_pipe, &data, sizeof(cloud_pipe_data));
}

static void
_mp_cloud_content_sync_status_cb(cloud_content_sync_status_e status, int error_code, void *user_data)
{
	EVENT_TRACE("status: %d, error_code: %d", status, error_code);
}

static void
_mp_cloud_content_sync_content_status_cb(cloud_content_sync_status_e status, char *path, int progress_percent, int error_code, void *user_data)
{
	EVENT_TRACE("status: %d, error_code: %d", status, error_code);
}

void _mp_cloude_pipe_cb(void *user_data, void *buffer, unsigned int nbyte)
{
	cloud_pipe_data *data = buffer;
	MP_CHECK(data);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	switch (data->type) {
	case MP_CLOUD_CONTENT_CHANGED:
		DEBUG_TRACE("content changed");
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_UPDATE);
		break;
	case MP_CLOUD_STREAMING_URL:
		DEBUG_TRACE("streaming url");
		mp_plst_item *current_item = mp_playlist_mgr_get_current(ad->playlist_mgr);
		MP_CHECK(current_item);

		mp_plst_item *req_item = data->user_data;
		MP_CHECK(req_item);

		req_item->cancel_id = 0;
		IF_FREE(req_item->streaming_uri);
		req_item->streaming_uri = data->data;

		if (g_strcmp0(current_item->uid,  req_item->uid)) {
			WARN_TRACE("Requested cloud track is not current track");
			return;
		}

		mp_streaming_mgr_play_new_streaming(ad);
		break;
	default:
		WARN_TRACE("Unhandled type: %d", data->type);
		break;
	}
}

int mp_cloud_create()
{
	if (!g_cloud_handle) {
		WARN_TRACE("Start");

		int res = cloud_content_sync_create(CLOUD_DOMAIN_NAME, &g_cloud_handle);
		MP_CHECK_VAL(res == CLOUD_CONTENT_SYNC_ERROR_NONE, -1);
		MP_CHECK_VAL(g_cloud_handle, -1);

		cloud_content_sync_content_set_changed_cb(g_cloud_handle, CLOUD_CONTENT_SYNC_TYPE_AUDIO, _mp_cloud_content_sync_content_changed_cb, NULL);

		g_cloud_pipe = ecore_pipe_add(_mp_cloude_pipe_cb, NULL);
		WARN_TRACE("End");
	}
	return 0;
}

int mp_cloud_destroy()
{
	WARN_TRACE("Start");
	MP_CHECK_VAL(g_cloud_handle, -1);
	cloud_content_sync_content_unset_changed_cb(g_cloud_handle, CLOUD_CONTENT_SYNC_TYPE_AUDIO);
	cloud_content_sync_destroy(g_cloud_handle);
	WARN_TRACE("End");
	return 0;
}

mp_cloud_playable_e mp_cloud_play_available(const char *uuid, mp_plst_item *plst_item)
{
	DEBUG_TRACE("uuid: %s", uuid);
	MP_CHECK_VAL(uuid, MP_CLOUD_PLAY_UNAVAILABLE);
	int res = 0;
	cloud_content_sync_content_h content_handle = NULL;

	if (!g_cloud_handle)
		mp_cloud_create();

	res = cloud_content_sync_get_content_by_media_uuid(g_cloud_handle, (char *)uuid,  &content_handle);
	if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
		ERROR_TRACE("Fail!!res: 0x%x", res);
		return MP_CLOUD_PLAY_UNAVAILABLE;
	}

	int result = 0;
	res = cloud_content_sync_content_get_is_available_offline(content_handle, &result);
	if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
		ERROR_TRACE("Fail cloud_content_sync_content_get_is_available_offline!!res: 0x%x", res);
		cloud_content_sync_content_destroy(content_handle);
		return MP_CLOUD_PLAY_UNAVAILABLE;
	}

	if (!result) {
		DEBUG_TRACE("offline unavailable content");
		MpHttpState_t state = mp_http_mgr_get_state(mp_util_get_appdata());
		if (state == MP_HTTP_STATE_NONE) {
			WARN_TRACE("Network Unavailable");
			cloud_content_sync_content_destroy(content_handle);
			return MP_CLOUD_PLAY_UNAVAILABLE;
		}

		int wifi_only = _mp_cloud_get_wifi_only();
		if (wifi_only && state == MP_HTTP_STATE_CELLULAR) {
			WARN_TRACE("WIFI only but network is cellular");
			cloud_content_sync_content_destroy(content_handle);
			return MP_CLOUD_PLAY_UNAVAILABLE;
		}
		if (plst_item) {
			unsigned int cancel_id = 0;
			res = cloud_content_sync_content_get_streaming_url(content_handle, &cancel_id, _mp_cloud_content_sync_streaming_url_cb, plst_item, MP_CLOUD_TIMEOUT);
			if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
				ERROR_TRACE("Fail  cloud_content_sync_content_get_streaming_url!!res: 0x%x", res);
				cloud_content_sync_content_destroy(content_handle);
				return MP_CLOUD_PLAY_UNAVAILABLE;
			}
			plst_item->cancel_id = cancel_id;
			cloud_content_sync_content_destroy(content_handle);
			return MP_CLOUD_PLAY_STREAMING;
		}
	} else {
		DEBUG_TRACE("offline available content");
		if (plst_item) {
			char *cache_path = NULL;
			res = cloud_content_sync_content_get_cache_path(content_handle, &cache_path);
			if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
				ERROR_TRACE("Fail  cloud_content_sync_content_get_cache_path!!res: 0x%x", res);
				cloud_content_sync_content_destroy(content_handle);
				return MP_CLOUD_PLAY_UNAVAILABLE;
			}
			plst_item->uri = g_strdup(cache_path);
		}
	}
	cloud_content_sync_content_destroy(content_handle);
	return MP_CLOUD_PLAY_OFFLINE;

}

int mp_cloud_delete_content(const char *uuid, int delete_server)
{
	startfunc;
	int res = 0;
	cloud_content_sync_content_h content_handle = NULL;

	if (!g_cloud_handle)
		mp_cloud_create();

	res = cloud_content_sync_get_content_by_media_uuid(g_cloud_handle, (char *)uuid,  &content_handle);
	if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
		ERROR_TRACE("Fail!!res: 0x%x", res);
		return -1;
	}

	if (delete_server)
		res = cloud_content_sync_content_delete(content_handle, _mp_cloud_content_sync_status_cb, NULL);
	else
		res = cloud_content_sync_content_delete_available_offline(content_handle);

	if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
		ERROR_TRACE("Fail!!delete_server: %d, res: 0x%x", delete_server, res);
	}
	cloud_content_sync_content_destroy(content_handle);

	return res;
}

int mp_cloud_is_on(int *is_on)
{
	WARN_TRACE("Start");
	int res = 0;

	if (!g_cloud_handle)
		mp_cloud_create();

	res = cloud_content_sync_setting_get_type_activation(g_cloud_handle, CLOUD_CONTENT_SYNC_TYPE_AUDIO, is_on);
	if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
		ERROR_TRACE("Fail!!cloud_content_sync_setting_get_type_activation, res: 0x%x", res);
	}
	WARN_TRACE("End");
	return res;
}

int mp_cloud_make_offline_available(const char *uuid)
{
	startfunc;
	int res = 0;
	cloud_content_sync_content_h content_handle = NULL;

	if (!g_cloud_handle)
		mp_cloud_create();

	res = cloud_content_sync_get_content_by_media_uuid(g_cloud_handle, (char *)uuid,  &content_handle);
	if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
		ERROR_TRACE("Fail!!res: 0x%x", res);
		return -1;
	}

	res = cloud_content_sync_content_make_available_offline(content_handle, _mp_cloud_content_sync_content_status_cb, NULL);

	if (res != CLOUD_CONTENT_SYNC_ERROR_NONE) {
		ERROR_TRACE("Fail!! res: 0x%x", res);
	}
	cloud_content_sync_content_destroy(content_handle);
	endfunc;
	return res;
}

int mp_cloud_cancel_request(int cancel_id)
{
	if (!g_cloud_handle)
		return 0;

	cloud_content_sync_cancel(g_cloud_handle, cancel_id);
	return 0;
}

#endif

