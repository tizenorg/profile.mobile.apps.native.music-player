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

#include "mp-playlist-mgr.h"
#include "mp-player-debug.h"
#include "mp-define.h"
#include "mp-util.h"
#include "mp-file-util.h"

#define MP_PLST_LAZY_APPENDER_TEMP_FILE		"/tmp/mp_plst_lazy_appender_temp"
#define MP_PLST_LAZY_APPENDER_MAX_COUNT		(500)

static void _mp_playlist_mgr_reset_lazy_appender(mp_plst_mgr *playlist_mgr);

#ifndef MP_SOUND_PLAYER
#include "mp-setting-ctrl.h"
/* static void __mp_playlist_mgr_remove_list(void); */
#endif

void __mp_playlist_mgr_item_free(void *data)
{
	mp_plst_item *node = (mp_plst_item *)data;
	MP_CHECK(node);
#ifdef MP_FEATURE_PLST_QUEUE
	if (node->is_queue) {
		DEBUG_TRACE("queued item will be remained");
		return;
	}
#endif
	IF_FREE(node->uri);
	IF_FREE(node->uid);
	IF_FREE(node->title);
	IF_FREE(node->artist);

#ifdef MP_FEATURE_CLOUD
	IF_FREE(node->streaming_uri);
	if (node->cancel_id) {
		mp_cloud_cancel_request(node->cancel_id);
	}
#endif
	IF_FREE(node);
}

mp_plst_mgr *mp_playlist_mgr_create(void)
{
	startfunc;
	mp_plst_mgr *playlist_mgr = calloc(1, sizeof(mp_plst_mgr));
	srand((unsigned int)time(NULL));
#ifndef MP_SOUND_PLAYER
	/* 	__mp_playlist_mgr_remove_list(); */
#endif
	endfunc;
	return playlist_mgr;

}

void mp_playlist_mgr_destroy(mp_plst_mgr *playlist_mgr)
{
	startfunc;
	MP_CHECK(playlist_mgr);
	mp_playlist_mgr_clear(playlist_mgr);
	mp_ecore_timer_del(playlist_mgr->save_timer);
	free(playlist_mgr);
	endfunc;
}

static int __mp_playlist_mgr_rand_position(int length, int queue_lenth)
{
	unsigned int seed = (unsigned int)time(NULL);
	unsigned int rand = 0;
	int pos = 0;

	if (length > 0) {
		rand = rand_r(&seed);
		pos =  rand % (length - queue_lenth + 1);
	}

	return pos;
}

static void __mp_playlist_mgr_select_list(mp_plst_mgr *playlist_mgr)
{
	if (playlist_mgr->shuffle_state) {
		playlist_mgr->list = playlist_mgr->shuffle_list;
	} else {
		playlist_mgr->list = playlist_mgr->normal_list;
	}
}

#ifndef MP_SOUND_PLAYER
void
__save_playing_list(mp_plst_mgr *playlist_mgr)
{
	startfunc;

	FILE *fp = NULL;
	mp_plst_item *item = NULL;

	int i;

	fp = fopen(MP_NOWPLAYING_LIST_DATA, "w");

	if (fp == NULL) {
		SECURE_ERROR("Failed to open ini files. : %s", MP_NOWPLAYING_LIST_DATA);
		return;
	}

	for (i = 0; i < mp_playlist_mgr_count(playlist_mgr); i++) {
		item = mp_playlist_mgr_normal_list_get_nth(playlist_mgr, i);
		if (item == NULL) {
			fclose(fp);
			return;
		}
		if (item->title) {
			fprintf(fp, "%s\n", item->title);
		} else {
			fprintf(fp, "%s\n", "");
		}

		if (item->artist) {
			fprintf(fp, "%s\n", item->artist);
		} else {
			fprintf(fp, "%s\n", "");
		}

		fprintf(fp, "%s\n", item->uri);
	}

	fclose(fp);

	endfunc;
}

static Eina_Bool
_playlist_save_timer_cb(void *data)
{
	mp_plst_mgr *playlist_mgr = data;

	if (playlist_mgr->lazy_appender) {
		/* item is still appending by lazy appender; */
		return EINA_TRUE;
	}

	DEBUG_TRACE("save playing list for livebox");
	__save_playing_list(playlist_mgr);

	playlist_mgr->save_timer = NULL;
	return EINA_FALSE;
}

static void
__mp_playlist_mgr_save_list(mp_plst_mgr *playlist_mgr)
{
	if (!playlist_mgr->save_timer) {
		playlist_mgr->save_timer = ecore_timer_add(0.1, _playlist_save_timer_cb, playlist_mgr);
	}
}
#endif

static inline void _mp_playlist_mgr_item_append_common(mp_plst_mgr *playlist_mgr, mp_plst_item *node, int position)
{
	MP_CHECK(playlist_mgr);
	MP_CHECK(node);

	int pos = 0;
	mp_plst_item *cur = NULL;

	if (position > g_list_length(playlist_mgr->normal_list)) {
		position = -1;
	}
	/*insert to normal list*/
	if (position < 0) {
		playlist_mgr->normal_list = g_list_append(playlist_mgr->normal_list, node);
	} else {
		playlist_mgr->normal_list = g_list_insert(playlist_mgr->normal_list, node, position);
	}

	/*insert to shuffle list*/
#ifdef MP_FEATURE_PLST_QUEUE
	int queue_lenth;
	queue_lenth = g_list_length(playlist_mgr->queue_list);
	pos = __mp_playlist_mgr_rand_position(g_list_length(playlist_mgr->normal_list), queue_lenth);

	int queue_start = g_list_index(playlist_mgr->shuffle_list, g_list_nth_data(playlist_mgr->queue_list, 0));
	if (pos >= queue_start - 1) {
		pos += queue_lenth;
	}
#else
	pos = __mp_playlist_mgr_rand_position(g_list_length(playlist_mgr->normal_list), 0);
#endif

	if (playlist_mgr->shuffle_state) {
		cur = mp_playlist_mgr_get_current(playlist_mgr);
	}

	playlist_mgr->shuffle_list = g_list_insert(playlist_mgr->shuffle_list, node, pos);

	if (cur) {
		int index = g_list_index(playlist_mgr->list, cur);
		playlist_mgr->current_index = index;
	}

	/*select list*/
	__mp_playlist_mgr_select_list(playlist_mgr);
#ifndef MP_SOUND_PLAYER
	__mp_playlist_mgr_save_list(playlist_mgr);
#endif
}

static mp_plst_item *_mp_playlist_mgr_create_node(const char *uri, const char *uid, const char *title, const char *artist, mp_track_type type)
{
	MP_CHECK_NULL(uri);

	mp_plst_item *node = NULL;

	/*create data*/
	node = calloc(1, sizeof(mp_plst_item));
	MP_CHECK_NULL(node);

	node->track_type = type;
	node->uid = g_strdup(uid);
	node->uri = g_strdup(uri);
	node->title = g_strdup(title);
	node->artist = g_strdup(artist);

	return node;
}

#ifdef __PRINT_PLAYLIST_ITEMS__
static void
__playlist_print_list(void *data, void *user_data)
{
	mp_plst_item *cur = data;
	WARN_TRACE("%s", cur->uri);
}
#endif

mp_plst_item *mp_playlist_mgr_item_append(mp_plst_mgr *playlist_mgr, const char *uri, const char *uid, const char *title, const char *artist, mp_track_type type)
{
	MP_CHECK_VAL(playlist_mgr, NULL);
	MP_CHECK_VAL(uri, NULL);

	mp_plst_item *node = _mp_playlist_mgr_create_node(uri, uid, title, artist, type);

	_mp_playlist_mgr_item_append_common(playlist_mgr, node, -1);

#ifdef __PRINT_PLAYLIST_ITEMS__
	DEBUG_TRACE("list");
	g_list_foreach(playlist_mgr->list, __playlist_print_list, playlist_mgr->list);
	DEBUG_TRACE("normal list");
	g_list_foreach(playlist_mgr->normal_list, __playlist_print_list, playlist_mgr->normal_list);
	DEBUG_TRACE("shuffle list");
	g_list_foreach(playlist_mgr->shuffle_list, __playlist_print_list, playlist_mgr->shuffle_list);
#endif

	return node;
}

mp_plst_item *mp_playlist_mgr_item_insert(mp_plst_mgr *playlist_mgr, const char *uri, const char *uid, const char *title, const char *artist, mp_track_type type, int index)
{
	MP_CHECK_NULL(playlist_mgr);
	MP_CHECK_NULL(uri);

	mp_plst_item *node = _mp_playlist_mgr_create_node(uri, uid, title, artist, type);

	_mp_playlist_mgr_item_append_common(playlist_mgr, node, index);
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MGR_ITEM_CHANGED);

	return node;
}

void mp_playlist_mgr_item_set_playlist_memeber_id(mp_plst_item *item, int memeber_id)
{
	MP_CHECK(item);
	item->playlist_memeber_id = memeber_id;
}

#ifdef MP_FEATURE_PLST_QUEUE
static GList *__mp_playlist_mgr_delete_queue_link(GList *list)
{
	startfunc;
	mp_plst_item *item;
	GList *new_list, *remove;
	int idx = 0;

	new_list = list;
	remove = g_list_nth(new_list, idx);
	MP_CHECK_NULL(remove);

	while (remove) {
		item = remove->data;

		if (item->is_queue) {
			DEBUG_TRACE("delete : %s", item->uid);
			new_list = g_list_remove_link(new_list, remove);
			g_list_free(remove);
		} else {
			idx++;
		}

		remove = g_list_nth(new_list, idx);
	}

	return new_list;
}

static void __mp_playlist_mgr_index(mp_plst_mgr *playlist_mgr, int *pos, int *shuffle_pos)
{
	MP_CHECK(playlist_mgr);
	int idx, s_idx;
	if (playlist_mgr->shuffle_state) {
		s_idx = playlist_mgr->current_index;
		idx = g_list_index(playlist_mgr->normal_list, g_list_nth_data(playlist_mgr->shuffle_list, s_idx));
	} else {
		idx = playlist_mgr->current_index;
		s_idx = g_list_index(playlist_mgr->shuffle_list, g_list_nth_data(playlist_mgr->normal_list, idx));
	}

	*pos = idx;
	*shuffle_pos = s_idx;
}

static void __mp_playlist_mgr_clear_queue(mp_plst_mgr *playlist_mgr)
{
	startfunc;
	int idx, s_idx;

	MP_CHECK(playlist_mgr);

	__mp_playlist_mgr_index(playlist_mgr, &idx, &s_idx);
	DEBUG_TRACE("idx: %d, s_idx: %d", idx, s_idx);

	playlist_mgr->normal_list = __mp_playlist_mgr_delete_queue_link(playlist_mgr->normal_list);
	playlist_mgr->shuffle_list = __mp_playlist_mgr_delete_queue_link(playlist_mgr->shuffle_list);

	__mp_playlist_mgr_select_list(playlist_mgr);

	endfunc;
}

static void
__mp_playlist_mgr_insert_queue_links(mp_plst_mgr *playlist_mgr)
{
	GList *list;
	int idx, s_idx;

	MP_CHECK(playlist_mgr);

	__mp_playlist_mgr_index(playlist_mgr, &idx, &s_idx);

	list = g_list_last(playlist_mgr->queue_list);
	MP_CHECK(list);

	idx++;
	s_idx++;

	do {
		playlist_mgr->normal_list = g_list_insert(playlist_mgr->normal_list, list->data, idx);
		playlist_mgr->shuffle_list = g_list_insert(playlist_mgr->shuffle_list, list->data, s_idx);
		list = g_list_previous(list);
	} while (list);

	if (playlist_mgr->queue_item_cb) {
		playlist_mgr->queue_item_cb(MP_PLSYLIST_QUEUE_MOVED, playlist_mgr->shuffle_state ? s_idx : idx, playlist_mgr->userdata);
	}
}

mp_plst_item *mp_playlist_mgr_item_queue(mp_plst_mgr *playlist_mgr, const char *uri, const char *uid, mp_track_type type)
{
	DEBUG_TRACE("uri: %s, uid:%s", uri, uid);
	MP_CHECK_VAL(playlist_mgr, NULL);

	mp_plst_item *p_data = NULL;
	GList *last;
	int pos, s_pos;

	if (playlist_mgr->queue_list) {
		last = g_list_last(playlist_mgr->queue_list);
		if (last) {
			p_data = last->data;
		}
	}
	/*create data*/
	mp_plst_item *node = calloc(1, sizeof(mp_plst_item));
	MP_CHECK_VAL(node, NULL);

	if (uri) {
		node->track_type = EINA_TRUE;
		node->uri = g_strdup(uri);
	}
	node->uid = g_strdup(uid);
	node->is_queue = EINA_TRUE;

	/*append item*/
	playlist_mgr->queue_list = g_list_append(playlist_mgr->queue_list, node);

	/*insert queue items to list*/
	if (p_data) {
		pos = g_list_index(playlist_mgr->normal_list, p_data) + 1;
		s_pos = g_list_index(playlist_mgr->shuffle_list, p_data) + 1;
	} else {
		__mp_playlist_mgr_index(playlist_mgr, &pos, &s_pos);
		pos++;
		s_pos++;
	}
	playlist_mgr->normal_list = g_list_insert(playlist_mgr->normal_list, node, pos);
	playlist_mgr->shuffle_list = g_list_insert(playlist_mgr->shuffle_list, node, s_pos);

	/*select list */
	__mp_playlist_mgr_select_list(playlist_mgr);

	if (playlist_mgr->queue_item_cb) {
		playlist_mgr->queue_item_cb(MP_PLAYLIST_QUEUE_ADDED, playlist_mgr->shuffle_state ? s_pos : pos, playlist_mgr->userdata);
	}

	return node;
}
#endif

void mp_playlist_mgr_item_remove_item(mp_plst_mgr *playlist_mgr, mp_plst_item *item)
{
	startfunc;
	GList *remove;
	MP_CHECK(playlist_mgr);
	MP_CHECK(item);

	MP_CHECK(playlist_mgr->shuffle_list);
	MP_CHECK(playlist_mgr->normal_list);

	/*remove from shuffle_list*/
	remove = g_list_find(playlist_mgr->shuffle_list, item);
	MP_CHECK(remove);

	mp_plst_item *cur = mp_playlist_mgr_get_current(playlist_mgr);

	playlist_mgr->shuffle_list = g_list_remove_link(playlist_mgr->shuffle_list, remove);
	g_list_free(remove);

	/*remove from normal_list*/
	remove = g_list_find(playlist_mgr->normal_list, item);
	MP_CHECK(remove);
#ifdef MP_FEATURE_PLST_QUEUE
	item->is_queue = EINA_FALSE;
#endif
	playlist_mgr->normal_list = g_list_remove_link(playlist_mgr->normal_list, remove);
	g_list_free_full(remove, __mp_playlist_mgr_item_free);

#ifdef MP_FEATURE_PLST_QUEUE
	/*remove from queue list*/
	if (playlist_mgr->queue_list) {
		remove = g_list_find(playlist_mgr->queue_list, item);
		if (remove) {
			playlist_mgr->queue_list = g_list_remove_link(playlist_mgr->queue_list, remove);
			g_list_free(remove);
		}
	}
#endif

	/*select list*/
	__mp_playlist_mgr_select_list(playlist_mgr);

	if (cur && cur != item) {
		playlist_mgr->current_index = g_list_index(playlist_mgr->list, cur);
	}

	if (playlist_mgr->current_index < 0 || playlist_mgr->current_index >= mp_playlist_mgr_count(playlist_mgr)) {
		playlist_mgr->current_index = 0;
	}

	if (cur == item) {
		mp_playlist_mgr_set_current(playlist_mgr, mp_playlist_mgr_get_nth(playlist_mgr, playlist_mgr->current_index));
	}
#ifndef MP_SOUND_PLAYER
	__mp_playlist_mgr_save_list(playlist_mgr);

	if (mp_playlist_mgr_count(playlist_mgr) <= 0) {
		mp_view_mgr_post_event(GET_VIEW_MGR, MP_UNSET_NOW_PLAYING);
		mp_setting_remove_now_playing();
	}
#endif
	endfunc;
}

void mp_playlist_mgr_item_remove_deleted_item(mp_plst_mgr *playlist_mgr)
{
	startfunc;
	MP_CHECK(playlist_mgr);
	MP_CHECK(playlist_mgr->list);
	mp_plst_item *item = NULL;

	GList *list = playlist_mgr->list;

	while (list) {
		item = list->data;
		list = g_list_next(list);
		if (item) {
			if (item->uri && item->track_type == MP_TRACK_URI && !mp_util_is_streaming(item->uri)) {
				if (!mp_check_file_exist(item->uri)) {
					SECURE_DEBUG("uri = %s", item->uri);
					mp_playlist_mgr_item_remove_item(playlist_mgr, item);
				}
			}
		}
	}

}

void mp_playlist_mgr_item_remove_nth(mp_plst_mgr *playlist_mgr, int index)
{
	startfunc;
	MP_CHECK(playlist_mgr);
	MP_CHECK(playlist_mgr->list);

	mp_playlist_mgr_item_remove_item(playlist_mgr, mp_playlist_mgr_get_nth(playlist_mgr, index));
	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MGR_ITEM_CHANGED);
	endfunc;
}

void mp_playlist_mgr_clear(mp_plst_mgr *playlist_mgr)
{
	startfunc;
	MP_CHECK(playlist_mgr);

	mp_playlist_mgr_set_playlist_id(playlist_mgr, 0);

	if (playlist_mgr->normal_list) {
		g_list_free_full(playlist_mgr->normal_list,  __mp_playlist_mgr_item_free);
	}
	if (playlist_mgr->shuffle_list) {
		g_list_free(playlist_mgr->shuffle_list);
	}

#ifdef MP_FEATURE_PLST_QUEUE
	if (playlist_mgr->queue_list) {
		playlist_mgr->normal_list = g_list_copy(playlist_mgr->queue_list);
		playlist_mgr->shuffle_list = g_list_copy(playlist_mgr->queue_list);

		__mp_playlist_mgr_select_list(playlist_mgr);
	} else
#endif
	{
		playlist_mgr->normal_list = NULL;
		playlist_mgr->shuffle_list = NULL;
		playlist_mgr->list = NULL;
	}

	mp_playlist_mgr_set_list_type(playlist_mgr, MP_PLST_TYPE_NONE);

	playlist_mgr->current_index = 0;

	if (playlist_mgr->item_change_cb) {
		playlist_mgr->item_change_cb(NULL, playlist_mgr->item_change_userdata);
	}

	if (playlist_mgr->lazy_appender) {
		_mp_playlist_mgr_reset_lazy_appender(playlist_mgr);
	}

#ifndef MP_SOUND_PLAYER
	/* __mp_playlist_mgr_remove_list(); */
#endif
	endfunc;
}

int mp_playlist_mgr_count(mp_plst_mgr *playlist_mgr)
{
	/* startfunc; */
	MP_CHECK_VAL(playlist_mgr, 0);
	MP_CHECK_VAL(playlist_mgr->list, 0);
	return g_list_length(playlist_mgr->list);
}

mp_plst_item *mp_playlist_mgr_get_current(mp_plst_mgr *playlist_mgr)
{
	MP_CHECK_VAL(playlist_mgr, 0);
	mp_plst_item *cur = NULL;

	if (playlist_mgr->list) {
		cur = g_list_nth_data(playlist_mgr->list, playlist_mgr->current_index);
	}

	if (!cur) {
		WARN_TRACE("no current!!!");
		cur = mp_playlist_mgr_get_nth(playlist_mgr, 0);
		mp_playlist_mgr_set_current(playlist_mgr, cur);
	}

	return cur;
}

static void
__mp_playlist_list_foreach(gpointer data, gpointer user_data)
{
	int pos;
	mp_plst_mgr *playlist_mgr = user_data;
	MP_CHECK(playlist_mgr);

	pos = __mp_playlist_mgr_rand_position(g_list_length(playlist_mgr->shuffle_list), 0);
	playlist_mgr->shuffle_list = g_list_insert(playlist_mgr->shuffle_list, data, pos);
}

static void
__mp_playlist_mgr_refresh_shuffle(mp_plst_mgr *playlist_mgr)
{
	DEBUG_TRACE("Shuffle list refreshed!");
	MP_CHECK(playlist_mgr);

	g_list_free(playlist_mgr->shuffle_list);
	playlist_mgr->shuffle_list = NULL;

	g_list_foreach(playlist_mgr->normal_list, __mp_playlist_list_foreach, playlist_mgr);
	playlist_mgr->list = playlist_mgr->shuffle_list;
}

mp_plst_item *mp_playlist_mgr_get_next(mp_plst_mgr *playlist_mgr, Eina_Bool force, Eina_Bool refresh_shuffle)
{
	startfunc;
	MP_CHECK_VAL(playlist_mgr, NULL);
	MP_CHECK_VAL(playlist_mgr->list, NULL);
	int index = 0;
	int count = 0;

	count = mp_playlist_mgr_count(playlist_mgr);

	if (playlist_mgr->repeat_state == MP_PLST_REPEAT_ONE && !force) {
		index = playlist_mgr->current_index;
	} else {
		index = playlist_mgr->current_index + 1;
	}

	if (count <= index) {
		if (playlist_mgr->repeat_state == MP_PLST_REPEAT_ALL || force) {
			if (playlist_mgr->shuffle_state && refresh_shuffle) {
				__mp_playlist_mgr_refresh_shuffle(playlist_mgr);
			}
			index = 0;
		} else {
			return NULL;
		}
	}

	if (index >= count) {
		DEBUG_TRACE("End of playlist");
		index = 0;
	}
	return (mp_plst_item *)g_list_nth_data(playlist_mgr->list, index);
}

mp_plst_item *mp_playlist_mgr_get_prev(mp_plst_mgr *playlist_mgr)
{
	startfunc;
	MP_CHECK_VAL(playlist_mgr, NULL);
	MP_CHECK_VAL(playlist_mgr->list, NULL);
	int index = 0;

	index = playlist_mgr->current_index;
	index--;
	if (index < 0) {
		DEBUG_TRACE("Begin of playlist. ");
		index = mp_playlist_mgr_count(playlist_mgr) - 1;
	}

	return (mp_plst_item *)g_list_nth_data(playlist_mgr->list, index);
}

mp_plst_item *mp_playlist_mgr_get_nth(mp_plst_mgr *playlist_mgr, int index)
{
	/* startfunc; */
	MP_CHECK_VAL(playlist_mgr, NULL);
	MP_CHECK_VAL(playlist_mgr->list, NULL);
	return (mp_plst_item *)g_list_nth_data(playlist_mgr->list, index);
}

mp_plst_item *mp_playlist_mgr_normal_list_get_nth(mp_plst_mgr *playlist_mgr, int index)
{
	/* startfunc; */
	MP_CHECK_VAL(playlist_mgr, NULL);
	MP_CHECK_VAL(playlist_mgr->normal_list, NULL);
	return (mp_plst_item *)g_list_nth_data(playlist_mgr->normal_list, index);
}

mp_plst_item *mp_playlist_mgr_get_item_by_uid(mp_plst_mgr *playlist_mgr, const char *uid)
{
	/* startfunc; */
	MP_CHECK_VAL(playlist_mgr, NULL);
	MP_CHECK_VAL(playlist_mgr->list, NULL);
	MP_CHECK_VAL(uid, NULL);

	GList *current = playlist_mgr->list;
	while (current) {
		mp_plst_item *item = current->data;
		if (item) {
			if (!g_strcmp0(uid, item->uid)) {
				return item;
			}
		}
		current = current->next;
	}

	return NULL;
}

mp_plst_item *mp_playlist_mgr_get_item_by_playlist_memeber_id(mp_plst_mgr *playlist_mgr, int member_id)
{
	MP_CHECK_VAL(playlist_mgr, NULL);
	MP_CHECK_VAL(playlist_mgr->list, NULL);
	MP_CHECK_VAL(member_id, NULL);

	GList *current = playlist_mgr->list;
	while (current) {
		mp_plst_item *item = current->data;
		if (item) {
			if (item->playlist_memeber_id == member_id) {
				return item;
			}
		}
		current = current->next;
	}

	return NULL;
}

void mp_playlist_mgr_set_shuffle_first_item(mp_plst_mgr *playlist_mgr, mp_plst_item *first)
{
	MP_CHECK(playlist_mgr);
	MP_CHECK(playlist_mgr->shuffle_list);
	MP_CHECK(first);

	GList *node = g_list_find(playlist_mgr->shuffle_list, first);
	MP_CHECK(node);
	void *data = node->data;

	playlist_mgr->shuffle_list = g_list_delete_link(playlist_mgr->shuffle_list, node);
	playlist_mgr->shuffle_list = g_list_prepend(playlist_mgr->shuffle_list, data);

	__mp_playlist_mgr_select_list(playlist_mgr);
#ifdef __PRINT_PLAYLIST_ITEMS__
	DEBUG_TRACE("shuffle list");
	g_list_foreach(playlist_mgr->shuffle_list, __playlist_print_list, playlist_mgr->shuffle_list);
#endif
}

void mp_playlist_mgr_set_current(mp_plst_mgr *playlist_mgr, mp_plst_item *cur)
{
	MP_CHECK(playlist_mgr);
	MP_CHECK(playlist_mgr->list);

	int index;

	if (!cur) {
		cur = mp_playlist_mgr_get_nth(playlist_mgr, 0);
	}
	MP_CHECK(cur);
#ifdef __PRINT_PLAYLIST_ITEMS__
	DEBUG_TRACE("normal list");
	g_list_foreach(playlist_mgr->normal_list, __playlist_print_list, playlist_mgr->normal_list);
	DEBUG_TRACE("shuffle list");
	g_list_foreach(playlist_mgr->shuffle_list, __playlist_print_list, playlist_mgr->shuffle_list);
#endif
#ifdef MP_FEATURE_PLST_QUEUE
	bool insert_queue = false;
	/*remove queue item*/
	before = mp_playlist_mgr_get_current(playlist_mgr);
	if (before && before->is_queue) {
		DEBUG_TRACE("queue spent");
		before_index = playlist_mgr->current_index;

		call_remove_item_callback = true;

		mp_playlist_mgr_item_remove_item(playlist_mgr, before);
	}

	/*clear queue item if needed*/
	if (!cur->is_queue && playlist_mgr->queue_list) {
		__mp_playlist_mgr_clear_queue(playlist_mgr);
		insert_queue = true;
	}
#endif
	/*set current*/
	index = g_list_index(playlist_mgr->list, cur);
	if (index < 0) {
		WARN_TRACE("No such item!! cur: %x", cur);
		goto finish;
	}
	playlist_mgr->current_index = index;

	DEBUG_TRACE("cur: %s, index: %d", cur->uri, playlist_mgr->current_index);

#ifdef MP_FEATURE_PLST_QUEUE
	/*insert queue item after cur*/
	if (insert_queue) {
		__mp_playlist_mgr_insert_queue_links(playlist_mgr);
	}
#endif
finish:
#ifdef MP_FEATURE_PLST_QUEUE
	if (call_remove_item_callback && playlist_mgr->queue_item_cb) {
		playlist_mgr->queue_item_cb(MP_PLSYLIST_QUEUE_REMOVED, before_index, playlist_mgr->userdata);
	}
#endif
	if (playlist_mgr->item_change_cb) {
		playlist_mgr->item_change_cb(cur, playlist_mgr->item_change_userdata);
	}

	return;
}

void mp_playlist_mgr_set_shuffle(mp_plst_mgr *playlist_mgr, Eina_Bool shuffle)
{
	DEBUG_TRACE("Shuffle: %d", shuffle);
	MP_CHECK(playlist_mgr);

	playlist_mgr->shuffle_state = shuffle;

	if (playlist_mgr->list) {
		mp_plst_item *cur;
		cur = mp_playlist_mgr_get_current(playlist_mgr);

		__mp_playlist_mgr_select_list(playlist_mgr);

		int index = g_list_index(playlist_mgr->list, cur);
		playlist_mgr->current_index = index;
	}
	endfunc;
}

bool mp_playlist_mgr_get_shuffle(mp_plst_mgr *playlist_mgr)
{
	MP_CHECK_FALSE(playlist_mgr);
	return playlist_mgr->shuffle_state;
}

Eina_Bool mp_playlist_mgr_is_shuffle(mp_plst_mgr *playlist_mgr)
{
	/* startfunc; */
	MP_CHECK_VAL(playlist_mgr, 0);
	return playlist_mgr->shuffle_state;
}

void mp_playlist_mgr_set_repeat(mp_plst_mgr *playlist_mgr, mp_plst_repeat_state repeat)
{
	DEBUG_TRACE("repeat: %d", repeat);
	MP_CHECK(playlist_mgr);
	playlist_mgr->repeat_state = repeat;
}

int mp_playlist_mgr_get_repeat(mp_plst_mgr *playlist_mgr)
{
	startfunc;
	MP_CHECK_VAL(playlist_mgr, 0);

	return playlist_mgr->repeat_state;
	endfunc;
}

#ifdef MP_FEATURE_PLST_QUEUE
Eina_Bool mp_playlist_mgr_set_queue_cb(mp_plst_mgr *playlist_mgr, mp_queue_item_removed_cb cb, void *userdata)
{
	MP_CHECK_VAL(playlist_mgr, 0);

	playlist_mgr->userdata = userdata;
	playlist_mgr->queue_item_cb = cb;

	return true;
}
#endif

int mp_playlist_mgr_set_item_change_callback(mp_plst_mgr *playlist_mgr, mp_playlist_item_change_callback cb, void *userdata)
{
	MP_CHECK_VAL(playlist_mgr, 0);

	playlist_mgr->item_change_userdata = userdata;
	playlist_mgr->item_change_cb = cb;

	return 0;
}

void mp_playlist_mgr_set_list_type(mp_plst_mgr *playlist_mgr, mp_plst_type type)
{
	MP_CHECK(playlist_mgr);

	playlist_mgr->list_type = type;
}

mp_plst_type mp_playlist_mgr_get_list_type(mp_plst_mgr *playlist_mgr)
{
	MP_CHECK_VAL(playlist_mgr, MP_PLST_TYPE_NONE);
	return playlist_mgr->list_type;
}

void mp_playlist_mgr_item_reorder(mp_plst_mgr *playlist_mgr, mp_plst_item *item, int new_index)
{
	MP_CHECK(playlist_mgr);
	MP_CHECK(item);
	MP_CHECK(playlist_mgr->list);
	MP_CHECK(new_index >= 0);

	mp_plst_item *current = mp_playlist_mgr_get_current(playlist_mgr);

	GList *target = g_list_find(playlist_mgr->list, item);
	MP_CHECK(target);
	playlist_mgr->list = g_list_delete_link(playlist_mgr->list, target);
	playlist_mgr->list = g_list_insert(playlist_mgr->list, item, new_index);

	if (current) {
		playlist_mgr->current_index = g_list_index(playlist_mgr->list, current);
	}

	if (playlist_mgr->shuffle_state) {
		playlist_mgr->shuffle_list = playlist_mgr->list;
	} else {
		playlist_mgr->normal_list = playlist_mgr->list;
	}
}

void mp_playlist_mgr_check_existance_and_refresh(mp_plst_mgr *playlist_mgr, bool *current_removed)
{
	MP_CHECK(playlist_mgr);
	MP_CHECK(playlist_mgr->normal_list);

	bool mmc_removed = mp_util_is_mmc_removed();

	mp_plst_item *cur = mp_playlist_mgr_get_current(playlist_mgr);

	GList *full_list = g_list_copy(playlist_mgr->normal_list);
	GList *list = full_list;

	while (list) {
		mp_plst_item *item = list->data;
		if (item) {
			if (item->uri && item->track_type == MP_TRACK_URI && !mp_util_is_streaming(item->uri)) {
				SECURE_DEBUG("uri = %s", item->uri);
				if (!mp_file_exists(item->uri) || (mmc_removed && strstr(item->uri, MP_MMC_ROOT_PATH) == item->uri)) {
					SECURE_DEBUG("removed uri = %s", item->uri);
					mp_playlist_mgr_item_remove_item(playlist_mgr, item);
					if (current_removed && cur == item) {
						WARN_TRACE("current track does NOT exist");
						*current_removed = true;
					}
				}
			}
		}
		list = list->next;
	}

	if (full_list) {
		g_list_free(full_list);
		full_list = NULL;
	}

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_PLAYLIST_MGR_ITEM_CHANGED);
}

void mp_playlist_mgr_set_playlist_id(mp_plst_mgr *playlist_mgr, int playlist_id)
{
	MP_CHECK(playlist_mgr);
	playlist_mgr->playlist_id = playlist_id;
}

int mp_playlist_mgr_get_playlist_id(mp_plst_mgr *playlist_mgr)
{
	MP_CHECK_VAL(playlist_mgr, 0);
	return playlist_mgr->playlist_id;
}

static void _mp_playlist_mgr_reset_lazy_appender(mp_plst_mgr *playlist_mgr)
{
	MP_CHECK(playlist_mgr);
	MP_CHECK(playlist_mgr->lazy_appender);

	mp_plst_lazy_appender_s *appender = playlist_mgr->lazy_appender;

	if (appender->fp) {
		fclose(appender->fp);
		appender->fp = NULL;
	}

	mp_ecore_timer_del(appender->timer);
	mp_file_remove(MP_PLST_LAZY_APPENDER_TEMP_FILE);

	free(playlist_mgr->lazy_appender);
	playlist_mgr->lazy_appender = NULL;
}

static Eina_Bool _mp_playlist_mgr_lazy_appender_timer_cb(void *data)
{
	mp_plst_mgr *playlist_mgr = data;
	MP_CHECK_VAL(playlist_mgr, ECORE_CALLBACK_CANCEL);
	MP_CHECK_VAL(playlist_mgr->lazy_appender, ECORE_CALLBACK_CANCEL);

	mp_plst_lazy_appender_s *appender = playlist_mgr->lazy_appender;
	if (!appender->fp) {
		_mp_playlist_mgr_reset_lazy_appender(playlist_mgr);
		return ECORE_CALLBACK_CANCEL;
	}

	int added = 0;
	char line[MAX_NAM_LEN + 1];

START:
	while (fgets(line, MAX_NAM_LEN, appender->fp)) {
		/* artist */
		if (!fgets(line, MAX_NAM_LEN, appender->fp)) {
			break;
		}

		/* uri */
		if (fgets(line, MAX_NAM_LEN, appender->fp)) {
			line[MAX_NAM_LEN] = 0;
			line[strlen(line) - 1] = 0;
		} else {
			break;
		}

		if (mp_check_file_exist(line)) {
			mp_plst_item *item = mp_playlist_mgr_item_append(playlist_mgr, line, NULL, NULL, NULL, MP_TRACK_URI);
			if (item) {
				++added;
			}
		}

		if (appender->add_remained) {
			appender->skip_count--;
			if (appender->skip_count == 0) {
				break;
			}
		}

		if (added >= MP_PLST_LAZY_APPENDER_MAX_COUNT) {
			WARN_TRACE("renew");
			ecore_timer_interval_set(appender->timer, 0.1);
			return ECORE_CALLBACK_RENEW;
		}
	}

	if (appender->skip_count > 0) {
		appender->add_remained = 1;
		fseek(appender->fp, SEEK_SET, 0);
		goto START;
	}

	WARN_TRACE("lazy appender done");
	_mp_playlist_mgr_reset_lazy_appender(playlist_mgr);

	return ECORE_CALLBACK_DONE;
}

mp_plst_item *mp_playlist_mgr_lazy_append_with_file(mp_plst_mgr *playlist_mgr, const char *list_file, const char *cur_file_path, int start_index)
{
	startfunc;
	MP_CHECK_NULL(playlist_mgr);
	MP_CHECK_NULL(list_file);

	DEBUG_TRACE("cur_file_path[%s] start_index[%d]", cur_file_path, start_index);

	mp_plst_item *cur_item = NULL;

	if (!mp_check_file_exist(cur_file_path)) {
		cur_file_path = NULL;
	}

	_mp_playlist_mgr_reset_lazy_appender(playlist_mgr);

	mp_file_cp(list_file, MP_PLST_LAZY_APPENDER_TEMP_FILE);

	FILE *fp = fopen(MP_PLST_LAZY_APPENDER_TEMP_FILE, "r");
	if (fp) {
		char line[MAX_NAM_LEN + 1];

		int added = 0;
		int index_of_first = -1;
		int index = 0;
		int skip_count = 0;
		int add_remained = 0;

START:
		while (fgets(line, MAX_NAM_LEN, fp)) { /*title */
			/*artist */
			if (!fgets(line, MAX_NAM_LEN, fp)) {
				break;
			}

			/* uri */
			if (fgets(line, MAX_NAM_LEN, fp)) {
				line[MAX_NAM_LEN] = 0;
				line[strlen(line) - 1] = 0;
			} else {
				break;
			}

			/* find first index */
			if (index_of_first == -1 &&   /*first track not found */
			        (g_strcmp0(cur_file_path, line)) && start_index != index) {
				skip_count++;
			} else if (index_of_first < 0) {
				DEBUG_TRACE("Find first track: %d", index);
				index_of_first = index; /*set first index if first index not found */
			}
			index++;
			if (index_of_first < 0) {
				continue;    /* continue to find first */
			}

			/* append item */
			if (mp_check_file_exist(line)) {
				mp_plst_item *item = mp_playlist_mgr_item_append(playlist_mgr, line, NULL, NULL, NULL, MP_TRACK_URI);
				++added;
				if (!cur_item) {
					cur_item = item;
				}
			}

			/* append remaining mode */
			if (add_remained) {
				skip_count--;
				if (skip_count == 0) {
					break;
				}
			}

			/* create lazy appender */
			if (added >= MP_PLST_LAZY_APPENDER_MAX_COUNT) {
				WARN_TRACE("lazy appender started");
				playlist_mgr->lazy_appender = calloc(1, sizeof(mp_plst_lazy_appender_s));
				playlist_mgr->lazy_appender->index_of_first = index_of_first;
				playlist_mgr->lazy_appender->cur_index = index;
				playlist_mgr->lazy_appender->skip_count = skip_count;
				playlist_mgr->lazy_appender->fp = fp;
				playlist_mgr->lazy_appender->add_remained = add_remained;
				playlist_mgr->lazy_appender->timer = ecore_timer_add(0.5, _mp_playlist_mgr_lazy_appender_timer_cb, playlist_mgr);
				goto END;
			}

		}

		/* append remained items */
		if (skip_count > 0) {
			index = 0;				/* reset index; */
			add_remained = 1;		/* append remaining mode */
			index_of_first = 0;		/* set first index as 0 */
			fseek(fp, SEEK_SET, 0);
			goto START;
		}

		fclose(fp);
		fp = NULL;
	}

	mp_file_remove(MP_PLST_LAZY_APPENDER_TEMP_FILE);

	endfunc;
END:
	mp_playlist_mgr_set_current(playlist_mgr, cur_item);
	return cur_item;
}

mp_plst_item *mp_playlist_mgr_custom_item_new(const char *uri)
{
	MP_CHECK_NULL(uri);

	mp_plst_item *item = _mp_playlist_mgr_create_node(uri, NULL, NULL, NULL, MP_TRACK_URI);
	MP_CHECK_NULL(item);
	item->out_of_list = true;
	return item;
}

void mp_playlist_mgr_custom_item_free(mp_plst_item *item)
{
	MP_CHECK(item);

	if (!item->out_of_list) {
		WARN_TRACE("This item is NOT custom item");
		mp_assert(1);
		return ;
	}

	__mp_playlist_mgr_item_free(item);
}

int mp_playlist_mgr_get_index(mp_plst_mgr *playlist_mgr)
{
	MP_CHECK_VAL(playlist_mgr, 0);
	return playlist_mgr->current_index;
}

int mp_playlist_mgr_get_normal_index(mp_plst_mgr *playlist_mgr)
{
	MP_CHECK_VAL(playlist_mgr, 0);
	mp_plst_item *item = mp_playlist_mgr_get_current(playlist_mgr);
	return g_list_index(playlist_mgr->normal_list, item);
}

