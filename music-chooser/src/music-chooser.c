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

#ifndef MC_MODULE_API
#define MC_MODULE_API __attribute__ ((visibility("default")))
#endif

#include <Elementary.h>
#include <stdbool.h>
#include <glib.h>
#include <efl_extension.h>

#include "music-chooser.h"
#include "mc-select-view.h"
#include "mc-library-view.h"
#include "mp-media-info.h"
#include "mc-list-play.h"

extern Evas_Object *mc_tabbar;

#define MC_SELECT_URI_KEY    "select_uri"

static bool
_mc_app_control_extra_data_cb(app_control_h service, const char *key, void *user_data)
{
	startfunc;
	char *val = NULL;
	struct app_data *ad = user_data;
	MP_CHECK_FALSE(ad);

	app_control_get_extra_data(service, key, &val);
	DEBUG_TRACE("key: %s, val: %s", key, val);

	if (!g_strcmp0(key, MC_REQ_TYPE_KEY))
	{
		if (!g_strcmp0(val, MC_REQ_SHORT_ALBUM_VAL))
			ad->select_type = MC_SHORTCUT_ALBUM;
		else if (!g_strcmp0(val, MC_REQ_SHORT_PLAYLIST_VAL))
			ad->select_type = MC_SHORTCUT_PLAYLIST;
		else if (!g_strcmp0(val, MC_REQ_SHORT_ARTIST_VAL))
			ad->select_type = MC_SHORTCUT_ARTIST;
		else if (!g_strcmp0(val, MC_REQ_SELECT_SINGLE))
			ad->select_type = MC_SELECT_SINGLE;
		else if (!g_strcmp0(val, MC_REQ_SELECT_SINGLE_RINGTONE))
		{
			ad->select_type = MC_SELECT_SINGLE_RINGTONE;
			app_control_get_extra_data(service, MC_SELECT_URI_KEY, &ad->select_uri);
		}
		else if (!g_strcmp0(val, MC_REQ_SELECT_MULTI))
			ad->select_type = MC_SELECT_MULTI;
		else if (!g_strcmp0(val, MC_REQ_VOICE_CLIP))
			ad->select_type = MC_SELECT_VOICE_CLIP;
		else if (!g_strcmp0(val, MC_REQ_GROUP_PLAY))
			ad->select_type = MC_SELECT_GROUP_PLAY;
		else
			WARN_TRACE("unsupported type: %s", val);
	} else if (!g_strcmp0(key, MC_REQ_SHOW_RECOMMENDED_KEY)) {
		if (!g_strcmp0(val, MC_SHOW_VAL))
			ad->auto_recommended_show = TRUE;
	}

	IF_FREE(val);
	endfunc;
	return true;
}

static void
_parse_service(struct app_data *ad, app_control_h service)
{
	startfunc;
	char *operation = NULL;
	char *value = NULL;
	int ret = 0;

	app_control_get_operation(service, &operation);
	DEBUG_TRACE("operation: %s", operation);

	if (!g_strcmp0(operation, APP_CONTROL_OPERATION_PICK))
	{
		ad->max_count = 0;
#if 0
		ret = app_control_get_extra_data(service, APP_CONTROL_DATA_TOTAL_COUNT, &value);
		if (value) {
			ad->max_count = atoi(value);
			DEBUG_TRACE("Maximum Count is: %d", ad->max_count);
			IF_FREE(value);
		}
#endif
		ad->limitsize = -1;
#if 0
		ret = app_control_get_extra_data(service, APP_CONTROL_DATA_TOTAL_SIZE, &value);
		if (value) {
			ad->limitsize = atoi(value);
			DEBUG_TRACE("Maximum Size is: %lld", ad->limitsize);
			IF_FREE(value);
		}
#endif
		ret = app_control_get_extra_data(service, MC_SELECT_MODE_KEY, &value);
		DEBUG_TRACE("Operation: %s", value);

		if (!g_strcmp0(value, MC_SELECT_MULTIPLE)) {
			ad->select_type = MC_SELECT_MULTI;
		} else {
			ad->select_type = MC_SELECT_SINGLE;
		}
		IF_FREE(value);
	}
	app_control_foreach_extra_data(service, _mc_app_control_extra_data_cb, ad);

	//END:
	IF_FREE(operation);
	endfunc;
}

static Evas_Object *
_mc_create_fullview(Evas_Object * parent, struct app_data *ad)
{
	Evas_Object *base_layout;

	base_layout = elm_layout_add(parent);

	mp_retv_if (base_layout == NULL, NULL);

	elm_layout_theme_set(base_layout, "layout", "application", "default");

	return base_layout;
}

static Evas_Object *
_mc_create_navigation_layout(Evas_Object * parent)
{
	Evas_Object *navi_bar;

	mp_retv_if (parent == NULL, NULL);

	navi_bar = elm_naviframe_add(parent);
	evas_object_show(navi_bar);

	elm_naviframe_prev_btn_auto_pushed_set(navi_bar, EINA_FALSE);
	eext_object_event_callback_add(navi_bar, EEXT_CALLBACK_BACK, eext_naviframe_back_cb, NULL);
	eext_object_event_callback_add(navi_bar, EEXT_CALLBACK_MORE, eext_naviframe_more_cb, NULL);

	return navi_bar;
}

static Evas_Object *
_mc_crete_bg(Evas_Object *parent)
{
    Evas_Object *bg = elm_bg_add(parent);
    evas_object_size_hint_weight_set(bg, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
    elm_object_style_set(bg, "group_list");
    evas_object_show(bg);
    return bg;
}

static bool
mc_create(void *data)
{
	DEBUG_TRACE("");

	struct app_data *ad = NULL;

	ad = (struct app_data *)data;

	ad->win = mc_create_win("music-lite-chooser");

	elm_win_conformant_set(ad->win, EINA_TRUE);
	elm_win_autodel_set(ad->win, EINA_TRUE);

	if (elm_win_wm_rotation_supported_get(ad->win)) {
		int rots[4] = { 0, 90, 180, 270 };
		elm_win_wm_rotation_available_rotations_set(ad->win, (const int *)(&rots), 4);
	}

	Evas_Object *conformant = NULL;
	conformant = elm_conformant_add(ad->win);
	evas_object_size_hint_weight_set(conformant, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_win_resize_object_add(ad->win, conformant);
	evas_object_show(conformant);
	ad->conformant = conformant;

	//support light theme
	elm_theme_extension_add(NULL,"/usr/apps/org.tizen.music-player/res/edje/mp-custom-winset-style.edj");


	DEBUG_TRACE("end");

	return EINA_TRUE;
}

static void
mc_app_control(app_control_h app_control, void* data)
{
	eventfunc;

	Evas_Object *parent = NULL;
	struct app_data *ad = NULL;
	ad = (struct app_data *)data;
	parent = ad->win;
	MP_CHECK(ad->win);

	app_control_clone(&ad->service, app_control);

	bindtextdomain(DOMAIN_NAME, LOCALE_DIR);

	mp_media_info_connect();
	_parse_service(ad, app_control);

	ad->base_layout = _mc_create_fullview(ad->conformant, ad);

	if (ad->base_layout)
	{
		Evas_Object *bg = _mc_crete_bg(ad->base_layout);
		if (bg)
			elm_win_resize_object_add(parent, bg);

		elm_object_part_content_set(ad->base_layout, "elm.swallow.bg", bg);
		ad->navi_bar = _mc_create_navigation_layout(ad->base_layout);
		elm_object_part_content_set(ad->base_layout, "elm.swallow.content", ad->navi_bar);
	}
	if (ad->select_type >= MC_SHORTCUT_ALBUM)
		mc_select_view_create(ad);
	else
		mc_library_view_create(ad);

	if (ad->select_type == MC_SELECT_SINGLE_RINGTONE)
	{
		mc_vol_type_set(SOUND_TYPE_MEDIA);
	}

	elm_object_content_set(ad->conformant, ad->base_layout);
	evas_object_show(parent);

	return;
}

static void
mc_pause(void *data)
{
	DEBUG_TRACE("");
	mc_pre_play_mgr_destroy_play();
	struct app_data *ad = NULL;
	ad = (struct app_data *)data;
	MP_CHECK(ad);
	mc_vol_reset_default_value(ad);

	return;
}

static void
mc_resume(void *data)
{
	DEBUG_TRACE("");
	struct app_data *ad = NULL;
	ad = (struct app_data *)data;
	MP_CHECK(ad);

	if (ad->select_type == MC_SELECT_SINGLE_RINGTONE) {
		mc_vol_type_set(SOUND_TYPE_MEDIA);
	}

	return;
}

static void
mc_device_orientation(app_event_info_h event_info, void *data)
{
	struct app_data *ad = data;

	int angle;

	app_device_orientation_e orientation = APP_DEVICE_ORIENTATION_0;
	app_event_get_device_orientation(event_info, &orientation);

	switch (orientation)
	{
	case APP_DEVICE_ORIENTATION_270:
		angle = -90;
		break;

	case APP_DEVICE_ORIENTATION_90:
		angle = 90;
		break;

	case APP_DEVICE_ORIENTATION_180:
		angle = 180;
		break;

	case APP_DEVICE_ORIENTATION_0:
	default:
		angle = 0;
		break;
	}

	ad->win_angle = angle;

	DEBUG_TRACE("window angle: %d", ad->win_angle);
}

static void
mc_destroy(void *data)
{
	DEBUG_TRACE("");
	struct app_data *ad = data;

	mp_media_info_disconnect();
	if (ad)
	{
		if (ad->base_layout)
		{
			evas_object_del(ad->base_layout);
			ad->base_layout = NULL;
		}

		//elm_theme_free(ad->th);
		mc_vol_reset_default_value(ad);

		elm_win_lower(ad->win);

		if (ad->smat_pipe)
		{
			ecore_pipe_del(ad->smat_pipe);
			ad->smat_pipe = NULL;
		}
	}
}

static void
__mc_language_changed_cb(app_event_info_h event_info, void *user_data)
{
	eventfunc;
	DEBUG_TRACE("Language changed triggered");
	struct appdata *ad = user_data;

	char *lang = NULL;
	app_event_get_language(event_info, &lang);
	if (lang) {
		elm_language_set(lang);
		free(lang);
	}
}

EXPORT_API int
main(int argc, char *argv[])
{
	struct app_data ad;
	ui_app_lifecycle_callback_s event_callbacks;

	memset(&event_callbacks, 0x0, sizeof(ui_app_lifecycle_callback_s));
	memset(&ad, 0x0, sizeof(struct app_data));

	int nRet = APP_ERROR_NONE;
	app_event_handler_h  hLowMemoryHandle;
	app_event_handler_h  hLowBatteryHandle;
	app_event_handler_h  hLanguageChangedHandle;
//	app_event_handler_h  hDeviceOrientationChangedHandle;
	app_event_handler_h  hRegionFormatChangedHandle;

	event_callbacks.create = mc_create;
	event_callbacks.app_control = mc_app_control;
	event_callbacks.pause = mc_pause;
	event_callbacks.resume = mc_resume;
	event_callbacks.terminate = mc_destroy;

	nRet = ui_app_add_event_handler(&hLowMemoryHandle, APP_EVENT_LOW_MEMORY,  NULL, (void*)&ad );
	if (nRet != APP_ERROR_NONE) {
		WARN_TRACE("APP_EVENT_LOW_MEMORY ui_app_add_event_handler failed : [%d]!!!", nRet);
	}

	nRet = ui_app_add_event_handler(&hLowBatteryHandle, APP_EVENT_LOW_BATTERY, NULL, (void*)&ad );
	if (nRet != APP_ERROR_NONE) {
		WARN_TRACE("APP_EVENT_LOW_BATTERY ui_app_add_event_handler failed : [%d]!!!", nRet);
	}

	nRet = ui_app_add_event_handler(&hLanguageChangedHandle, APP_EVENT_LANGUAGE_CHANGED, __mc_language_changed_cb, (void*)&ad );
	if (nRet != APP_ERROR_NONE) {
		WARN_TRACE("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
	}

//	nRet = ui_app_add_event_handler( &hDeviceOrientationChangedHandle, APP_EVENT_DEVICE_ORIENTATION_CHANGED, mc_device_orientation, (void*)&ad );
//	if (nRet != APP_ERROR_NONE) {
//		ERROR_TRACE("APP_EVENT_LANGUAGE_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
//		return -1;
//	}

	nRet = ui_app_add_event_handler(&hRegionFormatChangedHandle, APP_EVENT_REGION_FORMAT_CHANGED, NULL, (void*)&ad );
	if (nRet != APP_ERROR_NONE) {
		WARN_TRACE("APP_EVENT_REGION_FORMAT_CHANGED ui_app_add_event_handler failed : [%d]!!!", nRet);
	}

	int ret = ui_app_main(argc, argv, &event_callbacks, &ad);
	if (ret != 0)
		ERROR_TRACE("ret failed %d", ret);
	return ret;
}
