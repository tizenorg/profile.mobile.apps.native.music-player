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

#include "mp-albumart.h"
#include "mp-tag-mgr.h"
#include "music.h"
#include "mp-file-util.h"

typedef struct
{
	int ref_cnt;
	char *media_id;
	char *uri;

	/*for recognize music*/
	MpTagMgr_t *tag_mgr;
	Evas_Object *list_popup;
	Evas_Object *waiting_popup;
	mp_info_data_t *info_data;
	GList *downloads;
	char **albumart_path;

}MpAlbumart_t;

#define THUMBNAIL_SIZE (elm_config_scale_get()*320)
#define MP_ALBUMART_WORKING_DIR  DATA_DIR"/album_update"
#define MP_ALBUMART_IMAGE_DIR DATA_DIR"/albumarts"

static void _mp_albumart_popup_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info);
static void _mp_albumart_save_image(MpAlbumart_t *albumart ,const char *image_path);

void
_donwload_cancel(void *data)
{
	startfunc;
	mp_thumb_downloader_destroy(data);
}

void
_mp_albumart_destroy(MpAlbumart_t *albumart)
{
	eventfunc;
	MP_CHECK(albumart);
	int i;

	if (albumart->info_data && albumart->albumart_path)
	{
		for (i=0; i<albumart->info_data->find_album_cnt; i++)
		{
			IF_FREE(albumart->albumart_path[i]);
		}
		free(albumart->albumart_path);
	}

	//cancel remaining downloads..
	g_list_free_full(albumart->downloads, _donwload_cancel);

	mp_tag_mgr_destory(albumart->tag_mgr);
	mp_info_data_destory(albumart->info_data);
	IF_FREE(albumart->media_id);
	IF_FREE(albumart->uri);

	free(albumart);

	mp_tag_mgr_view_id_increase();

	mp_file_recursive_rm(MP_ALBUMART_WORKING_DIR);
}

void
_mp_albumart_unref(MpAlbumart_t *albumart)
{
	MP_CHECK(albumart);
	if (albumart->ref_cnt ==0)
		return;

	albumart->ref_cnt--;

	DEBUG_TRACE("ref cnt[%d]", albumart->ref_cnt);

	if (albumart->ref_cnt ==0)
		_mp_albumart_destroy(albumart);
}

void
mp_albumart_hide_waiting_popup(MpAlbumart_t *albumart)
{
	DEBUG_TRACE("");
	MP_CHECK(albumart);
	MP_CHECK(albumart->waiting_popup);

	if (albumart->waiting_popup)
	{
		evas_object_del(albumart->waiting_popup);
		albumart->waiting_popup = NULL;
	}
}

void
mp_albumart_show_waiting_popup(MpAlbumart_t *albumart, bool cancel_button)
{
	DEBUG_TRACE("");
	MP_CHECK(albumart);

	albumart->waiting_popup = mp_popup_create(NULL, MP_POPUP_PROGRESS, GET_SYS_STR("IDS_COM_BODY_LOADING"), albumart,
				NULL/*mp_albumart_waiting_popup_response_cb*/, NULL);
        cancel_button = false;

	evas_object_event_callback_add(albumart->waiting_popup, EVAS_CALLBACK_DEL, _mp_albumart_popup_del_cb, albumart);
	evas_object_show(albumart->waiting_popup);

}

static void
_mp_albumart_album_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpAlbumart_t *albumart = evas_object_data_get(obj, "albumart");
	MP_CHECK(albumart);

	int i = (int)data;

	_mp_albumart_save_image(albumart, albumart->albumart_path[i]);
	mp_popup_destroy(mp_util_get_appdata());
}

static char *
_mp_popup_gl_label_get2(void *data, Evas_Object * obj, const char *part)
{
	mp_info_data_t *info_data = evas_object_data_get(obj, "info_data");
	MP_CHECK_NULL(info_data);
	int i = (int)data;

	return g_strdup(info_data->find_album_title[i]);
}

static Evas_Object *
_mp_popup_gl_icon_get(void *data, Evas_Object * obj, const char *part)
{
	MpAlbumart_t *albumart = evas_object_data_get(obj, "albumart");
	MP_CHECK_NULL(albumart);
	int i = (int)data;

	return mp_util_create_thumb_icon(obj, albumart->albumart_path[i], 70*elm_config_scale_get(), 70*elm_config_scale_get());
}

static void
_mp_albumart_popup_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE("");
	MpAlbumart_t *albumart = (MpAlbumart_t *) data;
	MP_CHECK(albumart);

	_mp_albumart_unref(albumart);
}

static void
_mp_albumart_list_popup_create(MpAlbumart_t *albumart, mp_info_data_t *info_data)
{
	startfunc;
	MP_CHECK(albumart);
	MP_CHECK(info_data);

	struct appdata *ad = mp_util_get_appdata();

	Evas_Object *popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_INFO_LIST ,&info_data->find_album_cnt, NULL);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_albumart_popup_del_cb, albumart);
	albumart->ref_cnt++;

	albumart->list_popup = popup;

	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);

	Elm_Genlist_Item_Class * itc = elm_genlist_item_class_new();

	itc->item_style = "1text.1icon.2";
	itc->func.text_get = _mp_popup_gl_label_get2;
	itc->func.content_get = _mp_popup_gl_icon_get;
	itc->func.state_get = NULL;
	itc->func.del = NULL;

	evas_object_data_set(genlist, "info_data", info_data);
	evas_object_data_set(genlist, "albumart", albumart);

	int i = 0;
	for (i = 0; i < info_data->find_album_cnt; i++)
	{
		DEBUG_TRACE("path: %s", albumart->albumart_path[i]);
		elm_genlist_item_append(genlist, itc, (void *)i, NULL, ELM_GENLIST_ITEM_NONE, _mp_albumart_album_select_cb, (void *)i);
	}

	elm_genlist_item_class_unref(itc);

	mp_albumart_hide_waiting_popup(albumart);

}

#define IMAGE_URL_PREFIX "http://image.allmusic.com/01/"

void _thumbnail_download_cb(downloader_h download_handle, const char *url, const char *path, void *user_data)
{
	MpAlbumart_t *albumart = user_data;
	MP_CHECK(albumart);
	MP_CHECK(albumart->info_data);

	DEBUG_TRACE("path: %s", path);

	int i;
	for (i = 0; i < albumart->info_data->find_album_cnt; i++)
	{
		if (!albumart->albumart_path[i] && !g_strcmp0((url+strlen(IMAGE_URL_PREFIX)), albumart->info_data->find_albumart_url[i]))
		{
			albumart->albumart_path[i] = g_strdup(path);
			break;
		}
	}

	albumart->downloads = g_list_delete_link(albumart->downloads,
							g_list_find(albumart->downloads, download_handle));
	mp_thumb_downloader_destroy(download_handle);

	if (g_list_length(albumart->downloads) == 0)
	{
		DEBUG_TRACE("All images has been downloaded");
		_mp_albumart_list_popup_create(albumart, albumart->info_data);
	}

}


static void _mp_albumart_list_request_thumbnail(MpAlbumart_t *albumart, const char *uri)
{
	char *url = g_strdup_printf("%s%s", IMAGE_URL_PREFIX, uri);
	thumb_downloader_h download = mp_thumb_downloader_create(url, MP_ALBUMART_WORKING_DIR, _thumbnail_download_cb, albumart);
	albumart->downloads = g_list_append(albumart->downloads, download);

	IF_FREE(url);
}

static void
mp_albumart_download_imges(MpAlbumart_t *albumart, mp_info_data_t *info_data)
{
	startfunc;

	int i = 0;
	albumart->albumart_path = calloc(info_data->find_album_cnt, sizeof(char *));

	for (i = 0; i < info_data->find_album_cnt; i++)
	{
		_mp_albumart_list_request_thumbnail(albumart, info_data->find_albumart_url[i]);
	}
}

void
mp_albumart_error_popup(MpAlbumart_t *albumart, char *title, char *contents)
{
	MP_CHECK(albumart);
	mp_albumart_hide_waiting_popup(albumart);
	mp_widget_text_popup(NULL, contents);
}

void _mp_albumart_resp_cb(void *user_data, MpTagReqType_t type, bool success)
{
	EVENT_TRACE("Response type: %d, success: %d", type, success);
	MpAlbumart_t *albumart = user_data;

	mp_info_data_t *info_data = albumart->info_data;
	//MpTagMgr_t *tag_mgr = view->tag_mgr;
	if (!success)
	{
		mp_albumart_hide_waiting_popup(albumart);
		mp_albumart_error_popup(albumart,
			GET_SYS_STR("IDS_COM_POP_ERROR"), GET_STR("IDS_MUSIC_POP_UNABLE_TO_GET_TAG"));
	}

	switch (type) {
	case MP_HTTP_REQ_SHAZAM_REQUEST_RESULT:
		{
			if (success)
				mp_albumart_download_imges(albumart, info_data);
			break;
		}

	default:
		{
			DEBUG_TRACE("type: %d", type);
		}
	}

}

static void
_mp_albumart_recognize_music(MpAlbumart_t *albumart)
{
	eventfunc;
	mp_info_data_t *info_data = NULL;
	mp_info_data_create(&info_data, albumart->uri);
	if (!info_data) goto ERROR;
	albumart->info_data = info_data;

	mp_tag_mgr_view_id_increase();

	MpTagMgr_t *tag_mgr = mp_tag_mgr_create(info_data, _mp_albumart_resp_cb, albumart);
	if (!tag_mgr) goto ERROR;

	albumart->tag_mgr = tag_mgr;

	if (mp_tag_mgr_start_find_tag(tag_mgr))
		mp_albumart_show_waiting_popup(albumart, true);
	else
	{
		mp_albumart_error_popup(albumart, GET_SYS_STR("IDS_COM_POP_ERROR"), GET_STR("IDS_MUSIC_POP_UNABLE_TO_GET_TAG"));
		goto ERROR;
	}

	return;

	ERROR:
	_mp_albumart_unref(albumart);
	return;
}

static char *
_mp_albumart_get_thumbnail_path(const char *media_id)
{
	return g_strdup_printf("%s/%s",MP_ALBUMART_IMAGE_DIR, media_id);
}

static void
_mp_albumart_save_image(MpAlbumart_t *albumart ,const char *image_path)
{

	char *dest = NULL;
	struct appdata *ad = mp_util_get_appdata();

	MP_CHECK(albumart);
	MP_CHECK(image_path);

	/*
	Evas_Object *thumbnail = evas_object_image_add(evas_object_evas_get(ad->win_main));
	MP_CHECK(thumbnail);

	evas_object_size_hint_align_set(thumbnail, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(thumbnail, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_aspect_set(thumbnail, EVAS_ASPECT_CONTROL_BOTH, THUMBNAIL_SIZE, THUMBNAIL_SIZE);

	evas_object_image_file_set(thumbnail, image_path, NULL);
	evas_object_image_filled_set(thumbnail, true);
	evas_object_image_load_size_set(thumbnail, THUMBNAIL_SIZE, THUMBNAIL_SIZE);
	evas_object_show(thumbnail);
	*/
	mp_file_mkdir(MP_ALBUMART_IMAGE_DIR);

	dest = _mp_albumart_get_thumbnail_path(albumart->media_id);

	if (!mp_file_cp(image_path, dest))
		ERROR_TRACE("Unable to copy..src[%s],dest[%s]", image_path, dest);

	if (ad->current_track_info)
	{
		mp_util_free_track_info(ad->current_track_info);
		mp_util_load_track_info(ad, mp_playlist_mgr_get_current(ad->playlist_mgr), &ad->current_track_info);
	}

	mp_view_mgr_post_event(GET_VIEW_MGR, MP_VIEW_EVENT_ALBUMART_CHANGED);

	IF_FREE(dest);
}

static void _mp_albumart_gallery_result_cb(app_control_h request, app_control_h reply, app_control_result_e result, void *data)
{
	startfunc;
	int ret = 0;
	MpAlbumart_t *albumart = data;
	MP_CHECK(albumart);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	char *result_filename = NULL;

	if (result == APP_CONTROL_RESULT_SUCCEEDED) {

		ret = app_control_get_extra_data(reply, "path", &result_filename);
		if (ret != APP_CONTROL_ERROR_NONE) {
			ERROR_TRACE("app_control_get_extra_data() is failed : %d", ret);
			return;
		}
		DEBUG_TRACE("result_filename : %s", result_filename);

		_mp_albumart_save_image(albumart, result_filename);
		IF_FREE(result_filename);
	}
	else
		WARN_TRACE("Unable to get path from gallery");

	_mp_albumart_unref(albumart);
}


static int
_mp_albumart_gallery_service(MpAlbumart_t *albumart)
{
	eventfunc;
	int ret;
	app_control_h svc_handle = NULL;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, -1);

	if (app_control_create(&svc_handle) < 0 || svc_handle == NULL) {
		ERROR_TRACE("app_control_create() is failed !!");
		return -1;
	}
#if 0
	app_control_set_window(svc_handle, elm_win_xwindow_get(ad->win_main));
#endif
	app_control_set_operation(svc_handle, APP_CONTROL_OPERATION_PICK);
	app_control_set_app_id(svc_handle, "gallery-efl");
	app_control_add_extra_data(svc_handle, "launch-type", "select-one");
	app_control_add_extra_data(svc_handle, "file-type", "image");

	ret = app_control_send_launch_request(svc_handle, _mp_albumart_gallery_result_cb, albumart);
	if (ret != APP_CONTROL_ERROR_NONE) {
		ERROR_TRACE("app_control_send_launch_request is failed ret = %d", ret);
		app_control_destroy(svc_handle);
		return -1;
	}
	app_control_destroy(svc_handle);
	return 0;
}

void mp_albumart_app_control_select_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpAlbumart_t *albumart = evas_object_data_get(obj, "albumart");
	MP_CHECK(albumart);

	albumart->ref_cnt++;
	mp_popup_destroy(mp_util_get_appdata());

	if ((int)data == 0)
		_mp_albumart_recognize_music(albumart);
	else
		_mp_albumart_gallery_service(albumart);

}

static void
_create_select_popup(MpAlbumart_t *albumart)
{
	struct appdata *ad = mp_util_get_appdata();
	int cnt = 2;
	Evas_Object *popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_UPDATE_ALBUM_ART ,&cnt, NULL);
	evas_object_event_callback_add(popup, EVAS_CALLBACK_DEL, _mp_albumart_popup_del_cb, albumart);
	albumart->ref_cnt++;

	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);
	evas_object_data_set(genlist, "albumart", albumart);

	mp_genlist_popup_item_append(popup, STR_MP_SHAZAM, NULL, NULL, NULL, mp_albumart_app_control_select_cb, (void *)0);
	mp_genlist_popup_item_append(popup, STR_MP_GALLERY, NULL, NULL, NULL, mp_albumart_app_control_select_cb, (void *)1);

}

static void
_create_popup_job_cb(void *data)
{
	_create_select_popup(data);
}

void mp_albumart_update(const char *uri, const char *media_id)
{
	SECURE_DEBUG("%s", uri);
	MP_CHECK(uri);
	MP_CHECK(media_id);

	MpAlbumart_t *albumart = calloc(1, sizeof(MpAlbumart_t));
	MP_CHECK(albumart);
	albumart->uri = g_strdup(uri);
	albumart->media_id = g_strdup(media_id);

	ecore_job_add(_create_popup_job_cb, albumart);
}

char *mp_albumart_path_get(const char *media_id)
{
	char *path = _mp_albumart_get_thumbnail_path(media_id);
	MP_CHECK_NULL(path);

	if (mp_file_exists(path))
		return path;
	else
		IF_FREE(path);

	return path;
}

