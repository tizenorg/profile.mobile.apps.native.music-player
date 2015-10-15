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

#ifdef MP_FEATURE_CONTEXT_ENGINE

#include "mp-context.h"
#include <log_manager.h>
#include <context_manager.h>

/* #define TEST_GET_LOG */

#ifdef	TEST_GET_LOG
#include <context_manager.h>
#endif

/* log type */
#define 	LOG_TYPE 	"PLAY_MUSIC"

/* field type */
#define	TIMESTAMP	"Timestamp"
#define	TITLE		"Title"
#define	URI			"URI"
#define	STREAMED	"Streamed"
#define	FAVORITE	"Favorite"
#define	ARTISTS		"Artists"
#define	ALBUM		"Album"
#define	YEAR		"Year"
#define	GENRES		"Genres"
#define	LENGTH		"Length"
#define	START_TIME	"Start_Time"
#define	STOP_TIME	"Stop_Time"
#define	REMOVED	"Removed"



static bool gCaEngineInit;
static int 	gCreateReqId;
static long long gLastLowId;
static bool gTableCreated;


static void
__mp_context_create_table_cb(context_error_e error, int req_id, void *user_data)
{
	gCreateReqId = 0;
	mp_retm_if (error != CONTEXT_ERROR_NONE, "Error: 0x%x", error);

	DEBUG_TRACE("context table created. req_id: %d", req_id);
	gTableCreated = true;
}

#ifdef	TEST_GET_LOG

static void
_get_context_callback(context_error_e error, int req_id, context_data_s *data, int data_size, void *user_data)
{
	int i, j;

	if (error != CONTEXT_ERROR_NONE) {
		DEBUG_TRACE("Getting context failed: %x", error);
		return;
	}

	DEBUG_TRACE("req_id: %d", req_id);
	DEBUG_TRACE("data_size: %d", data_size);

	if (data != NULL) {
		for (i = 0; i < data_size; i++) {
			DEBUG_TRACE("---- [data %d] ----", i+1);
			for (j = 0; j < data[i].array_size; j++) {
				DEBUG_TRACE("[key %d] %s", j+1, data[i].array[j].key);
				DEBUG_TRACE("[value %d] %s", j+1, data[i].array[j].value);
			}
		}
	}
}

static void
_get_log(void)
{
	int r = CONTEXT_ERROR_NONE;
	int req_id = 0;
	char *context_item = "MUSIC_PLAYED_FREQUENTLY";
	context_option_s option;
	context_entry_s array[3];

	array[0].key = "DATA_PROVIDER";
	array[0].value = "q49soer24g";
	array[1].key = "TIME_SPAN";
	array[1].value = "00-03-00";
	array[2].key = "RESULT_SIZE";
	array[2].value = "10";

	option.array = array;
	option.array_size = 3;

	r = context_manager_get_context(context_item, &option, _get_context_callback, NULL, &req_id);
	if (r != CONTEXT_ERROR_NONE)
		DEBUG_TRACE("ERROR: %x", r);
	else
		DEBUG_TRACE("req_id: %d", req_id);
}
#endif

static void
__mp_context_insert_cb(context_error_e error, int req_id, long long row_id, void *user_data)
{
	gLastLowId = row_id;
#ifdef	TEST_GET_LOG
	_get_log();
#endif
}


static void
__mp_context_update_log_cb(context_error_e error, int req_id, void *user_data)
{
	return;
}

static char *
__mp_context_create_sql(struct appdata *ad)
{
	char *sql = NULL;
	mp_track_info_t *track_info = NULL;
	MP_CHECK_NULL(ad);

	track_info = ad->current_track_info;
	MP_CHECK_NULL(track_info);
	MP_CHECK_NULL(track_info->uri);

	sql = sqlite3_mprintf("%Q, %Q, %Q, %Q, %d, %Q, %d, CURRENT_TIMESTAMP",
		track_info->title, track_info->uri, track_info->artist, track_info->album, 0, track_info->genre, track_info->duration/1000
		);

	SECURE_DEBUG("sql: %s", sql);
	return sql;
}

int mp_context_log_connect(void)
{
	startfunc;
	int res = CONTEXT_ERROR_NONE ;
	if (gCaEngineInit) {
		DEBUG_TRACE("Already initialized");
		return 0;
	}

	res = log_manager_connect();
	if (res != CONTEXT_ERROR_NONE) {
		ERROR_TRACE("Error: log_manager_connect. 0x%x", res);
	} else
		gCaEngineInit = true;

	res = context_manager_connect();
	if (res != CONTEXT_ERROR_NONE) {
		ERROR_TRACE("Error: context_manager_connect. 0x%x", res);
	}

	res = log_manager_create_table(LOG_TYPE, __mp_context_create_table_cb, NULL, &gCreateReqId);
	DEBUG_TRACE("log_manager_create_table res: 0x%x, req_id: %d", res, gCreateReqId);

	endfunc;
	return res;
}

int mp_context_log_disconnect(void)
{
	startfunc;
	int res = CONTEXT_ERROR_NONE ;
	if (!gCaEngineInit) {
		DEBUG_TRACE("not initialized");
		return 0;
	}

	res = log_manager_disconnect();
	if (res != CONTEXT_ERROR_NONE) {
		ERROR_TRACE("Error: log_manager_connect. 0x%x", res);
	}

	res = context_manager_disconnect();
	if (res != CONTEXT_ERROR_NONE) {
		ERROR_TRACE("Error: context_manager_disconnect. 0x%x", res);
	}

	gCaEngineInit = false;
	gCreateReqId = 0;

	endfunc;
	return res;
}

int mp_context_log_insert(struct appdata *ad, bool start)
{
	startfunc;
	int res = CONTEXT_ERROR_NONE ;

	MP_CHECK_VAL(ad, -1);

	if (!gCaEngineInit) {
		res = mp_context_log_connect();
		if (res != CONTEXT_ERROR_NONE) {
			ERROR_TRACE("Error: Unable to init ca engine");
			return -1;
		}
	}

	if (!gTableCreated) {
		if (start && !gCreateReqId) {
			res = log_manager_create_table(LOG_TYPE, __mp_context_create_table_cb, ad, &gCreateReqId);
			DEBUG_TRACE("res: 0x%x, req_id: %d", res, gCreateReqId);
		} else
			DEBUG_TRACE("creating table is in progress..");
	} else {
		int id = 0;
		char *sql = NULL, *filter = NULL;

		if (start) {
			sql = __mp_context_create_sql(ad);
			MP_CHECK_VAL(sql, -1);
			res = log_manager_insert_log(LOG_TYPE,
				"TITLE, URI, ARTIST, ALBUM, YEAR, GENRE, LENGTH, START_TIME",
				sql, __mp_context_insert_cb, ad, &id);
		} else {
			sql = sqlite3_mprintf("CURRENT_TIMESTAMP");
			MP_CHECK_VAL(sql, -1);

			filter = sqlite3_mprintf("ID=%lld", gLastLowId);
			if (!filter) {
				free(sql);
				return -1;
			}

			res = log_manager_update_log(LOG_TYPE, "STOP_TIME", sql, filter, __mp_context_update_log_cb, ad, &id);
		}

		if (sql) {
			sqlite3_free(sql);
			sql = NULL;
		}
		if (filter) {
			sqlite3_free(filter);
			filter = NULL;
		}
	}
	endfunc;

	return res;
}

void
mp_context_notify_playback(bool start, const char *uri)
{
	static int playback_id = 0;
	if (start) {
		context_media_info_s media_info = {0,};
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);

		mp_track_info_t *t_info = ad->current_track_info;
		MP_CHECK(t_info);

		media_info.title = t_info->title;
		media_info.album = t_info->album;
		media_info.artist = t_info->artist;
		media_info.genre = t_info->genre;
		/* if (t_info->year)
			media_info.year = atoi(t_info->year); */
		context_manager_notify_audio_playback_started(&media_info, uri, &playback_id);
	} else
		context_manager_notify_audio_playback_stopped(playback_id);
}

#endif
