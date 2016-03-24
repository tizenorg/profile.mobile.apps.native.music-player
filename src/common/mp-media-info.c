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


#include <glib.h>
#include <stdlib.h>
#include <string.h>
#include <media_content.h>
#include <media_content_type.h>
#include <sqlite3.h>

#include "mp-media-info.h"
#include "mp-player-debug.h"
#include "mp-define.h"

#include <app_control.h>
#include <mp-file-util.h>

#define PRINT_STR(s)	//DEBUG_TRACE("%s", s);
#define PRINT_INT(i)	//DEBUG_TRACE("%d", i);

#ifdef MP_FEATURE_CLOUD
#define MP_MEDIA_COND_COMMON		""
#else
#define MP_MEDIA_COND_COMMON		"(MEDIA_STORAGE_TYPE IS NOT 101)"		// exclude dropbox
#endif

#ifdef UG_MUSIC_PLAYER
#define MP_MEDIA_TYPE 				MP_MEDIA_COND_COMMON"AND ((MEDIA_TYPE=3) OR (MEDIA_TYPE=2))"
#else
#define MP_MEDIA_TYPE 				MP_MEDIA_COND_COMMON"AND (MEDIA_TYPE=3)"
#endif
#define MP_SOUND_TYPE 				MP_MEDIA_COND_COMMON"AND (MEDIA_TYPE=2)"

struct mp_media_list_s {
	GList *list;
	int count;
	mp_group_type_e group_type;
	char *distinct_check;
};

struct mp_minfo_s {
	char *media_id;
	char *title;
	char *artist;
	char *album;
	char *genre;
	char *date;
	char *composer;
	char *year;
	char *copyright;
	char *track_num;
	char *format;
	char *file_path;
	char *thumbnail_path;
	char *display_name;
	int playlist_member_id;
};

struct mp_ginfo_s {
	char *main_info;
	char *sub_info;
	char *thumb_path;
	int	track_count;

	//for artist list
	char **album_thumb_paths;
	int album_count;
};

struct mp_media_info_s {
	union {
		media_info_h media;	//media_info_h
		void *group;			//handle for group item like media_playlist_h, media_album_h, media_folder_h...
	} h;

	union {
		audio_meta_h meta;	//audio_meta_h for a media
		mp_group_type_e group_type;
	} s;

	union {
		struct mp_minfo_s *minfo;	//media info
		struct mp_ginfo_s *ginfo;	//group info
	} i;
};

#define STRNCAT_LEN(dest) (sizeof(dest)-1-strlen(dest))

static void _mp_media_info_sql_strncat(char *buf, const char *query, int size)
{
	char *sql = sqlite3_mprintf("%q", query);
	SECURE_DEBUG("sql: %s", sql);
	strncat(buf, sql, size);
	sqlite3_free(sql);
}

static inline int _mp_media_info_compare_cb(const void *a , const void *b)
{
	mp_media_info_h media_info_a = (mp_media_info_h)a;
	mp_media_info_h media_info_b = (mp_media_info_h)b;
	char *s_a = NULL, *s_b = NULL;
	int n_a = 0, n_b = 0, res = 0;

	mp_media_info_get_track_num(media_info_a, &s_a);
	mp_media_info_get_track_num(media_info_b, &s_b);
	if (s_a == NULL && s_b == NULL) {
		return 0;
	} else if (s_a == NULL) {
		return 1;
	} else if (s_b == NULL) {
		return -1;
	}

	n_a = atoi(s_a);
	n_b = atoi(s_b);

	if (n_a < n_b) {
		res =  1;
	} else if (n_a > n_b)	{
		res =  -1;
	}

	DEBUG_TRACE("a: %d, b: %d, res: %d", n_a, n_b, res);

	return res;
}

static inline bool __mp_media_info_of_album_cb(media_info_h media, void *user_data)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	mp_media_list_h media_list = user_data;
	mp_media_info_h media_info = NULL;
	media_info_h m = NULL;
	MP_CHECK_FALSE(media_list);

	res = media_info_clone(&m, media);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, true);
	MP_CHECK_VAL(m, true);

	media_info = calloc(1, sizeof(struct mp_media_info_s));
	if (!media_info) {
		media_info_destroy(m);
		return false;
	}

	media_info->i.minfo = calloc(1, sizeof(struct mp_minfo_s));
	if (!media_info->i.minfo) {
		media_info_destroy(m);
		IF_FREE(media_info);
		return false;
	}

	res = media_info_get_audio(m, &media_info->s.meta);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		media_info_destroy(m);
		mp_media_info_destroy(media_info);
		return true;
	}

	media_info->h.media = m;
	media_list->list = g_list_insert_sorted(media_list->list, media_info, _mp_media_info_compare_cb);

	return true;
}


static inline bool __mp_media_info_cb(media_info_h media, void *user_data)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	mp_media_list_h media_list = user_data;
	mp_media_info_h media_info = NULL;
	media_info_h m = NULL;
	MP_CHECK_FALSE(media_list);

	res = media_info_clone(&m, media);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, true);
	MP_CHECK_VAL(m, true);

	media_info = calloc(1, sizeof(struct mp_media_info_s));
	if (!media_info) {
		media_info_destroy(m);
		return false;
	}

	media_info->i.minfo = calloc(1, sizeof(struct mp_minfo_s));
	if (!media_info->i.minfo) {
		free(media_info);
		media_info_destroy(m);
		return false;
	}

	media_info->h.media = m;
	media_list->list = g_list_prepend(media_list->list, media_info);

	res = media_info_get_audio(m, &media_info->s.meta);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, true);

	return true;
}

static inline bool __mp_playlist_media_info_cb(int playlist_member_id, media_info_h media, void *user_data)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	mp_media_list_h media_list = user_data;
	mp_media_info_h media_info = NULL;
	media_info_h m = NULL;
	MP_CHECK_FALSE(media_list);

	res = media_info_clone(&m, media);
	MP_CHECK_FALSE(res == MEDIA_CONTENT_ERROR_NONE);
	MP_CHECK_FALSE(m);

	media_info = calloc(1, sizeof(struct mp_media_info_s));
	if (!media_info) {
		media_info_destroy(m);
		return false;
	}

	media_info->i.minfo = calloc(1, sizeof(struct mp_minfo_s));
	if (!media_info->i.minfo) {
		media_info_destroy(m);
		SAFE_FREE(media_info);
		return false;
	}
	media_info->i.minfo->playlist_member_id = playlist_member_id;

	media_info->h.media = m;
	media_list->list = g_list_prepend(media_list->list, media_info);

	res = media_info_get_audio(m, &media_info->s.meta);
	MP_CHECK_FALSE(res == MEDIA_CONTENT_ERROR_NONE);

	return true;
}

static inline bool __mp_media_album_cb(media_album_h album, void *user_data)
{
	mp_media_info_h media_info = NULL;
	mp_media_list_h media_list = user_data;
	MP_CHECK_FALSE(media_list);

	char *album_name = NULL;
	media_album_get_name(album, &album_name);

	if (!g_strcmp0(media_list->distinct_check, album_name)) {
		IF_FREE(album_name);
		return true;
	}

	IF_FREE(media_list->distinct_check);
	media_list->distinct_check = album_name;

	media_info = calloc(1, sizeof(struct mp_media_info_s));
	MP_CHECK_FALSE(media_info);

	media_info->i.ginfo = calloc(1, sizeof(struct mp_ginfo_s));
	if (!media_info->i.ginfo) {
		SAFE_FREE(media_info);
		return false;
	}

	media_album_clone((media_album_h *)&media_info->h.group, album);
	media_info->s.group_type = media_list->group_type;

	media_list->list = g_list_prepend(media_list->list, media_info);

	return true;
}

static inline bool __mp_media_folder_cb(media_folder_h folder, void *user_data)
{
	mp_media_info_h media_info = NULL;
	mp_media_list_h media_list = user_data;
	MP_CHECK_FALSE(media_list);

	media_info = calloc(1, sizeof(struct mp_media_info_s));
	MP_CHECK_FALSE(media_info);

	media_info->i.ginfo = calloc(1, sizeof(struct mp_ginfo_s));
	if (!media_info->i.ginfo) {
		SAFE_FREE(media_info);
		return false;
	}

	media_folder_clone((media_folder_h *)&media_info->h.group, folder);
	media_info->s.group_type = media_list->group_type;

	media_list->list = g_list_prepend(media_list->list, media_info);

	return true;
}

static inline bool __mp_media_playlist_cb(media_playlist_h playlist, void *user_data)
{
	mp_media_info_h media_info = NULL;
	mp_media_list_h media_list = user_data;
	MP_CHECK_FALSE(media_list);

	media_info = calloc(1, sizeof(struct mp_media_info_s));
	MP_CHECK_FALSE(media_info);

	media_info->i.ginfo = calloc(1, sizeof(struct mp_ginfo_s));
	if (!media_info->i.ginfo) {
		SAFE_FREE(media_info);
		return false;
	}

	media_playlist_clone((media_playlist_h *)&media_info->h.group, playlist);
	media_info->s.group_type = media_list->group_type;

	media_list->list = g_list_prepend(media_list->list, media_info);

	char *path = NULL;
	media_playlist_get_thumbnail_path(playlist, &path);
	if (path) {
		media_info->i.ginfo->thumb_path = g_strdup(path);
	}
	IF_FREE(path);

	return true;
}

static inline bool __mp_media_group_cb(const char* name, void *user_data)
{
	mp_media_info_h media_info = NULL;
	mp_media_list_h media_list = user_data;
	MP_CHECK_FALSE(media_list);

	media_info = calloc(1, sizeof(struct mp_media_info_s));
	MP_CHECK_FALSE(media_info);

	media_info->i.ginfo = calloc(1, sizeof(struct mp_ginfo_s));
	if (!media_info->i.ginfo) {
		SAFE_FREE(media_info);
		return false;
	}

	media_info->h.group = g_strdup(name);
	media_info->s.group_type = media_list->group_type;

	media_list->list = g_list_prepend(media_list->list, media_info);

	return true;
}

static void __mp_media_info_destory(void *data)
{
	mp_media_info_destroy(data);
}

static void __mp_media_group_destory(void *data)
{
	mp_media_info_h media_info = data;
	MP_CHECK(media_info);

	if (media_info->s.group_type == MP_GROUP_BY_FOLDER) {
		media_folder_destroy(media_info->h.group);
	} else if (media_info->s.group_type == MP_GROUP_BY_ALBUM) {
		media_album_destroy(media_info->h.group);
	} else if (media_info->s.group_type == MP_GROUP_BY_PLAYLIST) {
		media_playlist_destroy(media_info->h.group);
	} else {
		IF_FREE(media_info->h.group);
	}

	if (media_info->i.ginfo) {
		IF_FREE(media_info->i.ginfo->main_info);
		IF_FREE(media_info->i.ginfo->sub_info);
		IF_FREE(media_info->i.ginfo->thumb_path);

		while (media_info->i.ginfo->album_count) {
			media_info->i.ginfo->album_count--;
			IF_FREE(media_info->i.ginfo->album_thumb_paths[media_info->i.ginfo->album_count]);
		}
		IF_FREE(media_info->i.ginfo->album_thumb_paths);

		free(media_info->i.ginfo);
	}

	free(media_info);
}


static void __mp_media_info_set_group_filter(mp_filter_h filter, mp_group_type_e group_type, const char *type_string, const char *filter_string)
{
	char cond[MAX_FILTER_LEN] = {0,};
	if (group_type != MP_GROUP_BY_PLAYLIST) {
		strncat(cond, MP_MEDIA_TYPE, STRNCAT_LEN(cond));
	} else {
		strncat(cond, "((MEDIA_TYPE=3 and PLAYLIST_MEDIA_COUNT>0) or PLAYLIST_MEDIA_COUNT=0)", STRNCAT_LEN(cond));
	}

	switch (group_type) {
	case MP_GROUP_BY_ALBUM:
		if (type_string) {
			strncat(cond, " AND MEDIA_ALBUM = '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		if (filter_string) {
			strncat(cond, " AND MEDIA_ALBUM like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_ALBUM, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_GROUP_BY_ARTIST:
		if (type_string) {
			strncat(cond, " AND MEDIA_ARTIST = '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		if (filter_string) {
			strncat(cond, " AND MEDIA_ARTIST like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_ARTIST, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_GROUP_BY_ARTIST_ALBUM:
		MP_CHECK(type_string && strlen(type_string));
		if (filter_string) {
			strncat(cond, " AND MEDIA_ALBUM like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%' AND ", STRNCAT_LEN(cond));
		}
		strncat(cond, " AND MEDIA_ARTIST = '", STRNCAT_LEN(cond));
		_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
		strncat(cond, "'", STRNCAT_LEN(cond));
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_ALBUM, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_GROUP_BY_GENRE:
		if (filter_string) {
			strncat(cond, " AND MEDIA_GENRE like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_GENRE, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_GROUP_BY_FOLDER:
		if (filter_string) {
			strncat(cond, " AND FOLDER_PATH like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, FOLDER_NAME, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_GROUP_BY_YEAR:
		if (filter_string) {
			strncat(cond, " AND MEDIA_YEAR like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_YEAR, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_GROUP_BY_COMPOSER:
		if (filter_string) {
			strncat(cond, " AND MEDIA_COMPOSER like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_COMPOSER, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_GROUP_BY_PLAYLIST:
		if (filter_string) {
			strncat(cond, " AND PLAYLIST_NAME like '\%", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
			strncat(cond, "\%'", STRNCAT_LEN(cond));
		}
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, PLAYLIST_NAME, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	default:
		WARN_TRACE("Unhandled type: %d", group_type);
		break;
	}
	if (strlen(cond)) {
		media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);
	}

}

static int __mp_media_info_get_thumbnail_path(media_info_h media, char **path)
{
	int res = 0;
#ifdef MP_FEATURE_ALBUMART_UPDATE
	char *media_id = NULL;
	char *albumart_path = NULL;
	media_info_get_media_id(media, &media_id);
	albumart_path = mp_albumart_path_get(media_id);

	IF_FREE(media_id);
	if (albumart_path) {
		*path = albumart_path;
	} else
#endif
	{
		media_info_get_thumbnail_path(media, path);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}

	return res;
}

static void __mp_media_info_set_filter(filter_h filter, mp_track_type_e track_type, const char *type_string, const char *type_string2, const char *filter_string)
{
	char cond[MAX_FILTER_LEN] = {0,};
	if (track_type == MP_TRACK_BY_VOICE_CLIP) {
		strncat(cond, MP_SOUND_TYPE, STRNCAT_LEN(cond));
	} else {
		strncat(cond, MP_MEDIA_TYPE, STRNCAT_LEN(cond));
	}

	switch (track_type) {
	case MP_TRACK_ALL:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_TRACK_BY_ALBUM:
		if (type_string) {
			strncat(cond, " AND MEDIA_ALBUM='", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "'", STRNCAT_LEN(cond));
		} else {
			strncat(cond, " AND MEDIA_ALBUM is null", STRNCAT_LEN(cond));
		}
		//strncat(cond, " ORDER BY CAST(MEDIA_TRACK_NUM as integer)", STRNCAT_LEN(cond));
		break;
	case MP_TRACK_BY_ARTIST:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, "MEDIA_ALBUM, MEDIA_TITLE", MEDIA_CONTENT_COLLATE_NOCASE);
		if (type_string) {
			strncat(cond, " AND MEDIA_ARTIST='", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "'", STRNCAT_LEN(cond));
		} else {
			strncat(cond, " AND MEDIA_ARTIST is null", STRNCAT_LEN(cond));
		}
		break;
	case MP_TRACK_BY_GENRE:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
		if (type_string) {
			strncat(cond, " AND MEDIA_GENRE='", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "'", STRNCAT_LEN(cond));
		} else {
			strncat(cond, " AND MEDIA_GENRE is null", STRNCAT_LEN(cond));
		}
		break;
	case MP_TRACK_BY_FOLDER:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_DISPLAY_NAME, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_TRACK_BY_YEAR:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
		if (type_string) {
			strncat(cond, " AND MEDIA_YEAR='", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "'", STRNCAT_LEN(cond));
		} else {
			strncat(cond, " AND MEDIA_YEAR is null", STRNCAT_LEN(cond));
		}
		break;
	case MP_TRACK_BY_COMPOSER:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
		if (type_string) {
			strncat(cond, " AND MEDIA_COMPOSER='", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "'", STRNCAT_LEN(cond));
		} else {
			strncat(cond, " AND MEDIA_COMPOSER is null", STRNCAT_LEN(cond));
		}
		break;
	case MP_TRACK_BY_ARTIST_ALBUM:
		if (type_string) {
			strncat(cond, " AND MEDIA_ALBUM='", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
			strncat(cond, "'", STRNCAT_LEN(cond));
		} else {
			strncat(cond, " AND MEDIA_ALBUM is null", STRNCAT_LEN(cond));
		}
		if (type_string2) {
			strncat(cond, " AND MEDIA_ARTIST='", STRNCAT_LEN(cond));
			_mp_media_info_sql_strncat(cond, type_string2, STRNCAT_LEN(cond));
			strncat(cond, "'", STRNCAT_LEN(cond));
		} else {
			strncat(cond, " AND MEDIA_ARTIST is null", STRNCAT_LEN(cond));
		}
		//strncat(cond, " ORDER BY CAST(MEDIA_TRACK_NUM as integer)", STRNCAT_LEN(cond));
		break;
	case MP_TRACK_BY_FAVORITE:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, MEDIA_FAVOURITE, MEDIA_CONTENT_COLLATE_NOCASE);
		strncat(cond, " AND MEDIA_FAVOURITE>0", STRNCAT_LEN(cond));
		break;
	case MP_TRACK_BY_PLAYED_TIME:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, MEDIA_LAST_PLAYED_TIME, MEDIA_CONTENT_COLLATE_NOCASE);
		strncat(cond, " AND MEDIA_LAST_PLAYED_TIME>0", STRNCAT_LEN(cond));
		break;
	case MP_TRACK_BY_ADDED_TIME:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, MEDIA_ADDED_TIME, MEDIA_CONTENT_COLLATE_NOCASE);
		strncat(cond, " AND MEDIA_ADDED_TIME>0", STRNCAT_LEN(cond));
		break;
	case MP_TRACK_BY_PLAYED_COUNT:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, MEDIA_PLAYED_COUNT, MEDIA_CONTENT_COLLATE_NOCASE);
		strncat(cond, " AND MEDIA_PLAYED_COUNT>0", STRNCAT_LEN(cond));
		break;
	case MP_TRACK_BY_PLAYLIST:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, PLAYLIST_MEMBER_ORDER, MEDIA_CONTENT_COLLATE_NOCASE);
		break;
	case MP_TRACK_BY_VOICE_CLIP:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
		strncat(cond, " AND MEDIA_MIME_TYPE=\"audio/AMR\" OR MEDIA_MIME_TYPE=\"audio/mp4\" OR MEDIA_MIME_TYPE=\"audio/3gpp\"", STRNCAT_LEN(cond));
		break;
	case MP_TRACK_BY_GROUP_PLAY:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
		strncat(cond, " AND MEDIA_MIME_TYPE=\"audio/x-wav\" OR MEDIA_MIME_TYPE=\"audio/mpeg\"", STRNCAT_LEN(cond));
		break;
	default:
		WARN_TRACE("Unhandled type: %d", track_type);
		break;
	}

	if (filter_string && strlen(filter_string)) {
		strncat(cond, " AND MEDIA_TITLE like '\%", STRNCAT_LEN(cond));
		_mp_media_info_sql_strncat(cond, filter_string, STRNCAT_LEN(cond));
		strncat(cond, "\%'", STRNCAT_LEN(cond));
	}

	//DEBUG_TRACE("cond: %s", cond);
	media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);

}


int mp_media_info_connect(void)
{
	int res = media_content_connect();
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error: media_content_connect");
	}

	return res;
}
int mp_media_info_disconnect(void)
{
	int res = media_content_disconnect();
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error: media_content_disconnect");
	}

	return res;
}

/*filter*/
int mp_media_filter_create(mp_filter_h *filter)
{
	//startfunc;
	int res = media_filter_create(filter);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}
int mp_media_filter_destory(mp_filter_h filter)
{
	//startfunc;
	int res = media_filter_destroy(filter);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}
int mp_media_filter_set_offset(mp_filter_h filter, int offset, int count)
{
	//startfunc;
	int res = media_filter_set_offset(filter, offset, count);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}
int mp_media_filter_set_order(mp_filter_h filter, bool descending, const char *order_keyword, mp_media_content_collation_e collation)
{
	//startfunc;
	int res = media_filter_set_order(filter, descending, order_keyword, collation);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}
int mp_media_filter_set_condition(mp_filter_h filter, const char *condition, mp_media_content_collation_e collation)
{
	//startfunc;
	int res = media_filter_set_condition(filter, condition, collation);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

/*media infomation*/
int mp_media_info_list_count_w_filter(mp_track_type_e track_type, const char * folder_id, int playlist_id, mp_filter_h filter, int *count)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	if (track_type == MP_TRACK_BY_FOLDER) {
		res = media_folder_get_media_count_from_db(folder_id, filter, count);
	} else if (track_type == MP_TRACK_BY_PLAYLIST) {
		res = media_playlist_get_media_count_from_db(playlist_id, filter, count);
	} else {
		res = media_info_get_media_count_from_db(filter, count);
	}
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}

	if (track_type == MP_TRACK_BY_PLAYED_TIME || track_type == MP_TRACK_BY_ADDED_TIME || track_type == MP_TRACK_BY_PLAYED_COUNT) {
		if (*count > MP_AUTO_PLAYLIST_ITEM_MAX) {
			*count = MP_AUTO_PLAYLIST_ITEM_MAX;
		}
	} else if (track_type == MP_TRACK_BY_PLAYLIST) {
		if (*count > MP_PLAYLIST_MAX_ITEM_COUNT) {
			*count = MP_PLAYLIST_MAX_ITEM_COUNT;
		}
	}

	return res;
}

int mp_media_info_list_count(mp_track_type_e track_type, const char *type_string, const char *type_string2, const char *filter_string, int playlist_id, int *count)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;


	res = media_filter_create(&filter);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);
	__mp_media_info_set_filter(filter, track_type, type_string, type_string2, filter_string);

	res = mp_media_info_list_count_w_filter(track_type, type_string, playlist_id, filter, count);
	media_filter_destroy(filter);
	//endfunc;
	return res;
}

int mp_media_info_list_create_w_filter(mp_track_type_e track_type, const char *folder_id, int playlist_id, mp_filter_h filter, mp_media_list_h *media_list)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(media_list, -1);

	*media_list = calloc(1, sizeof(struct mp_media_list_s));
	MP_CHECK_VAL(*media_list, -1);
	(*media_list)->group_type = MP_GROUP_NONE;

	if (track_type == MP_TRACK_BY_FOLDER) {
		res = media_folder_foreach_media_from_db(folder_id, filter, __mp_media_info_cb, *media_list);
	} else if (track_type == MP_TRACK_BY_PLAYLIST) {
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, PLAYLIST_MEMBER_ORDER, MEDIA_CONTENT_COLLATE_NOCASE);
		res = media_playlist_foreach_media_from_db(playlist_id, filter, __mp_playlist_media_info_cb, *media_list);
	} else if (track_type == MP_TRACK_BY_ALBUM || track_type == MP_TRACK_BY_ARTIST_ALBUM) {
		res = media_info_foreach_media_from_db(filter, __mp_media_info_of_album_cb, *media_list);
	} else {
		res = media_info_foreach_media_from_db(filter, __mp_media_info_cb, *media_list);
	}

	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		free(*media_list);
		*media_list = NULL;
		return res;
	}

	(*media_list)->list = g_list_reverse((*media_list)->list);
	(*media_list)->count = g_list_length((*media_list)->list);

	return res;
}

int mp_media_info_list_create(mp_media_list_h *out_list,
                              mp_track_type_e track_type, const char *type_string, const char *type_string2, const char *filter_string, int playlist_id, int offset, int count)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;


	res = media_filter_create(&filter);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);

	res = media_filter_set_offset(filter, offset, count);
	//DEBUG_TRACE("offset: %d, count: %d", offset, count);
	__mp_media_info_set_filter(filter, track_type, type_string, type_string2, filter_string);

	res = mp_media_info_list_create_w_filter(track_type, type_string, playlist_id, filter, out_list);
	media_filter_destroy(filter);

	return res;
}

#ifdef MP_SOUND_PLAYER
int mp_media_info_sorted_track_list_create(mp_media_list_h *out_list, char *sort_type)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;

	res = media_filter_create(&filter);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);

	char cond[MAX_FILTER_LEN] = {0,};
	strncat(cond, "(MEDIA_TYPE=3) OR (MEDIA_TYPE=2)", STRNCAT_LEN(cond));
	media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);

	if (!g_strcmp0(sort_type, "MYFILE_SORT_BY_NAME_A2Z")) {
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, MEDIA_DISPLAY_NAME, MEDIA_CONTENT_COLLATE_NOCASE);
	} else if (!g_strcmp0(sort_type, "MYFILE_SORT_BY_NAME_Z2A")) {
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_DISPLAY_NAME, MEDIA_CONTENT_COLLATE_NOCASE);
	} else if (!g_strcmp0(sort_type, "MYFILE_SORT_BY_DATE_O2R")) {
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_DESC, MEDIA_MODIFIED_TIME, MEDIA_CONTENT_COLLATE_NOCASE);
	} else if (!g_strcmp0(sort_type, "MYFILE_SORT_BY_SIZE_L2S")) {
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_SIZE, MEDIA_CONTENT_COLLATE_NOCASE);
	}

	res = mp_media_info_list_create_w_filter(MP_TRACK_ALL, NULL, 0, filter, out_list);
	media_filter_destroy(filter);

	MP_CHECK_VAL(*out_list, -1);

	(*out_list)->list = g_list_reverse((*out_list)->list);
	(*out_list)->count = g_list_length((*out_list)->list);

	return res;
}
#endif

int mp_media_info_list_destroy(mp_media_list_h media_list)
{
	//startfunc;
	MP_CHECK_VAL(media_list, -1);

	if (media_list->list) {
		g_list_free_full(media_list->list, __mp_media_info_destory);
	}
	free(media_list);
	media_list = NULL;
	return 0;
}

mp_media_info_h mp_media_info_list_nth_item(mp_media_list_h media_list, int index)
{
	MP_CHECK_NULL(media_list);
	MP_CHECK_NULL(index < media_list->count);

	return g_list_nth_data(media_list->list, index);
}

int mp_media_infor_list_get_count(mp_media_list_h media_list)
{
	MP_CHECK_VAL(media_list, -1);
	return media_list->count;
}

int mp_media_info_create(mp_media_info_h *media_info, const char *media_id)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	*media_info = calloc(1, sizeof(struct mp_media_info_s));
	MP_CHECK_VAL(*media_info, -1);

	(*media_info)->i.minfo = calloc(1, sizeof(struct mp_minfo_s));
	MP_CHECK_VAL((*media_info)->i.minfo, -1);
	if (!(*media_info)->i.minfo) {
		SAFE_FREE(*media_info);
		return -1;
	}

	res = media_info_get_media_from_db(media_id, &(*media_info)->h.media);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		SAFE_FREE((*media_info)->i.minfo);
		SAFE_FREE(*media_info);
		return res;
	}

	res = media_info_get_audio((*media_info)->h.media, &(*media_info)->s.meta);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		SAFE_FREE((*media_info)->i.minfo);
		SAFE_FREE(*media_info);
		return res;
	}

	return res;
}

int mp_media_info_create_by_path(mp_media_info_h *media_info, const char *file_path)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	mp_filter_h filter = NULL;
	char sql[MAX_NAM_LEN] = {0,};
	char *cond = NULL;
	mp_media_list_h list = NULL;

	res = media_filter_create(&filter);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);
	_mp_media_info_sql_strncat(sql, file_path, STRNCAT_LEN(sql));
	cond = g_strdup_printf("MEDIA_PATH = '%s'", sql);

	media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);
	IF_FREE(cond);

	media_filter_set_offset(filter, 0, 1);

	res = mp_media_info_list_create_w_filter(MP_TRACK_ALL, NULL, 0, filter, &list);
	media_filter_destroy(filter);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);
	MP_CHECK_VAL(list, -1);
	if (list->count <= 0) {
		DEBUG_TRACE("list count 0");
		mp_media_info_list_destroy(list);
		return -1;
	}

	list->list = g_list_nth(list->list, 0);
	if (list->list == NULL) {
		mp_media_info_list_destroy(list);
		return res;
	}
	*media_info = list->list->data;
	list->list = g_list_delete_link(list->list, list->list);

	mp_media_info_list_destroy(list);

	return res;
}

int mp_media_info_destroy(mp_media_info_h media_info)
{
	MP_CHECK_VAL(media_info, -1);

	audio_meta_destroy(media_info->s.meta);
	media_info_destroy(media_info->h.media);

	if (media_info->i.minfo) {
		IF_FREE(media_info->i.minfo->media_id);
		IF_FREE(media_info->i.minfo->title);
		IF_FREE(media_info->i.minfo->album);
		IF_FREE(media_info->i.minfo->artist);
		IF_FREE(media_info->i.minfo->genre);
		IF_FREE(media_info->i.minfo->date);
		IF_FREE(media_info->i.minfo->composer);
		IF_FREE(media_info->i.minfo->year);
		IF_FREE(media_info->i.minfo->copyright);
		IF_FREE(media_info->i.minfo->track_num);
		IF_FREE(media_info->i.minfo->format);
		IF_FREE(media_info->i.minfo->file_path);
		IF_FREE(media_info->i.minfo->thumbnail_path);
		IF_FREE(media_info->i.minfo->display_name);
		free(media_info->i.minfo);
	}

	free(media_info);
	return 0;
}

bool mp_media_info_uri_is_exist_in_db(const char *file_path)
{
	startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	mp_filter_h filter = NULL;
	char sql[MAX_NAM_LEN] = {0,};
	char *cond = NULL;
	bool exist = false;
	int count = 0;

	res = media_filter_create(&filter);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);
	_mp_media_info_sql_strncat(sql, file_path, STRNCAT_LEN(sql));
	cond = g_strdup_printf("MEDIA_PATH = '%s'", sql);

	media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);
	IF_FREE(cond);

	media_filter_set_offset(filter, 0, 1);

	res = media_info_get_media_count_from_db(filter, &count);
	media_filter_destroy(filter);

	MP_CHECK_FALSE(res == MEDIA_CONTENT_ERROR_NONE);

	if (count > 0) {
		exist = true;
	}

	return exist;
}

int mp_media_info_get_media_id(mp_media_info_h media, char **media_id)
{
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.minfo, -1);
	int res = MEDIA_CONTENT_ERROR_NONE;

	if (!media->i.minfo->media_id) {
		res = media_info_get_media_id(media->h.media, &media->i.minfo->media_id);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*media_id = media->i.minfo->media_id;
	PRINT_STR(*media_id);

	return res;
}

int mp_media_info_get_file_path(mp_media_info_h media, char **path)
{
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.minfo, -1);
	int res = MEDIA_CONTENT_ERROR_NONE;

	if (!media->i.minfo->file_path) {
		res = media_info_get_file_path(media->h.media, &media->i.minfo->file_path);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*path = media->i.minfo->file_path;
	PRINT_STR(*path);
	return res;
}
int mp_media_info_get_thumbnail_path(mp_media_info_h media, char **path)
{
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.minfo, -1);
	int res = MEDIA_CONTENT_ERROR_NONE;

	if (media->i.minfo->thumbnail_path) {
		IF_FREE(media->i.minfo->thumbnail_path);
	}
	res = __mp_media_info_get_thumbnail_path(media->h.media, &media->i.minfo->thumbnail_path);
	*path = media->i.minfo->thumbnail_path;
	PRINT_STR(*path);
	return res;
}
int mp_media_info_get_favorite(mp_media_info_h media, bool *favorite)
{
	MP_CHECK_VAL(media, -1);

	int res = media_info_get_favorite(media->h.media, favorite);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

int mp_media_info_is_drm(mp_media_info_h media, bool *drm)
{
	MP_CHECK_VAL(media, -1);
	int res = media_info_is_drm(media->h.media, drm);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

int mp_media_info_get_media_type(mp_media_info_h media, int *media_type)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->h.media, -1);

	media_content_type_e mtype;

	res = media_info_get_media_type(media->h.media, &mtype);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}

	if (mtype == MEDIA_CONTENT_TYPE_SOUND) {
		*media_type = MP_MEDIA_TYPE_SOUND;
	} else {
		*media_type = MP_MEDIA_TYPE_MUSIC;
	}

	return res;
}

int mp_media_info_get_display_name(mp_media_info_h media, char **display_name)
{
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.minfo, -1);
	int res = MEDIA_CONTENT_ERROR_NONE;

	if (!media->i.minfo->display_name) {
		res = media_info_get_display_name(media->h.media, &media->i.minfo->display_name);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*display_name = media->i.minfo->display_name;
	PRINT_STR(*display_name);
	return res;
}

int mp_media_info_get_title(mp_media_info_h media, char **title)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->h.media, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->title) {
		res = media_info_get_title(media->h.media, &media->i.minfo->title);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*title = media->i.minfo->title;
	PRINT_STR(*title);
	return res;
}
int mp_media_info_get_album(mp_media_info_h media, char **album)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->album) {
		res = audio_meta_get_album(media->s.meta, &media->i.minfo->album);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*album = media->i.minfo->album;
	PRINT_STR(*album);
	return res;
}
int mp_media_info_get_artist(mp_media_info_h media, char **artist)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->artist) {
		res = audio_meta_get_artist(media->s.meta, &media->i.minfo->artist);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*artist = media->i.minfo->artist;
	PRINT_STR(*artist);
	return res;
}

int mp_media_info_get_recorded_date(mp_media_info_h media, char **date)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->date) {
		res = audio_meta_get_recorded_date(media->s.meta, &media->i.minfo->date);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	if (!(media->i.minfo->date)) {
		char default_date[] = "Unknown";
		*date = strdup(default_date);
	} else {
		*date = strdup(media->i.minfo->date);
	}
	PRINT_STR(*date);
	return res;
}

int mp_media_info_get_genre(mp_media_info_h media, char **genre)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->genre) {
		res = audio_meta_get_genre(media->s.meta, &media->i.minfo->genre);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*genre = media->i.minfo->genre;
	PRINT_STR(*genre);
	return res;
}
int mp_media_playlist_get_playlist_count_from_db()
{
	mp_filter_h filter = NULL;
	int res = media_filter_create(&filter);
	ERROR_TRACE("media filter %d" , res);
	int count = 0;
	res = media_playlist_get_playlist_count_from_db(filter, &count);
	if (res == 0) {
		ERROR_TRACE("successful");
	}
	media_filter_destroy(filter);
	ERROR_TRACE("%d the count is", count);
	return count;
}
int mp_media_info_get_composer(mp_media_info_h media, char **composer)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->composer) {
		res = audio_meta_get_composer(media->s.meta, &media->i.minfo->composer);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*composer = media->i.minfo->composer;
	PRINT_STR(*composer);
	return res;
}
int mp_media_info_get_year(mp_media_info_h media, char **year)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->year) {
		res = audio_meta_get_year(media->s.meta, &media->i.minfo->year);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	if (!g_strcmp0(media->i.minfo->year, "Unknown")) {
		char *default_year = "2014";
		strncpy(media->i.minfo->year, default_year, strlen(default_year));
	}
	*year = media->i.minfo->year;
	PRINT_STR(*year);
	return res;
}
int mp_media_info_get_copyright(mp_media_info_h media, char **copyright)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->copyright) {
		res = audio_meta_get_copyright(media->s.meta, &media->i.minfo->copyright);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*copyright = media->i.minfo->copyright;
	PRINT_STR(*copyright);
	return res;
}

int mp_media_info_get_track_num(mp_media_info_h media, char **track_num)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	if (!media->i.minfo->track_num) {
		res = audio_meta_get_track_num(media->s.meta, &media->i.minfo->track_num);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
	}
	*track_num = media->i.minfo->track_num;
	PRINT_STR(*track_num);
	return res;
}

int mp_media_info_get_format(mp_media_info_h media, char **format)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	int bit_rate = 0;
	int sample_rate = 0;
	int channel = 0;

	if (!media->i.minfo->format) {
		res = audio_meta_get_sample_rate(media->s.meta, &sample_rate);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
		res = audio_meta_get_bit_rate(media->s.meta, &bit_rate);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}
		res = audio_meta_get_channel(media->s.meta, &channel);
		if (res != MEDIA_CONTENT_ERROR_NONE) {
			ERROR_TRACE("Error code 0x%x", res);
		}

		media->i.minfo->format = g_strdup_printf("%dKbps %.1fKHz %dch", bit_rate / 1000, sample_rate / 1000.0, channel);
	}
	*format = media->i.minfo->format;
	PRINT_STR(*format);
	return res;
}

int mp_media_info_get_bit_rate(mp_media_info_h media, int *bitrate)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);

	res = audio_meta_get_bit_rate(media->s.meta, bitrate);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

int mp_media_info_get_bitpersample(mp_media_info_h media, int *bitpersample)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);
	MP_CHECK_VAL(bitpersample, -1);

	res = audio_meta_get_bitpersample(media->s.meta, bitpersample);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	if (*bitpersample == 0) { //get 0 except wav/flac, show 16
		*bitpersample = 16;
	}
	return res;
}

int mp_media_info_get_sample_rate(mp_media_info_h media, int *sample_rate)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);

	res = audio_meta_get_sample_rate(media->s.meta, sample_rate);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}
int mp_media_info_get_duration(mp_media_info_h media, int *duration)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);

	res = audio_meta_get_duration(media->s.meta, duration);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}

	if (*duration < 0) {
		*duration = 0;
	}

	return res;
}

int mp_media_info_get_played_time(mp_media_info_h media, time_t *time)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);

	res = audio_meta_get_played_time(media->s.meta, time);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

int mp_media_info_get_played_count(mp_media_info_h media, int *count)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);

	res = audio_meta_get_played_count(media->s.meta, count);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

int mp_media_info_get_added_time(mp_media_info_h media, time_t *time)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->h.media, -1);

	res = media_info_get_added_time(media->h.media, time);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		return res;
	}

	return res;
}

int mp_media_info_get_playlist_member_id(mp_media_info_h media, int *member_id)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	*member_id = media->i.minfo->playlist_member_id;
	return res;
}

int mp_media_info_get_storage_type(mp_media_info_h media, mp_storage_type_e *storage_type)
{
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->h.media, -1);

	*storage_type = MP_STORAGE_INTERNAL_EX;
	return res;
}

int mp_media_info_set_favorite_media_db_only(mp_media_info_h media, bool favorite)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->h.media, -1);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		return res;
	}

	res = media_info_set_favorite(media->h.media, favorite);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		return res;
	}

	res = media_info_update_to_db(media->h.media);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}

	return res;
}

int mp_media_info_set_favorite(mp_media_info_h media, bool favorite)
{
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->h.media, -1);

	int res = mp_media_info_set_favorite_media_db_only(media, favorite);

	return res;
}

int mp_media_info_set_played_time(mp_media_info_h media, time_t time)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);

	res = audio_meta_set_played_time(media->s.meta, time);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		return res;
	}
	res = audio_meta_update_to_db(media->s.meta);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}
int mp_media_info_set_played_count(mp_media_info_h media, int count)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.meta, -1);

	res = audio_meta_set_played_count(media->s.meta, count);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		return res;
	}
	res = audio_meta_update_to_db(media->s.meta);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

int mp_media_info_set_added_time(mp_media_info_h media, time_t time)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->h.media, -1);

	res = media_info_set_added_time(media->h.media, time);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		return res;
	}
	res = media_info_update_to_db(media->h.media);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}
	return res;
}

/*int mp_media_info_set_cloud_thumbnail_path(mp_media_info_h media, const char *path)
{
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.minfo, -1);

	char *current_thumbnail = NULL;
	mp_media_info_get_thumbnail_path(media, &current_thumbnail);
	if (!g_strcmp0(path, current_thumbnail)) {
		// already updated
		return 0;
	}

	mp_storage_type_e storage = MP_STORAGE_INTERNAL_EX;
	mp_media_info_get_storage_type(media, &storage);

	if (storage != MP_STORAGE_CLOUD_EX && storage != MP_STORAGE_MUSICHUB_EX) {
		WARN_TRACE("media is not cloud item [storage type = %d]", storage);
		return -1;
	}

	IF_FREE(media->i.minfo->thumbnail_path);
	media->i.minfo->thumbnail_path = g_strdup(path);

	int res = MEDIA_CONTENT_ERROR_NONE;
	char *media_id = NULL;
	mp_media_info_get_media_id(media, &media_id);
	res = media_cloud_info_update_thumbnail_path_to_db(media_id, path);
	if (res != MEDIA_CONTENT_ERROR_NONE)
		ERROR_TRACE("Error code 0x%x", res);

	PRINT_STR(path);
	return res;
}*/

int mp_media_info_group_list_count(mp_group_type_e group_type, const char *type_string, const char *filter_string, int *count)
{
	//startfunc;
	mp_filter_h filter = NULL;
	int res = MEDIA_CONTENT_ERROR_NONE;

	res = media_filter_create(&filter);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);
	DEBUG_TRACE("group_type: %d, type_string:%s, filter_string:%s", group_type, type_string, filter_string);

	__mp_media_info_set_group_filter(filter, group_type, type_string, filter_string);

	res = mp_media_info_group_list_count_w_filter(group_type, filter, count);
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		media_filter_destroy(filter);
		return res;
	}

	media_filter_destroy(filter);

	return res;
}
int mp_media_info_group_list_count_w_filter(mp_group_type_e group_type, mp_filter_h filter, int *count)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	MP_CHECK_VAL(filter, -1);
	switch (group_type) {
	case MP_GROUP_BY_ALBUM:
	case MP_GROUP_BY_ARTIST_ALBUM:
		res = media_album_get_album_count_from_db(filter, count);
		break;
	case MP_GROUP_BY_ARTIST:
		res = media_group_get_group_count_from_db(filter, MEDIA_CONTENT_GROUP_ARTIST, count);
		break;
	case MP_GROUP_BY_GENRE:
		res = media_group_get_group_count_from_db(filter, MEDIA_CONTENT_GROUP_GENRE, count);
		break;
	case MP_GROUP_BY_FOLDER:
		res = media_folder_get_folder_count_from_db(filter, count);
		break;
	case MP_GROUP_BY_YEAR:
		res = media_group_get_group_count_from_db(filter, MEDIA_CONTENT_GROUP_YEAR, count);
		break;
	case MP_GROUP_BY_COMPOSER:
		res = media_group_get_group_count_from_db(filter, MEDIA_CONTENT_GROUP_COMPOSER, count);
		break;
	case MP_GROUP_BY_PLAYLIST:
		res = media_playlist_get_playlist_count_from_db(filter, count);
		break;
	default:
		WARN_TRACE("Unhandled type: %d", group_type);
		res = -1;
		break;
	}
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
	}

	return res;
}

typedef struct {
	char *info; //album, artist, folder name
	char *album; //album name for artist list
	int count; //track_count
	char *thumb_path; //thumbnail of first track
	char **album_thumnails; //thumbnail list of albums
	int thumb_count; //album thumbnail count
	int track_num; //tracknumber for album
} MpCountData_t;

static inline bool _folder_track_foreach_cb(media_info_h media, void *user_data)
{
	char *path = NULL;
	char *folder = NULL;
	MpCountData_t *data = NULL;

	media_info_get_file_path(media, &path);
	folder = mp_file_dir_get(path);
	IF_FREE(path);

	GList **list = user_data;
	if (!*list) {
		data = calloc(1, sizeof(MpCountData_t));
		if (data == NULL) {
			IF_FREE(folder);
			return false;
		}
		data->info = folder;
		__mp_media_info_get_thumbnail_path(media, &data->thumb_path);

		data->count++;
		*list = g_list_append(*list, data);
		//DEBUG_TRACE("folder: %s", folder);
		return true;
	}

	*list = g_list_last(*list);
	data = (*list)->data;
	if (!g_strcmp0(data->info, folder))	{
		IF_FREE(folder);
		data->count++;
	} else {
		data = calloc(1, sizeof(MpCountData_t));
		if (data == NULL) {
			IF_FREE(folder);
			return false;
		}
		data->info = folder;
		__mp_media_info_get_thumbnail_path(media, &data->thumb_path);

		data->count++;
		*list = g_list_append(*list, data);
	}
	IF_FREE(folder);
	return true;
}

static inline bool _album_track_foreach_cb(media_info_h media, void *user_data)
{
	char *album = NULL;
	MpCountData_t *data = NULL;
	int res = MEDIA_CONTENT_ERROR_NONE;
	char *track_num;

	audio_meta_h audio;

	res = media_info_get_audio(media, &audio);
	if (res  != MEDIA_CONTENT_ERROR_NONE) {
		audio_meta_destroy(audio);
		IF_FREE(album);
		return false;
	}

	res = audio_meta_get_album(audio, &album);
	if (res  != MEDIA_CONTENT_ERROR_NONE) {
		audio_meta_destroy(audio);
		IF_FREE(album);
		return false;
	}

	res = audio_meta_get_track_num(audio, &track_num);
	if (res  != MEDIA_CONTENT_ERROR_NONE) {
		audio_meta_destroy(audio);
		IF_FREE(album);
		IF_FREE(track_num)
		return false;
	}

	GList **list = user_data;
	if (!*list) {
		data = calloc(1, sizeof(MpCountData_t));
		audio_meta_destroy(audio);
		IF_FREE(track_num);

		if (!data) {
			IF_FREE(album);
			return true;
		}

		if (data->info != NULL) {
			free(data->info);
		}
		data->info = strdup(album);
		__mp_media_info_get_thumbnail_path(media, &data->thumb_path);

		data->count++;
		*list = g_list_append(*list, data);
		//DEBUG_TRACE("info: %s", album);
		IF_FREE(album);
		return true;
	}

	*list = g_list_last(*list);
	data = (*list)->data;
	if (data == NULL) {
		ERROR_TRACE("data is null, audio destroyed");
		IF_FREE(track_num);
		IF_FREE(album);
		audio_meta_destroy(audio);
		return EINA_TRUE;
	}
	if (!g_strcmp0(data->info, album))	{
		data->count++;
		IF_FREE(album);
		int track = 0;
		if (track_num) {
			track = atoi(track_num);
			if (track < data->track_num) {
				IF_FREE(data->thumb_path);
				__mp_media_info_get_thumbnail_path(media, &data->thumb_path);
			}
			free(track_num);
		}
	} else {
		data = calloc(1, sizeof(MpCountData_t));
		if (data == NULL) {
			ERROR_TRACE("data is null, audio destroyed");
			IF_FREE(track_num);
			IF_FREE(album);
			audio_meta_destroy(audio);
			return EINA_TRUE;
		}

		if (data->info != NULL) {
			free(data->info);
		}
		data->info = strdup(album);
		__mp_media_info_get_thumbnail_path(media, &data->thumb_path);

		data->count++;
		*list = g_list_append(*list, data);

		if (track_num) {
			data->track_num = atoi(track_num);
			free(track_num);
		}
	}
	IF_FREE(album);
	audio_meta_destroy(audio);
	return true;
}

static inline bool _artist_track_foreach_cb(media_info_h media, void *user_data)
{
	char *album = NULL, *artist = NULL;
	MpCountData_t *data = NULL;
	int res = MEDIA_CONTENT_ERROR_NONE;

	audio_meta_h audio;

	res = media_info_get_audio(media, &audio);
	if (res  != MEDIA_CONTENT_ERROR_NONE) {
		audio_meta_destroy(audio);
		return false;
	}

	res = audio_meta_get_album(audio, &album);
	if (res  != MEDIA_CONTENT_ERROR_NONE) {
		audio_meta_destroy(audio);
		IF_FREE(album);
		return false;
	}

	res = audio_meta_get_artist(audio, &artist);
	if (res  != MEDIA_CONTENT_ERROR_NONE) {
		audio_meta_destroy(audio);
		IF_FREE(artist);
		IF_FREE(album);
		return false;
	}

	//DEBUG_TRACE("info: %s, artist: %s", album, artist);

	GList **list = user_data;
	if (!*list) {
		audio_meta_destroy(audio);
		data = calloc(1, sizeof(MpCountData_t));
		if (data == NULL) {
			IF_FREE(album);
			IF_FREE(artist);
			return EINA_TRUE;
		}
		if (data->info != NULL) {
			free(data->info);
		}
		if (data->album != NULL) {
			free(data->album);
		}
		data->info = strdup(artist);
		data->album = strdup(album);
		data->album_thumnails = calloc(1, sizeof(char *));
		__mp_media_info_get_thumbnail_path(media, &(data->album_thumnails[data->thumb_count]));
		data->thumb_count++;

		data->count++;
		*list = g_list_append(*list, data);

		IF_FREE(artist);
		IF_FREE(album);
		return true;
	}

	*list = g_list_last(*list);
	data = (*list)->data;
	if (data == NULL) {
		ERROR_TRACE("data is null, audio destroyed");
		IF_FREE(album);
		IF_FREE(artist);
		audio_meta_destroy(audio);
		return EINA_TRUE;
	}
	if (!g_strcmp0(data->info, artist))	{
		data->count++;
		if (g_strcmp0(data->album, album)) {
			if (data->album != NULL) {
				free(data->album);
			}
			data->album = strdup(album);
			data->album_thumnails = realloc(data->album_thumnails, sizeof(char *) * (data->thumb_count + 1));
			__mp_media_info_get_thumbnail_path(media, &(data->album_thumnails[data->thumb_count]));
			data->thumb_count++;
			//DEBUG_TRACE("artist: %s, album: %s", artist, album);
			IF_FREE(album);
		} else {
			IF_FREE(album);
		}

		IF_FREE(artist);
	} else {
		data = calloc(1, sizeof(MpCountData_t));
		if (data == NULL) {
			IF_FREE(album);
			IF_FREE(artist);
			audio_meta_destroy(audio);
			return EINA_TRUE;
		};
		if (data->info != NULL) {
			free(data->info);
		}
		if (data->album != NULL) {
			free(data->album);
		}
		data->info = strdup(artist);
		data->album = strdup(album);
		data->album_thumnails = calloc(1, sizeof(char *));
		__mp_media_info_get_thumbnail_path(media, &(data->album_thumnails[data->thumb_count]));
		data->thumb_count++;

		data->count++;
		*list = g_list_append(*list, data);
		IF_FREE(artist);
		IF_FREE(album);
	}

	audio_meta_destroy(audio);
	return true;
}

static void
_mp_media_info_set_group_extra_info(mp_media_list_h media_list, mp_group_type_e group_type, const char *type_string, const char *filter_string)
{
	mp_filter_h filter = NULL;
	char cond[MAX_FILTER_LEN] = {0,};

	GList *list = NULL;
	GList *node = NULL;
	int res = MEDIA_CONTENT_ERROR_NONE;

	media_info_cb media_foreach_cb = NULL;

	res = media_filter_create(&filter);
	MP_CHECK(res == MEDIA_CONTENT_ERROR_NONE);

	strncat(cond, MP_MEDIA_TYPE, STRNCAT_LEN(cond));

	//create track list
	switch (group_type) {
	case MP_GROUP_BY_ALBUM:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_ALBUM, MEDIA_CONTENT_COLLATE_NOCASE);
		media_foreach_cb = _album_track_foreach_cb;
		break;
		//case MP_GROUP_BY_ARTIST_ALBUM:
		//break;
	case MP_GROUP_BY_ARTIST:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, "MEDIA_ARTIST, MEDIA_ALBUM", MEDIA_CONTENT_COLLATE_NOCASE);
		media_foreach_cb = _artist_track_foreach_cb;
		break;
	case MP_GROUP_BY_GENRE:
		break;
	case MP_GROUP_BY_FOLDER:
		media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, "folder_uuid", MEDIA_CONTENT_COLLATE_NOCASE);
		media_foreach_cb = _folder_track_foreach_cb;
		break;
		//case MP_GROUP_BY_YEAR:
		//	break;
		//case MP_GROUP_BY_COMPOSER:
		//	break;
		//case MP_GROUP_BY_PLAYLIST:
		//	break;
	default:
		WARN_TRACE("Unsupported type: %d", group_type);
		media_filter_destroy(filter);
		return;
	}
	media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);
	res = media_info_foreach_media_from_db(filter, media_foreach_cb, &list);
	media_filter_destroy(filter);
	MP_CHECK(res == MEDIA_CONTENT_ERROR_NONE);

	int i = 0;
	mp_media_info_h media = NULL;
	MpCountData_t *data;
	do {
		list = node = g_list_first(list);
		media = mp_media_info_group_list_nth_item(media_list, i++);
		if (media) {
			char *info = NULL;
			int ret = MEDIA_CONTENT_ERROR_NONE;
			if (group_type != MP_GROUP_BY_FOLDER) {
				ret = mp_media_info_group_get_main_info(media, &info);
			} else {
				ret = mp_media_info_group_get_sub_info(media, &info);
			}
			//DEBUG_TRACE("find folder from glist: %s", folder);
			//DEBUG_TRACE("list count: %d", g_list_length(node));
			if (ret != MEDIA_CONTENT_ERROR_NONE) {
				ERROR_TRACE("Get group info error");
			}
			while (node) {
				data = node->data;
				if (!g_strcmp0(data->info, info)) {
					media->i.ginfo->track_count = data->count;
					if (data->thumb_path) {
						media->i.ginfo->thumb_path = data->thumb_path;
					} else if (data->album_thumnails && data->thumb_count) {
						media->i.ginfo->thumb_path = g_strdup(data->album_thumnails[0]);
					}

					media->i.ginfo->album_thumb_paths = data->album_thumnails;
					media->i.ginfo->album_count = data->thumb_count;

					IF_FREE(data->info);
					IF_FREE(data->album);
					free(data);
					list = g_list_delete_link(list, node);
					//DEBUG_TRACE("Found data!! count: %d, thumb: %s", media->i.ginfo->track_count, media->i.ginfo->thumb_path);
					break;
				}
				node = g_list_next(node);
			}

		}
	} while (media);

	if (list) {
		ERROR_TRACE("Shouldn't be here...");
		while (list) {
			data = list->data;
			ERROR_TRACE("This item not  matched...: %s", data->info);
			IF_FREE(data->info);
			IF_FREE(data->album);
			IF_FREE(data->thumb_path);

			while (data->thumb_count) {
				data->thumb_count--;
				IF_FREE(data->album_thumnails[data->thumb_count]);
			}
			IF_FREE(data->album_thumnails);

			free(data);
			list = g_list_next(list);
		}

		g_list_free(list);
	}

}

static int
_mp_media_info_set_extra_info(mp_media_list_h media_list, mp_group_type_e group_type, const char *type_string, const char *filter_string)
{
	//startfunc;
	_mp_media_info_set_group_extra_info(media_list, group_type, type_string, filter_string);
	return 0;
}

int mp_media_info_group_list_create(mp_media_list_h *media_list, mp_group_type_e group_type, const char *type_string, const char *filter_string, int offset, int count)
{
	//startfunc;
	mp_filter_h filter = NULL;
	int res = MEDIA_CONTENT_ERROR_NONE;

	if (group_type != MP_GROUP_BY_SYS_PLAYLIST) {
		res = media_filter_create(&filter);
		MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);
		media_filter_set_offset(filter, offset, count);

		__mp_media_info_set_group_filter(filter, group_type, type_string, filter_string);

		res = mp_media_info_group_list_create_w_filter(filter, group_type, media_list);
		media_filter_destroy(filter);
		MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);

		_mp_media_info_set_extra_info(*media_list, group_type, type_string, filter_string);
	} else {

		int i;
		mp_media_info_h media_info = NULL;
		*media_list = calloc(1, sizeof(struct mp_media_list_s));
		MP_CHECK_VAL(*media_list, -1);
		(*media_list)->group_type = group_type;

		char names[][50] =
		{ STR_MP_FAVOURITES, STR_MP_RECENTLY_ADDED, STR_MP_MOST_PLAYED, STR_MP_RECENTLY_PLAYED};
		char thumb[][4096] =
		{ THUMBNAIL_QUICK_LIST, THUMBNAIL_RECENTLY_ADDED, THUMBNAIL_MOST_PLAYED, THUMBNAIL_RECENTLY_PLAYED};

		for (i = 0; i < 4; i++) {
			media_info = calloc(1, sizeof(struct mp_media_info_s));
			MP_CHECK_FALSE(media_info);

			media_info->i.ginfo = calloc(1, sizeof(struct mp_ginfo_s));
			if (!media_info->i.ginfo) {
				SAFE_FREE(media_info);
				return FALSE;
			}

			media_info->i.ginfo->main_info = g_strdup(names[i]);
			media_info->i.ginfo->thumb_path = g_strdup(thumb[i]);
			media_info->s.group_type = group_type;
			(*media_list)->list = g_list_append((*media_list)->list, media_info);
		}
		(*media_list)->count = g_list_length((*media_list)->list);

	}
	return res;
}

int mp_media_info_group_list_create_w_filter(mp_filter_h filter, mp_group_type_e group_type, mp_media_list_h *media_list)
{
	//startfunc;
	MP_CHECK_VAL(media_list, -1);

	*media_list = calloc(1, sizeof(struct mp_media_list_s));
	MP_CHECK_VAL(*media_list, -1);
	(*media_list)->group_type = group_type;

	int res = MEDIA_CONTENT_ERROR_NONE;
	switch (group_type) {
	case MP_GROUP_BY_ALBUM:
	case MP_GROUP_BY_ARTIST_ALBUM:
		res = media_album_foreach_album_from_db(filter, __mp_media_album_cb, *media_list);
		break;
	case MP_GROUP_BY_ARTIST:
		res = media_group_foreach_group_from_db(filter, MEDIA_CONTENT_GROUP_ARTIST, __mp_media_group_cb, *media_list);
		break;
	case MP_GROUP_BY_GENRE:
		res = media_group_foreach_group_from_db(filter, MEDIA_CONTENT_GROUP_GENRE, __mp_media_group_cb, *media_list);
		break;
	case MP_GROUP_BY_FOLDER:
		res = media_folder_foreach_folder_from_db(filter, __mp_media_folder_cb, *media_list);
		break;
	case MP_GROUP_BY_YEAR:
		res = media_group_foreach_group_from_db(filter, MEDIA_CONTENT_GROUP_YEAR, __mp_media_group_cb, *media_list);
		break;
	case MP_GROUP_BY_COMPOSER:
		res = media_group_foreach_group_from_db(filter, MEDIA_CONTENT_GROUP_COMPOSER, __mp_media_group_cb, *media_list);
		break;
	case MP_GROUP_BY_PLAYLIST:
		res = media_playlist_foreach_playlist_from_db(filter, __mp_media_playlist_cb, *media_list);
		break;
	default:
		WARN_TRACE("Unhandled type: %d", group_type);
		res = -1;
		break;
	}
	if (res != MEDIA_CONTENT_ERROR_NONE) {
		ERROR_TRACE("Error code 0x%x", res);
		free(*media_list);
		*media_list = NULL;
	} else {
		(*media_list)->list = g_list_reverse((*media_list)->list);
		(*media_list)->count = g_list_length((*media_list)->list);
	}

	return res;
}

int mp_media_info_group_list_destroy(mp_media_list_h media_list)
{
//	startfunc;
	MP_CHECK_VAL(media_list, -1);
	g_list_free_full(media_list->list, __mp_media_group_destory);
	IF_FREE(media_list->distinct_check);
	free(media_list);
	return 0;
}
mp_media_info_h mp_media_info_group_list_nth_item(mp_media_list_h media_list, int index)
{
	MP_CHECK_NULL(media_list);
	MP_CHECK_NULL(index < media_list->count);

	return g_list_nth_data(media_list->list, index);
}


int mp_media_info_group_get_type(mp_media_info_h media, mp_group_type_e *group_type)
{
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(group_type, -1);

	*group_type = media->s.group_type;
	return MEDIA_CONTENT_ERROR_NONE;
}

int mp_media_info_group_get_main_info(mp_media_info_h media, char **main_info)
{
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(main_info, -1);
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.ginfo, -1);
	if (!media->i.ginfo->main_info && media->s.group_type != MP_GROUP_BY_SYS_PLAYLIST) {
		if (media->s.group_type == MP_GROUP_BY_FOLDER) {
			res = media_folder_get_name(media->h.group, &media->i.ginfo->main_info);
		} else if (media->s.group_type == MP_GROUP_BY_ALBUM || media->s.group_type == MP_GROUP_BY_ARTIST_ALBUM) {
			res = media_album_get_name(media->h.group, &media->i.ginfo->main_info);
		} else if (media->s.group_type == MP_GROUP_BY_PLAYLIST) {
			res = media_playlist_get_name(media->h.group, &media->i.ginfo->main_info);
		} else {
			media->i.ginfo->main_info = g_strdup(media->h.group);
		}
	}
	*main_info = g_strdup(media->i.ginfo->main_info);
	PRINT_STR(*main_info);
	return res;
}

int mp_media_info_group_get_sub_info(mp_media_info_h media, char **sub_info)
{
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(sub_info, -1);
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.ginfo, -1);
	if (!media->i.ginfo->sub_info && media->s.group_type != MP_GROUP_BY_SYS_PLAYLIST) {
		if (media->s.group_type == MP_GROUP_BY_FOLDER) {
			res = media_folder_get_path(media->h.group, &media->i.ginfo->sub_info);
		} else if (media->s.group_type == MP_GROUP_BY_ALBUM || media->s.group_type == MP_GROUP_BY_ARTIST_ALBUM) {
			res = media_album_get_artist(media->h.group, &media->i.ginfo->sub_info);
		}
	}
	*sub_info = media->i.ginfo->sub_info;
	PRINT_STR(*sub_info);
	return res;
}

int mp_media_info_group_get_playlist_id(mp_media_info_h media, int *playlist_id)
{
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist_id, -1);
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.group_type == MP_GROUP_BY_PLAYLIST || media->s.group_type == MP_GROUP_BY_SYS_PLAYLIST, -1);

	if (media->s.group_type == MP_GROUP_BY_SYS_PLAYLIST) {
		*playlist_id = mp_media_info_get_auto_playlist_id_by_name(media->i.ginfo->main_info);
	} else {
		res = media_playlist_get_playlist_id(media->h.group, playlist_id);
	}
	PRINT_INT(*playlist_id);
	return res;
}

int mp_media_info_group_get_folder_id(mp_media_info_h media, char **folder_id)
{
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(folder_id, -1);
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->s.group_type == MP_GROUP_BY_FOLDER, -1);

	res = media_folder_get_folder_id(media->h.group, folder_id);
	PRINT_INT(*folder_id);
	return res;
}

int mp_media_info_group_get_thumbnail_path(mp_media_info_h media, char **path)
{
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(path, -1);
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.ginfo, -1);
	char *folde_id = NULL;
	if (!media->i.ginfo->thumb_path) {

		/*if (media->s.group_type == MP_GROUP_BY_ALBUM || media->s.group_type == MP_GROUP_BY_ARTIST_ALBUM)
			res = media_album_get_album_art(media->h.group, &media->i.ginfo->thumb_path);
		else*/{
			filter_h filter;
			mp_media_list_h list = NULL;
			mp_media_info_h minfo = NULL;
			char *type_string = NULL;
			char cond[MAX_FILTER_LEN] = {0,};
			int count = 0;

			mp_media_info_group_get_main_info(media, &type_string);

			res = media_filter_create(&filter);
			if (res != MEDIA_CONTENT_ERROR_NONE) {
				mp_error("media_filter_create:error=%d", res);
				IF_FREE(folde_id);
				return res;
			}

			strncat(cond, MP_MEDIA_TYPE, STRNCAT_LEN(cond));
			if (media->s.group_type == MP_GROUP_BY_PLAYLIST) {
				media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, PLAYLIST_MEMBER_ORDER, MEDIA_CONTENT_COLLATE_NOCASE);
			} else {
				media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, MEDIA_TITLE, MEDIA_CONTENT_COLLATE_NOCASE);
			}

			if (media->s.group_type == MP_GROUP_BY_ALBUM || media->s.group_type == MP_GROUP_BY_ARTIST_ALBUM) {
				strncat(cond, " AND MEDIA_ALBUM='", STRNCAT_LEN(cond));
				_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
				strncat(cond, "' AND MEDIA_THUMBNAIL_PATH is not null", STRNCAT_LEN(cond));
				media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);

				mp_media_info_list_count_w_filter(MP_TRACK_ALL, NULL, 0, filter, &count);
				if (count > 0) {
					media_filter_set_offset(filter, 0, 1);
					mp_media_info_list_create_w_filter(MP_TRACK_ALL, NULL, 0, filter, &list);
				}

			} else if (media->s.group_type == MP_GROUP_BY_ARTIST && type_string) {
				strncat(cond, " AND MEDIA_ARTIST='", STRNCAT_LEN(cond));
				_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
				strncat(cond, "' AND MEDIA_THUMBNAIL_PATH is not null", STRNCAT_LEN(cond));
				media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);

				mp_media_info_list_count_w_filter(MP_TRACK_ALL, NULL, 0, filter, &count);
				if (count > 0) {
					media_filter_set_offset(filter, 0, 1);
					mp_media_info_list_create_w_filter(MP_TRACK_ALL, NULL, 0, filter, &list);
				}

			} else if (media->s.group_type == MP_GROUP_BY_GENRE && type_string) {
				strncat(cond, " AND MEDIA_GENRE='", STRNCAT_LEN(cond));
				_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
				strncat(cond, "' AND MEDIA_THUMBNAIL_PATH is not NULL", STRNCAT_LEN(cond));
				media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);

				mp_media_info_list_count_w_filter(MP_TRACK_ALL, NULL, 0, filter, &count);
				if (count > 0) {
					media_filter_set_offset(filter, 0, 1);
					mp_media_info_list_create_w_filter(MP_TRACK_ALL, NULL, 0, filter, &list);
				}
			} else if (media->s.group_type == MP_GROUP_BY_FOLDER) {
				mp_media_info_group_get_folder_id(media, &folde_id);

				strncat(cond, " AND MEDIA_THUMBNAIL_PATH is not NULL", STRNCAT_LEN(cond));

				media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);
				mp_media_info_list_count_w_filter(MP_TRACK_BY_FOLDER, folde_id, 0, filter, &count);
				if (count > 0) {
					media_filter_set_offset(filter, 0, 1);
					mp_media_info_list_create_w_filter(MP_TRACK_BY_FOLDER, folde_id, 0, filter, &list);
				}
			} else if (media->s.group_type == MP_GROUP_BY_YEAR && type_string) {
				strncat(cond, " AND MEDIA_YEAR='", STRNCAT_LEN(cond));
				_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
				strncat(cond, "' AND MEDIA_THUMBNAIL_PATH is not NULL", STRNCAT_LEN(cond));
				media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);

				mp_media_info_list_count_w_filter(MP_TRACK_ALL, NULL, 0, filter, &count);
				if (count > 0) {
					media_filter_set_offset(filter, 0, 1);
					mp_media_info_list_create_w_filter(MP_TRACK_ALL, NULL, 0, filter, &list);
				}
			} else if (media->s.group_type == MP_GROUP_BY_COMPOSER && type_string) {
				strncat(cond, " AND MEDIA_COMPOSER='", STRNCAT_LEN(cond));
				_mp_media_info_sql_strncat(cond, type_string, STRNCAT_LEN(cond));
				strncat(cond, "' AND MEDIA_THUMBNAIL_PATH is not NULL", STRNCAT_LEN(cond));
				media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);

				mp_media_info_list_count_w_filter(MP_TRACK_ALL, NULL, 0, filter, &count);
				if (count > 0) {
					media_filter_set_offset(filter, 0, 1);
					mp_media_info_list_create_w_filter(MP_TRACK_ALL, NULL, 0, filter, &list);
				}
			} else if (media->s.group_type == MP_GROUP_BY_PLAYLIST) {
				int playlist_id = 0;
				mp_media_info_group_get_playlist_id(media, &playlist_id);

				strncat(cond, " AND MEDIA_THUMBNAIL_PATH is not NULL", STRNCAT_LEN(cond));

				media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);
				mp_media_info_list_count_w_filter(MP_TRACK_BY_PLAYLIST, NULL, playlist_id, filter, &count);
				if (count > 0) {
					media_filter_set_offset(filter, 0, 1);
					mp_media_info_list_create_w_filter(MP_TRACK_BY_PLAYLIST, NULL, playlist_id, filter, &list);
				}
			} else {
				WARN_TRACE("Unhandled type: %d", media->s.group_type);
				media_filter_destroy(filter);
				goto END;
			}
			WARN_TRACE("count: %d", count);

			if (list) {
				char *thumb_path = NULL;
				minfo = mp_media_info_list_nth_item(list, 0);
				if (!minfo) {
					media_filter_destroy(filter);
					return -1;
				}
				mp_media_info_get_thumbnail_path(minfo, &thumb_path);
				media->i.ginfo->thumb_path = g_strdup(thumb_path);
				mp_media_info_list_destroy(list);
			}
			media_filter_destroy(filter);
		}

	}
END:
	IF_FREE(folde_id);
	*path = media->i.ginfo->thumb_path;
	PRINT_STR(*path);
	return res;
}

int mp_media_info_group_get_track_count(mp_media_info_h media, int *count)
{
	MP_CHECK_VAL(media, -1);
	if (count) {
		*count = media->i.ginfo->track_count;
	}
	return 0;
}

int mp_media_info_group_get_album_thumnail_paths(mp_media_info_h media, char ***thumbs, int *count)
{
	MP_CHECK_VAL(media, -1);
	if (thumbs) {
		*thumbs = media->i.ginfo->album_thumb_paths;
	}
	if (count) {
		*count = media->i.ginfo->album_count;
	}
	return 0;
}

int mp_media_info_playlist_get_thumbnail_path(mp_media_info_h media, char **path)
{
	/*currently, it only works for playlist thumbnail update*/
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(path, -1);
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.ginfo, -1);

	*path = media->i.ginfo->thumb_path;

	return res;
}

int mp_media_info_playlist_set_thumbnail_path(mp_media_info_h media, const char *path)
{
	/*currently, it only works for playlist thumbnail update*/
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(path, -1);
	MP_CHECK_VAL(media, -1);
	MP_CHECK_VAL(media->i.ginfo, -1);

	res = media_playlist_set_thumbnail_path(media->h.group, path);
	if (res != 0) {
		media_playlist_destroy(media->h.group);
		return res;
	}

	media_playlist_update_to_db(media->h.group);

	IF_FREE(media->i.ginfo->thumb_path) ;
	media->i.ginfo->thumb_path = g_strdup(path);
	return res;
}

int mp_media_info_playlist_get_id_by_name(const char *playlist_name, int *playlist_id)
{
	//startfunc;
	mp_filter_h filter = NULL;
	mp_media_list_h list;
	mp_media_info_h media;
	char *cond = NULL;
	char sql[MAX_FILTER_LEN] = {0,};
	int res = MEDIA_CONTENT_ERROR_NONE;

	res = mp_media_filter_create(&filter);
	MP_CHECK_VAL(res == 0, res);

	_mp_media_info_sql_strncat(sql, playlist_name, STRNCAT_LEN(sql));
	cond = g_strdup_printf("PLAYLIST_NAME = '%s'", sql);
	if (!cond) {
		media_filter_destroy(filter);
		return -1;
	}
	res = mp_media_filter_set_condition(filter, cond, MP_MEDIA_CONTENT_COLLATE_DEFAULT);
	free(cond);
	if (res != 0) {
		media_filter_destroy(filter);
		return res;
	}
	res = mp_media_filter_set_offset(filter, 0, 1);
	if (res != 0) {
		media_filter_destroy(filter);
		return res;
	}

	res = mp_media_info_group_list_create_w_filter(filter, MP_GROUP_BY_PLAYLIST, &list);
	mp_media_filter_destory(filter);
	MP_CHECK_VAL(res == 0, res);
	MP_CHECK_VAL(list, -1);

	media = mp_media_info_group_list_nth_item(list, 0);
	if (!media) {
		mp_media_info_group_list_destroy(list);
		return -1;
	}

	res = mp_media_info_group_get_playlist_id(media, playlist_id);
	mp_media_info_group_list_destroy(list);

	return res;
}

int mp_media_info_playlist_delete_from_db(int playlist_id)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	res = media_playlist_delete_from_db(playlist_id);
	return res;
}

int mp_media_info_playlist_handle_create(mp_playlist_h *playlist_h, int playlist_id)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	media_playlist_h playlist = NULL;
	res = media_playlist_get_playlist_from_db(playlist_id, &playlist);
	*playlist_h = (mp_playlist_h)playlist;
	return res;
}

int mp_media_info_playlist_add_media(int playlist_id, const char *media_id)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	media_playlist_h playlist = NULL;

	res = media_playlist_get_playlist_from_db(playlist_id, &playlist);
	if (res != 0) {
		media_playlist_destroy(playlist);
		return res;
	}

	res =  media_playlist_add_media(playlist, media_id);
	if (res != 0) {
		media_playlist_destroy(playlist);
		return res;
	}

	res = media_playlist_update_to_db(playlist);
	if (res != 0) {
		media_playlist_destroy(playlist);
		return res;
	}

	media_playlist_destroy(playlist);

	return res;
}

int mp_media_info_playlist_remove_media(mp_media_info_h playlist, int memeber_id)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist, -1);

	res =  media_playlist_remove_media(playlist->h.group, memeber_id);
	MP_CHECK_VAL(res == 0, res);
	media_playlist_update_to_db(playlist->h.group);

	return res;
}

int mp_media_info_playlist_is_exist(const char *playlist_name, bool *exist)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	filter_h filter = NULL;
	char cond[MAX_FILTER_LEN] = {0,};
	int count = 0;
	res = media_filter_create(&filter);
	MP_CHECK_VAL(res == 0, res);
	strncat(cond, "((MEDIA_TYPE=3 and PLAYLIST_MEDIA_COUNT>0) or PLAYLIST_MEDIA_COUNT=0) and PLAYLIST_NAME = '", STRNCAT_LEN(cond));
	_mp_media_info_sql_strncat(cond, playlist_name, STRNCAT_LEN(cond));
	strncat(cond, "'", STRNCAT_LEN(cond));
	media_filter_set_condition(filter, cond, MEDIA_CONTENT_COLLATE_NOCASE);
	res = mp_media_info_group_list_count_w_filter(MP_GROUP_BY_PLAYLIST, filter, &count);
	media_filter_destroy(filter);
	if (count == 0) {
		*exist = false;
	} else {
		*exist = true;
	}
	return res;
}

int mp_media_info_playlist_unique_name(const char *orig_name, char *unique_name, size_t max_unique_name_length)
{
	//startfunc;
	bool exist = false;
	int i = 1;

	snprintf(unique_name, max_unique_name_length, orig_name, "001");
	mp_media_info_playlist_is_exist(unique_name, &exist);

	if (exist) {
		while (i < 1000) {
			char id_str[4] = {0};
			snprintf(id_str, 4, "%.3d", i + 1);
			snprintf(unique_name, max_unique_name_length, orig_name, id_str);
			mp_media_info_playlist_is_exist(unique_name, &exist);
			if (!exist) {
				return 0;
			} else {
				i++;
			}
		}
		MP_CHECK_VAL(i < 1000, -1);
	}
	return 0;
}

int mp_media_info_playlist_rename(mp_media_info_h playlist, const char *new_name)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist, -1);
	MP_CHECK_VAL(playlist->h.group, -1);
	MP_CHECK_VAL(playlist->i.ginfo, -1);
	MP_CHECK_VAL(new_name, -1);
	MP_CHECK_VAL(playlist->s.group_type == MP_GROUP_BY_PLAYLIST, -1);

	res = media_playlist_set_name((media_playlist_h)playlist->h.group, new_name);
	MP_CHECK_VAL(res == 0, res);

	res = media_playlist_update_to_db(playlist->h.group);
	MP_CHECK_VAL(res == 0, res);

	IF_FREE(playlist->i.ginfo->main_info);
	playlist->i.ginfo->main_info = g_strdup(new_name);

	return res;
}

int mp_media_info_playlist_db_update(mp_playlist_h playlist_handle)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist_handle, -1);

	res = media_playlist_update_to_db((media_playlist_h)playlist_handle);
	MP_CHECK_VAL(res == 0, res);

	return res;
}

int mp_media_info_playlist_get_play_order(mp_media_info_h playlist, int playlist_member_id, int * play_order)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist, -1);
	MP_CHECK_VAL(playlist->h.group, -1);
	MP_CHECK_VAL(playlist->s.group_type == MP_GROUP_BY_PLAYLIST, -1);

	res = media_playlist_get_play_order(playlist->h.group, playlist_member_id, play_order);
	MP_CHECK_VAL(res == 0, res);

	return res;
}

int mp_media_info_playlist_set_play_order(mp_media_info_h playlist, int playlist_member_id, int play_order)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist, -1);
	MP_CHECK_VAL(playlist->h.group, -1);
	MP_CHECK_VAL(playlist->s.group_type == MP_GROUP_BY_PLAYLIST, -1);

	res = media_playlist_set_play_order(playlist->h.group, playlist_member_id, play_order);
	MP_CHECK_VAL(res == 0, res);

	return res;
}

int mp_media_info_playlist_update_db(mp_media_info_h playlist)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist, -1);
	MP_CHECK_VAL(playlist->h.group, -1);
	MP_CHECK_VAL(playlist->s.group_type == MP_GROUP_BY_PLAYLIST, -1);

	res = media_playlist_update_to_db(playlist->h.group);
	MP_CHECK_VAL(res == 0, res);

	return res;
}

int
mp_media_info_delete_from_db(const char *path)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	res = media_content_scan_file(path);
	MP_CHECK_VAL(res == MEDIA_CONTENT_ERROR_NONE, res);
	return res;
}

int mp_media_info_playlist_add_item(mp_playlist_h playlist_handle, const char *media_id, const char *thumbnail_path)
{
	MP_CHECK_VAL(playlist_handle, -1);
	MP_CHECK_VAL(media_id, -1);

	if (thumbnail_path) {
		media_playlist_set_thumbnail_path((media_playlist_h)playlist_handle, thumbnail_path);
	}

	return media_playlist_add_media((media_playlist_h)playlist_handle, media_id);;
}

int mp_media_info_dup_playlist_handle_from_list_item(mp_media_info_h playlist, mp_playlist_h *playlist_handle)
{
	int res = 0;
	MP_CHECK_VAL(playlist, -1);
	MP_CHECK_VAL(playlist_handle, -1);

	res =  media_playlist_clone((media_playlist_h *)playlist_handle, playlist->h.group);
	MP_CHECK_VAL(res == 0, res);

	return res;
}

int mp_media_info_playlist_insert_to_db(const char * name, int *playlist_id, mp_playlist_h *playlist_handle)
{
	//startfunc;
	int res = MEDIA_CONTENT_ERROR_NONE;
	media_playlist_h playlist = NULL;

	res = media_playlist_insert_to_db(name, &playlist);
	MP_CHECK_VAL(res == 0, res);

	res = media_playlist_get_playlist_id(playlist, playlist_id);

	if (playlist_handle) {
		*playlist_handle = (mp_playlist_h)playlist;
	} else {
		media_playlist_destroy(playlist);
	}
	return res;
}

int mp_media_info_playlist_get_name_by_id(int playlist_id, char **playlist_name)
{
	//startfunc;
	mp_filter_h filter = NULL;
	mp_media_list_h list;
	mp_media_info_h media = NULL;
	int index = 0;
	int id = 0;

	int res = MEDIA_CONTENT_ERROR_NONE;

	MP_CHECK_VAL(playlist_name, -1);

	switch (playlist_id) {
	case MP_SYS_PLST_QUICK_LIST:
		*playlist_name = g_strdup(STR_MP_FAVOURITES);
		return 0;
	case MP_SYS_PLST_RECENTELY_PLAYED:
		*playlist_name = g_strdup(STR_MP_RECENTLY_PLAYED);
		return 0;
	case MP_SYS_PLST_RECENTELY_ADDED:
		*playlist_name = g_strdup(STR_MP_RECENTLY_ADDED);
		return 0;
	case MP_SYS_PLST_MOST_PLAYED:
		*playlist_name = g_strdup(STR_MP_MOST_PLAYED);
		return 0;
	default:
		break;
	}

	res = mp_media_filter_create(&filter);
	if (res != 0) {
		if (filter) {
			media_filter_destroy(filter);
		}
		return res;
	}

	res = mp_media_info_group_list_create_w_filter(filter, MP_GROUP_BY_PLAYLIST, &list);
	mp_media_filter_destory(filter);
	MP_CHECK_VAL(res == 0, res);
	if (list == NULL) {
		mp_media_info_group_list_destroy(list);
		return -1;
	}

	do {
		media = mp_media_info_group_list_nth_item(list, index);
		mp_media_info_group_get_playlist_id(media, &id);
		if (playlist_id == id) {
			break;
		}
		index++;
	} while (media);

	if (!media) {
		WARN_TRACE("No such playlist.. ID: %d", playlist_id);
		return -1;
	}

	char *main_info = NULL;
	mp_media_info_group_get_main_info(media, &main_info);
	*playlist_name = g_strdup(main_info);

	media = mp_media_info_group_list_nth_item(list, 0);
	if (!media) {
		mp_media_info_group_list_destroy(list);
		return -1;
	}

	mp_media_info_group_list_destroy(list);

	return res;
}

static bool _mp_media_info_playlist_foreach_cb_by_media_id(int playlist_member_id, media_info_h media, void *user_data)
{
	GList **list = (GList **)user_data;
	MP_CHECK_FALSE(list);
	*list = g_list_append(*list, (void *)playlist_member_id);
	return true;
}

int mp_media_info_playlist_prepend_media(mp_playlist_h playlist_handle, const char *media_id)
{
	int res = MEDIA_CONTENT_ERROR_NONE;
	media_playlist_h playlist = (media_playlist_h)playlist_handle;
	MP_CHECK_VAL(playlist, -1);
	MP_CHECK_VAL(media_id, -1);

	int playlist_id = 0;

	res = media_playlist_add_media(playlist, media_id);
	if (res) {
		mp_error("media_playlist_add_media() .. [0x%x]", res);
		return res;
	}

	res = media_playlist_update_to_db(playlist);
	if (res) {
		mp_error("media_playlist_update_to_db() .. [0x%x]", res);
		return res;
	}

	res = media_playlist_get_playlist_id(playlist, &playlist_id);
	if (res) {
		mp_error("media_playlist_get_playlist_id() .. [0x%x]", res);
		return res;
	}

	GList *member_id_list = NULL;
	filter_h filter = NULL;
	res = media_filter_create(&filter);
	if (res) {
		mp_error("media_filter_create() .. [0x%x]", res);
		return res;
	}
	media_filter_set_order(filter, MEDIA_CONTENT_ORDER_ASC, PLAYLIST_MEMBER_ORDER, MEDIA_CONTENT_COLLATE_NOCASE);
	res = media_playlist_foreach_media_from_db(playlist_id, filter, _mp_media_info_playlist_foreach_cb_by_media_id, &member_id_list);
	media_filter_destroy(filter);
	filter = NULL;
	if (res) {
		mp_error("media_playlist_foreach_media_from_db() .. [0x%x]", res);
		return res;
	}

	GList *list = member_id_list;
	int index = 0;
	while (list) {
		int order = ++index;
		int member_id = (int)list->data;
		if (list == g_list_last(member_id_list)) {
			order = 0;
		}

		res = media_playlist_set_play_order(playlist, member_id, order);
		if (res) {
			mp_error("media_playlist_set_play_order() .. [0x%x]", res);
			return res;
		}

		list = list->next;
	}
	g_list_free(member_id_list);
	member_id_list = NULL;

	res = media_playlist_update_to_db(playlist);
	if (res) {
		mp_error("media_playlist_update_to_db() .. [0x%x]", res);
		return res;
	}

	return res;
}

int mp_media_info_playlist_handle_destroy(mp_playlist_h playlist_handle)
{
	MP_CHECK_VAL(playlist_handle, -1);
	media_playlist_update_to_db((media_playlist_h)playlist_handle);
	return media_playlist_destroy((media_playlist_h)playlist_handle);
}

int mp_media_info_get_folder_path_by_folder_id(const char *folder_id, char **path)
{
	MP_CHECK_VAL(folder_id, -1);
	MP_CHECK_VAL(path, -1);

	media_folder_h folder = NULL;
	int ret = media_folder_get_folder_from_db(folder_id, &folder);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mp_error("media_folder_get_folder_from_db().. [0x%x]", ret);
		goto END;
	}

	ret = media_folder_get_path(folder, path);
	if (ret != MEDIA_CONTENT_ERROR_NONE) {
		mp_error("media_folder_get_path().. [0x%x]", ret);
		goto END;
	}

END:
	if (folder) {
		media_folder_destroy(folder);
	}
	return ret;
}

int mp_media_info_get_auto_playlist_id_by_name(const char *name)
{
	MP_CHECK_VAL(name, 0);

	int playlist_id = 0;

	if (!g_strcmp0(name, STR_MP_FAVOURITES)) {
		playlist_id = MP_SYS_PLST_QUICK_LIST;
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_PLAYED)) {
		playlist_id = MP_SYS_PLST_RECENTELY_PLAYED;
	} else if (!g_strcmp0(name, STR_MP_RECENTLY_ADDED)) {
		playlist_id = MP_SYS_PLST_RECENTELY_ADDED;
	} else if (!g_strcmp0(name, STR_MP_MOST_PLAYED)) {
		playlist_id = MP_SYS_PLST_MOST_PLAYED;
	}

	return playlist_id;
}

static db_update_cb g_db_update_cb;
static void *g_db_update_data;
static Ecore_Timer *g_db_update_timer;

static Eina_Bool
_mp_media_content_db_update_timer_cb(void *data)
{
	DEBUG_TRACE("Trigger Update list");

#ifdef MP_FEATURE_SUGGEST_FOR_YOU
	app_control_h service = NULL;

	app_control_create(&service);
	app_control_set_app_id(service, "org.tizen.music-player.service");
	//char *type_str = g_strdup_printf("%d", update_type);
	//app_control_add_extra_data(service, "update_type", type_str);
	//app_control_add_extra_data(service, "url", path);
	app_control_send_launch_request(service, NULL, NULL);
	app_control_destroy(service);
	//SAFE_FREE(type_str);
#endif
	if (g_db_update_cb) {
		g_db_update_cb(g_db_update_data);
	}

	g_db_update_timer = NULL;
	return false;
}

void
mp_media_content_db_update_cb(
    media_content_error_e error,	int pid, media_content_db_update_item_type_e update_item,
    media_content_db_update_type_e update_type,	media_content_type_e media_type,
    char *uuid, char *path, char *mime_type, void *user_data)
{

	//if music-player update db, do not update views
	if (pid == getpid()) {
		return;
	}

	DEBUG_TRACE("url: %s, media_type: %d, update_type: %d, item_type: %d", path, media_type, update_type, update_item);
	/*
		if (update_type == MEDIA_CONTENT_UPDATE)
		{
			DEBUG_TRACE("don't update list when some item is just updated");
			return;
		}
	*/

	if ((media_type != MEDIA_CONTENT_TYPE_MUSIC) && (media_type != MEDIA_CONTENT_TYPE_SOUND) && (update_item == MEDIA_ITEM_FILE)) {
		DEBUG_TRACE("ignore not music file type");
		return;
	}

	mp_ecore_timer_del(g_db_update_timer);
	g_db_update_timer = ecore_timer_add(1, _mp_media_content_db_update_timer_cb, NULL);
}

int mp_media_info_set_db_update_cb(db_update_cb cb, void *data)
{
	MP_CHECK_VAL(cb, -1);
	if (g_db_update_cb) {
		WARN_TRACE("registering multiple callback not supported!!");
	}

	g_db_update_cb = cb;
	g_db_update_data = data;

	return media_content_set_db_updated_cb(mp_media_content_db_update_cb, NULL);
}

int mp_media_info_unset_db_update_cb(void)
{
	mp_ecore_timer_del(g_db_update_timer);
	g_db_update_cb = NULL;
	g_db_update_data = NULL;
	return media_content_unset_db_updated_cb();
}
