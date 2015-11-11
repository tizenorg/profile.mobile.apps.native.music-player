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



#ifndef __MP_SQUARE_MGR_H_
#define __MP_SQUARE_MGR_H_

#include <glib.h>
#include <sqlite3.h>
#include "mp-media-info.h"

#include <Elementary.h>


//#include "mp-define.h"
#ifdef PATH_MAX
#	define SQUARE_MAX_NAM_LEN   PATH_MAX
#else
#	define SQUARE_MAX_NAM_LEN   4096
#endif

#define MP_SQUARE_METADATA_LEN_MAX	193

#define MP_SQUARE_MUSIC_FILE_PATH_LEN_MAX 		1024

#define MP_SQUARE_DB_ERROR_CONNECT				-201			/**< connect DB error */
#define MP_SQUARE_DB_ERROR_DISCONNECT 			-202			/**< disconnect DB error  */
#define MP_SQUARE_DB_ERROR_CREATE_TABLE 		-203			/**< create table error */
#define MP_SQUARE_DB_ERROR_NO_RECORD 			-204			/**< No record */
#define MP_SQUARE_DB_ERROR_OUT_OF_RANGE 		-205			/**< DB out of table records range*/
#define MP_SQUARE_DB_ERROR_INTERNAL	 			-206			/**< internal db error  */
#define MP_SQUARE_DB_ERROR_INVALID_PARAMETER 	-1
#define MP_SQUARE_NORMAL_ERROR					-1
#define MP_SQUARE_DB_ERROR_NONE					0

#define MP_SQUARE_AXIS_X_LEN 5
#define MP_SQUARE_AXIS_Y_LEN 5
#define MP_SQUARE_CELLS_COUNT MP_SQUARE_AXIS_X_LEN*MP_SQUARE_AXIS_Y_LEN*4

#define MP_SQUARE_POSITION_TO_INT(position, pos) do { \
		*pos = (int)position;\
	} while (0)

#define MP_SQUARE_INT_TO_POSITION(pos, position) do { \
		*position.x = pos&0xFF;\
		*position.y = pos>>8;\
	} while (0)

#define MP_SQUARE_POSITION_TO_INDEX(position, index) do { \
		*index = (position.y*MP_SQUARE_AXIS_X_LEN - (MP_SQUARE_AXIS_Y_LEN - position.x)) - 1;\
	} while (0)

#define MP_SQUARE_INDEX_TO_POSITION(index, position) do { \
		*position.x = index%MP_SQUARE_AXIS_Y_LEN+1;\
		*position.y = index/MP_SQUARE_AXIS_X_LEN+1;\
	} while (0)

typedef enum {
	MP_SQUARE_TYPE_MOOD,
	MP_SQUARE_TYPE_YEAR,
	MP_SQUARE_TYPE_ADDED,
	MP_SQUARE_TYPE_TIME,
	MP_SQUARE_TYPE_MAX,
} mp_square_type_t;

enum {
	MP_SQUARE_CELL_0,
	MP_SQUARE_CELL_1,
	MP_SQUARE_CELL_2,
	MP_SQUARE_CELL_3,
	MP_SQUARE_CELL_4,
	MP_SQUARE_CELL_MAX
};

typedef struct {
	int x: 8;
	int y: 8;
} mp_square_position_t;

typedef struct {
	char path[MP_SQUARE_MUSIC_FILE_PATH_LEN_MAX + 1];
	mp_square_position_t pos;
	mp_square_type_t type;
} mp_square_item_t;

typedef struct {
	struct appdata *ad;

	sqlite3 *sqlite_handle;
	//void *square_lib_handle;
	//void *smp123_lib_handle;

	Ecore_Timer *create_record_timer;
	int record_count; /* current inserted record count */
	int total_count; /* total record counts */

	GList *list[MP_SQUARE_TYPE_MAX][MP_SQUARE_CELLS_COUNT]; /* mp_square_item_t */

	GList *selected_list_item; /* mp_square_item_info_t */
	int selected_type;

	/*all tracks svc handle*/
	mp_media_list_h svc_handle;
	int media_count;
	bool *added_media_array;
	bool terminal_status;
	int current_index;
	void *record;

	GList *base_list;
	GList *mood_x[MP_SQUARE_CELL_MAX];
	GList *year_x[MP_SQUARE_CELL_MAX];
} mp_square_mgr_t;

typedef struct {
	char audio_id[SQUARE_MAX_NAM_LEN + 1];		/**< Unique ID of item */
	char pathname[SQUARE_MAX_NAM_LEN];			/**< Full path and file name of media file */
	char title[MP_SQUARE_METADATA_LEN_MAX];			/**< title of track */
	char artist[MP_SQUARE_METADATA_LEN_MAX];		/**< artist of track */
	char thumbnail_path[SQUARE_MAX_NAM_LEN];	/**< Thumbnail image file path */
	int duration;									/**< track duration*/
	int rating;										/**< track rating*/
} mp_square_list_item_t;

typedef enum {
	MP_SQUARE_LIST_ITEM_AUDIO_ID,				/**< Unique media file index*/
	MP_SQUARE_LIST_ITEM_PATHNAME,				/**< Full path and file name of media file*/
	MP_SQUARE_LIST_ITEM_THUMBNAIL_PATH,			/**< Thumbnail path of first item in the group */
	MP_SQUARE_LIST_ITEM_TITLE,					/**< Title of media file */
	MP_SQUARE_LIST_ITEM_ARTIST,					/**< Artist of media file */
	MP_SQUARE_LIST_ITEM_DURATION,				/**< Duration of media file*/
	MP_SQUARE_LIST_ITEM_RATING,					/**< The rating used in mtp*/
	MP_SQUARE_LIST_ITEM_ALBUM,					/**< Album of media file*/
} mp_square_list_item_type_t;

bool mp_square_mgr_records_get_by_type_and_positions(
    mp_square_mgr_t *square_mgr,
    mp_square_type_t type,
    GList *list_pos,
    GList **list_record);
bool mp_square_mgr_get_positon_by_type_and_path(
    mp_square_mgr_t *square_mgr,
    mp_square_type_t type,
    const char *path,
    mp_square_position_t *pos);
int mp_square_mgr_create(struct appdata *ad);
bool mp_square_mgr_destory(struct appdata *ad);
int mp_square_mgr_update(struct appdata *ad);
int mp_square_mgr_update_diff_only(struct appdata * ad);
int mp_square_mgr_reset(struct appdata *ad);
int mp_square_mgr_records_count_get(mp_square_mgr_t *square_mgr, int *count);
void mp_square_mgr_selected_list_items_clear(mp_square_mgr_t *square_mgr);
int mp_square_mgr_selected_list_items_get(mp_square_mgr_t *square_mgr, int *type, GList **item_list);

/* for cell list view */
int mp_square_mgr_list_items_count_get(mp_square_mgr_t *square_mgr);
int mp_square_mgr_list_item_new(mp_media_list_h *list_handle, int count);
int mp_square_mgr_list_item_free(mp_media_list_h list_handle);
int mp_square_mgr_list_items_get(mp_square_mgr_t *square_mgr, int count, mp_media_list_h list_handle);
int mp_square_mgr_list_item_get(mp_media_list_h list_handle, int index, mp_media_info_h *item);
int mp_square_mgr_list_item_val_get(mp_media_info_h item_handle, int index, mp_square_list_item_type_t first_field_name, ...);
bool mp_square_mgr_square_item_update_position_by_type(mp_square_mgr_t *square_mgr, mp_square_item_t *item, mp_square_type_t type);

#endif /* __MP_SQUARE_MGR_H_ */
