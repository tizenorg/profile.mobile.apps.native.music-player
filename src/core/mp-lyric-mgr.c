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



#include "mp-lyric-mgr.h"
#include "regex.h"
#include "mp-player-debug.h"
#include "music.h"
#include <metadata_extractor.h>

#define MP_LRC_LINE_COUNT_MAX (int)100 /* The max count of line */
#define MP_LRC_LINE_COUNT_INC (int)50 /* The size increased when exceed the max count */
#define MP_LRC_SUBS_COUNT_MAX (int)10 /* The max count of sub string */
#define MP_LRC_ERROR_BUF_LEN (int)128 /* The max length of error buffer */

#define MP_LYRIC_PARSE_TIME_PATTERN "\\[[0-9]{2}:[0-9]{2}((\\.|:)[0-9]{2})?\\]"
#define MP_LYRIC_PARSE_ITEM_TITLE "[ti:"
#define MP_LYRIC_PARSE_ITEM_ARTIST "[ar:"
#define MP_LYRIC_PARSE_ITEM_ALBUM "[al:"
#define MP_LYRIC_PARSE_ITEM_OFFSET "[offset:"
#define MP_LYRIC_PARSE_START_INDEX "["
#define MP_LYRIC_PARSE_END_INDEX "]"
#define MP_LYRIC_PARSE_START_CHAR '['
#define MP_LYRIC_PARSE_END_CHAR ']'

static long _mp_lyric_mgr_str2time(const char *text);
static void _mp_lyric_mgr_data_free(mp_lyric_mgr_t **data);
static void _mp_lyric_mgr_parse_line(mp_lyric_mgr_t **data, const char *line);
static char *_mp_lyric_mgr_extract_lyric(const char *line);
static mp_lrc_node_t *_mp_lyric_mgr_node_new();

static int _mp_lyric_mgr_node_sort(const void *data1, const void *data2)
{
	MP_CHECK_VAL(data1, 0);
	MP_CHECK_VAL(data2, 0);

	return (((mp_lrc_node_t *)data1)->time > ((mp_lrc_node_t *)data2)->time) ? 1 : -1;
}

static long
_mp_lyric_mgr_str2time(const char *text)
{
	startfunc;

	MP_CHECK_VAL(text, 0);

	int len = strlen(text);
	long time = 0;
	char buf[10] = {'0'};

	memcpy(buf, text, 2);
	buf[2] = '\0';
	time = atoi(buf) * 60 * 1000;

	if (len == 5) {
		memcpy(buf, &text[3], 2);
		buf[2] = '\0';
		time += atoi(buf) * 1000;
	} else if (len == 8) {
		if (text[5] == ':') {
			memcpy(buf, &text[3], 2);
			buf[2] = '\0';
			time += atoi(buf) * 1000;

			memcpy(buf, &text[6], 2);
			buf[2] = '\0';
			time += atoi(buf) * 10;
		} else if (text[5] == '.') {
			memcpy(buf, &text[3], 5);
			buf[5] = '\0';
			time += atof(buf) * 1000;
		}
	}

	endfunc;

	return time;
}

static void
_mp_lryic_mgr_list_free(Eina_List **list)
{
	mp_lrc_node_t *node = NULL;
	Eina_List *next = NULL;
	EINA_LIST_FOREACH(*list, next, node) {
		if (node) {
			if (node->lyric != NULL) {
				free(node->lyric);
			}
			free(node);
		}
	}

	eina_list_free(*list);
	*list = NULL;
}

static void
_mp_lyric_mgr_data_free(mp_lyric_mgr_t **data)
{
	startfunc;

	MP_CHECK(data);
	mp_lyric_mgr_t **lrc_data = data;
	if (*lrc_data != NULL) {
		if ((*lrc_data)->title != NULL) {
			free((*lrc_data)->title);
		}

		if ((*lrc_data)->artist != NULL) {
			free((*lrc_data)->artist);
		}

		if ((*lrc_data)->album != NULL) {
			free((*lrc_data)->album);
		}

		if ((*lrc_data)->synclrc_list != NULL) {
			_mp_lryic_mgr_list_free(&(*lrc_data)->synclrc_list);
		}

		if ((*lrc_data)->unsynclrc_list != NULL) {
			_mp_lryic_mgr_list_free(&(*lrc_data)->unsynclrc_list);
		}

		free(*lrc_data);
		*lrc_data = NULL;
	}

	endfunc;
}

static char*
_mp_lyric_mgr_extract_lyric(const char *line)
{
	MP_CHECK_NULL(line);

	char *lyric = (char *)malloc(sizeof(char) * strlen(line));
	MP_CHECK_NULL(line);
	MP_CHECK_NULL(lyric);
	memset(lyric, 0, sizeof(char)*strlen(line));

	int i = 0;
	const char *p = line;
	bool bTag = false;
	while ((*p != '\0') && (*p != '\n')) {
		if (bTag) {
			if (*p == MP_LYRIC_PARSE_END_CHAR) {
				bTag = false;
			}
		} else {
			if (*p == MP_LYRIC_PARSE_START_CHAR) {
				bTag = true;
			} else {
				lyric[i++] = *p;
			}
		}

		p++;
	}

	return lyric;
}

static void
_mp_lyric_mgr_parse_line(mp_lyric_mgr_t **data, const char *line)
{
	MP_CHECK(data);
	MP_CHECK(line);
	/* 	DEBUG_TRACE("line=%s\n", line); */

	mp_lyric_mgr_t **lrc_data = data;
	MP_CHECK(*lrc_data);

	regex_t regex;
	regmatch_t subs[MP_LRC_SUBS_COUNT_MAX];
	const size_t nmatch = MP_LRC_SUBS_COUNT_MAX;
	char errbuf[MP_LRC_ERROR_BUF_LEN];
	const char *start = NULL, *end = NULL, *head = NULL;
	char *pattern_lyric = MP_LYRIC_PARSE_TIME_PATTERN; /* [mm:ss.ff],[mm:ss:ff],[mm:ss] */
	char *lyric = NULL;

	/* Get title */
	head = line;
	start = strstr(head, MP_LYRIC_PARSE_ITEM_TITLE);
	int ti_len = strlen(MP_LYRIC_PARSE_ITEM_TITLE);
	if (start != NULL) {
		end = strstr(start, MP_LYRIC_PARSE_END_INDEX);
		if (end != NULL) {
			(*lrc_data)->title = malloc(sizeof(char) * (end - start - ti_len + 1));
			MP_CHECK((*lrc_data)->title);
			mp_assert((*lrc_data)->title);
			strncpy((*lrc_data)->title, start + ti_len, end - start - ti_len);
			(*lrc_data)->title[end - start - ti_len] = '\0';
			/* DEBUG_TRACE("title: %s\n", (*lrc_data)->title); */
		}
	}

	/* Get artist */
	start = strstr(head, MP_LYRIC_PARSE_ITEM_ARTIST);
	int ar_len = strlen(MP_LYRIC_PARSE_ITEM_ARTIST);
	if (start != NULL) {
		end = strstr(start, MP_LYRIC_PARSE_END_INDEX);
		if (end != NULL) {
			(*lrc_data)->artist = malloc(sizeof(char) * (end - start - ar_len + 1));
			MP_CHECK((*lrc_data)->artist);
			mp_assert((*lrc_data)->artist);
			strncpy((*lrc_data)->artist, start + ar_len, end - start - ar_len);
			(*lrc_data)->artist[end - start - ar_len] = '\0';
			/* DEBUG_TRACE("title: %s\n", (*lrc_data)->artist); */
		}
	}

	/* Get album */
	start = strstr(head, MP_LYRIC_PARSE_ITEM_ALBUM);
	int al_len = strlen(MP_LYRIC_PARSE_ITEM_ALBUM);
	if (start != NULL) {
		end = strstr(start, MP_LYRIC_PARSE_END_INDEX);
		if (end != NULL) {
			(*lrc_data)->album = malloc(sizeof(char) * (end - start - al_len + 1));
			MP_CHECK((*lrc_data)->album);
			mp_assert((*lrc_data)->album);
			strncpy((*lrc_data)->album, start + al_len, end - start - al_len);
			(*lrc_data)->album[end - start - al_len] = '\0';
			/* DEBUG_TRACE("title: %s\n", (*lrc_data)->album); */
		}
	}

	/* Get offset */
	start = strstr(head, MP_LYRIC_PARSE_ITEM_OFFSET);
	int offset_len = strlen(MP_LYRIC_PARSE_ITEM_OFFSET);
	if (start != NULL) {
		end = strstr(start, MP_LYRIC_PARSE_END_INDEX);
		if (end != NULL) {
			char buf[MP_LRC_LINE_BUF_LEN] = {'0'};
			memcpy(buf, start + offset_len, end - start - offset_len);
			buf[end - start - offset_len] = '\0';
			(*lrc_data)->offset = atoi(buf);
			/* DEBUG_TRACE("offset: %d\n", (*lrc_data)->offset); */
		}
	}

	/* Get lyric */
	size_t len;
	int err;

	err = regcomp(&regex, pattern_lyric, REG_EXTENDED);
	if (err != 0) {
		len = regerror(err, &regex, errbuf, sizeof(errbuf));
		/* DEBUG_TRACE("errinfo: regcomp: %s\n", errbuf); */
		goto FAIL_GET_PARSE_LINE;
	}

	/* DEBUG_TRACE("Total has subexpression: %d\n", regex.re_nsub); */

	lyric = _mp_lyric_mgr_extract_lyric(head);

	while (!(err = regexec(&regex, head, nmatch, subs, 0))) {
		/* DEBUG_TRACE("\nOK, has matched ...\n\n"); */

		len = subs[0].rm_eo - subs[0].rm_so - 2;

		/* DEBUG_TRACE("begin: %d, len = %d ", subs[0].rm_so, len); */

		char buf[MP_LRC_LINE_BUF_LEN] = {'0'};
		memcpy(buf, head + subs[0].rm_so + 1, len);
		buf[len] = '\0';

		long time = _mp_lyric_mgr_str2time(buf);
		/* Add the new node */
		if (lyric != NULL) {
			mp_lrc_node_t *new_node = _mp_lyric_mgr_node_new();

			if (new_node == NULL) {
				IF_FREE(lyric);
				return ;
			}
			new_node->time = time;
			new_node->lyric = strdup(lyric);
			(*lrc_data)->synclrc_list = eina_list_append((*lrc_data)->synclrc_list, (gpointer)new_node);

			DEBUG_TRACE("time%d: %d\n", eina_list_count((*lrc_data)->synclrc_list), new_node->time);
			DEBUG_TRACE("lyric%d: %s\n", eina_list_count((*lrc_data)->synclrc_list), new_node->lyric);
		} else {
			goto FAIL_GET_PARSE_LINE;
		}

		head += subs[0].rm_eo;
	}

	if (head != NULL) {
		len = regerror(err, &regex, errbuf, sizeof(errbuf));
		/* DEBUG_TRACE("error: regexec: %s\n", errbuf); */
	}

	regfree(&regex);
	IF_FREE(lyric);
	return;

FAIL_GET_PARSE_LINE:

	regfree(&regex);
	IF_FREE(lyric);
	return;
}

static int
_mp_lyric_mgr_get_line(const char *buffer, char **line)
{
	MP_CHECK_VAL(buffer, -1);
	MP_CHECK_VAL(line, -1);

	int i = 0;
	const char *p = buffer;

	while ((*p != '\0') && (*p != '\n')) {
		i++;
		p++;
	}

	i++;

	*line = malloc(sizeof(char) * i);
	MP_CHECK_VAL(line, -1);
	memset(*line, 0, sizeof(char)*i);
	strncpy(*line, buffer, i - 1);

	/* DEBUG_TRACE("line=%s\n",*line);
	DEBUG_TRACE("i=%d\n",i); */

	return i;
}

static mp_lrc_node_t*
_mp_lyric_mgr_node_new()
{
	mp_lrc_node_t *lrc_node = NULL;

	lrc_node = (mp_lrc_node_t *)malloc(sizeof(mp_lrc_node_t));
	if (lrc_node != NULL) {
		memset(lrc_node, 0, sizeof(mp_lrc_node_t));
	}
	return lrc_node;
}

mp_lyric_mgr_t*
mp_lyric_mgr_parse_buffer(mp_lyric_mgr_t *lyric_mgr, const char *lrcBuffer)
{
	startfunc;

	MP_CHECK_NULL(lrcBuffer);
	mp_lyric_mgr_t *lrc_data = lyric_mgr;
	MP_CHECK_NULL(lrc_data);

	/* Parse lyric data line by line */
	char *line = NULL;
	const char *p = lrcBuffer;
	int len = strlen(p);
	int pos = 0;
	do {
		pos += _mp_lyric_mgr_get_line(&p[pos], &line);

		/* Save the lyric */
		mp_lrc_node_t *new_node = _mp_lyric_mgr_node_new();
		if (new_node == NULL) {
			IF_FREE(line);
			return NULL;
		}
		new_node->time = 0;
		if (new_node->lyric != NULL) {
			free(new_node->lyric);
		}
		new_node->lyric = strdup(line);

		lrc_data->unsynclrc_list = eina_list_append(lrc_data->unsynclrc_list, (gpointer)new_node);

		/* Parse and sort the lyric by time tag */
		_mp_lyric_mgr_parse_line(&lrc_data, line);
	} while (pos <= len);
	DEBUG_TRACE("unsync count=%d\n", eina_list_count(lrc_data->unsynclrc_list));

	endfunc;
	IF_FREE(line);
	return lrc_data;
}

static bool
_mp_parse_lyric(mp_lyric_mgr_t *lyric_mgr, const char *musicPath)
{
	Eina_List *synclrc_list = NULL;
	char *unsynclrc_buffer = NULL;
	int sync = 0;
	bool ret = false;

	metadata_extractor_h handle = NULL;
	int mmf_error = -1;
	MP_CHECK_FALSE(musicPath);

	mmf_error = metadata_extractor_create(&handle);
	MP_CHECK_FALSE(mmf_error == METADATA_EXTRACTOR_ERROR_NONE);

	mmf_error = metadata_extractor_set_path(handle, musicPath);
	if (mmf_error == METADATA_EXTRACTOR_ERROR_NONE) {
		int sync_len = 0;
		char *unsynclyrics = NULL;
		int unsync_len = 0;

		char *value = NULL;
		mmf_error = metadata_extractor_get_metadata(handle, METADATA_SYNCLYRICS_NUM, &value);
		if (mmf_error == METADATA_EXTRACTOR_ERROR_NONE && value) {
			sync_len = atoi(value);
		}
		SAFE_FREE(value);

		mmf_error = metadata_extractor_get_metadata(handle, METADATA_UNSYNCLYRICS, &unsynclyrics);
		if (mmf_error == METADATA_EXTRACTOR_ERROR_NONE && unsynclyrics) {
			unsync_len = strlen(unsynclyrics);
		} else {
			DEBUG_TRACE("fail to metadata_extractor_get_metadata() %x", mmf_error);
		}
		/* unsynclyrics must be freed */

		if (sync_len || unsync_len) {
			DEBUG_TRACE("sync_len=%d", sync_len);

			if (sync_len > 0) {
				int ret = 0;
				int idx = 0;
				unsigned long time_info = 0;
				char *lyrics_info = NULL;

				sync = 1;

				for (idx = 0; idx < sync_len ; idx++) {
					/*Get one time and lyrics info.*/
					ret = metadata_extractor_get_synclyrics(handle, idx, &time_info, &lyrics_info);
					if (ret == METADATA_EXTRACTOR_ERROR_NONE && lyrics_info != NULL && strlen(lyrics_info) > 0) {
						mp_lrc_node_t *new_node = malloc(sizeof(mp_lrc_node_t));
						if (new_node == NULL) {
							if (handle) {
								metadata_extractor_destroy(handle);
								handle = NULL;
							}
							IF_FREE(lyrics_info);
							return false;
						}
						new_node->time = time_info;
						new_node->lyric = g_strdup(lyrics_info);

						synclrc_list = eina_list_append(synclrc_list, (gpointer)new_node);

						DEBUG_TRACE("[%2d][%6d][%s]", idx, time_info, lyrics_info);
					} else {
						ERROR_TRACE("Error when get lyrics");
						sync = 0;
						mp_lrc_node_t *node = NULL;
						Eina_List *next = NULL;
						EINA_LIST_FOREACH(synclrc_list, next, node) {
							if (node) {
								if (node->lyric != NULL) {
									free(node->lyric);
								}
								free(node);
							}
						}
						eina_list_free(synclrc_list);
						synclrc_list = NULL;
						break;
					}
				}
				IF_FREE(lyrics_info);
			} else {
				unsynclrc_buffer = unsynclyrics;
			}
			ret = true;
		} else {
			DEBUG_TRACE("No lyric infomation");
			IF_FREE(unsynclyrics);
		}
	}

	if (handle) {
		metadata_extractor_destroy(handle);
		handle = NULL;
	}

	if (sync) {
		lyric_mgr->synclrc_list = synclrc_list;
	} else {
		mp_lyric_mgr_parse_buffer(lyric_mgr, unsynclrc_buffer);
		IF_FREE(unsynclrc_buffer);
	}

	return ret;
}

mp_lyric_mgr_t *
mp_lyric_mgr_create(const char *path)
{
	startfunc;
	mp_lyric_mgr_t *lyric_mgr = calloc(1, sizeof(mp_lyric_mgr_t));
	MP_CHECK_NULL(lyric_mgr);
	if (!_mp_parse_lyric(lyric_mgr, path)) {
		IF_FREE(lyric_mgr);
		return NULL;
	}


	/* Sort lyric line */
	Eina_List *list = lyric_mgr->synclrc_list;
	int count = eina_list_count(list);

	if (count <= 0) {
		list = lyric_mgr->unsynclrc_list;
		count = eina_list_count(list);
	}

	list = eina_list_sort(list, count, _mp_lyric_mgr_node_sort);

	return lyric_mgr;
}


void
mp_lyric_mgr_destory(mp_lyric_mgr_t *lyric_mgr)
{
	MP_CHECK(lyric_mgr);
	_mp_lyric_mgr_data_free(&lyric_mgr);
}
