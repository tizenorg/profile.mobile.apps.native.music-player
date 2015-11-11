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

#ifndef __MP_PLAY_LIST_H_
#define __MP_PLAY_LIST_H_

#include <glib.h>
#include <Eina.h>
#include "mp-define.h"

typedef enum {
	MP_TRACK_URI,
#ifdef MP_FEATURE_CLOUD
	MP_TRACK_CLOUD,
#endif
} mp_track_type;

typedef enum {
	MP_PLST_TYPE_NONE,
	MP_PLST_TYPE_MUSIC_SQUARE,
} mp_plst_type;

typedef enum _mp_plst_repeat_state {
	MP_PLST_REPEAT_ALL,
	MP_PLST_REPEAT_NONE,
	MP_PLST_REPEAT_ONE,
} mp_plst_repeat_state;

#ifdef MP_FEATURE_PLST_QUEUE
typedef enum {
	MP_PLAYLIST_QUEUE_ADDED,
	MP_PLSYLIST_QUEUE_REMOVED,
	MP_PLSYLIST_QUEUE_MOVED,
} mp_playlist_queue_cmd_type;
typedef void (*mp_queue_item_removed_cb)(mp_playlist_queue_cmd_type cmd_type, int index, void *userdata);
#endif

typedef struct {
	FILE *fp;
	Ecore_Timer *timer;
	int index_of_first;
	int cur_index;
	int skip_count;
	int add_remained;
} mp_plst_lazy_appender_s;

typedef struct _mp_list_item {
	mp_track_type track_type;
#ifdef MP_FEATURE_PLST_QUEUE
	Eina_Bool is_queue;
#endif
	char *uri;		//local track uri..
	char *uid;  	//unique id (media_id or allshare item id)
	char *title;
	char *artist;
	char *thumbnail_path;
#ifdef MP_FEATURE_CLOUD
	char *streaming_uri;
	unsigned int cancel_id;
#endif
	int playlist_memeber_id;
	Eina_Bool isDiffAP;
	bool out_of_list;
} mp_plst_item;

typedef void (*mp_playlist_item_change_callback)(mp_plst_item *item, void *userdata);

typedef struct _mp_plst_mgr {
	int current_index;
	Eina_Bool shuffle_state;	//shuffle on/off
	mp_plst_repeat_state repeat_state; //off:0/one:1/all:2
	GList *list;		//normal list do not free, just refer normal_list or shuffle_list
	GList *normal_list;
	GList *shuffle_list;
#ifdef MP_FEATURE_PLST_QUEUE
	GList *queue_list;

	void *userdata;
	void(*queue_item_cb)(mp_playlist_queue_cmd_type cmd_type, int index, void *userdata);
#endif
	void *item_change_userdata;
	mp_playlist_item_change_callback item_change_cb;

	Ecore_Timer *save_timer;

	mp_plst_type list_type;

	int playlist_id;

	mp_plst_lazy_appender_s *lazy_appender;
} mp_plst_mgr;


mp_plst_mgr *mp_playlist_mgr_create(void);
void mp_playlist_mgr_destroy(mp_plst_mgr *playlist_mgr);

mp_plst_item * mp_playlist_mgr_item_append(mp_plst_mgr *playlist_mgr, const char *uri, const char *uid, const char *title, const char *artist, mp_track_type type);
mp_plst_item * mp_playlist_mgr_item_insert(mp_plst_mgr *playlist_mgr, const char *uri, const char *uid, const char *title, const char *artist, mp_track_type type, int index);
void mp_playlist_mgr_item_set_playlist_memeber_id(mp_plst_item *item, int memeber_id);
#ifdef MP_FEATURE_PLST_QUEUE
mp_plst_item * mp_playlist_mgr_item_queue(mp_plst_mgr *playlist_mgr, const char *uri, const char *uid, mp_track_type type);
#endif

void mp_playlist_mgr_item_remove_item(mp_plst_mgr *playlist_mgr, mp_plst_item *item);
void mp_playlist_mgr_item_remove_nth(mp_plst_mgr *playlist_mgr, int index);
void mp_playlist_mgr_clear(mp_plst_mgr *playlist_mgr);

int mp_playlist_mgr_count(mp_plst_mgr *playlist_mgr);
mp_plst_item *mp_playlist_mgr_get_current(mp_plst_mgr *playlist_mgr);
mp_plst_item *mp_playlist_mgr_get_next(mp_plst_mgr *playlist_mgr, Eina_Bool force, Eina_Bool refresh_shuffle);
mp_plst_item *mp_playlist_mgr_get_prev(mp_plst_mgr *playlist_mgr);
mp_plst_item *mp_playlist_mgr_get_nth(mp_plst_mgr *playlist_mgr, int index);
mp_plst_item *mp_playlist_mgr_normal_list_get_nth(mp_plst_mgr *playlist_mgr, int index);
mp_plst_item *mp_playlist_mgr_get_item_by_uid(mp_plst_mgr *playlist_mgr, const char *uid);
mp_plst_item *mp_playlist_mgr_get_item_by_playlist_memeber_id(mp_plst_mgr *playlist_mgr, int member_id);

void mp_playlist_mgr_set_shuffle_first_item(mp_plst_mgr *playlist_mgr, mp_plst_item *first);
void mp_playlist_mgr_set_current(mp_plst_mgr *playlist_mgr, mp_plst_item *cur);

void mp_playlist_mgr_set_shuffle(mp_plst_mgr *playlist_mgr, Eina_Bool shuffle);
bool mp_playlist_mgr_get_shuffle(mp_plst_mgr *playlist_mgr);

Eina_Bool mp_playlist_mgr_is_shuffle(mp_plst_mgr *playlist_mgr);
void mp_playlist_mgr_set_repeat(mp_plst_mgr *playlist_mgr, mp_plst_repeat_state repeat);
int mp_playlist_mgr_get_repeat(mp_plst_mgr *playlist_mgr);
#ifdef MP_FEATURE_PLST_QUEUE
Eina_Bool mp_playlist_mgr_set_queue_cb(mp_plst_mgr* playlist_mgr, mp_queue_item_removed_cb queue_item_removed, void *userdata);
#endif
int mp_playlist_mgr_set_item_change_callback(mp_plst_mgr *playlist_mgr, mp_playlist_item_change_callback cb, void *userdata);

void mp_playlist_mgr_set_list_type(mp_plst_mgr *playlist_mgr, mp_plst_type type);
mp_plst_type mp_playlist_mgr_get_list_type(mp_plst_mgr *playlist_mgr);

void mp_playlist_mgr_item_reorder(mp_plst_mgr *playlist_mgr, mp_plst_item *item, int new_index);
void mp_playlist_mgr_check_existance_and_refresh(mp_plst_mgr *playlist_mgr, bool *current_removed);

void mp_playlist_mgr_set_playlist_id(mp_plst_mgr *playlist_mgr, int playlist_id);
int mp_playlist_mgr_get_playlist_id(mp_plst_mgr *playlist_mgr);

mp_plst_item *mp_playlist_mgr_lazy_append_with_file(mp_plst_mgr *playlist_mgr, const char *list_file, const char *cur_file_path, int start_index);

mp_plst_item *mp_playlist_mgr_custom_item_new(const char *uri);
void mp_playlist_mgr_custom_item_free(mp_plst_item *item);
int mp_playlist_mgr_get_index(mp_plst_mgr *playlist_mgr);
int mp_playlist_mgr_get_normal_index(mp_plst_mgr *playlist_mgr);
#endif




