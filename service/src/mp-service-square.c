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


#include <util-func.h>
#include "mp-media-info.h"
#include "mp-service-square.h"
/*#include "mp-file-tag-info.h" */
#include <dlfcn.h>
#include "mp-file-util.h"

#define MP_SQUARE_ARM_LIB_PATH 
	"/usr/apps/org.tizen.music-player/lib/lib_MusicSquare_TIZEN.so"
#define MP_SQUARE_I386_LIB_PATH 
	"/usr/apps/org.tizen.music-player/lib/libMusicSquare_Tizen_SL.so"

#define MP_SQUARE_DB_NAME DATA_DIR"/.music_square.db"
#define MP_SQUARE_DB_TABLE_NAME "music_square"

/* query string */
#define MP_SQUARE_DB_TABLE_EXIST_QUERY_STRING  
	"select name from sqlite_master WHERE name='%q';"
#define MP_SQUARE_DB_DELETE_TABLE_QUERY_STRING  "DELETE FROM %q ;"
#define MP_SQUARE_DB_DROP_TABLE_QUERY_STRING  "DROP TABLE %q ;"

#define MP_SQUARE_DB_TABLE_RECORD_INSERT "INSERT INTO music_square (path, \
	"score_excite, score_cheerful, score_violent, year, title) VALUES \
	"('%q', %f, %f, %f, %d, '%q');"

#define MP_SQUARE_DB_TABLE_RECORD_COUNT_ALL 
	"select count(*) FROM music_square;"

#define MP_SQUARE_DB_RAND_MAX 5
#define MP_SQUARE_DB_QUERY_STR_LEN_MAX 512
#define MP_SQUARE_DB_FILE_PATH_LEN_MAX 512

#define SQL_SAFE_FREE(sql_string)
		if (sql_string) sqlite3_free(sql_string)

typedef struct {
	int id;
	char path[MP_SQUARE_MUSIC_FILE_PATH_LEN_MAX+1]; /* music path */
	mp_square_position_t pos_mood;
	mp_square_position_t pos_year;
	mp_square_position_t pos_added;
	mp_square_position_t pos_time;
	char title[MP_SQUARE_MUSIC_FILE_PATH_LEN_MAX+1]; /* music title */
	int year;
	time_t added_time;
	time_t played_time;
	double scores[3];
} mp_square_record_t;

static int _mp_app_control_square_mgr_db_sql_query(mp_square_mgr_t 
	*square_mgr, const char *sql_str);
static int _mp_app_control_square_mgr_db_table_create(mp_square_mgr_t 
	*square_mgrconst, const char *table_name);
static int _mp_app_control_square_mgr_db_table_exist(mp_square_mgr_t 
	*square_mgrconst, const char *table_name);
static int _mp_app_control_square_mgr_db_table_drop(mp_square_mgr_t 
	*square_mgrconst, const char *table_name);
static int _mp_app_control_square_mgr_db_clear(mp_square_mgr_t *square_mgr);
static int _mp_app_control_square_mgr_db_busy_handler(void *pData, int count);
static int _mp_app_control_square_mgr_db_disconnect(sqlite3 *handle);
static int _mp_app_control_square_mgr_db_connect(sqlite3 **handle);
static int _mp_app_control_square_mgr_db_table_record_insert(
	mp_square_mgr_t *square_mgr, mp_square_record_t *record);
static int _mp_app_control_square_mgr_db_table_media_append(
	mp_square_mgr_t *square_mgrconst);
static int _mp_app_control_square_mgr_MpWitchMoodEngineInit(
	mp_square_mgr_t *square_mgr);
static double* _mp_app_control_square_mgr_MpWitchMoodEngineEXE(
	mp_square_mgr_t *square_mgr, const char *path);
static int _mp_app_control_square_mgr_MpWitchMoodEngineRelease
	(mp_square_mgr_t *square_mgr);

int WitchMoodEngineInit(void)
{
}
int WitchMoodEngineRelease(void)
{
}
double *WitchMoodEngineEXE(const char *path)
{
}

static int _mp_app_control_square_mgr_MpWitchMoodEngineInit(
		mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();
	int ret = WitchMoodEngineInit();
	return ret;
}

static double* _mp_app_control_square_mgr_MpWitchMoodEngineEXE(
	mp_square_mgr_t *square_mgr, const char *path)
{
	/* DEBUG_TRACE_FUNC(); */
	return WitchMoodEngineEXE((char *)path);
}

static int _mp_app_control_square_mgr_MpWitchMoodEngineRelease
	(mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();
	return WitchMoodEngineRelease();
}

static int _mp_app_control_square_mgr_db_sql_query(mp_square_mgr_t *square_mgr
	, const char *sql_str)
{
	/* DEBUG_TRACE_FUNC(); */
	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(sql_str, -1);

	int err = -1;
	char *zErrMsg = NULL;

	/* mp_debug("SQL = [%s]", sql_str); */

	err = sqlite3_exec(square_mgr->sqlite_handle, sql_str, NULL, 
		NULL, &zErrMsg);

	if (SQLITE_OK != err) {
		mp_error("failed to execute [%s], err[%d]", zErrMsg, err);
	}

	if (zErrMsg)
		sqlite3_free(zErrMsg);

	return err;
}

static int _mp_app_control_square_mgr_db_table_create(mp_square_mgr_t 
		*square_mgr, const char *table_name)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(table_name, -1);

	int err = -1;
	MP_CHECK_VAL(table_name, MP_SQUARE_DB_ERROR_INVALID_PARAMETER);

	char *sql = sqlite3_mprintf("create table if not exists %s (\
			_id
				INTEGER PRIMARY KEY AUTOINCREMENT,\
			path			TEXT NOT NULL,\
			pos_mood		INTEGER,\
			pos_year		INTEGER,\
			pos_added		INTEGER,\
			pos_time		INTEGER,\
			score_excite	DOUBLE,\
			score_cheerful	DOUBLE,\
			score_violent	DOUBLE,\
			year			INTEGER,\
			title			TEXT NOT NULL\
			);"
			, table_name);
	err = _mp_app_control_square_mgr_db_sql_query(square_mgr, sql);
	sqlite3_free(sql);
	if (err != SQLITE_OK) {
		mp_error("It failed to create square table (%d)", err);
		if (err == MP_SQUARE_DB_ERROR_CONNECT) {
			return err;
		}
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	return MP_SQUARE_DB_ERROR_NONE;
}

static int _mp_app_control_square_mgr_db_table_exist(
	mp_square_mgr_t *square_mgr, const char *table_name)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(table_name, -1);

	int ret = -1;
	char szTableName[MP_SQUARE_DB_FILE_PATH_LEN_MAX + 1] = { '0', };
	char *pszTempName = NULL;
	sqlite3_stmt *pstStmt_pb = NULL;
	int len = 0;

	sqlite3 *handle = square_mgr->sqlite_handle;
	MP_CHECK_VAL(handle, MP_SQUARE_DB_ERROR_INTERNAL);

	char *szQuery = sqlite3_mprintf(MP_SQUARE_DB_TABLE_EXIST_QUERY_STRING, 
		table_name);
	MP_CHECK_VAL(szQuery, MP_SQUARE_DB_ERROR_INTERNAL);
	len = strlen(szQuery);
	ret =
	    sqlite3_prepare_v2(handle, szQuery, len, &pstStmt_pb,
			       NULL);
	SQL_SAFE_FREE(szQuery);
	if (SQLITE_OK != ret) {
		mp_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mp_debug("query string is %s\n", szQuery);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	ret = sqlite3_step(pstStmt_pb);
	if (SQLITE_ROW != ret) {
		mp_debug("end of row [%s]\n", sqlite3_errmsg(handle));
		mp_debug("query string is %s\n", szQuery);
		sqlite3_finalize(pstStmt_pb);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	pszTempName = (char *)sqlite3_column_text(pstStmt_pb, 0);
	if (NULL != pszTempName) {
		len = strlen(szQuery);
		strncpy(szTableName, pszTempName, len);
	}
	sqlite3_finalize(pstStmt_pb);

	ret = !(strcmp(table_name, szTableName));

	return ret;
}

static int _mp_app_control_square_mgr_db_table_drop(mp_square_mgr_t 
	*square_mgr, const char *table_name)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(table_name, -1);

	int err;

	mp_debug("_table_drop--enter\n");

	err = _mp_app_control_square_mgr_db_table_exist(square_mgr, table_name);
	if (err > 0) {
		char *query_string = sqlite3_mprintf(
			MP_SQUARE_DB_DROP_TABLE_QUERY_STRING, table_name);
		MP_CHECK_VAL(query_string, MP_SQUARE_NORMAL_ERROR);
		err = _mp_app_control_square_mgr_db_sql_query(
			square_mgr, query_string);
		SQL_SAFE_FREE(query_string);
		if (err < 0) {
			mp_debug("truncate table failed\n");
			mp_debug("query string is %s\n",
				query_string);

			return MP_SQUARE_DB_ERROR_INTERNAL;
		}
	}

	mp_debug("_table_drop--leave\n");
	return MP_SQUARE_DB_ERROR_NONE;
}

static int _mp_app_control_square_mgr_db_clear(mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();

	int ret = 0;

	ret = _mp_app_control_square_mgr_db_table_drop(square_mgr, 
		MP_SQUARE_DB_TABLE_NAME);

	return ret;
}

static int _mp_app_control_square_mgr_db_busy_handler(void *pData, int count)
{
	DEBUG_TRACE_FUNC();

	usleep(50000);

	mp_debug("_square_db_busy_handler called : %d\n", count);

	return 100 - count;
}

static int _mp_app_control_square_mgr_db_disconnect(sqlite3 *handle)
{
	DEBUG_TRACE_FUNC();

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

static int _mp_app_control_square_mgr_db_connect(sqlite3 **handle)
{
	DEBUG_TRACE_FUNC();

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
	ret = sqlite3_busy_handler(*handle, 
		_mp_app_control_square_mgr_db_busy_handler, NULL);
	if (SQLITE_OK != ret) {
		mp_debug("Fail to register busy handler\n");
		if (*handle)
			mp_debug("[sqlite] %s\n", sqlite3_errmsg(*handle));

		sqlite3_close(*handle);
		if (SQLITE_OK != ret) {
			mp_debug("can not disconnect database\n");
			return MP_SQUARE_DB_ERROR_DISCONNECT;
		}
		*handle = NULL;

		return MP_SQUARE_DB_ERROR_CONNECT;
	}

	mp_debug("connected to db-server\n");

	return 0;
}

static int _mp_app_control_square_mgr_db_table_record_insert(mp_square_mgr_t 
		*square_mgr, mp_square_record_t *record)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(record, -1);

	if (record == NULL) {
		ERROR_TRACE("record pointer is null");
		return MP_SQUARE_DB_ERROR_INVALID_PARAMETER;
	}

	int err = -1;
	char *query_string = NULL;

	query_string =
		sqlite3_mprintf(MP_SQUARE_DB_TABLE_RECORD_INSERT,
			record->path,
			record->scores[0],
			record->scores[1],
			record->scores[2],
			record->year,
			record->title);

	SECURE_DEBUG("[%s]", query_string);

	err = _mp_app_control_square_mgr_db_sql_query(square_mgr, query_string);
	sqlite3_free(query_string);

	if (err < 0) {
		mp_debug("Inserting record failed\n");
		mp_debug("query string is %s\n", query_string);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	return err;
}

static gint _mp_app_control_square_mgr_record_excite_score_sort_func(
	gconstpointer a, gconstpointer b)
{
	if (!a || !b)
		return 1;

	mp_square_record_t *a_record = (mp_square_record_t *)a;
	mp_square_record_t *b_record = (mp_square_record_t *)b;

	return (gint)((b_record->scores[0] - a_record->scores[0]) * 100);
}


static int _mp_app_control_square_mgr_db_load(mp_square_mgr_t *square_mgr)
{
	startfunc;

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);

	int err = -1;

	int count = 0;
	err = mp_app_control_square_mgr_records_count_get(square_mgr, &count);
	MP_CHECK_VAL(count, -1);
	DEBUG_TRACE("record count = %d", count);

	sqlite3 *handle = square_mgr->sqlite_handle;
	sqlite3_stmt *stmt = NULL;

	const char *q_string = 
		"SELECT _id, score_excite, score_cheerful, score_violent, \
		year FROM music_square";
	err = sqlite3_prepare_v2(handle, q_string, 
		strlen(q_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mp_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mp_debug("query string is %s\n", q_string);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	while (sqlite3_step(stmt) == SQLITE_ROW) {
		mp_square_record_t *record = calloc(1, 
			sizeof(mp_square_record_t));
		record->id = (int)sqlite3_column_int(stmt, 0);
		record->scores[0] = (double)sqlite3_column_double(stmt, 1);
		record->scores[1] = (double)sqlite3_column_double(stmt, 2);
		record->scores[2] = (double)sqlite3_column_double(stmt, 3);
		record->year = (int)sqlite3_column_int(stmt, 4);

		DEBUG_TRACE("[%d] %f, %f, %f, %d", record->id, record->
			scores[0], record->scores[1], record->scores[2], 
			record->year);
		square_mgr->base_list = g_list_insert_sorted(square_mgr->
			base_list, record, _mp_app_control_square_mgr_record_
			excite_score_sort_func);
	}

	sqlite3_finalize(stmt);
	endfunc;
	return 0;
}

static inline int _get_cell_num(int *cell, int index)
{
	int i = 0;
	int j = 0;
	while (i < 5) {
		j += cell[i];
		/* DEBUG_TRACE("i = %d, j= %d index = %d", i, j, index); */
		if (j > index)
			return i;

		++i;
	}

	return 0;
}

static int _mp_app_control_square_mgr_db_update(mp_square_mgr_t *square_mgr)
{
	startfunc;

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(square_mgr->base_list, -1);

	GList *list = square_mgr->base_list;

	int ret = 0;
	ret = _mp_app_control_square_mgr_db_sql_query(square_mgr, "BEGIN");
	if (ret) {
		mp_error("transaction BEGIN failed");
		return ret;
	}

	int cell[MP_SQUARE_CELL_MAX][MP_SQUARE_CELL_MAX];
	int i = 0;
	int j = 0;
	for (i = 0; i < MP_SQUARE_CELL_MAX; i++) {
		int total = g_list_length(square_mgr->mood_x[i]);
		int item_per_cell = total / MP_SQUARE_CELL_MAX;
		int remain = total % MP_SQUARE_CELL_MAX;

		for (j = 0; j < MP_SQUARE_CELL_MAX; j++) {
			cell[i][j] = item_per_cell;
			if (j < remain)
				cell[i][j]++;

			/* DEBUG_TRACE("sell[%d][%d] = [%d]", i, j, 
					cell[i][j]); */
		}
	}

	for (i = 0; i < g_list_length(list) ; i++) {
		mp_square_record_t *record = g_list_nth_data(list, i);
		int y = record->pos_mood.y;
		record->pos_mood.y = y + 1;
		record->pos_mood.x = _get_cell_num(cell[y], 
			g_list_index(square_mgr->mood_x[y], record)) + 1;

		record->pos_year.y = y + 1;
		record->pos_year.x = _get_cell_num(cell[y], g_list_index(
			square_mgr->year_x[y], record)) + 1;

		char *query = sqlite3_mprintf(
		"UPDATE music_square SET pos_mood=%d, pos_year=%d WHERE _id=%d"
		, record->pos_mood, record->pos_year, record->id);
		DEBUG_TRACE("MOOD[%d, %d] => %f", record->pos_mood.x, 
			record->pos_mood.y, record->scores[0]);
		DEBUG_TRACE("YEAR[%d, %d] => %d", record->pos_mood.x, 
			record->pos_mood.y, record->year);
		ret = _mp_app_control_square_mgr_db_sql_query
			(square_mgr, query);
		SQL_SAFE_FREE(query);
		if (ret) {
			mp_error("Inserting record failed");
			_mp_app_control_square_mgr_db_sql_query
				(square_mgr, "ROLLBACK");
			return ret;
		}
	}

	ret = _mp_app_control_square_mgr_db_sql_query(square_mgr, "COMMIT");
	if (ret) {
		mp_error("transaction COMMIT failed");
		return ret;
	}

	endfunc;
	return 0;
}

static int _mp_app_control_square_mgr_db_update_track_less_than_25
	(mp_square_mgr_t *square_mgr)
{
	startfunc;

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(square_mgr->base_list, -1);

	GList *list = square_mgr->base_list;

	int ret = 0;
	ret = _mp_app_control_square_mgr_db_sql_query(square_mgr, "BEGIN");
	if (ret) {
		mp_error("transaction BEGIN failed");
		return ret;
	}

	int i = 0;

	for (i = 0; i < g_list_length(list) ; i++) {
		mp_square_record_t *record = g_list_nth_data(list, i);
		record->pos_mood.y = MP_SQUARE_CELL_MAX - i%MP_SQUARE_CELL_MAX;
		record->pos_mood.x = MP_SQUARE_CELL_MAX - 
			(int)(i/MP_SQUARE_CELL_MAX);

		record->pos_year.y = MP_SQUARE_CELL_MAX - i%MP_SQUARE_CELL_MAX;
		record->pos_year.x = MP_SQUARE_CELL_MAX - 
			(int)(i/MP_SQUARE_CELL_MAX);

		char *query = sqlite3_mprintf(
		"UPDATE music_square SET pos_mood=%d, pos_year=%d WHERE _id=%d"
		, record->pos_mood, record->pos_year, record->id);
		DEBUG_TRACE("MOOD[%d, %d] => %f", record->pos_mood.x, 
			record->pos_mood.y, record->scores[0]);
		DEBUG_TRACE("YEAR[%d, %d] => %d", record->pos_mood.x, 
			record->pos_mood.y, record->year);
		ret = _mp_app_control_square_mgr_db_sql_query(
			square_mgr, query);
		SQL_SAFE_FREE(query);
		if (ret) {
			mp_error("Inserting record failed");
			_mp_app_control_square_mgr_db_sql_query(
				square_mgr, "ROLLBACK");
			return ret;
		}
	}

	ret = _mp_app_control_square_mgr_db_sql_query(square_mgr, "COMMIT");
	if (ret) {
		mp_error("transaction COMMIT failed");
		return ret;
	}

	endfunc;
	return 0;
}

static gint _mp_app_control_square_mgr_mood_sort_func(gconstpointer a, 
		gconstpointer b)
{
	if (!a || !b)
		return 1;

	mp_square_record_t *a_record = (mp_square_record_t *)a;
	mp_square_record_t *b_record = (mp_square_record_t *)b;

	double a_mood = a_record->scores[1] - a_record->scores[2];
	double b_mood = b_record->scores[1] - b_record->scores[2];

	return (gint)((b_mood - a_mood) * 100);
}

static gint _mp_app_control_square_mgr_year_sort_func(gconstpointer a, 
		gconstpointer b)
{
	if (!a || !b)
		return 1;

	mp_square_record_t *a_record = (mp_square_record_t *)a;
	mp_square_record_t *b_record = (mp_square_record_t *)b;

	return (a_record->year - b_record->year);
}

static void
_mp_app_control_square_mgr_position_recalculate(mp_square_mgr_t *square_mgr)
{
	MP_CHECK(square_mgr);
	GList *list = square_mgr->base_list;
	MP_CHECK(list);

	int total_count = g_list_length(list);

	int i = 0;
	int item_per_cell = total_count / MP_SQUARE_CELL_MAX;
	int current_item_count = 0;
	int remain = total_count % MP_SQUARE_CELL_MAX;

	int cell = 0;
	for (i = 0; i < total_count; i++) {
		mp_square_record_t *record = g_list_nth_data(list, i);
		record->pos_mood.y = cell;

		square_mgr->mood_x[cell] = g_list_insert_sorted(
			square_mgr->mood_x[cell], record, 
			_mp_app_control_square_mgr_mood_sort_func);
		square_mgr->year_x[cell] = g_list_insert_sorted(
			square_mgr->year_x[cell], record, 
			_mp_app_control_square_mgr_year_sort_func);
		++current_item_count;

		if (current_item_count == item_per_cell) {
			if (remain > 0) {
				++i;
				record = g_list_nth_data(list, i);
				record->pos_mood.y = cell;
				square_mgr->mood_x[cell] = g_list_insert_sorted
				(square_mgr->mood_x[cell], record, 
				_mp_app_control_square_mgr_mood_sort_func);
				square_mgr->year_x[cell] = g_list_insert_sorted
				(square_mgr->year_x[cell], record, 
				_mp_app_control_square_mgr_year_sort_func);
				--remain;
			}

			++cell;
			current_item_count = 0;
		}
	}
}

static void _mp_app_control_square_mgr_db_table_records_update(
		mp_square_mgr_t *square_mgr)
{
	startfunc;
	MP_CHECK(square_mgr);

	int i = 0;

	/* load all data from DB */
	_mp_app_control_square_mgr_db_load(square_mgr);
	MP_CHECK(square_mgr->base_list);

	if (square_mgr->media_count >= 25) {
		/* recalculate pos */
		_mp_app_control_square_mgr_position_recalculate(square_mgr);

		/* update db */
		_mp_app_control_square_mgr_db_update(square_mgr);

		/* free list data */
		for (i = 0; i < MP_SQUARE_CELL_MAX; i++) {
			if (square_mgr->mood_x[i]) {
				g_list_free(square_mgr->mood_x[i]);
				square_mgr->mood_x[i] = NULL;
			}

			if (square_mgr->year_x[i]) {
				g_list_free(square_mgr->year_x[i]);
				square_mgr->year_x[i] = NULL;
			}
		}
	} else {
		_mp_app_control_square_mgr_db_update_track_less_than_25(
			square_mgr);
	}

	for (i = 0; i < g_list_length(square_mgr->base_list); i++) {
		mp_square_record_t *record = g_list_nth_data(square_mgr->
			base_list, i);
		IF_FREE(record);
	}

	g_list_free(square_mgr->base_list);
	square_mgr->base_list = NULL;



	endfunc;
}

static void _mp_app_control_square_mgr_db_table_records_append_timer_cb
	(void *data)
{
	DEBUG_TRACE_FUNC();

	TIMER_TRACE();
	mp_square_mgr_t *square_mgr = data;
	MP_CHECK(square_mgr);

	int index = square_mgr->current_index;
	int count = square_mgr->media_count;
	bool exit_flag = true;

	ERROR_TRACE("COUNT:%d", count);
	while (exit_flag) {
		ERROR_TRACE("go");
		if (square_mgr->current_index < count && square_mgr->
				total_count && !square_mgr->terminal_status) {
			ERROR_TRACE("id: %d", square_mgr->current_index);
			mp_square_record_t *record = (mp_square_record_t *)
				square_mgr->record;
			MP_CHECK(record);

			if (square_mgr->added_media_array[square_mgr->
					current_index] == false) {
				/* mp_debug("index=%d,count=%d\n", 
						index, count); */
				char *path = NULL;
				char *title = NULL;
				char *fid = NULL;
				char *year = NULL;
				time_t added_time;
				memset(&added_time, 0, sizeof(time_t));
				time_t played_time;
				memset(&played_time, 0, sizeof(time_t));

				mp_media_info_h audio_handle = NULL;
				audio_handle = mp_media_info_list_nth_item(
					square_mgr->svc_handle, 
					square_mgr->current_index);
				mp_media_info_get_file_path(audio_handle, 
					&path);
				MP_CHECK(path);

				mp_media_info_get_title(audio_handle, &title);
				mp_media_info_get_media_id(audio_handle, &fid);
				mp_media_info_get_year(audio_handle, &year);
				mp_media_info_get_added_time(audio_handle, 
					&added_time);
				mp_media_info_get_played_time(audio_handle, 
					&played_time);
				DEBUG_TRACE("year=%s", year);
				/* DEBUG_TRACE("added_time=%ld", added_time);
				DEBUG_TRACE("played_time=%ld", played_time);
				DEBUG_TRACE("path=%s", path); */

				if (path != NULL)
					strncpy(record[square_mgr->record_
						count].path, path, sizeof(
						record[square_mgr->record_
						count].path)-1);
				if (title != NULL)
					strncpy(record[square_mgr->record_
						count].title, title, sizeof(
						record[square_mgr->record_
						count].title)-1);
				if (year != NULL)
					/* strncpy(record[square_mgr->record_
						count].year, year, sizeof(
						record[square_mgr->record_
						count].year)-1); */
					record[square_mgr->record_count].year 
						= atoi(year);
				record[square_mgr->record_count].added_time = 
					added_time;
				record[square_mgr->record_count].added_time = 
					played_time;

				double *scores = NULL;
				struct stat s_stat;
				int result = stat(path, &s_stat);
				if (result == -1) {
					ERROR_TRACE("stat:error=%d", result);
				}

				if (mp_file_exists(path) && result != -1 && 
						s_stat.st_size > 0) {
					scores = _mp_app_control_square_mgr_
						MpWitchMoodEngineEXE(
						square_mgr, path);
					/* mp_debug("scores=%p", scores); */
					if (scores) {
						/* mp_debug("scores[0]=%lf,
								scores[1]=%lf,
								scores[2]=%lf"
								, scores[0], 
								scores[1], 
								scores[2]); */
						record[square_mgr->record_
							count].scores[0] = 
							scores[0];
						record[square_mgr->record_
							count].scores[1] = 
							scores[1];
						record[square_mgr->record
							_count].scores[2] = 
							scores[2];
					}
				}

				_mp_app_control_square_mgr_db_table_record_
					insert(square_mgr, 
					&record[square_mgr->record_count]);
				square_mgr->record_count++;
			}
			square_mgr->current_index++;
		} else {
			ERROR_TRACE("else");
			square_mgr->terminal_status = false;
			IF_FREE(square_mgr->record);

			square_mgr->current_index = -1;
			square_mgr->record_count = -1;
			square_mgr->total_count = -1;

			if (square_mgr->svc_handle)
				mp_media_info_list_destroy
					(square_mgr->svc_handle);
			square_mgr->svc_handle = NULL;

			SAFE_FREE(square_mgr->added_media_array);

		/* release music square engine */
			_mp_app_control_square_mgr_
				MpWitchMoodEngineRelease(square_mgr);

		/* update db */
			_mp_app_control_square_mgr_db_table_records_update
				(square_mgr);

			/* update square list 
			_mp_app_control_square_mgr_list_destroy(square_mgr);
			_mp_app_control_square_mgr_list_create(square_mgr); */

			exit_flag = false;

			}
		}

	}

static int _mp_app_control_square_mgr_db_table_media_append
	(mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->svc_handle, -1);

	int ret = 0;
	int count = 0;
	int index = 0;

	if (_mp_app_control_square_mgr_db_table_exist(square_mgr, 
			MP_SQUARE_DB_TABLE_NAME) != 1) {
		if (_mp_app_control_square_mgr_db_table_create(square_mgr, 
				MP_SQUARE_DB_TABLE_NAME) != 
					MP_SQUARE_DB_ERROR_NONE)
			return -1;
	}

	while (index < square_mgr->media_count) {
		if (square_mgr->added_media_array[index] == false)
			++count;

		++index;
	}
	WARN_TRACE("Not added media count = %d", count);

	mp_square_record_t *record = calloc(count, sizeof(mp_square_record_t));
	MP_CHECK_VAL(record, -1);
	IF_FREE(square_mgr->record);
	square_mgr->record = record;

	square_mgr->total_count = count;
	square_mgr->record_count = 0;
	square_mgr->current_index = 0;

	if (square_mgr->total_count == 0) {
		IF_FREE(square_mgr->record);
	} else {
		/* init music square engine */
		ret = _mp_app_control_square_mgr_MpWitchMoodEngineInit
			(square_mgr);
	}

	ERROR_TRACE("Add timer");
	_mp_app_control_square_mgr_db_table_records_append_timer_cb(square_mgr);
	/* square_mgr->create_record_timer = ecore_timer_add(0.005, 
		_mp_app_control_square_mgr_db_table_records_append_timer_cb, 
		square_mgr);*/

	return ret;
}

int _mp_app_control_square_mgr_db_table_records_list_get(
			mp_square_mgr_t *square_mgr,
			const char *q_string,
			GList **list_records)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(list_records, -1);
	MP_CHECK_VAL(q_string, -1);

	sqlite3 *handle = square_mgr->sqlite_handle;
	sqlite3_stmt *stmt = NULL;
	int err = -1;

	err =
	    sqlite3_prepare_v2(handle, q_string, strlen(q_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mp_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mp_debug("query string is %s\n", q_string);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	int i = 0;
	int position = 0;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		mp_square_item_t *item = calloc(1, sizeof(mp_square_item_t));

		char *path = (char *)sqlite3_column_text(stmt, 0);
		mp_debug("====path=%s====", path);
		if (!mp_check_file_exist(path)) {
			IF_FREE(item);
			continue;
		}
		if (path != NULL)
			strncpy(item->path, path, 
	MP_SQUARE_MUSIC_FILE_PATH_LEN_MAX);

		position = sqlite3_column_int(stmt, 1);
		item->pos.x = position&0xFF;
		item->pos.y = position>>8;

		*list_records = g_list_append(*list_records, item);
/*
		char *title = (char*)sqlite3_column_text(stmt, 2);
		mp_debug("====title=%s====", title);
*/
		i++;
	}

	sqlite3_finalize(stmt);

	return 0;
}



int mp_app_control_square_mgr_records_count_get(mp_square_mgr_t *square_mgr, 
	int *count)
{
	/* DEBUG_TRACE_FUNC();*/

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);

	sqlite3 *handle = square_mgr->sqlite_handle;
	sqlite3_stmt *stmt = NULL;

	int err = -1;
	char query[MP_SQUARE_DB_QUERY_STR_LEN_MAX] = {0};

	snprintf(query, sizeof(query), MP_SQUARE_DB_TABLE_RECORD_COUNT_ALL);

	err = sqlite3_prepare_v2(handle, query, -1, &stmt, NULL);
	if (err != SQLITE_OK) {
		mp_error("prepare error [%s]", sqlite3_errmsg(handle));
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	/* mp_debug("[SQL query] : %s", query);*/

	err = sqlite3_step(stmt);
	if (err != SQLITE_ROW) {
		mp_error("end of row [%s]", sqlite3_errmsg(handle));
		sqlite3_finalize(stmt);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}
	*count = sqlite3_column_int(stmt, 0);

	sqlite3_finalize(stmt);

	return MP_SQUARE_DB_ERROR_NONE;
}

int mp_app_control_square_mgr_reset(mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_FALSE(square_mgr);
	MP_CHECK_FALSE(square_mgr->sqlite_handle);

	int ret = 0;

	/* clear square db */
	ret = _mp_app_control_square_mgr_db_clear(square_mgr);
	if (ret != MP_SQUARE_DB_ERROR_NONE) {
		return ret;
	}

	return 0;
}

int mp_app_control_square_mgr_update(mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_FALSE(square_mgr);
	MP_CHECK_FALSE(square_mgr->sqlite_handle);

	int ret = 0;

	/* update square db */
	ret = _mp_app_control_square_mgr_db_clear(square_mgr);
	if (ret != MP_SQUARE_DB_ERROR_NONE) {
		return ret;
	}

	ret = _mp_app_control_square_mgr_db_table_create
		(square_mgr, MP_SQUARE_DB_TABLE_NAME);
	if (ret != MP_SQUARE_DB_ERROR_NONE) {
		return ret;
	}

	int count = 0;
	ret = mp_media_info_list_count(MP_TRACK_ALL, 
		NULL, NULL, NULL, 0, &count);
	MP_CHECK_VAL(ret == 0, -1);
	MP_CHECK_VAL(count > 0, -1);
	mp_debug("track_count: %d", count);

	if (square_mgr->svc_handle)
		mp_media_info_list_destroy(square_mgr->svc_handle);
	square_mgr->svc_handle = NULL;
	ret = mp_media_info_list_create(&square_mgr->svc_handle, MP_TRACK_ALL, 
		NULL, NULL, NULL, 0, 0, count);
	MP_CHECK_VAL(ret == 0, -1);

	square_mgr->media_count = count;
	SAFE_FREE(square_mgr->added_media_array);
	square_mgr->added_media_array = calloc(count, sizeof(bool));
	mp_assert(square_mgr->added_media_array);

	ret = _mp_app_control_square_mgr_db_table_media_append(square_mgr);

	return ret;
}

static int
_mp_app_control_square_mgr_find_path_from_media_list(mp_media_list_h media_list
	, int count, bool *added_media_array, const char *finding_path)
{
	MP_CHECK_VAL(media_list, -1);
	MP_CHECK_VAL(count > 0, -1);
	MP_CHECK_VAL(added_media_array, -1);
	MP_CHECK_VAL(finding_path, -1);

	int index = 0;
	mp_media_info_h media = NULL;
	char *path = NULL;
	while (index < count) {
		if (added_media_array[index] == false) {
			media = mp_media_info_list_nth_item(media_list, index);
			if (media) {
				mp_media_info_get_file_path(media, &path);
				if (!g_strcmp0(path, finding_path)) {
					/*mp_debug("index of [%s] is [%d]", 
					path, index);*/
					return index;
				}
			}
		}

		++index;
	}

	return -1;
}

static void
_mp_app_control_square_mgr_compare_list(mp_square_mgr_t *square_mgr, 
	GList *path_list, bool *record_check_list)
{
	startfunc;
	MP_CHECK(square_mgr);

	mp_media_list_h media_list = square_mgr->svc_handle;
	MP_CHECK(media_list);

	bool *added_media_array = square_mgr->added_media_array;
	MP_CHECK(added_media_array);

	int media_count = square_mgr->media_count;
	MP_CHECK(media_count > 0);

	MP_CHECK(path_list);
	MP_CHECK(record_check_list);

	int record_index = 0;
	int media_index = -1;
	int record_count = g_list_length(path_list);
	while (record_index < record_count) {
		char *path = g_list_nth_data(path_list, record_index);
		if (path) {
			media_index = _mp_app_control_square_mgr_find_path_from
				_media_list(media_list, media_count, 
					added_media_array, path);
			if (media_index >= 0) {
				added_media_array[media_index] = true;
				record_check_list[record_index] = true;
			}
		}

		++record_index;
	}
}

static void
_mp_app_control_square_mgr_remove_record_from_square_db(mp_square_mgr_t 
	*square_mgr, GList *path_list, bool *exist_path_list_array)
{
	startfunc;
	MP_CHECK(square_mgr);
	MP_CHECK(path_list);
	MP_CHECK(exist_path_list_array);

	int index = 0;
	int record_count = g_list_length(path_list);
	int max_rec = (record_count > 20) ? 20 : record_count;

	while (index < record_count) {
		int  loop_cnt = 0;
		char *where = NULL;
		char *path = NULL;
		while (loop_cnt < max_rec && index < record_count) {
			if (exist_path_list_array[index] == false) {
				path = g_list_nth_data(path_list, index);
				if (path && strlen(path)) {
					mp_debug("remove path : %s, index %d", 
						path, index);
					char *cond = sqlite3_mprintf(
						"path=\'%q\'", path);
					if (where) {
						char *old = where;
						where = sqlite3_mprintf(
							"%s OR %s", 
							old, cond);
						sqlite3_free(cond);
						sqlite3_free(old);
					} else {
						where = cond;
					}
					loop_cnt++;
				}
			}

			++index;

		}

		if (where == NULL) {
			mp_debug("no remove record");
			return;
		}

		char *query_string = sqlite3_mprintf(
			"DELETE FROM music_square WHERE ( %s )", where);
		sqlite3_free(where);
		mp_debug("Query : %s", query_string);
		_mp_app_control_square_mgr_db_sql_query(square_mgr, 
			query_string);
		sqlite3_free(query_string);
	}
	endfunc;
}

int _mp_app_control_square_mgr_db_table_path_list_get(
	mp_square_mgr_t *square_mgr, GList **path_list)
{
	startfunc;

	MP_CHECK_VAL(square_mgr, -1);
	MP_CHECK_VAL(square_mgr->sqlite_handle, -1);
	MP_CHECK_VAL(path_list, -1);

	sqlite3 *handle = square_mgr->sqlite_handle;
	sqlite3_stmt *stmt = NULL;
	int err = -1;

	const char *q_string = "SELECT path FROM music_square";
	err = sqlite3_prepare_v2
	(handle, q_string, strlen(q_string), &stmt, NULL);
	if (SQLITE_OK != err) {
		mp_debug("prepare error [%s]\n", sqlite3_errmsg(handle));
		mp_debug("query string is %s\n", q_string);
		return MP_SQUARE_DB_ERROR_INTERNAL;
	}

	int i = 0;
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		char *path = (char *)sqlite3_column_text(stmt, 0);
		if (path) {
			mp_debug("[path : %s]", path);
			*path_list = g_list_append(*path_list, g_strdup(path));
		}
		i++;
	}

	sqlite3_finalize(stmt);
	endfunc;
	return 0;
}


int mp_app_control_square_mgr_update_diff_only(mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_FALSE(square_mgr);
	MP_CHECK_FALSE(square_mgr->sqlite_handle);

	int ret = 0;

	ret = mp_media_info_connect();
	MP_CHECK_VAL(ret == 0, -1);

	GList *path_list = NULL;
	int path_count = 0;
	bool *exist_path_list_array = NULL;

	if (_mp_app_control_square_mgr_db_table_exist
			(square_mgr, MP_SQUARE_DB_TABLE_NAME) != 1) {
		if (_mp_app_control_square_mgr_db_table_create
				(square_mgr, MP_SQUARE_DB_TABLE_NAME) != 
					MP_SQUARE_DB_ERROR_NONE) {
			ret = -1;
			goto exception;
		}
	}

	if (square_mgr->svc_handle) {
		mp_media_info_list_destroy(square_mgr->svc_handle);
		square_mgr->svc_handle = NULL;
	}
	SAFE_FREE(square_mgr->added_media_array);
	square_mgr->media_count = 0;

	/* get media list */
	ret = mp_media_info_list_count(
	MP_TRACK_ALL, NULL, NULL, NULL, 0, &square_mgr->media_count);
	MP_CHECK_VAL(ret == 0, -1);

	mp_debug("track_count: %d", square_mgr->media_count);
	if (square_mgr->media_count > 0) {
		square_mgr->added_media_array = 
	calloc(square_mgr->media_count, sizeof(bool));
		mp_assert(square_mgr->added_media_array);

		ret = mp_media_info_list_create(&square_mgr->svc_handle, 
	MP_TRACK_ALL, NULL, NULL, NULL, 0, 0, square_mgr->media_count);
		if (ret != 0) {
			mp_error("mp_media_info_list_create().. [0x%x]", ret);
			goto exception;
		}
	} else {
		_mp_app_control_square_mgr_db_clear(square_mgr);
		return -1;
	}

	/* get list from Square DB */
	if (_mp_app_control_square_mgr_db_table_path_list_get
	(square_mgr, &path_list) != 0) {
		mp_error(
	"_mp_app_control_square_mgr_db_table_records_list_get()");
		ret = -1;
		goto exception;
	}

	path_count = g_list_length(path_list);
	mp_debug("square DB count = %d", path_count);
	if (path_count > 0) {
		exist_path_list_array = calloc(path_count, sizeof(bool));
		mp_assert(exist_path_list_array);
	}

	/* compare */
	_mp_app_control_square_mgr_compare_list(square_mgr, 
	path_list, exist_path_list_array);

	/* remove records from Sqaure DB */
	_mp_app_control_square_mgr_remove_record_from_square_db(square_mgr, 
	path_list, exist_path_list_array);
	SAFE_FREE(exist_path_list_array);
	GList *temp = path_list;
	while (temp) {
		char *path = temp->data;
		SAFE_FREE(path);
		temp = temp->next;
	}
	g_list_free(path_list);
	path_list = NULL;

	/* append new media to Sqaure DB */
	ret = _mp_app_control_square_mgr_db_table_media_append(square_mgr);
	if (ret != 0) {
		mp_error(
	"_mp_app_control_square_mgr_db_table_media_append() ... [0x%x]", ret);
		goto exception;
	}

	mp_media_info_disconnect();

	return 0;

exception:
	if (square_mgr->svc_handle) {
		mp_media_info_list_destroy(square_mgr->svc_handle);
		square_mgr->svc_handle = NULL;
	}
	square_mgr->media_count = 0;
	SAFE_FREE(square_mgr->added_media_array);
	SAFE_FREE(exist_path_list_array);

	return ret;
}

bool mp_app_control_square_mgr_destory(mp_square_mgr_t *square_mgr)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_FALSE(square_mgr);
	MP_CHECK_FALSE(square_mgr->sqlite_handle);

	if (square_mgr->svc_handle) {
		mp_media_info_list_destroy(square_mgr->svc_handle);
		square_mgr->svc_handle = NULL;
	}
	SAFE_FREE(square_mgr->added_media_array);

	/* disconnect db */
	if (square_mgr->sqlite_handle != NULL)
		_mp_app_control_square_mgr_db_disconnect(
	square_mgr->sqlite_handle);

	/* free square mgr */
	free(square_mgr);
	square_mgr = NULL;

	return true;
}

mp_square_mgr_t *mp_app_control_square_mgr_create(void)
{
	startfunc;
	DEBUG_TRACE_FUNC();

	mp_square_mgr_t *square_mgr = NULL;
	square_mgr = calloc(1, sizeof(mp_square_mgr_t));
	MP_CHECK_NULL(square_mgr);
	square_mgr->record_count = -1;
	square_mgr->total_count = -1;

	int ret = 0;

	ret = _mp_app_control_square_mgr_db_connect(&square_mgr->sqlite_handle);

	if (ret == 0) {
		if (_mp_app_control_square_mgr_db_table_exist(square_mgr, 
	MP_SQUARE_DB_TABLE_NAME) != 1) {
			ERROR_TRACE("Db not exist");
		}
	}

	return square_mgr;
}

