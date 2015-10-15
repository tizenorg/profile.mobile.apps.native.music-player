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

#include <media_content.h>
#include <player.h>

#include "mp-detail-view.h"
#include "mp-file-tag-info.h"
#include "music.h"
#include "mp-widget.h"
#include "mp-popup.h"
#include "mp-common.h"
#include "mp-search-view.h"
#include "mp-util.h"
#include "mp-scroll-page.h"
#include "mp-player-mgr.h"

#ifndef _EDJ
#define _EDJ(obj) elm_layout_edje_get(obj) /**< get evas object from elm layout */
#endif

typedef struct {
	char *header;
	char *detail;
} mp_media_info_t;

#define NAVIFRAME_DETAIL_STYLE	NULL /*"music/player_view"*/
#define MP_ALBUM_INDEX_ICON_SIZE      (360 * elm_config_scale_get())
#define MP_ALBUM_INDEX_ICON_SIZE_W_LD (280 * elm_config_scale_get())
#define MP_ALBUM_INDEX_ICON_SIZE_H_LD (272 * elm_config_scale_get())

static void _mp_detail_view_pop_on_back_button(void *data, Evas_Object *obj, void *event_info);

static char *
_mp_detail_view_get_location_info_from_file_path(char *file_path)
{
	mp_retvm_if (!file_path, NULL, "File path is null...");

	int prefix_pos;
	if (!strncmp(file_path, MP_PHONE_ROOT_PATH, strlen(MP_PHONE_ROOT_PATH))) {
		prefix_pos = strlen(MP_PHONE_ROOT_PATH);
		return g_strdup_printf("Device memory%s", file_path + prefix_pos);
	} else if (!strncmp(file_path, MP_MMC_ROOT_PATH, strlen(MP_MMC_ROOT_PATH))) {
		prefix_pos = strlen(MP_MMC_ROOT_PATH);
		return g_strdup_printf("SD card%s", file_path + prefix_pos);
	} else if (!strncmp(file_path, "/mnt/mmc", strlen("/mnt/mmc"))) {
		prefix_pos = strlen("/mnt/mmc");
		return g_strdup_printf("Memory%s", file_path + prefix_pos);
	} else {
		WARN_TRACE("Unable to get proper location...");
		return strdup(file_path);
	}
}

static void
_mp_detail_view_append_media_info_item(Evas_Object *genlist, char *header, char *detail)
{
	/*startfunc;*/
	MpDetailView_t *view = NULL;
	mp_media_info_t *info = NULL;

	MP_CHECK(genlist);

	view = (MpDetailView_t *)evas_object_data_get(genlist, "view");
	MP_CHECK(view);
	MP_CHECK(view->meta_itc);

	info = calloc(1, sizeof(mp_media_info_t));
	MP_CHECK(info);
	info->header = g_strdup(header);
	info->detail = g_strdup(detail);

	Elm_Object_Item *item = elm_genlist_item_append(genlist, view->meta_itc,
							info,
							NULL,
							ELM_GENLIST_ITEM_NONE,
							NULL, NULL);
	if (item)
		elm_genlist_item_select_mode_set(item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);

}

static void
_mp_detail_info_gl_item_del(void *data, Evas_Object * obj)
{
	mp_media_info_t *info = data;
	MP_CHECK(info);
	IF_FREE(info->header);
	IF_FREE(info->detail);
	IF_FREE(info);
}


static char *
_mp_detail_view_info_gl_text_get(void *data, Evas_Object * obj, const char *part)
{
	mp_media_info_t *info = data;
	MP_CHECK_NULL(info);

	if (!strcmp(part, "elm.text.main.left.top")) {
		char *title = GET_STR(info->header);

		return g_strdup(title);

	} else if (!strcmp(part, "elm.text.sub.left.bottom")) {
		char *sub_title = info->detail;

		MP_CHECK_NULL(sub_title);
		return elm_entry_utf8_to_markup(sub_title);
	}

	return NULL;
}

static void
_mp_detail_view_set_itc(void *thiz, mp_detail_view_itc type)
{
	startfunc;
	MpDetailView_t *view = thiz;
	MP_CHECK(view);

	switch (type) {
	case DETAIL_VIEW_ITC_NO_META:
	case DETAIL_VIEW_ITC_META_INFO:
		view->meta_itc = elm_genlist_item_class_new();
		MP_CHECK(view->meta_itc);
		/*view->meta_itc->item_style = "music/2line.top";*/
		view->meta_itc->item_style = "2line.top";
		view->meta_itc->func.text_get = _mp_detail_view_info_gl_text_get;
		view->meta_itc->func.content_get = NULL;
		view->meta_itc->func.del = _mp_detail_info_gl_item_del;
		break;
	default:
		ERROR_TRACE("itc type out of bound");
		break;
	}
}

static Evas_Object *
_mp_detail_view_create_genlist(Evas_Object *parent)
{
	startfunc;
	Evas_Object *genlist = NULL;
	genlist = mp_widget_genlist_create(parent);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	elm_genlist_homogeneous_set(genlist, EINA_FALSE);

	endfunc;
	return genlist;
}

#define MP_ROUND_D(x, dig) (floor((x) * pow(10, dig+1) + 0.5) / pow(10, dig+1))

int _mp_detail_view_get_file_size(const char *filename, off_t *size)
{
	MP_CHECK_VAL(filename, -1);
	struct stat statbuf;
	if (stat(filename, &statbuf) == -1) {
		return -1;
	}
	*size = statbuf.st_size;
	return 0;

}

void _mp_detail_view_get_file_size_info(char **file_size, off_t src_size)
{
	MP_CHECK(file_size);
	unsigned long long original_size = 0;
	double size = 0;
	int index = 0;
	int len = 0;

	original_size = src_size;
	size = (double)original_size;

	while (size >= 1024) {
		size /= 1024;
		index++;
	}

	if (index == 0) {
		snprintf(NULL, 0, "%llu B%n", original_size, &len);
	} else {
		size = MP_ROUND_D(size, 1);
		snprintf(NULL, 0, "%0.1lf XB%n", size, &len);
	}

	if (len <= 0) {
		*file_size = NULL;
		return;
	}
	len += 1;
	*file_size = (char *)calloc(len, sizeof(char));
	if (*file_size == NULL) {
		return;

	}

	if (index == 0) {
		snprintf(*file_size, len, "%llu B", original_size);
	} else {
		if (index == 1) {
			snprintf(*file_size, len, "%0.1lf KB", size);
		} else if (index == 2) {
			snprintf(*file_size, len, "%0.1lf MB", size);
		} else if (index == 3) {
			snprintf(*file_size, len, "%0.1lf GB", size);
		} else {
			free(*file_size);
			*file_size = NULL;
		}
	}
	return;
}

static Evas_Object *
_mp_detail_view_create_local_without_metadata(void *thiz)
{
	startfunc;
	MpDetailView_t *view = thiz;
	MP_CHECK_NULL(thiz);

	int ret = 0;
	Evas_Object *genlist = view->minfo_genlist;

	mp_media_info_h svc_item = NULL;

	bool get_item = false;


	char *pathname = NULL, *title = NULL, *album = NULL, *artist = NULL, *thumbname = NULL, *date = NULL;
	char *author = NULL, *copyright = NULL,  *track = NULL, *sample = NULL, *mime_type = NULL;

	int duration = 0;
	char *location = NULL;
	int sample_rate = 0;
	int channel = 0;
	int bitpersample = 0;
	char *size_string = NULL;
	char *bitdepth = NULL;

	genlist = _mp_detail_view_create_genlist(view->content);
	MP_CHECK_NULL(genlist);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);
	evas_object_data_set(genlist, "view", view);
	_mp_detail_view_set_itc(view, DETAIL_VIEW_ITC_NO_META);

	if (view->id) {
		ret = mp_media_info_create(&svc_item, view->id);
		DEBUG_TRACE("svc_item is %p", svc_item);
		if (ret == MEDIA_CONTENT_ERROR_NONE && svc_item) {
			mp_media_info_get_file_path(svc_item, &pathname);
			mp_media_info_get_thumbnail_path(svc_item, &thumbname);
			mp_media_info_get_title(svc_item, &title);
			mp_media_info_get_album(svc_item, &album);
			mp_media_info_get_artist(svc_item, &artist);
			mp_media_info_get_recorded_date(svc_item, &date);
			mp_media_info_get_copyright(svc_item, &copyright);
			mp_media_info_get_composer(svc_item, &author);
			mp_media_info_get_duration(svc_item, &duration);
			mp_media_info_get_track_num(svc_item, &track);
			mp_media_info_get_sample_rate(svc_item, &sample_rate);
			location = _mp_detail_view_get_location_info_from_file_path(view->uri);
			mime_type = mp_util_file_mime_type_get(view->uri);
			mp_media_info_get_bitpersample(svc_item, &bitpersample);
			off_t size = 0;
			_mp_detail_view_get_file_size(pathname, &size);
			_mp_detail_view_get_file_size_info(&size_string, size);
			get_item = true;
		}
	}

	if (get_item) {
	_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_TITLE_STR, title);

		if (artist && strlen(artist)) {
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_ARTIST_STR, artist);
		}

		if (album && strlen(album)) {
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_ALBUM_STR, album);

		}

		char duration_format[10] = { 0, };
		int dur_sec = duration / 1000;
		int sec = dur_sec % 60;
		int min = dur_sec / 60;
		snprintf(duration_format, sizeof(duration_format), "%02u:%02u", min, sec);


			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_LENGTH_STR,
							 duration_format);


		if (date && strlen(date)) {
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_RECORDED_DATE_STR, date);

		}

		if (author && strlen(author)) {
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_AUTHOR_STR, author);

		}

		if (copyright && strlen(copyright)) {
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_COPYRIGHT_STR, copyright);

		}

		if (track && strlen(track)) {
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_NUMBER_STR, track);
		}
		_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_FORMAT_STR, mime_type);

		bitdepth = g_strdup_printf("%d bit", bitpersample);
		_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_BITDEPTH_STR, bitdepth);
		SAFE_FREE(bitdepth);

		sample =  g_strdup_printf("%dKHz", sample_rate/1000);
		_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_SAMPLING_STR, sample);
		SAFE_FREE(sample);

		_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_SIZE_STR, size_string);
		SAFE_FREE(size_string);

		_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_LOCATION_STR, location);
		SAFE_FREE(location);

		if (svc_item) {
			mp_media_info_destroy(svc_item);
		}
	} else {
		/*
		**	There are 2 kinds of URI to play streaming and local
		**	detail view is created from player view, it is to say player already exists.
		**	in the precondition, we can use player to get info.
		**	use metadata_extractor to get detail information of local files case
		**	use player to get information of streaming case
		*/
		mp_dir_e located = mp_util_get_file_location(view->uri);
		if (located == MP_DIR_PHONE || located == MP_DIR_MMC) /*local file case*/ {
			mp_tag_info_t tag_info;
			mp_file_tag_info_get_all_tag(view->uri, &tag_info);


			if (tag_info.artist && strlen(tag_info.artist)) {

					_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_ARTIST",
									 tag_info.artist);

			}

			if (tag_info.title && strlen(tag_info.title)) {

					_mp_detail_view_append_media_info_item(genlist, "IDS_COM_BODY_DETAILS_TITLE",
									 tag_info.title);

			}

			if (tag_info.album && strlen(tag_info.album)) {

					_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_ALBUM",
									 tag_info.album);

			}

			char duration_format[10] = { 0, };
			int dur_sec = tag_info.duration / 1000;
			int sec = dur_sec % 60;
			int min = dur_sec / 60;
			snprintf(duration_format, sizeof(duration_format), "%02u:%02u", min, sec);


				_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_TRACK_LENGTH",
								 duration_format);


			if (tag_info.date && strlen(tag_info.date)) {
					_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_RECORDED_DATE",
									 tag_info.date);
			}

			if (tag_info.author && strlen(tag_info.author)) {
					_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_AUTHOR",
									 tag_info.author);
			}

			if (tag_info.copyright && strlen(tag_info.copyright)) {
				_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_COPYRIGHT", tag_info.copyright);
			}

			if (tag_info.track && strlen(tag_info.track)) {
				if (!strstr(tag_info.track, "-") && strcmp(tag_info.track, "0")) {
						_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_TRACK_NUMBER",
										 tag_info.track);
				}
			}

			if (tag_info.rating && strlen(tag_info.rating)) {

					_mp_detail_view_append_media_info_item(genlist, "IDS_MF_BODY_PARENT_RATING",
									 tag_info.rating);
			}

			GString *format = g_string_new("");
			if (tag_info.audio_bitrate > 0)
				g_string_append_printf(format, "%dbps ", tag_info.audio_bitrate);

			if (tag_info.audio_samplerate > 0)
				g_string_append_printf(format, "%.1fHz ", (double)tag_info.audio_samplerate);

			if (tag_info.audio_channel > 0)
				g_string_append_printf(format, "%dch", tag_info.audio_channel);

			if (format) {

				_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_FORMAT",
									 format->str);
				g_string_free(format, TRUE);
			}

			location =
				_mp_detail_view_get_location_info_from_file_path(view->uri);

				_mp_detail_view_append_media_info_item(genlist, "IDS_MUSIC_BODY_MUSIC_LOCATION", location);
			SAFE_FREE(location);


			mp_file_tag_free(&tag_info);
		} else if (located != MP_DIR_NONE) /*streaming case*/ {
			player_h current_player = mp_player_mgr_get_player();
			int error_code = PLAYER_ERROR_NONE;
			char *temp_string = NULL;
			if (mp_player_mgr_is_active() == false) {
				ERROR_TRACE("Player is not actived");
			}

			player_state_e player_state = mp_player_mgr_get_state();
			if (player_state != PLAYER_STATE_PLAYING && player_state != PLAYER_STATE_PAUSED) {
				ERROR_TRACE("Player is not in playing/pause status, unable to get infor from player");
			}

			/*get title*/
			error_code = player_get_content_info(current_player, PLAYER_CONTENT_INFO_TITLE, &title);
			if (!title || strlen(title) == 0 || error_code != PLAYER_ERROR_NONE) {
				IF_FREE(title);
				title = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
			}

			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_TITLE_STR, title);
			IF_FREE(title);

			/*get artist*/
			error_code = player_get_content_info(current_player, PLAYER_CONTENT_INFO_ARTIST, &artist);
			if (!artist || strlen(artist) == 0 || error_code != PLAYER_ERROR_NONE) {
				IF_FREE(artist);
				artist = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
			}
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_ARTIST_STR, artist);
			IF_FREE(artist);

			/*get album*/
			error_code = player_get_content_info(current_player, PLAYER_CONTENT_INFO_ALBUM, &album);
			if (!album || strlen(album) == 0 || error_code != PLAYER_ERROR_NONE) {
				IF_FREE(album);
				album = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
			}
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_ALBUM_STR, album);
			IF_FREE(album);

			/*get length(duration)*/
			error_code = player_get_duration(current_player, &duration);
			if (error_code != PLAYER_ERROR_NONE) {
				IF_FREE(temp_string);
				temp_string = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
			}

			int dur_sec = duration / 1000;
			int sec = dur_sec % 60;
			int min = dur_sec / 60;
			temp_string = g_strdup_printf ("%02u:%02u", min, sec);

			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_LENGTH_STR, temp_string);

			/*get track number*/
			IF_FREE(temp_string);
			temp_string = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_NUMBER_STR, temp_string);

			/*get format ------->need capi to get format*/
			IF_FREE(temp_string);
			temp_string = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_FORMAT_STR, temp_string);

			/*get bit rate*/
			/*get sample rate*/
			error_code = player_get_audio_stream_info(current_player, &sample_rate, &channel, &bitpersample);

			bitdepth = g_strdup_printf("%d bit", bitpersample);
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_BITDEPTH_STR, bitdepth);
			SAFE_FREE(bitdepth);

			sample =  g_strdup_printf("%dKHz", sample_rate/1000);
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_SAMPLING_STR, sample);
			SAFE_FREE(sample);

			/*get size ------->need capi to get sizet*/
			IF_FREE(temp_string);
			temp_string = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_SIZE_STR, temp_string);

			/*get location ------->need capi to get location*/
			IF_FREE(temp_string);
			temp_string = g_strdup(view->uri);
			_mp_detail_view_append_media_info_item(genlist, MP_DETAIL_LOCATION_STR, temp_string);

			IF_FREE(temp_string);
		}
		if (svc_item) {
			mp_media_info_destroy(svc_item);
		}
	}

	evas_object_show(genlist);
	SAFE_FREE(mime_type);
	SAFE_FREE(date);
	DEBUG_TRACE("");
	return genlist;
}

static void
_mp_detail_view_create_default_layout(MpDetailView_t *view)
{
	MP_CHECK(view);
	view->content = mp_common_load_edj(view->layout, MP_EDJ_NAME, "local_media_info_layout");
	MP_CHECK(view->content);

	view->minfo_genlist = _mp_detail_view_create_local_without_metadata(view);
	elm_object_part_content_set(view->content, "list-content", view->minfo_genlist);
	elm_object_part_content_set(view->layout, "list_content", view->content);
}

#ifdef MP_FEATURE_LANDSCAPE
static void _mp_detail_view_content_layout_load(void *thiz)
{
	startfunc;
	MpDetailView_t *view = (MpDetailView_t *)thiz;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->win_main);
	{
		_mp_detail_view_create_default_layout(view);
	}
	endfunc;
}

static void _mp_detail_view_layout_content_set(void *thiz)
{
	startfunc;
	MpDetailView_t *view = (MpDetailView_t *)thiz;
	MP_CHECK(view);
	return;
}
#endif

static void
_mp_detail_view_create_content(void *thiz)
{
	startfunc;
	MpDetailView_t *view = (MpDetailView_t *)thiz;
	MP_CHECK(view);
	_mp_detail_view_create_default_layout(view);
}

static void
_mp_detail_view_destory_cb(void *thiz)
{
	eventfunc;
	MpDetailView_t *view = thiz;
	MP_CHECK(view);

	IF_FREE(view->title);
	IF_FREE(view->uri);
	IF_FREE(view->artist);
	IF_FREE(view->album);
	IF_FREE(view->albumart);
	IF_FREE(view->id);
	IF_FREE(view->thumb);

	if (view->meta_itc)
		elm_genlist_item_class_free(view->meta_itc);
	if (view->credit_itc)
		elm_genlist_item_class_free(view->credit_itc);
	if (view->video_itc)
		elm_genlist_item_class_free(view->video_itc);

	mp_view_fini((MpView_t *)view);

	free(view);
}

static void _mp_detail_view_get_playing_track_data(MpDetailView_t *view)
{
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_track_info_t *info = ad->current_track_info;
	MP_CHECK(info);

	char *uid = NULL;

	uid = mp_util_get_fid_by_full_path(info->uri, NULL, NULL);

	view->title = g_strdup(info->title);
	view->uri = g_strdup(info->uri);
	view->albumart = g_strdup(info->title);
	if (info->artist != NULL)
		view->artist = g_strdup(info->artist);
	else
		view->artist = g_strdup(GET_SYS_STR("IDS_COM_BODY_UNKNOWN"));
	view->album = g_strdup(info->album);
	view->id = g_strdup(uid);
	view->thumb = g_strdup(info->thumbnail_path);

}

static int _mp_detail_view_update(void *thiz)
{
	startfunc;
	MpDetailView_t *view = thiz;
	MP_CHECK_VAL(view, -1);

	_mp_detail_view_get_playing_track_data(view);
	_mp_detail_view_create_content(view);

	return 0;
}

static Eina_Bool _mp_detail_view_pop_cb(void *data, Elm_Object_Item *it)
{
	startfunc;
	MpDetailView_t *view = (MpDetailView_t *)data;
	MP_CHECK_VAL(view, EINA_TRUE);

	mp_view_mgr_pop_view(GET_VIEW_MGR, true);
	endfunc;
	return EINA_TRUE;
}

static void _mp_detail_view_pop_on_back_button(void *data, Evas_Object * obj, void *event_info)
{
	elm_naviframe_item_pop(GET_NAVIFRAME);
}

int _mp_detail_view_update_options(void *thiz)
{

	startfunc;
	MpDetailView_t *view = (MpDetailView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	Evas_Object *btn1 = elm_button_add(view->layout);
	elm_object_style_set(btn1, "naviframe/end_btn/default");
	elm_object_item_part_content_set(view->navi_it, "prev_btn", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _mp_detail_view_pop_on_back_button, view);
	mp_view_set_title((MpView_t *)view, STR_MP_DETAILS);
	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_detail_view_pop_cb, view);
	endfunc;
	return 0;
}

static void
_mp_detail_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	switch (event) {
	case MP_NETWORK_STATE_CHANGED:
		_mp_detail_view_update_options(thiz);
		break;
	case MP_UNSET_NOW_PLAYING: /*unset now playing means there is no track in playlist*/
		mp_view_mgr_pop_a_view(GET_VIEW_MGR, (MpView_t *)thiz);
		break;
	default:
		break;
	}
}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_detail_view_rotate(void *thiz, int init_rotate)
{
	DEBUG_TRACE("mp_player rotated %d", init_rotate);
	MpDetailView_t *view = thiz;
	MP_CHECK(view);

	_mp_detail_view_content_layout_load(view);
	_mp_detail_view_layout_content_set(view);

	mp_view_set_title((MpView_t *)view, view->title);
	mp_view_set_sub_title((MpView_t *)view, view->artist);
	_mp_detail_view_update_options((MpView_t *)view);
}
#endif

static int _mp_detail_view_set_title(void *thiz, char *title)
{
	startfunc;
	MP_CHECK_VAL(thiz, -1);
	MpDetailView_t *view = (MpDetailView_t *)thiz;

	/* set title */
	if (view->inner_navi_it != NULL)
		elm_object_item_part_text_set(view->inner_navi_it, NULL, title);
	else
		elm_object_item_part_text_set(view->navi_it, "elm.text.title", title);
	return 0;
}

static int _mp_detail_view_set_sub_title(void *thiz, char *subtitle)
{
	startfunc;
	MP_CHECK_VAL(thiz, -1);
	MpDetailView_t *view = (MpDetailView_t *)thiz;

	/* set sub title */
	if (view->inner_navi_it != NULL)
		elm_object_item_part_text_set(view->inner_navi_it, "subtitle", subtitle);
	else
		elm_object_item_part_text_set(view->navi_it, "subtitle", subtitle);
	return 0;
}


static int
_mp_detail_view_init(Evas_Object *parent, MpDetailView_t *view, const char *uri)
{
	startfunc;
	int ret = 0;

	ret =  mp_view_init(parent, (MpView_t *)view, MP_VIEW_DETAIL);
	MP_CHECK_VAL(ret == 0, -1);

	view->disable_title_icon = true;

	view->update = _mp_detail_view_update;
	view->update_options = _mp_detail_view_update_options;
	view->update_options_edit = NULL;
	view->view_destroy_cb = _mp_detail_view_destory_cb;
	view->on_event = _mp_detail_view_on_event;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_detail_view_rotate;
#endif

	return ret;
}

MpDetailView_t *mp_detail_view_create(Evas_Object *parent)
{
	eventfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpDetailView_t *view = calloc(1, sizeof(MpDetailView_t));
	MP_CHECK_NULL(view);

	_mp_detail_view_get_playing_track_data(view);
	ret = _mp_detail_view_init(parent, view, view->uri);
	if (ret) goto Error;

	return view;

Error:
	ERROR_TRACE("Error: mp_detail_view_create()");
	IF_FREE(view);
	return NULL;
}




