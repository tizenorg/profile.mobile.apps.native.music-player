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

#ifdef GBSBUILD
#include <util-func.h>
#endif
#include <dlfcn.h>
#include "music.h"
#include "mp-util.h"
#include "mp-db-client.h"

#define SQL_SAFE_FREE(sql_string)\
if (sql_string) {\
        sqlite3_free(sql_string);\
        sql_string = NULL;\
}

#define MP_SQUARE_ARM_LIB_PATH DATA_PREFIX"/lib/lib_MusicSquare_TIZEN.so"
#define MP_SQUARE_I386_LIB_PATH DATA_PREFIX"/lib/libMusicSquare_Tizen_SL.so"

#define MP_SQUARE_DB_NAME DATA_DIR"/.music_square.db"
#define MP_SQUARE_DB_TABLE_NAME "music_square"

#define MP_SQUARE_DB_RAND_MAX 5
#define MP_SQUARE_DB_QUERY_STR_LEN_MAX 512
#define MP_SQUARE_DB_FILE_PATH_LEN_MAX 512

/* query string */
#define MP_SQUARE_DB_TABLE_EXIST_QUERY_STRING  "select name from sqlite_master WHERE name='%q';"
#define MP_SQUARE_DB_DELETE_TABLE_QUERY_STRING  "DELETE FROM %q ;"
#define MP_SQUARE_DB_DROP_TABLE_QUERY_STRING  "DROP TABLE %q ;"

static int _mp_db_client_disconnect(sqlite3 *handle)
{
	startfunc;

	/* disconnect from database-server */
	if (handle == NULL)
		return 0;

	int ret = 0;

	ret = sqlite3_close(handle);

	if (SQLITE_OK != ret) {
		mp_debug("can not disconnect database\n");
		mp_debug("[sqlite] %s\n", sqlite3_errmsg(handle));

		return MP_SQUARE_DB_ERROR_DISCONNECT;
	}
	handle = NULL;

	mp_debug("Disconnected successfully\n");
	return 0;
}

static int _mp_db_client_busy_handler(void *pData, int count)
{
	startfunc;

	sleep(1);

	mp_debug("_square_db_busy_handler called : %d\n", count);

	return 100 - count;
}

static int _mp_db_client_connect(sqlite3 **handle)
{
	startfunc;

	if (handle == NULL) {
		mp_debug("Invalid input parameter error");
		return MP_SQUARE_DB_ERROR_INVALID_PARAMETER;
	}

	int ret = 0;
	ret = sqlite3_open(MP_SQUARE_DB_NAME, handle);

	if (SQLITE_OK != ret) {
		mp_debug("can not connect to db-server\n");
		if (*handle)
			mp_debug("[sqlite] %s\n", sqlite3_errmsg(*handle));
		*handle = NULL;

		return MP_SQUARE_DB_ERROR_CONNECT;
	}

	/* Register Busy handler */
	MP_CHECK_VAL(*handle, -1);
	ret = sqlite3_busy_handler(*handle, _mp_db_client_busy_handler, NULL);
	if (SQLITE_OK != ret) {
		mp_debug("Fail to register busy handler\n");
		if (*handle)
			mp_debug("[sqlite] %s\n", sqlite3_errmsg(*handle));

		sqlite3_close(*handle);
		*handle = NULL;

		return MP_SQUARE_DB_ERROR_CONNECT;
	}

	mp_debug("connected to db-server\n");

	return 0;
}

static int _mp_db_client_table_exist(mp_dbc_mgr_t *dbc_mgr, const char *table_name)
{
	startfunc;

	MP_CHECK_VAL(dbc_mgr, -1);
	MP_CHECK_VAL(dbc_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(table_name, -1);

	int ret = -1;
	char szTableName[MP_SQUARE_DB_FILE_PATH_LEN_MAX + 1] = { '0', };
	char *pszTempName = NULL;
	sqlite3_stmt *pstStmt_pb = NULL;
	int len = 0;

	sqlite3 *handle = dbc_mgr->sqlite_handle;
	MP_CHECK_VAL(handle, MP_SQUARE_DB_ERROR_INTERNAL);

	char *szQuery = sqlite3_mprintf(MP_SQUARE_DB_TABLE_EXIST_QUERY_STRING, table_name);
	MP_CHECK_VAL(szQuery, MP_SQUARE_DB_ERROR_INTERNAL);
	len = strlen(szQuery);
	ret =
	    sqlite3_prepare_v2(handle, szQuery, len, &pstStmt_pb,
			       NULL);
	if (SQLITE_OK != ret) {
		mp_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
                SQL_SAFE_FREE(szQuery);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	ret = sqlite3_step(pstStmt_pb);
	if (SQLITE_ROW != ret) {
		mp_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		sqlite3_finalize(pstStmt_pb);
                SQL_SAFE_FREE(szQuery);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	pszTempName = (char *)sqlite3_column_text(pstStmt_pb, 0);
	if (NULL != pszTempName) {
		len = strlen(szQuery);
		strncpy(szTableName, pszTempName, len);
	}
	sqlite3_finalize(pstStmt_pb);

 	ret = !(strcmp(table_name, szTableName));

	SQL_SAFE_FREE(szQuery);

	return ret;
}


bool mp_db_client_get_mood_by_path(mp_dbc_mgr_t *dbc_mgr, int *mood, char *path)
{
	MP_CHECK_FALSE(dbc_mgr);
	MP_CHECK_FALSE(dbc_mgr->sqlite_handle);
	MP_CHECK_FALSE(path);

	bool ret = false;

	char *query = g_strdup_printf("SELECT %s FROM music_square WHERE path=\"%s\"", "pos_mood", path);
	MP_CHECK_FALSE(query);

	sqlite3_stmt *stmt = NULL;
	int err = sqlite3_prepare_v2(dbc_mgr->sqlite_handle, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != err) {
		mp_debug("prepare error [%s]\n", sqlite3_errmsg(dbc_mgr->sqlite_handle));
		mp_debug("query string is %s\n", query);

		SAFE_FREE(query);
		return false;
	}

	if (sqlite3_step(stmt) == SQLITE_ROW) {
		*mood = sqlite3_column_int(stmt, 0);
		DEBUG_TRACE("mood: %d   path: %s", *mood, path);
		ret = true;
	}
	sqlite3_finalize(stmt);

	SAFE_FREE(query);
	return ret;
}

int mp_db_client_get_paths_by_mood(mp_dbc_mgr_t *dbc_mgr, int mood)
{
        startfunc;

	MP_CHECK_FALSE(dbc_mgr);
	MP_CHECK_FALSE(dbc_mgr->sqlite_handle);

	bool ret = false;

        dbc_mgr->path_list = NULL;

	char *query = g_strdup_printf("SELECT %s FROM music_square WHERE pos_mood=\"%d\"", "path", mood);
	MP_CHECK_FALSE(query);

	sqlite3_stmt *stmt = NULL;
	int err = sqlite3_prepare_v2(dbc_mgr->sqlite_handle, query, strlen(query), &stmt, NULL);
	if (SQLITE_OK != err) {
		mp_debug("prepare error [%s]\n", sqlite3_errmsg(dbc_mgr->sqlite_handle));
		mp_debug("query string is %s\n", query);

		SAFE_FREE(query);
		return false;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		char *path = (char*)sqlite3_column_text(stmt, 0);
                dbc_mgr->path_list = g_list_append(dbc_mgr->path_list, g_strdup(path));
		ret = true;
	}
	sqlite3_finalize(stmt);

	SAFE_FREE(query);


	return ret;
}


int mp_db_client_mgr_create(struct appdata *ad)
{
        startfunc;

	MP_CHECK_VAL(ad, -1);
	//MP_CHECK_VAL(!ad->dbc_mgr, -1);

	mp_dbc_mgr_t *dbc_mgr = NULL;
        dbc_mgr = calloc(1, sizeof(mp_dbc_mgr_t));
	MP_CHECK_VAL(dbc_mgr, -1);

        ad->dbc_mgr = dbc_mgr;
	//ad->dbc_mgr->ad = ad;

	int ret = 0;
	ret = _mp_db_client_connect(&(ad->dbc_mgr->sqlite_handle));

	if (ret == 0) {
		if (_mp_db_client_table_exist(ad->dbc_mgr, MP_SQUARE_DB_TABLE_NAME) != 1) {
                        ERROR_TRACE("table not exsit");
		}
	}

	return ret;
}

bool mp_db_client_mgr_destory(mp_dbc_mgr_t *dbc_mgr)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_FALSE(dbc_mgr);
	MP_CHECK_FALSE(dbc_mgr->sqlite_handle);

	/* disconnect db */
	if (dbc_mgr->sqlite_handle != NULL)
		_mp_db_client_disconnect(dbc_mgr->sqlite_handle);

        //g_list_free(dbc_mgr->path_list);

	/* free square mgr */
	free(dbc_mgr);
	dbc_mgr = NULL;

	return true;
}


