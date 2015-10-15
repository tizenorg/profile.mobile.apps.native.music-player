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


#include "music.h"
#include "mp-util.h"
#include <telephony.h>
#include "mp-list.h"
#include "mp-list-view.h"
#include "mp-file-tag-info.h"
#include "mp-playlist-mgr.h"
#include "mp-ug-launch.h"
#include "mp-widget.h"
#include "mp-player-mgr.h"
#include <app.h>
#include <sound_manager.h>
#include <player.h>
#include <system_settings.h>
#include <mime_type.h>
#include <mp-file-util.h>
#include <runtime_info.h>
#include <storage.h>
#include <device/display.h>
#include <device/callback.h>
#include <device/power.h>
//TEMP_BLOCK
//#include <power.h>
#include <notification.h>
#include "metadata_extractor.h"
#ifdef MP_SOUND_PLAYER
#else
#include "mp-common.h"
#endif

bool track_deleted = false;
#define SINGLE_BYTE_MAX 0x7F

struct index_s
{
	const char *index;
	unsigned short start;
	unsigned short end;
};

static struct index_s multi_index[] = {
	{"\xE3\x84\xB1", 0xAC00, 0xB098},	/* Kiyeok + A */
	{"\xE3\x84\xB4", 0xB098, 0xB2E4},	/* Nieun + A */
	{"\xE3\x84\xB7", 0xB2E4, 0xB77C},
	{"\xE3\x84\xB9", 0xB77C, 0xB9C8},
	{"\xE3\x85\x81", 0xB9C8, 0xBC14},
	{"\xE3\x85\x82", 0xBC14, 0xC0AC},
	{"\xE3\x85\x85", 0xC0AC, 0xC544},
	{"\xE3\x85\x87", 0xC544, 0xC790},
	{"\xE3\x85\x88", 0xC790, 0xCC28},
	{"\xE3\x85\x8A", 0xCC28, 0xCE74},
	{"\xE3\x85\x8B", 0xCE74, 0xD0C0},
	{"\xE3\x85\x8C", 0xD0C0, 0xD30C},
	{"\xE3\x85\x8D", 0xD30C, 0xD558},
	{"\xE3\x85\x8E", 0xD558, 0xD7A4},	/* Hieuh + A */

	{"\xE3\x84\xB1", 0x3131, 0x3134},	/* Kiyeok */
	{"\xE3\x84\xB4", 0x3134, 0x3137},	/* Nieun */
	{"\xE3\x84\xB7", 0x3137, 0x3139},
	{"\xE3\x84\xB9", 0x3139, 0x3141},
	{"\xE3\x85\x81", 0x3141, 0x3142},
	{"\xE3\x85\x82", 0x3142, 0x3145},
	{"\xE3\x85\x85", 0x3145, 0x3147},
	{"\xE3\x85\x87", 0x3147, 0x3148},
	{"\xE3\x85\x88", 0x3148, 0x314A},
	{"\xE3\x85\x8A", 0x314A, 0x314B},
	{"\xE3\x85\x8B", 0x314B, 0x314C},
	{"\xE3\x85\x8C", 0x314C, 0x314D},
	{"\xE3\x85\x8D", 0x314D, 0x314E},
	{"\xE3\x85\x8E", 0x314E, 0x314F},	/* Hieuh */
};

static char *single_upper_index[] = {
	"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
	"O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"
};

static char *single_lower_index[] = {
	"a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m", "n",
	"o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z"
};


static char *single_numeric_index[] = {
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "."
};

static const char *kor_sec[] = {
	"\xe3\x85\x8f",		/* A */
	"\xe3\x85\x90",		/* AE */
	"\xe3\x85\x91",		/* YA */
	"\xe3\x85\x92",
	"\xe3\x85\x93",
	"\xe3\x85\x94",
	"\xe3\x85\x95",
	"\xe3\x85\x96",
	"\xe3\x85\x97",
	"\xe3\x85\x98",
	"\xe3\x85\x99",
	"\xe3\x85\x9a",
	"\xe3\x85\x9b",
	"\xe3\x85\x9c",
	"\xe3\x85\x9d",
	"\xe3\x85\x9e",
	"\xe3\x85\x9f",
	"\xe3\x85\xa0",
	"\xe3\x85\xa1",
	"\xe3\x85\xa2",
	"\xe3\x85\xa3",
};

static unsigned char mask_len[] = {
	0x80, /* 1000 0000 */ 0x00,	/* 0xxx xxxx */
	0xE0, /* 1110 0000 */ 0xC0,	/* 110x xxxx */
	0xF0, /* 1111 0000 */ 0xE0,	/* 1110 xxxx */
	0xF8, /* 1111 1000 */ 0xF0,	/* 1111 0xxx */
	0xFC, /* 1111 1100 */ 0xF8,	/* 1111 10xx */
	0xFE, /* 1111 1110 */ 0xFC,	/* 1111 110x */
};

static int externalStorageId = -1;

extern struct appdata *g_ad;

EXPORT_API struct appdata *mp_util_get_appdata(void)
{
	return g_ad;
}

static int
_mp_util_get_len(const char *p)
{
	int i, r = -1;
	unsigned char c;

	if (p)
	{
		c = *p;
		for (i = 0; i < sizeof(mask_len) / sizeof(char); i = i + 2)
		{
			if ((c & mask_len[i]) == mask_len[i + 1])
			{
				r = (i >> 1) + 1;
				break;
			}
		}
	}

	return r;
}

static unsigned short
_mp_util_utf8_to_ucs2(const char *p)
{
	unsigned short r = 0;
	int len;

	len = _mp_util_get_len(p);
	if (len == -1 || len > 3)
	{
		return r;
	}

	switch (len)
	{
	case 1:
		{
			r = *p & 0x7F;
			break;
		}
	case 2:
		{
			r = *p & 0x1F;
			break;
		}
	case 3:
		{
			r = *p & 0x0F;
			break;
		}
	default:
		{
			break;
		}
	}

	while (len > 1)
	{
		r = r << 6;
		p++;
		r |= *p & 0x3F;
		len--;
	}

	return r;
}

static const char *
_mp_util_get_single(const char *p)
{
	int c = (int)*p;

	if (islower(c) != 0)
	{
		return single_lower_index[c - 'a'];
	}
	else if (isupper(c) != 0)
	{
		return single_upper_index[c - 'A'];
	}
	else if (48 <= c && 57 >= c)
	{
		return single_numeric_index[c - '0'];
	}
	else
	{
		return single_numeric_index[10];
	}

	return NULL;
}

static const char *
_mp_util_get_multi(unsigned short u)
{
	int i;

	for (i = 0; i < sizeof(multi_index) / sizeof(struct index_s); i++)
	{
		if (u >= multi_index[i].start && u < multi_index[i].end)
		{
			return multi_index[i].index;
		}
	}
	return NULL;
}

static char *
_mp_util_get_next_char(const char *p)
{
	int n;

	MP_CHECK_NULL(p);

	n = _mp_util_get_len(p);
	if (n == -1)
	{
		return NULL;
	}

	if (strlen(p) < n)
	{
		return NULL;
	}

	DEBUG_TRACE("%s", &p[n]);

	return (char *)&p[n];
}


static const char *
_mp_util_get_second_kor(unsigned short u)
{
	unsigned short t;

	t = u - 0xAC00;
	t = (t / 28) % 21;

	return kor_sec[t];
}

void
mp_util_format_duration(char *time, int ms)
{
	int sec = (ms) / 1000;
	int min = sec / 60;

	int hour = min / 60;
	snprintf(time, TIME_FORMAT_LEN, "%02u:%02u:%02u", hour, min % 60, sec % 60);
}

void
mp_util_song_format_duration(char *time, int ms)
{
	int sec = (ms) / 1000;
	int min = sec / 60;

	if (min >= 10) {
			int hour = min / 60;
			snprintf(time, TIME_FORMAT_LEN, "%02u:%02u:%02u", hour, min % 60, sec % 60);
		} else {
			snprintf(time, TIME_FORMAT_LEN, "%02u:%02u", min, sec % 60);
		}
}

const char *
mp_util_get_index(const char *p)
{
	if (p == NULL)
	{
		return NULL;
	}

	if ((unsigned char)*p < SINGLE_BYTE_MAX)
	{
		return _mp_util_get_single(p);
	}

	return _mp_util_get_multi(_mp_util_utf8_to_ucs2(p));
}

const char *
mp_util_get_second_index(const char *p)
{
	unsigned short u2;

	if (p == NULL)
	{
		return NULL;
	}

	if ((unsigned char)*p < SINGLE_BYTE_MAX)
	{
		return mp_util_get_index(_mp_util_get_next_char(p));
	}

	u2 = _mp_util_utf8_to_ucs2(p);
	if (u2 >= 0xAC00 && u2 < 0xD7A4)
	{
		return _mp_util_get_second_kor(u2);
	}

	return mp_util_get_index(_mp_util_get_next_char(p));
}

bool
mp_util_add_to_playlist_by_key(int playlist_id, char *key_id)
{
	int err;
	{
		err = mp_media_info_playlist_add_media(playlist_id, key_id);
		if (err != 0)
		{
			ERROR_TRACE("Error in mp_media_info_playlist_add_media (%d)\n", err);
			return FALSE;
		}
	}
	return TRUE;
}

Evas_Object *
mp_util_create_image(Evas_Object * obj, const char *path, const char *group, int w, int h)
{
	MP_CHECK_NULL(obj);

	Evas_Object *image = elm_image_add(obj);
	if (w == h)
	{
		elm_image_prescale_set(image, w);
		elm_image_fill_outside_set(image, true);
	}

	if (!path)
		path = DEFAULT_THUMBNAIL;
	elm_image_file_set(image, path, group);

	elm_image_resizable_set(image, EINA_TRUE, EINA_TRUE);
	evas_object_size_hint_align_set(image, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(image, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(image);

	return image;
}

Evas_Object *
mp_util_create_thumb_icon(Evas_Object * obj, const char *path, int w, int h)
{
	//startfunc;
	Evas_Object *thumbnail = elm_image_add(obj);
	if (w == h)
	{
		elm_image_prescale_set(thumbnail, w);
		elm_image_fill_outside_set(thumbnail, EINA_FALSE);
		elm_image_aspect_fixed_set(thumbnail, EINA_TRUE);
	}

	if ((!path) || !g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR) || !strcmp(BROKEN_ALBUMART_IMAGE_PATH, path))
		path = DEFAULT_THUMBNAIL;
	elm_image_file_set(thumbnail, path, NULL);

	evas_object_size_hint_align_set(thumbnail, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(thumbnail);
	//endfunc;

	return thumbnail;
}

Evas_Object *
mp_util_create_lazy_update_thumb_icon(Evas_Object * obj, const char *path, int w, int h)
{
	return mp_util_create_thumb_icon(obj, path, w, h);
}


Evas_Object *
mp_util_create_selectioninfo_with_count(void *data, int count)
{
	startfunc;
	MpView_t *view = data;
	MP_CHECK_NULL(view);
    if (count > 0)
    {
        char *text =  g_strdup_printf(GET_STR(STR_MP_SELECT_ITEMS),count);
        mp_view_set_title(view, text);
        IF_FREE(text);
    }
    else
    {
        mp_view_set_title(view, STR_MP_TILTE_SELECT_ITEM);
    }

	return NULL;

}

void
mp_util_post_status_message(struct appdata *ad, const char *text)
{
	int ret = notification_status_message_post(text);
	if (ret != 0)
		ERROR_TRACE("notification_status_message_post()... [0x%x]", ret);
	else
		mp_debug("message: [%s]", text);
}

void
mp_util_post_add_to_playlist_popup_message(int count)
{
        MP_CHECK(count>0);
	int ret = 0;
        char *message = NULL;
        if (count > 1)
        {
                message = g_strdup_printf(GET_STR(STR_MP_POP_ADDED_TO_PLAYLIST), count);
                ret = notification_status_message_post(message);
                IF_FREE(message);
        }
        else if (count == 1)
        {
                ret = notification_status_message_post(GET_STR(STR_MP_POP_ADDED_1_TO_PLAYLIST));
        }
	if (ret != 0)
		ERROR_TRACE("notification_status_message_post()... [0x%x]", ret);
}

int
mp_util_share_via_email(const char *formed_path, void *data)
{
	if (mp_ug_email_attatch_file(formed_path, data))
		return -1;

	return 0;
}

char *
mp_util_get_new_playlist_name(void)
{
	char unique_name[MP_PLAYLIST_NAME_SIZE] = "\0";
	int ret = 0;
	ret = mp_media_info_playlist_unique_name(GET_STR(STR_MP_MY_PLAYLIST), unique_name, MP_PLAYLIST_NAME_SIZE);
	if (ret == 0) {
		if (strlen(unique_name) <= 0) {
			ERROR_TRACE("playlist name is NULL");
			return NULL;
		} else {
			return g_strdup(unique_name);
		}
	} else {
		ERROR_TRACE("fail to mp_media_info_playlist_unique_name() : error code [%x] ", ret);
		return NULL;
	}

	return NULL;
}

//Note: This function can be called from delete thread.
//Do not call any function that is not thread safty!! especially UIFW functions
mp_file_delete_err_t
mp_util_delete_track(void *data, char *fid, char *file_path)
{
	int ret = 0;
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	ad->is_sdcard_removed = true;

 	MP_CHECK_VAL(fid, MP_FILE_DELETE_ERR_INVALID_FID);

	char *path = NULL;
	mp_media_info_h item = NULL;
	if (!file_path)
	{
		mp_media_info_create(&item, fid);
		if (item) {
			mp_media_info_get_file_path(item, &path);
			if (!path) {
				mp_media_info_destroy(item);
			}
		}
		MP_CHECK_VAL(path, MP_FILE_DELETE_ERR_INVALID_FID);
	}
	else
		path = file_path;

	DEBUG_TRACE("path: %s", path);
	ret = remove(path);
	mp_media_info_delete_from_db(path);
	track_deleted = true;

	if (item)
		mp_media_info_destroy(item);

	if (ret < 0)
	{
		ERROR_TRACE("fail to remove file, ret: %d", ret);
		//if (show_popup)
			//mp_widget_text_popup(ad, GET_SYS_STR("IDS_COM_POP_FAILED"));
		return MP_FILE_DELETE_ERR_REMOVE_FAIL;
	}

	return MP_FILE_DELETE_ERR_NONE;
}

int
mp_util_file_is_in_phone_memory(const char *path)
{
	MP_CHECK_VAL(path, 0);
	if (!strncmp(MP_PHONE_ROOT_PATH, path, strlen(MP_PHONE_ROOT_PATH)))
		return 1;
	else
		return 0;
}

// return value must be freed.
char *
mp_util_isf_get_edited_str(Evas_Object * isf_entry, bool permit_first_blank)
{

	const char *buf = NULL;
	char *strip_msg = NULL;
	int strip_len = 0;

	if (!isf_entry)
		return strdup("");
	buf = elm_entry_entry_get(isf_entry);
	if (!buf)
		return strdup("");

	strip_msg = elm_entry_markup_to_utf8(buf);

	if (strip_msg != NULL)
	{
		strip_len = strlen(strip_msg);

		if (strip_len > 0)
		{
			if (strip_msg[0] == ' ' && !permit_first_blank)	//start with space
			{
				DEBUG_TRACE("Filename should not be started with blank");
				free(strip_msg);
				return strdup("");
			}

			if (strip_msg[strip_len - 1] == '\n' || strip_msg[strip_len - 1] == '\r')
			{
				strip_msg[strip_len - 1] = '\0';
			}
			DEBUG_TRACE("=====  The new edited str = %s", strip_msg);
			return strip_msg;
		}
		else
		{
			DEBUG_TRACE(" strip_msg length is [%d], strip_msg [%s]", strip_len, strip_msg);
			return strip_msg;
		}
	}
	else
	{
		DEBUG_TRACE("strip_msg is NULL");
		return strdup("");
	}
}

bool
mp_util_set_screen_mode(void *data, int mode)
{
	struct appdata *ad = data;

	ad->current_appcore_rm = mode;	//set current appcore rm
	elm_win_screen_size_get(ad->win_main, NULL, NULL, &ad->screen_width, &ad->screen_height);

	if (mode == APP_DEVICE_ORIENTATION_270 || mode == APP_DEVICE_ORIENTATION_90)
	{
		ad->screen_mode = MP_SCREEN_MODE_LANDSCAPE;
		mp_debug("Set MP_SCREEN_MODE_LANDSCAPE");
	}
	else if (mode == APP_DEVICE_ORIENTATION_0 || mode == APP_DEVICE_ORIENTATION_180)
	{
		ad->screen_mode = MP_SCREEN_MODE_PORTRAIT;
		mp_debug("Set MP_SCREEN_MODE_PORTRAIT");
	}
	return true;
}

bool
mp_util_is_streaming(const char *uri)
{
	if (uri == NULL || strlen(uri) == 0)
	{
		return FALSE;
	}

	if (uri == strstr(uri, "http://") || uri == strstr(uri, "https://")
			|| uri == strstr(uri, "rtp://") || uri == strstr(uri, "rtsp://")) {
		DEBUG_TRACE("Streaming URI... OK");
		return TRUE;
	}
	else
	{
		//DEBUG_TRACE("uri check failed : [%s]", uri);
		return FALSE;
	}
}

bool
mp_check_file_exist(const char *path)
{
	if (path == NULL || strlen(path) == 0)
	{
		return FALSE;
	}

	bool mmc_removed = mp_util_is_mmc_removed();

	if (mmc_removed && strstr(path, MP_MMC_ROOT_PATH) == path)
	{
		return false;
	}

	if (strstr(path,MP_FILE_PREFIX))
        {
                if (!g_file_test(path+strlen(MP_FILE_PREFIX), G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
                {
                	ERROR_TRACE("file not exist: %s", path);
                        return FALSE;
                }
                return TRUE;
        }
        else
        {
                if (!g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_REGULAR))
                {
                	ERROR_TRACE("file not exist: %s", path);
                        return FALSE;
                }
                return TRUE;
        }
	return TRUE;
}

bool mp_util_text_multiline_check(Evas_Object *obj,const char*text, const char*textstyle, int text_width, int text_height)
{
        MP_CHECK_FALSE(obj);
	Evas_Object *tb;
	Evas_Coord ww = 0;
	Evas_Coord hh = 0;
	Evas_Textblock_Style *st = NULL;
	Evas_Textblock_Cursor *cur = NULL;
	char *strbuf = NULL;

	tb = evas_object_textblock_add(evas_object_evas_get(obj));
	if (!tb)
		goto END;
	evas_object_textblock_legacy_newline_set(tb, EINA_FALSE);
	st = evas_textblock_style_new();
	if (!st)
		goto END;
	evas_textblock_style_set(st, textstyle);
	evas_object_textblock_style_set(tb, st);
	cur = evas_object_textblock_cursor_new(tb);
	strbuf = elm_entry_markup_to_utf8(text);
	if (!strbuf)
		goto END;
	evas_object_textblock_text_markup_set(tb, strbuf);
	evas_textblock_cursor_format_prepend(cur, "+ wrap=mixed");
	evas_object_resize(tb, text_width, 1000);
	evas_object_textblock_size_formatted_get(tb, &ww, &hh);

END:
	IF_FREE(strbuf);
	mp_evas_object_del(tb);

	if (st)
	{
		evas_textblock_style_free(st);
		st = NULL;
	}
	if (cur)
	{
		evas_textblock_cursor_free(cur);
		cur = NULL;
	}
	if (hh > text_height)
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


bool mp_util_file_playable(const char *uri)
{
	metadata_extractor_h metadata = NULL;
	char *value = NULL;
	bool res = false;
        int ret = METADATA_EXTRACTOR_ERROR_NONE;

	if (!mp_check_file_exist(uri))
		return false;

	ret = metadata_extractor_create(&metadata);
	MP_CHECK_FALSE(metadata);
        if (ret != METADATA_EXTRACTOR_ERROR_NONE)
                ERROR_TRACE("create error");

	ret = metadata_extractor_set_path(metadata, uri);
        if (ret != METADATA_EXTRACTOR_ERROR_NONE)
                ERROR_TRACE("set path error");

	metadata_extractor_get_metadata(metadata, METADATA_HAS_AUDIO, &value);

	if (value && g_strcmp0(value, "0"))
		res = true;

	IF_FREE(value);
	metadata_extractor_destroy(metadata);
	DEBUG_TRACE("playable[%d]", res);
	return res;
}

char * mp_util_file_mime_type_get(const char *uri)
{
	int retcode = -1;
	char *mime = NULL;

	if (!mp_check_file_exist(uri))
		return NULL;

	char *extension = strrchr(uri, '.');
	char *file_ext = g_strdup(extension + 1);
	retcode = mime_type_get_mime_type(file_ext, &mime);
	if ((mime == NULL) || (retcode != MIME_TYPE_ERROR_NONE)) {
		WARN_TRACE("Fail to get mime type with return value [%d]", retcode);
		return NULL;
	}
	return mime;
}

bool
mp_util_launch_browser(const char *url, struct appdata * ad)
{

	app_control_h service;
	bool res;
	app_control_create(&service);
	app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
	app_control_set_app_id(service, "com.samsung.browser");
	app_control_set_uri(service, url);

	if (app_control_send_launch_request(service, NULL, NULL) == APP_CONTROL_ERROR_NONE) {
		DEBUG_TRACE("Succeeded to launch a calculator app.");
		res = true;
	} else {
		DEBUG_TRACE("Failed to launch a calculator app.");
		res = false;
	}
	app_control_destroy(service);

	return res;
}


//korean initial consonant
gunichar
mp_util_get_utf8_initial_value(const char *name)
{
	gunichar first;
	char *next = NULL;
	MP_CHECK_VAL(name, 0);

	if (g_utf8_strlen(name, -1) <= 0)
	{
		return 0;
	}

	first = g_utf8_get_char_validated(name, g_utf8_strlen(name, -1));
	if (first == (gunichar) - 1 || first == (gunichar) - 2) {
		DEBUG_TRACE ("failed to convert a sequence of bytes encoded as UTF-8 to a Unicode character.");
		return 0;
	}

	next = (char *)name;

	while (!g_unichar_isgraph(first))
	{
		next = g_utf8_next_char(next);
		first = g_utf8_get_char_validated(next, g_utf8_strlen(name, -1));
		if (first == (gunichar) - 1 || first == (gunichar) - 2) {
			DEBUG_TRACE ("failed to convert a sequence of bytes encoded as UTF-8 to a Unicode character.");
			return 0;
		}
	}

	if (first >= 0xAC00 && first <= 0xD7A3)
	{			//korean
		int index = 0;
		index = ((((first - 0xAC00) - ((first - 0xAC00) % 28)) / 28) / 21);
		if (index < 20 && index >= 0)
		{
			const gunichar chosung[20] = { 0x3131, 0x3132, 0x3134, 0x3137, 0x3138,
				0x3139, 0x3141, 0x3142, 0x3143, 0x3145,
				0x3146, 0x3147, 0x3148, 0x3149, 0x314a,
				0x314b, 0x314c, 0x314d, 0x314e, 0
			};

			return chosung[index];
		}
	}
	else
	{
		return first;
	}
	return 0;
}


char *
mp_util_get_title_from_path(const char *path)
{
	gchar *file_ext = NULL, *file_name = NULL, *title = NULL;

	if (path == NULL || strlen(path) == 0)
	{
		return NULL;
	}

	file_name = g_path_get_basename(path);
	if (file_name)
	{
		file_ext = g_strrstr(file_name, ".");
		if (file_ext)
		{
			title = g_strndup(file_name, strlen(file_name) - strlen(file_ext));
		}
		free(file_name);
	}
	DEBUG_TRACE("title = %s", title);
	return title;
}

bool
mp_util_is_playlist_name_valid(char *name)
{
	MP_CHECK_NULL(name);

	char *test_space = strdup(name);
	if (strlen(g_strchug(test_space)) == 0)
	{
		IF_FREE(test_space);
		return FALSE;
	}
	IF_FREE(test_space);
	return TRUE;
}

int
mp_util_create_playlist(struct appdata *ad, char *name, mp_playlist_h *playlist_handle)
{
	MP_CHECK_VAL(ad, -1);
	MP_CHECK_VAL(name, -1);

	int plst_uid = -1;

	if (!mp_util_is_playlist_name_valid(name))
	{
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_CREATE_PLAYLIST"));
		return -1;
	}

	bool exist = false;
	int ret = mp_media_info_playlist_is_exist(name, &exist);
	if (ret != 0)
	{
		ERROR_TRACE("Fail to get playlist count by name: %d", ret);
		mp_widget_text_popup(ad, GET_STR("IDS_MUSIC_POP_UNABLE_CREATE_PLAYLIST"));
		return -1;
	}

	if (exist)
	{
		char *text = g_strdup_printf(GET_STR(STR_MP_POP_PLAYLIST_EXISTS), name);
		mp_widget_text_popup(ad, text);
		IF_FREE(text);
		return -1;
	}

	ret = mp_media_info_playlist_insert_to_db(name, &plst_uid, playlist_handle);
	if (ret != 0)
	{
		ERROR_TRACE("Fail to get playlist count by name: %d", ret);
		mp_widget_text_popup(ad, GET_SYS_STR("IDS_COM_BODY_UNABLE_TO_ADD"));
		*playlist_handle = NULL;
		return -1;
	}

	return plst_uid;
}

void
mp_util_reset_genlist_mode_item(Evas_Object *genlist)
{
	MP_CHECK(genlist);
	Elm_Object_Item *gl_item =
		(Elm_Object_Item *)elm_genlist_decorated_item_get(genlist);
	if (gl_item) {
		elm_genlist_item_decorate_mode_set(gl_item, "slide", EINA_FALSE);
		elm_genlist_item_select_mode_set(gl_item, ELM_OBJECT_SELECT_MODE_DEFAULT);
		mp_list_item_data_t *item_data = elm_object_item_data_get(gl_item);
		item_data->checked = true;
	}
}

bool
mp_util_is_image_valid(Evas *evas, const char *path)
{
	if (!path) return false;
	MP_CHECK_FALSE(evas);

	if (!mp_file_exists(path)) {
		//VER_TRACE("file not exitst, path: %s", path);
		return false;
	}

	Evas_Object *image = NULL;
	int width = 0;
	int height = 0;

	image = evas_object_image_add(evas);
	MP_CHECK_FALSE(image);
	evas_object_image_file_set(image, path, NULL);
	evas_object_image_size_get(image, &width, &height);
	evas_object_del(image);

	if (width <= 0 || height <= 0) {
		//mp_debug("Cannot load file : %s", path);
		return false;
	}
	return true;
}

#define MP_PATH_INFO_MAX_LENGTH		30
#define MP_PATH_INFO_TRANS_OMIT		".."
#define MP_PATH_INFO_LEVEL_BOUNDARY		3
#define MP_PATH_INFO_LEN_THRESHOLD	3
#define MP_PATH_INFO_SEP		"/"
#define MP_PATH_INFO_RETRENCH		128

bool
mp_util_is_string_elipsized(char *path)
{
	MP_CHECK_FALSE(path);
	if (strlen(path) < MP_PATH_INFO_MAX_LENGTH)
	{
		return false;
	}
	else
		return true;
}

char *mp_util_path_info_retrench(const char *string)
{
	mp_retvm_if (string == NULL, g_strdup(MP_PATH_INFO_TRANS_OMIT), "input path is NULL");
	char *retrench = NULL;
	if (strlen (string) > MP_PATH_INFO_LEN_THRESHOLD) {
		char *utf8_string = elm_entry_utf8_to_markup(string);
		MP_CHECK_NULL(utf8_string);
		if (g_utf8_strlen(utf8_string, -1) > 2) {
			retrench = calloc(1, MP_PATH_INFO_RETRENCH);
			if (retrench) {
				g_utf8_strncpy(retrench, utf8_string, 2);
				char *temp = retrench;
				retrench = g_strconcat(retrench, MP_PATH_INFO_TRANS_OMIT, NULL);
				free(utf8_string);
				free(temp);
			}
			else
				free(utf8_string);

		} else {
			retrench = utf8_string;
		}
	} else {
		retrench = elm_entry_utf8_to_markup(string);
	}
	return retrench;
}

char *
mp_util_shorten_path(char *path_info)
{
	int start = 0;
	gchar **params = NULL;
	int count = 0;
//	int len;
	int i = 0;
	int j = 0;
	char *output = NULL;
	char *temp = NULL;
	char *base = NULL;
	bool exception = true;

	MP_CHECK_EXCEP(path_info);

	if (!mp_util_is_string_elipsized(path_info))
		return g_strdup(path_info);

	params = g_strsplit(path_info, "/", 0);
	MP_CHECK_EXCEP(params);

	count = g_strv_length(params);

	if (count > MP_PATH_INFO_LEVEL_BOUNDARY)
	{
		start = count - MP_PATH_INFO_LEVEL_BOUNDARY;
		output = g_strdup("..");
	}
	else
	{
		output = g_strdup("");
	}
	MP_CHECK_EXCEP(output);

	for (i=start ; i < count; i++)
	{
		base = g_strdup(output);
		MP_CHECK_EXCEP(base);
		for (j=i ; j < count; j++)
		{
			temp = g_strconcat(base, MP_PATH_INFO_SEP, params[j], NULL);
			IF_FREE(base);
			base = temp;
			temp = NULL;
		}

		if (i == (count-1) || !mp_util_is_string_elipsized(base))
		{
			IF_FREE(output);
			output = base;
			base = NULL;
			break;
		}
		else
		{
			char *retrench = mp_util_path_info_retrench(params[i]);
			MP_CHECK_EXCEP(retrench);
			//len = strlen(params[i]);
			IF_FREE(base);
			base = g_strconcat(output, MP_PATH_INFO_SEP, retrench, NULL);
			IF_FREE(output);
			free(retrench);
			output = base;
			base = NULL;
		}
	}

	exception = false;

	mp_exception:


	if (params)
		g_strfreev(params);

	if (exception)
	{
		IF_FREE(output);
		IF_FREE(base);
		return g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
	}
	else
		return output;
}

bool
mp_util_is_earjack_inserted(void)
{
	int value;
	if (runtime_info_get_value_int(RUNTIME_INFO_KEY_AUDIO_JACK_STATUS, &value)) {
		DEBUG_TRACE("Failed to get ear jack status");
		return false;
	}

	if (value == RUNTIME_INFO_AUDIO_JACK_STATUS_UNCONNECTED)
		return false;
	else
		return true;
}

void
mp_util_get_sound_path(mp_snd_path *snd_path)
{
	//Replaced for _prod dependency start
	sound_device_h device;
	sound_device_type_e type = SOUND_DEVICE_BUILTIN_SPEAKER;
	sound_device_list_h g_device_list = NULL;
	sound_device_mask_e g_device_mask = SOUND_DEVICE_IO_DIRECTION_OUT_MASK;
	WARN_TRACE("Enter sound_manager_get_active_device");
	int ret;
	if ((ret= sound_manager_get_current_device_list(g_device_mask,&g_device_list)))
		ERROR_TRACE("sound_manager_get_active_device()... [0x%x]", ret);

	if (!(ret = sound_manager_get_next_device(g_device_list, &device))) {
		ERROR_TRACE("success to get next device\n");
		if ((ret = sound_manager_get_device_type (device, &type)))
			ERROR_TRACE("failed to get device type, ret[0x%x]\n", ret);
	}

	switch (type) {
	case SOUND_DEVICE_BUILTIN_SPEAKER:
		*snd_path = MP_SND_PATH_SPEAKER;
		break;
	case SOUND_DEVICE_AUDIO_JACK:
		*snd_path = MP_SND_PATH_EARPHONE;
		break;
	case SOUND_DEVICE_BLUETOOTH:
		*snd_path = MP_SND_PATH_BT;
		break;
	case SOUND_DEVICE_HDMI:
		*snd_path = MP_SND_PATH_HDMI;
		break;
	case SOUND_DEVICE_MIRRORING:
		*snd_path = MP_SND_PATH_MIRRORING;
		break;
	case SOUND_DEVICE_USB_AUDIO:
		*snd_path = MP_SND_PATH_USB_AUDIO;
		break;
	default:
		*snd_path = MP_SND_PATH_SPEAKER;
		break;
	}
	//Replaced for _prod dependency end
}

#define DEF_BUF_LEN				(512)
const char *
mp_util_search_markup_keyword(const char *string, char *searchword, bool *result)
{
	char pstr[DEF_BUF_LEN + 1] = {0,};
	static char return_string[DEF_BUF_LEN + 1] = { 0, };
	int word_len = 0;
	int search_len = 0;
	int i = 0;
	bool found = false;
	gchar* markup_text_start = NULL;
	gchar* markup_text_end= NULL;
	gchar* markup_text= NULL;

	MP_CHECK_NULL(string && strlen(string));
	MP_CHECK_NULL(searchword && strlen(searchword));
	MP_CHECK_NULL(result);

	if (g_utf8_validate(string,-1,NULL)) {

		word_len = strlen(string);
		if (word_len > DEF_BUF_LEN) {
			char *temp = (char*)calloc((word_len + 1), sizeof(char));
			MP_CHECK_NULL(temp);
			strncpy(temp, string, strlen(string));
			i = 0;
			while (word_len > DEF_BUF_LEN)
			{
				/*truncate uft8 to byte_size DEF_BUF_LEN*/
				gchar *pre_ch = g_utf8_find_prev_char(temp, (temp+ DEF_BUF_LEN - 1 - i*3));
				if (!pre_ch) {
					break;
				}
				gchar *next_ch = g_utf8_find_next_char(pre_ch, NULL);
				if (!next_ch) {
					break;
				}
				/*truncate position*/
				*next_ch = '\0';
				word_len = strlen(temp);
				i++;
			}
			if (strlen(temp) <= DEF_BUF_LEN)
				strncpy(pstr, temp, strlen(temp));
			IF_FREE(temp);
		} else {
			if (strlen(string) <= DEF_BUF_LEN)
				strncpy(pstr, string, strlen(string));
		}

		word_len = strlen(pstr);
		search_len = strlen(searchword);

		for (i = 0; i < word_len; i++) {
			if (!strncasecmp(searchword, &pstr[i], search_len)) {
				found = true;
				break;
			}
		}

		*result = found;
		memset(return_string, 0x00, DEF_BUF_LEN+1);

		if (found) {
			int r = 222;
			int g = 111;
			int b = 31;
			int a = 255;

			if (i == 0) {
				markup_text = g_markup_escape_text(&pstr[0], search_len);
				MP_CHECK_NULL(markup_text);
				markup_text_end = g_markup_escape_text(&pstr[search_len], word_len-search_len);
				if (!markup_text_end) {
					IF_FREE(markup_text);
					ERROR_TRACE("markup_text_end  NULL !!!");
					return NULL;
				}
				snprintf(return_string,
						 DEF_BUF_LEN,
						 "<color=#%02x%02x%02x%02x>%s</color>%s", r, g, b, a,
						 markup_text,
						 (char*)markup_text_end);
				IF_FREE(markup_text);
				IF_FREE(markup_text_end);
			} else {
				markup_text_start = g_markup_escape_text(&pstr[0], i);
				MP_CHECK_NULL(markup_text_start);
				markup_text = g_markup_escape_text(&pstr[i], search_len);
				if (!markup_text) {
					IF_FREE(markup_text_start);
					ERROR_TRACE("markup_text  NULL !!!");
					return NULL;
				}
				markup_text_end =  g_markup_escape_text(&pstr[i+search_len], word_len-(i+search_len));
				if (!markup_text_end) {
					IF_FREE(markup_text_start);
					IF_FREE(markup_text);
					ERROR_TRACE("markup_text_end  NULL !!!");
					return NULL;
				}

				snprintf(return_string,
						 DEF_BUF_LEN,
						 "%s<color=#%02x%02x%02x%02x>%s</color>%s",
						 (char*)markup_text_start,
						 r, g, b, a,
						 markup_text,
						 (char*)markup_text_end);
				IF_FREE(markup_text);
				IF_FREE(markup_text_start);
				IF_FREE(markup_text_end);
			}
		} else {
			snprintf(return_string, DEF_BUF_LEN, "%s", pstr);
		}
	}

	return return_string;
}


bool
mp_util_is_other_player_playing(void)
{
	bool ret = FALSE;

	int state = 0;
	if (preference_get_int(PREF_MUSIC_STATE, &state) == 0) {
		if (state == PREF_MUSIC_PLAY) {
			int pid = mp_setting_get_nowplaying_id();
			if (pid != -1) {
				if (pid > 0 && pid != getpid()) {
					mp_debug("## other player is playing some music ##");
					ret = TRUE;
				}
			} else {
				mp_error("mp_setting_get_nowplaying_id() error");
			}
		}
	} else {
		mp_error("preference_get_int() error");
	}

	return ret;
}

bool
mp_util_is_now_active_player(void)
{
	bool ret = FALSE;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	if (ad->player_state == PLAY_STATE_PLAYING || ad->player_state == PLAY_STATE_PAUSED)
	{
		int pid = mp_setting_get_nowplaying_id();
		if (pid != -1)
		{
			if (pid == 0 || pid == getpid())
			{
				ret = TRUE;
			}
		}
		else
		{
			mp_error("mp_setting_get_nowplaying_id() error");
		}
	}

	return ret;
}

int
mp_commmon_check_rotate_lock(void)
{
	bool lock = FALSE;
	int retcode = -1;
	retcode = system_settings_get_value_bool(SYSTEM_SETTINGS_KEY_DISPLAY_SCREEN_ROTATION_AUTO, &lock);
	if (retcode == SYSTEM_SETTINGS_ERROR_NONE) {
		mp_debug("lock state: %d", lock);
		return (int)lock;
	}
	else {
		ERROR_TRACE("Could not get the lock state.Invalid parameter return [%d]", retcode);
		return -1;
	}
}

int
mp_check_mass_storage_mode(void)
{
	return 0;
}

/*static Eina_Bool
_sleep_timer_cb(void *data)
{
	TIMER_TRACE();
	struct appdata * ad = g_ad;
	MP_CHECK_FALSE(ad);

	EVENT_TRACE("sleep_unlock");
	// TEMP_BLOCK
	//power_unlock_state(POWER_STATE_SCREEN_OFF);
	mp_player_mgr_safety_volume_set(ad->app_is_foreground);
	ad->sleep_locked = false;
	ad->sleep_unlock_timer = NULL;
	return false;
}*/

bool
mp_util_sleep_lock_set(bool lock, bool force_unlock)
{
// TEMP_BLOCK #if 0
#if 0
	int ret = POWER_ERROR_NONE;
	struct appdata * ad = mp_util_get_appdata();

	MP_CHECK_FALSE(ad);
	mp_ecore_timer_del(ad->sleep_unlock_timer);

	if (ad->sleep_locked == lock)
	{
		return true;
	}

	if (lock) {
		EVENT_TRACE("sleep_lock");
		ret = power_lock_state(POWER_STATE_SCREEN_OFF, 0);
		mp_player_mgr_safety_volume_set(ad->app_is_foreground);
	} else {
		if (!force_unlock)
			ad->sleep_unlock_timer = ecore_timer_add(30, _sleep_timer_cb, ad);
		else
		{
			//unlock power state immediately
			_sleep_timer_cb(ad);
		}
	}
	ad->sleep_locked = lock;

	if (ret != POWER_ERROR_NONE) {
		mp_error("pm_lock(un_lock) error.. [%d]", ret);
		return FALSE;
	}
#endif
	return TRUE;
}

bool
mp_util_is_nfc_feature_on(void)
{
	return false;
}

void
mp_util_strncpy_safe(char *x_dst, const char *x_src, int max_len)
{
	if (!x_src || strlen(x_src) == 0) {
		mp_error("x_src is NULL");
		return;
	}

	if (max_len < 1) {
		mp_error("length is Wrong");
		return;
	}

    strncpy(x_dst, x_src, max_len-1);
	x_dst[max_len-1] = '\0';
}
#ifdef MP_IMAGE_EFFECT
static const double gaussian_template[7][7] =
{
	{0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067},
	{0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292},
	{0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117},
	{0.00038771, 0.01330373, 0.11098164, 0.22508352, 0.11098164, 0.01330373, 0.00038771},
	{0.00019117, 0.00655965, 0.05472157, 0.11098164, 0.05472157, 0.00655965, 0.00019117},
	{0.00002292, 0.00078633, 0.00655965, 0.01330373, 0.00655965, 0.00078633, 0.00002292},
	{0.00000067, 0.00002292, 0.00019117, 0.00038771, 0.00019117, 0.00002292, 0.00000067}
};

#define DARK_SCALE 0.6

static void __mp_util_gaussian_blur(unsigned char *src, unsigned char *dest, int w, int h)
{
         MP_CHECK(src);
         MP_CHECK(dest);

         int x, y, i, j, idx, idx2, xx, yy;
         for (y = 0; y < h; y++) {
                   for (x = 0; x < w; x++) {
		   idx = (y*w+x)*4;
                   double v1 = 0, v2 = 0, v3 = 0;

                   for (i = 0; i < 7; i++) {
                            for (j = 0; j < 7; j++) {
                                     yy = y + j;
                                     xx = x + i;
                                     if (xx >= w)
                                               xx = w - 1;
                                     if (yy >= h)
                                               yy = h - 1;
                                     idx2 = (yy*w+xx)*4;
                                     v1 += (*(src+idx2))*gaussian_template[i][j];
                                     v2 += (*(src+idx2+1))*gaussian_template[i][j];
                                     v3 += (*(src+idx2+2))*gaussian_template[i][j];
                            }
                   }
                   *(dest+idx) = v1 * DARK_SCALE;
                   *(dest+idx+1) = v2 * DARK_SCALE;
                   *(dest+idx+2) = v3 * DARK_SCALE;
                   *(dest+idx+3) = (*(src+idx+3));
                   }
         }
}


bool mp_util_edit_image(Evas *evas, Evas_Object *src_image, const char *path, int mode)
{
	startfunc;
	MP_CHECK_FALSE(evas);
	MP_CHECK_FALSE(src_image);
	MP_CHECK_FALSE(path);
	MP_CHECK_FALSE(mode >= MP_PLAYING_VIEW_TOP_LEFT);
	MP_CHECK_FALSE(mode <= MP_PLAYING_VIEW_BOTTOM_RIGHT);

	float rate_w = 720.0/1500.0;
	float rate_h = 1280.0/1500.0;

	DEBUG_TRACE("rate_w=%f, rate_h=%f", rate_w, rate_h);

	Evas_Object *image = evas_object_image_add(evas);
	evas_object_image_file_set(image, path, NULL);

	int w, h, dest_w, dest_h, x, y;
	evas_object_image_size_get(image, &w, &h);
	unsigned char *src = NULL;
	src = (unsigned char *)evas_object_image_data_get(image, EINA_FALSE);

	MP_CHECK_FALSE(src);
	DEBUG_TRACE("path=%s, w=%d, h=%d", path, w, h);
	dest_w = (int)(rate_w * w);
	dest_h = (int)(rate_h * h);
	DEBUG_TRACE("rate_w=%f, rate_h=%f, dest_w=%d, dest_h=%d", rate_w, rate_h, dest_w, dest_h);

	int start_x, start_y, end_x, end_y;

	switch (mode) {
	case MP_PLAYING_VIEW_TOP_LEFT:
		start_x = 0;
		start_y = 0;
		break;

	case MP_PLAYING_VIEW_TOP_CENTER:
		start_x = (w - dest_w)/2;
		start_y = 0;
		break;

	case MP_PLAYING_VIEW_TOP_RIGHT:
		start_x = w - dest_w;
		start_y = 0;
		break;

	case MP_PLAYING_VIEW_BOTTOM_LEFT:
		start_x = 0;
		start_y = h - dest_h;
		break;

	case MP_PLAYING_VIEW_BOTTOM_CENTER:
		start_x = (w - dest_w)/2;
		start_y = h - dest_h;
		break;

	case MP_PLAYING_VIEW_BOTTOM_RIGHT:
		start_x = w - dest_w;
		start_y = h - dest_h;
		break;

	default:
		return false;
	}

	unsigned char *dest = NULL;
	dest = (unsigned char *)malloc(dest_w * dest_h * 4);
	MP_CHECK_EXCEP(dest);
	memset(dest, 0, dest_w * dest_h * 4);

	end_x = start_x + dest_w;
	end_y = start_y + dest_h;
	DEBUG_TRACE("(%d, %d), (%d, %d)", start_x, start_y, end_x, end_y);

	int dest_idx = 0;
	int src_idx = 0;
	unsigned char gray = 0;
	for (y = start_y; y < end_y; y++) {
		for (x = start_x; x < end_x; x++) {
			dest_idx = ((y-start_y)*dest_w+(x-start_x))*4;
			src_idx = (y*w+x)*4;

			gray = (*(src+src_idx))*0.3+(*(src+src_idx+1))*0.59+(*(src+src_idx+2))*0.11;
			*(dest+dest_idx) = gray;
			*(dest+dest_idx+1) = gray;
			*(dest+dest_idx+2) = gray;
			*(dest+dest_idx+3) = 0;
			//*(dest+dest_idx+3) = (*(src+src_idx+3));
		}
	}

	unsigned char *dest_data = NULL;
	dest_data = (unsigned char *)malloc(dest_w * dest_h * 4);
	MP_CHECK_EXCEP(dest_data);
	memset(dest_data, 0, dest_w * dest_h * 4);
	__mp_util_gaussian_blur(dest, dest_data, dest_w, dest_h);
	IF_FREE(dest);

	evas_object_image_data_set(src_image, NULL);
	evas_object_image_size_set(src_image, dest_w, dest_h);
	evas_object_image_smooth_scale_set(src_image, EINA_TRUE);
	evas_object_image_data_copy_set(src_image, dest_data);
	evas_object_image_data_update_add(src_image, 0, 0, dest_w, dest_h);
	IF_FREE(dest_data);

	mp_evas_object_del(image);

	endfunc;
	return true;

	mp_exception:
	mp_evas_object_del(image);
	IF_FREE(dest);
	IF_FREE(dest_data);
	return false;
}
#endif

void
mp_util_free_track_info(mp_track_info_t *track_info)
{
	if (!track_info) return;

	IF_FREE(track_info->uri);
	IF_FREE(track_info->title);
	IF_FREE(track_info->artist);
	IF_FREE(track_info->album);
	IF_FREE(track_info->genre);
	IF_FREE(track_info->date);
	IF_FREE(track_info->location);
	IF_FREE(track_info->format);
	IF_FREE(track_info->media_id);

	IF_FREE(track_info->thumbnail_path);
	IF_FREE(track_info->copyright);

	IF_FREE(track_info->author);
	IF_FREE(track_info->track_num);
	IF_FREE(track_info->year);

	free(track_info);
}

void
mp_util_load_track_info(struct appdata *ad, mp_plst_item *cur_item, mp_track_info_t **info)
{
	MP_CHECK(ad);
	MP_CHECK(cur_item);
	MP_CHECK(info);

	int ret = 0;
	mp_media_info_h svc_audio_item = NULL;
	mp_track_info_t *track_info = NULL;

	*info = track_info = calloc(1, sizeof(mp_track_info_t));
	MP_CHECK(track_info);

	track_info->track_type = cur_item->track_type;
	track_info->uri = g_strdup(cur_item->uri);
	track_info->playlist_member_id = cur_item->playlist_memeber_id;
	if (!cur_item->uid)
	{
		char *media_id = NULL;

		ret = mp_media_info_create_by_path(&svc_audio_item, track_info->uri);
		if (ret == 0)
		{
			mp_media_info_get_media_id(svc_audio_item, &media_id);
			cur_item->uid = g_strdup(media_id);
		}
	}

	if (cur_item->uid)
	{
		if (!svc_audio_item)
			ret = mp_media_info_create(&svc_audio_item, cur_item->uid);

		mp_media_info_get_title(svc_audio_item, &track_info->title);
		mp_media_info_get_album(svc_audio_item, &track_info->album);
		mp_media_info_get_artist(svc_audio_item, &track_info->artist);
		mp_media_info_get_thumbnail_path(svc_audio_item, &track_info->thumbnail_path);
		mp_media_info_get_genre(svc_audio_item, &track_info->genre);
		mp_media_info_get_recorded_date(svc_audio_item, &track_info->date);
		mp_media_info_get_copyright(svc_audio_item, &track_info->copyright);
		mp_media_info_get_composer(svc_audio_item, &track_info->author);
		mp_media_info_get_duration(svc_audio_item, &track_info->duration);
		mp_media_info_get_track_num(svc_audio_item, &track_info->track_num);
		mp_media_info_get_format(svc_audio_item, &track_info->format);
		mp_media_info_get_favorite(svc_audio_item, &track_info->favorite);
		mp_media_info_get_year(svc_audio_item, &track_info->year);

		track_info->media_id = g_strdup(cur_item->uid);

		track_info->title = g_strdup(track_info->title);
		track_info->album = g_strdup(track_info->album);
		track_info->artist = g_strdup(track_info->artist);
		track_info->thumbnail_path = g_strdup(track_info->thumbnail_path);
		track_info->genre = g_strdup(track_info->genre);
		//track_info->date = g_strdup(track_info->date);
		track_info->copyright = g_strdup(track_info->copyright);
		track_info->author = g_strdup(track_info->author);
		track_info->track_num = g_strdup(track_info->track_num);
		track_info->format = g_strdup(track_info->format);
		track_info->location = g_strdup(track_info->location);
		track_info->year = g_strdup(track_info->year);

	}
	else if (mp_check_file_exist(cur_item->uri))
	{
		mp_tag_info_t tag_info;
		mp_file_tag_info_get_all_tag(cur_item->uri, &tag_info);


		track_info->title = g_strdup(tag_info.title);
		track_info->album = g_strdup(tag_info.album);
		track_info->artist = g_strdup(tag_info.artist);
		track_info->thumbnail_path = g_strdup(tag_info.albumart_path);
		track_info->genre = g_strdup(tag_info.genre);
		track_info->date = g_strdup(tag_info.date);
		track_info->copyright = g_strdup(tag_info.copyright);
		track_info->author = g_strdup(tag_info.author);
		track_info->track_num = g_strdup(tag_info.track);

		track_info->duration = tag_info.duration;

		GString *format = g_string_new("");
		if (format)
		{
			if (tag_info.audio_bitrate > 0)
				g_string_append_printf(format, "%dbps ", tag_info.audio_bitrate);

			if (tag_info.audio_samplerate > 0)
				g_string_append_printf(format, "%.1fHz ", (double)tag_info.audio_samplerate);

			if (tag_info.audio_channel > 0)
				g_string_append_printf(format, "%dch", tag_info.audio_channel);

			track_info->format = g_strdup(format->str);
			g_string_free(format, TRUE);
		}

		mp_file_tag_free(&tag_info);
	}
	else
	{
		track_info->title = g_strdup(cur_item->title);
		track_info->artist = g_strdup(cur_item->artist);
		track_info->thumbnail_path = g_strdup(cur_item->thumbnail_path);
	}

	track_info->isDiffAP = cur_item->isDiffAP;

	if (track_info->duration < 0)
		track_info->duration = 0;

	if (svc_audio_item) {
		mp_media_info_destroy(svc_audio_item);
	}
}

void
mp_util_append_media_list_item_to_playlist(mp_plst_mgr *playlist_mgr, mp_media_list_h media_list, int count, int current_index, const char *path)
{
	int i;
	char *uid = NULL;
	char *uri = NULL;
	char *title = NULL;
	char *artist = NULL;
	mp_plst_item *cur_item = NULL;

	for (i = 0; i < count; i++)
	{
		mp_plst_item *plst_item;
		mp_track_type track_type = MP_TRACK_URI;
		mp_media_info_h item = mp_media_info_list_nth_item(media_list, i);
		mp_media_info_get_media_id(item, &uid);
		mp_media_info_get_file_path(item, &uri);
		mp_media_info_get_title(item, &title);
		mp_media_info_get_artist(item, &artist);
#ifdef MP_FEATURE_CLOUD
		mp_storage_type_e storage;
		mp_media_info_get_storage_type(item, &storage);
		if (storage == MP_STORAGE_CLOUD)
			track_type = MP_TRACK_CLOUD;
#endif
		plst_item = mp_playlist_mgr_item_append(playlist_mgr, uri, uid, title, artist, track_type);
		if (i == current_index || !g_strcmp0(uri, path))
			cur_item = plst_item;
	}
	mp_playlist_mgr_set_current(playlist_mgr, cur_item);

}

char* mp_util_get_fid_by_full_path(const char *full_path, char **title, char **artist)
{
	startfunc;

	char *uid = NULL;
	char *val = NULL;

	MP_CHECK_NULL(full_path);

	int ret = 0;
	mp_media_info_h record = NULL;
	if (mp_check_file_exist(full_path))
	{
		ret = mp_media_info_create_by_path(&record, full_path);
		if (ret == 0)
		{
			ret = mp_media_info_get_media_id(record, &uid);
			uid = g_strdup(uid);

			if (title)
			{
				mp_media_info_get_title(record, &val);
				*title = g_strdup(val);
			}
			if (artist)
			{
				mp_media_info_get_artist(record, &val);
				*artist = g_strdup(val);
			}
			mp_media_info_destroy(record);
		}
	}
	return uid;
}

static inline const char *_mp_util_get_text_domain(const char *string_id)
{
	const char *domain = DOMAIN_NAME;

	if (string_id) {
#ifdef STORE_DOMAIN_NAME
		else if (strstr(string_id, "IDS_MH") || strstr(string_id, "IDS_IS"))
			domain = STORE_DOMAIN_NAME;
#endif
	}

	return domain;
}

EXPORT_API char *mp_util_get_text(const char *str)
{
        MP_CHECK_NULL(str);

	const char *domain_name = _mp_util_get_text_domain(str);
	return dgettext(domain_name, str);
}

void mp_util_more_btn_move_ctxpopup(Evas_Object *ctxpopup, Evas_Object *btn)
{
	MP_CHECK(ctxpopup);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *win;
	Evas_Coord w, h;
	int pos = -1;

	win = elm_object_top_widget_get(ctxpopup);
	elm_win_screen_size_get(win, NULL, NULL, &w, &h);
	pos = elm_win_rotation_get(win);

	switch (pos) {
		case 0:
		case 180:
			evas_object_move(ctxpopup, (w/2), h);
			break;
		case 90:
			evas_object_move(ctxpopup, 0, w);
			break;
		case 270:
			evas_object_move(ctxpopup, (h/2), w);
			break;

	}
}

void mp_util_object_item_translate_set(Elm_Object_Item *item, const char *ID)
{
	MP_CHECK(ID);
	MP_CHECK(item);
	const char *domain = _mp_util_get_text_domain(ID);
 	elm_object_item_domain_text_translatable_set(item, domain, EINA_TRUE);
}

Elm_Object_Item *
mp_util_ctxpopup_item_append_ext(Evas_Object *obj, const char *label, const char *file,
	                         const char *group, Evas_Smart_Cb func,
	                         const void *data)
{
	Elm_Object_Item *item = elm_ctxpopup_item_append(obj, label, NULL, func, data);
	MP_CHECK_NULL(item);
	mp_util_object_item_translate_set(item, label);
	return item;
}

Elm_Object_Item *
mp_util_ctxpopup_item_append(Evas_Object *obj, const char *label,
	                         const char *group, Evas_Smart_Cb func,
	                         const void *data)
{
	return mp_util_ctxpopup_item_append_ext(obj, label, NULL, group, func, data);
}

Elm_Object_Item *mp_util_toolbar_item_append(Evas_Object *obj, const char *icon,
				const char *label, Evas_Smart_Cb func,
				const void *data)
{
	Elm_Object_Item *item = elm_toolbar_item_append(obj, icon, label, func, data);
	MP_CHECK_NULL(item);

	const char *domain = _mp_util_get_text_domain(label);
	elm_object_item_domain_text_translatable_set(item, domain, EINA_TRUE);
	return item;
}

Elm_Object_Item *mp_util_toolbar_nth_item(Evas_Object *obj, int n)
{
	MP_CHECK_NULL(obj);
	Elm_Object_Item *it = elm_toolbar_first_item_get(obj);
	int i = 0;
	for (i = 0; i<n; i++)
	{
		it  = elm_toolbar_item_next_get(it );
	}
	return it;
}

bool
mp_util_get_sip_state(void)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	return ad->sip_state;
}

bool
mp_util_is_landscape(void)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_FALSE(ad);

	return (ad->win_angle == 90 || ad->win_angle == -90) ? true : false;
}

int
mp_util_parse_device_type(const char *name)
{
	MP_CHECK_VAL(name, MP_DEVICE_TYPE_UNKNOWN);

	char *p = strstr(name, "[");
	if (p)
		p++;
	else
		return MP_DEVICE_TYPE_UNKNOWN;
	char * str = NULL;
	p = g_strdup(p);	/* strtok make string dirty */
	char *device = strtok_r(p, "]" ,&str);
	int type = MP_DEVICE_TYPE_UNKNOWN;
	if (!g_strcmp0(device, "TV"))
		type = MP_DEVICE_TYPE_TV;
	if (!g_strcmp0(device, "PC"))
		type = MP_DEVICE_TYPE_DESKTOP_PC;
	else
		type = MP_DEVICE_TYPE_UNKNOWN;

	//mp_debug("type = %d [%s]", type, device);

	SAFE_FREE(str);
	SAFE_FREE(p);
	return type;
}

void
mp_util_set_livebox_update_timer(void)
{
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	DEBUG_TRACE("ad->is_lcd_off[%d] ad->is_focus_out[%d] ad->app_is_foreground[%d] ", ad->is_lcd_off, ad->is_focus_out, ad->app_is_foreground);

	if (ad->live_pos_timer)
	{
		if (ad->is_focus_out && !ad->app_is_foreground)
		{
			if (!ad->is_lcd_off && mp_player_mgr_get_state() == PLAYER_STATE_PLAYING)
			{
				WARN_TRACE("thaw livebox pos timer");
				MP_TIMER_THAW(ad->live_pos_timer);
				return;
			}
		}
		WARN_TRACE("freeze livebox pos timer");
		MP_TIMER_FREEZE(ad->live_pos_timer);
	}
}

static Eina_Bool _print_geometry_cb(void *data)
{
	int x, y, w, h;
	Evas_Object *obj = data;

	evas_object_geometry_get(obj, &x, &y, &w, &h);

 	return false;
}

void mp_util_print_geometry(Evas_Object *obj, const char *name)
{
	ecore_timer_add(3, _print_geometry_cb, obj);
	evas_object_data_set(obj, "obj_name", name);
}

static inline void _mp_util_launch_unlock_password()
{
	eventfunc;

	app_control_h service = NULL;
	int ret = APP_CONTROL_ERROR_NONE;

	ret = app_control_create(&service);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_create() .. [0x%x]", ret);
		return;
	}

	ret = app_control_set_operation(service, APP_CONTROL_OPERATION_DEFAULT);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_set_operation() .. [0x%x]", ret);
		app_control_destroy(service);
		return;
	}

	ret = app_control_add_extra_data(service, "lock_type", "go_to_password");
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_add_extra_data() .. [0x%x]", ret);
		app_control_destroy(service);
		return;
	}

	ret = app_control_send_launch_request(service, NULL, NULL);
	if (ret != APP_CONTROL_ERROR_NONE) {
		mp_error("app_control_send_launch_request() .. [0x%x]", ret);
		app_control_destroy(service);
		return;
	}

	app_control_destroy(service);
}

void mp_util_lock_cpu()
{
	int ret = device_power_request_lock(POWER_LOCK_CPU, 0);

	if (ret) {
		ERROR_TRACE("device_power_request_lock()...[0x%x]", ret);
	}
}

void mp_util_release_cpu()
{
	int ret = device_power_release_lock(POWER_LOCK_CPU);

	if (ret) {
		ERROR_TRACE("device_power_release_lock()...[0x%x]", ret);
	}
}

void mp_util_hide_lock_screen()
{
	display_state_e lock_state;
	int ret = device_display_get_state(&lock_state);
	if(ret == DEVICE_ERROR_NONE) {
		ERROR_TRACE("[successful] Return value is %d",ret);
	} else {
		ERROR_TRACE("[ERROR] Return value is %d",ret);
	}

	if (lock_state == DISPLAY_STATE_NORMAL) {
		WARN_TRACE("already unlocked");
		return;
	}

	int result = device_display_change_state(DISPLAY_STATE_NORMAL);
	if( result < 0 )
		printf("[ERROR] return value result =%d",result);
	else
		printf("[SUCCESS] return value result =%d",result);
}

bool
mp_util_app_resume(void)
{
	char *app_id = NULL;
	int ret = app_get_id(&app_id);
	if (ret != APP_ERROR_NONE || !app_id) {
		SECURE_ERROR("app_get_id().. [0x%x], app_id[%s]", ret, app_id);
		return false;
	}

	app_context_h context = NULL;
	ret = app_manager_get_app_context(app_id, &context);
	if (ret != APP_MANAGER_ERROR_NONE) {
		mp_error("app_manager_get_app_context().. [0x%x]", ret);
		SAFE_FREE(app_id);
		return false;
	}

	ret = app_manager_resume_app(context);
	if (ret != APP_MANAGER_ERROR_NONE) {
		mp_error("app_manager_resume_app().. [0x%x]", ret);
		SAFE_FREE(app_id);
		return false;
	}

	SAFE_FREE(app_id);
	app_context_destroy(context);

	mp_util_hide_lock_screen();

	return true;
}

bool
mp_util_system_volume_popup_show(void)
{
	bool ret = false;
	app_control_h service = NULL;

	int err = app_control_create(&service);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_create().. [0x%x]", err);
		return false;
	}
	MP_CHECK_FALSE(service);

	err = app_control_set_app_id(service, "org.tizen.volume");
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_set_package().. [0x%x]", err);
		goto END;
	}

	err = app_control_add_extra_data(service, "show_volume", "TRUE");
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_add_extra_data().. [0x%x]", err);
		goto END;
	}

	err = app_control_send_launch_request(service, NULL, NULL);
	if (err != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_send_launch_request().. [0x%x]", err);
		goto END;
	}

	ret = true;

END:
	if (service)
		app_control_destroy(service);

	return ret;
}

bool
mp_util_is_call_connected(void)
{
	telephony_call_state_e state;
	telephony_handle_list_s tel_list;
	int tel_valid = telephony_init(&tel_list);
	if (tel_valid != 0) {
		ERROR_TRACE("telephony is not initialized. ERROR Code is %d",tel_valid);
		return false;
	}

	telephony_h *newhandle = tel_list.handle;

	int error = telephony_call_get_voice_call_state(*newhandle , &state );

	telephony_deinit(&tel_list);

	if (error == TELEPHONY_ERROR_NONE) {
		if (state == TELEPHONY_CALL_STATE_IDLE) {
			return false;   /*There exists no calls*/
		}
		/* There exists at least one call that is dialing, alerting or incoming*/
		return true;
	} else {
		ERROR_TRACE("ERROR: state error is %d",error);
	}

	return false;
}

void mp_util_domain_translatable_text_set(Evas_Object *obj, const char* text)
{
	const char *domain = _mp_util_get_text_domain(text);
	elm_object_domain_translatable_text_set(obj, domain, text);
}

void mp_util_domain_translatable_part_text_set(Evas_Object *obj, const char* part, const char* text)
{
	const char *domain = _mp_util_get_text_domain(text);
	elm_object_domain_translatable_part_text_set(obj, part, domain, text);
}

void mp_util_item_domain_translatable_part_text_set(Elm_Object_Item *item, const char* part, const char* text)
{
	const char *domain = _mp_util_get_text_domain(text);
	char * text_id = elm_entry_utf8_to_markup(text);
	elm_object_item_domain_translatable_part_text_set(item, part, domain, text_id);
	IF_FREE(text_id);
}

bool mp_util_get_supported_storages_callback(int storageId, storage_type_e type, storage_state_e state, const char *path, void *userData)
{
	if (type == STORAGE_TYPE_EXTERNAL) {
		externalStorageId = storageId;
		return false;
	}
	return true;
}

bool mp_util_is_mmc_removed(void)
{
	int error = storage_foreach_device_supported(mp_util_get_supported_storages_callback, NULL);
	if (error == STORAGE_ERROR_NONE) {
		storage_state_e state;
		storage_get_state(externalStorageId, &state);
		if (state == STORAGE_STATE_REMOVED || state == STORAGE_STATE_UNMOUNTABLE) {
 			return true;
 		}
 	}
	return false;
}


#define ELM_INTERNAL_API_ARGESFSDFEFC
#include <elm_widget.h>

EXPORT_API void dump_win(Evas_Object *obj, int max_depth)
{

	if (evas_object_smart_type_check(obj, "elm_win") == false )
	{
		ERROR_TRACE("Obj(0x%08x) is not elm_win Object", obj);
		return;
	}

	void *pData = evas_object_smart_data_get(obj);
	volatile int Diff;

	Diff = 0xC4;

	Eina_List **ppList = (Eina_List **)( (char *)pData + ((Diff)) /* B - A */);
	Eina_List *subobjs = *ppList;

	DEBUG_TRACE("pData=0x%08x SubObj=0x%08x pData+C4=0x%08x SubObjCnt=%d",
		pData, subobjs, (unsigned int)(pData) + (Diff), eina_list_count(subobjs));

	{
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		DEBUG_TRACE("Win=%s(%s,0x%08x) %s(%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj),
			elm_object_widget_type_get(obj), obj, evas_object_type_get(obj), x, y, w, h, pass, repeat, visible, propagate);
	}

	const Eina_List *l;
	Evas_Object *child;

	void *MyData = NULL;

	EINA_LIST_FOREACH(subobjs, l, MyData)
	{
		child = (Evas_Object *)MyData;

		dump_obj(child, 0, max_depth-1);
	}

}

EXPORT_API void dump_obj(Evas_Object *obj, int lvl, int max_depth)
{
	Eina_List *list = evas_object_smart_members_get(obj);

	if (max_depth <= 0)
		return;

	if (lvl == 0 )
	{
		int x, y, w, h;

		evas_object_geometry_get(obj, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(obj);
		Eina_Bool pass = evas_object_pass_events_get(obj);
		Eina_Bool visible = evas_object_visible_get(obj);
		Eina_Bool propagate = evas_object_propagate_events_get(obj);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(obj, &mW, &mH);
		evas_object_size_hint_max_get(obj, &MW, &MH);

		DEBUG_TRACE("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), evas_object_type_get(obj), obj, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);
		lvl++;
	}

	Evas_Object *data;
	Eina_List *l;

	for (l = list, data = (Evas_Object *)eina_list_data_get(l); l; l = eina_list_next(l), data = (Evas_Object *)eina_list_data_get(l))
	{
		int x, y, w, h;

		evas_object_geometry_get(data, &x, &y, &w, &h);
		Eina_Bool repeat = evas_object_repeat_events_get(data);
		Eina_Bool pass = evas_object_pass_events_get(data);
		Eina_Bool visible = evas_object_visible_get(data);
		Eina_Bool propagate = evas_object_propagate_events_get(data);

		int mW, mH, MW, MH;

		evas_object_size_hint_min_get(data, &mW, &mH);
		evas_object_size_hint_max_get(data, &MW, &MH);

		char *space = calloc(sizeof(char), (lvl*2+1));
		if (space)
		{
			int i;
			for ( i = 0; i < lvl*2; i++)
			{
				space[i] = ' ';
			}

			space[lvl*2] = '\0';

			DEBUG_TRACE("%sObj=%s(%s,0x%08x) (%d,%d,%d,%d) m(%d,%d) M(%d,%d) P%d|R%d|V%d|E%d",
				space, evas_object_name_get(data), evas_object_type_get(data), data, x, y, w, h, mW, mH, MW, MH, pass, repeat, visible, propagate);
		}

		IF_FREE(space);

		dump_obj(data, lvl+1, max_depth-1);

	}
}

EXPORT_API void dump_widget(Evas_Object *obj, int lvl, int max_depth)
 {
	 Eina_List *list = evas_object_smart_members_get(obj);

	 if (max_depth <= 0)
		return;

	 if (lvl == 0 )
	 {
		 int x, y, w, h;

		 evas_object_geometry_get(obj, &x, &y, &w, &h);
		 Eina_Bool repeat = evas_object_repeat_events_get(obj);
		 Eina_Bool pass = evas_object_pass_events_get(obj);
		 Eina_Bool visible = evas_object_visible_get(obj);
		 Eina_Bool propagate = evas_object_propagate_events_get(obj);

		 SECURE_DEBUG("Obj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d", evas_object_name_get(obj), elm_object_widget_type_get(obj), obj, x, y, w, h, pass, repeat, visible, propagate);
		 lvl++;
	 }

	 Evas_Object *data;
	 Eina_List *l;

	 for (l = list, data = (Evas_Object *)eina_list_data_get(l); l; l = eina_list_next(l), data = (Evas_Object *)eina_list_data_get(l))
	 {
		 int x, y, w, h;

		 evas_object_geometry_get(data, &x, &y, &w, &h);
		 Eina_Bool repeat = evas_object_repeat_events_get(data);
		 Eina_Bool pass = evas_object_pass_events_get(data);
		 Eina_Bool visible = evas_object_visible_get(data);
		 Eina_Bool propagate = evas_object_propagate_events_get(data);

		if (elm_object_widget_type_get(data) != NULL || evas_object_name_get(data) != NULL )
		{
			char *space = calloc(sizeof(char),(lvl*2+1));
			if (space)
			{
				int i;
				for ( i = 0; i < lvl*2; i++)
				{
					space[i] = ' ';
				}

				space[lvl*2] = '\0';

				SECURE_DEBUG("%sObj=%s(%s,0x%08x) (%d,%d,%d,%d) P%d|R%d|V%d|E%d",
					space, evas_object_name_get(data), elm_object_widget_type_get(data), data, x, y, w, h, pass, repeat, visible, propagate);
			}
			IF_FREE(space);
		 }

		 dump_widget(data, lvl+1, max_depth-1);

	 }
 }

#ifdef MP_FEATURE_PERSONAL_PAGE
int mp_util_get_file_ext(const char *filename, char **file_ext)
{
	assert(filename);
	assert(file_ext);

	char *pdot = strrchr(filename, '.');

	if (!pdot) {
		return -1;
	} else if (pdot != filename) {
		*file_ext = g_strdup(pdot + 1);
		return 0;
	} else {
		return -1;
	}
}

int mp_util_is_duplicated_name(const char *dir, const char *name)
{
	MP_CHECK_VAL(dir,-1);
	MP_CHECK_VAL(name,-1);
	char *file_path = g_strconcat(dir, "/", name, NULL);
	if (mp_file_exists(file_path)) {
		IF_FREE(file_path);
		return -1;
	} else {
		IF_FREE(file_path);
		return 0;
	}
}

static int _mp_util_get_next_number(char *file_name_without_ext)
{
	int nCount = 0;
	int nLength = 0;
	int nUnderline = 0;
	bool bAllDigits = true;
	int i;

	/* check _02d format */
	nLength = strlen(file_name_without_ext);

	if (nLength < 3) {	/*4 means the # of minimum characters (*_n) */
		return 1;	/*doesn't match */
	} else {	/* input is more than 3 bytes */
		/* find '_' */
		for (nUnderline = nLength - 1; nUnderline >= 0; nUnderline--) {
			if (file_name_without_ext[nUnderline] == '_') {
				break;
			}
		}

		if (nUnderline == 0 && file_name_without_ext[0] != '_') {
			return 1;	/* doesn't match */
		}
		/* check the right characters are all digits */
		for (i = nUnderline + 1; i < nLength; i++) {
			if (file_name_without_ext[i] < '0' || file_name_without_ext[i] > '9') {
				bAllDigits = false;
				break;
			}
		}

		if (bAllDigits) {
			for (i = nUnderline + 1; i < nLength; i++) {
				nCount *= 10;
				nCount += file_name_without_ext[i] - '0';
			}

			file_name_without_ext[nUnderline] = '\0';	/* truncate the last  '_dd' */
		}
	}

	/* increase nCount by 1 */
	nCount++;

	return nCount;
}

void mp_util_get_unique_name(char *original_file_name, char **unique_file_name)
{
	assert(unique_file_name);

	char *file_name_without_ext = NULL;
	char *file_ext = NULL;
	char *new_file_name = NULL;
	int nCount = 0;
	bool bExt = false;

	if (original_file_name == NULL) {
		mp_error("original file is NULL");
		goto Exception;
	}

	bExt = mp_util_get_file_ext(original_file_name, &file_ext);
	file_name_without_ext = g_strdup(original_file_name);

	if (file_name_without_ext == NULL) {
		goto Exception;
	}

	/* add a condition, whether extention is or not. */
	if (bExt == 0) {
		file_name_without_ext[strlen(file_name_without_ext) - strlen(file_ext) - 1] = '\0';
	}

	nCount = _mp_util_get_next_number(file_name_without_ext);
	if (nCount == 1) {
		char *file_name_with_space = g_strconcat(file_name_without_ext, " ", NULL);
		if (file_name_with_space) {
			IF_FREE(file_name_without_ext);
			file_name_without_ext = file_name_with_space;
			file_name_with_space = NULL;
		}
	}

	if (bExt == 0) {
		new_file_name = g_strdup_printf("%s_%d.%s", file_name_without_ext, nCount, file_ext);
	}
	else {
		new_file_name = g_strdup_printf("%s_%d", file_name_without_ext, nCount);
	}
	mp_debug("new_file_name [%s]", new_file_name);
	IF_FREE(file_name_without_ext);

	*unique_file_name = g_strdup(new_file_name);

Exception:
	IF_FREE(file_ext);
	IF_FREE(new_file_name);

}

bool mp_util_is_in_personal_page(const char *path)
{
	MP_CHECK_FALSE(path);
	return g_str_has_prefix(path, MP_PERSONAL_PAGE_DIR);
}

bool mp_util_is_personal_page_on()
{
	bool status = FALSE;
	if (preference_get_boolean(KEY_MP_PERSONAL_PAGE, &status) != 0) {
		mp_error("preference_get_int() fail!!");
		status = FALSE;
	}
	if (status)
		return true;
	else
		return false;
}
#endif

bool
mp_util_mirroring_is_connected(void)
{
	return false;
}

bool
mp_util_is_scan_nearby_available()
{
        return false;
}

bool
mp_util_is_store_enable(void)
{
	struct appdata *ad = g_ad;
	MP_CHECK_FALSE(ad);
	return ad->store_enable;
}

bool
mp_util_free_space_check(double size)
{
	struct statvfs s;
	memset(&s, 0, sizeof(struct statvfs));
	int r;

	r = storage_get_internal_memory_size(&s);
	if (r < 0)
	{
		mp_error("get free space failed");
		return false;
	}
	else
	{
		if ((double)s.f_bsize*s.f_bavail >= size)
		{
			return true;
		}
		else
		{
			struct appdata *ad = mp_util_get_appdata();
			if (ad) {
				Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, NULL, NULL, NULL, ad);
				mp_util_domain_translatable_text_set(popup, STR_MP_NOT_ENOUGH_SPACE_ERROR_MSG);
				mp_popup_button_set(popup, MP_POPUP_BTN_1, STR_MP_OK, MP_POPUP_YES);
				evas_object_show(popup);
			}
			return false;
		}
	}
}

bool
mp_util_media_is_uhqa(const char *media_id)
{
	MP_CHECK_FALSE(media_id);

	mp_media_info_h media_info = NULL;
	mp_media_info_create(&media_info, media_id);
	MP_CHECK_FALSE(media_info);

	int sample_rate = 0;

	mp_media_info_get_sample_rate(media_info, &sample_rate);

	mp_media_info_destroy(media_info);
	media_info = NULL;

	DEBUG_TRACE("sample_rate = %d", sample_rate);
	return (bool)(sample_rate >= 192000);
}

mp_dir_e mp_util_get_file_location(const char *uri)
{

	int len_phone = strlen(MP_MUSIC_DIR);
	int len_memory = strlen(MP_MMC_DIR);
	int len_http = strlen(MP_HTTP_DIR);

	if (strncmp(uri, MP_MUSIC_DIR, len_phone) == 0) {
		return MP_DIR_PHONE;
	} else if (strncmp(uri, MP_MMC_DIR, len_memory) == 0) {
		return MP_DIR_MMC;
	}
	else if (strncmp(uri, MP_HTTP_DIR, len_http) == 0) {
		return MP_DIR_HTTP;
	} else {
		return MP_DIR_NONE;
	}
}


