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

#include "mp-shortcut.h"
#include "mp-play.h"
#include "mp-player-view.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-common.h"

#define MP_SHORTCUT_LARGE_BOX_SIZE 480*elm_config_scale_get()
#define MP_SHORT_CUT_SMALL_BOX_SIZE 240*elm_config_scale_get()
#define MP_SHORTCUT_BOX_LD_SIZE 377*elm_config_scale_get()
#define MP_SHORTCUT_BOX_WIDTH_SIZE 1280*elm_config_scale_get()

#define MP_SHORTCUT_BOX_COUNT 3

typedef struct {
	char *thumb_path[MP_SHORTCUT_BOX_COUNT];
	char *playlist_title[MP_SHORTCUT_BOX_COUNT];
} MpShortcutCache;

MpShortcutCache *g_cache_data;
static int favourite_index;

static void
_mp_shortcut_box_tts_double_action_cb(void *data, Evas_Object * obj, Elm_Object_Item *item_data)
{
	eventfunc;
	mp_media_list_h list = NULL;
	int type = (int)data;
	int count = 0;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_media_info_list_count(type, NULL, NULL, NULL, 0, &count);
	if (count == 0) {
		mp_widget_text_popup(mp_util_get_appdata(), GET_STR(STR_MP_NO_SONGS));
		return;
	}

	mp_media_info_list_create(&list, type, NULL, NULL, NULL, 0, 0, count);

	if (!ad->playlist_mgr) {
		mp_common_create_playlist_mgr();
	}
	mp_playlist_mgr_clear(ad->playlist_mgr);

	if (type == MP_TRACK_BY_FAVORITE) {
		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, list, count, favourite_index, NULL);
	} else {
		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, list, count, 0, NULL);
	}

	int ret = mp_play_new_file(ad, TRUE);
	if (ret) {
		ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
		}
#endif
		return;
	}

	mp_common_show_player_view(MP_PLAYER_NORMAL, false, true, true);
}


static void
_mp_shortcut_box_click_cb(void *data, Evas_Object * obj, const char *emission, const char *source)
{
	eventfunc;
	mp_media_list_h list = NULL;
	int type = (int)data;
	int count = 0;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_media_info_list_count(type, NULL, NULL, NULL, 0, &count);
	if (count == 0) {
		mp_widget_text_popup(mp_util_get_appdata(), GET_STR(STR_MP_NO_SONGS));
		return;
	}

	mp_media_info_list_create(&list, type, NULL, NULL, NULL, 0, 0, count);

	if (!ad->playlist_mgr) {
		mp_common_create_playlist_mgr();
	}
	mp_playlist_mgr_clear(ad->playlist_mgr);

	if (type == MP_TRACK_BY_FAVORITE) {
		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, list, count, favourite_index, NULL);
	} else {
		mp_util_append_media_list_item_to_playlist(ad->playlist_mgr, list, count, 0, NULL);
	}
	mp_media_info_list_destroy(list);

	int ret = mp_play_new_file(ad, TRUE);
	if (ret) {
		ERROR_TRACE("Error: mp_play_new_file..");
#ifdef MP_FEATURE_CLOUD
		if (ret == MP_PLAY_ERROR_NETWORK) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_THIS_FILE_IS_UNABAILABLE));
		}
#endif
		return;
	}

	mp_common_show_player_view(MP_PLAYER_NORMAL, false, true, true);
}

Evas_Object *
mp_shortcut_add(Evas_Object *parent, int index)
{
	startfunc;
	Evas_Object *layout = NULL;
	MP_CHECK_NULL(parent);

	if (!g_cache_data) {
		g_cache_data = calloc(1, sizeof(MpShortcutCache));
		MP_CHECK_NULL(g_cache_data);
	}

#ifdef MP_FEATURE_LANDSCAPE
	bool landscape = mp_util_is_landscape();
	if (landscape) {
		layout = mp_common_load_edj(parent, MP_EDJ_NAME, "landscape/shortcut_layout");
		mp_shortcut_update_cache(layout, index);
		evas_object_size_hint_min_set(layout, 0, MP_SHORTCUT_BOX_LD_SIZE);
	} else
#endif
	{
		PROFILE_IN("mp_common_load_edj");
		layout = mp_common_load_edj(parent, MP_EDJ_NAME, "shortcut_layout");
		PROFILE_OUT("mp_common_load_edj");

		PROFILE_IN("mp_shortcut_update_cache");
		mp_shortcut_update_cache(layout, index);
		PROFILE_OUT("mp_shortcut_update_cache");
		evas_object_size_hint_min_set(layout, 0, MP_SHORTCUT_LARGE_BOX_SIZE);
	}

	return layout;
}

int mp_shortcut_get_height(void)
{
#ifdef MP_FEATURE_LANDSCAPE
	bool landscape = mp_util_is_landscape();
	if (landscape) {
		return MP_SHORTCUT_BOX_LD_SIZE;
	} else
#endif
	{
		return MP_SHORTCUT_LARGE_BOX_SIZE;
	}
}

void
mp_shortcut_update_cache(Evas_Object *layout, int index)
{
	startfunc;
	MP_CHECK(layout);

	MP_CHECK(g_cache_data);

	Evas_Object *box = NULL;
	int favourite_count = 0;
	int count[MP_SHORTCUT_BOX_COUNT] = {0};

	mp_media_list_h list = NULL;
	mp_media_info_h item = NULL;
	char *label[MP_SHORTCUT_BOX_COUNT] = {STR_MP_FAVOURITES, STR_MP_RECENTLY_ADDED, STR_MP_MOST_PLAYED};
	int track_type[MP_SHORTCUT_BOX_COUNT] = {MP_TRACK_BY_FAVORITE, MP_TRACK_BY_ADDED_TIME, MP_TRACK_BY_PLAYED_COUNT};
	char *default_thumbnail[MP_SHORTCUT_BOX_COUNT] = {MP_ICON_STARRED_SONGS, MP_ICON_RECENTLY_ADDED, MP_ICON_MOST_PLAYED};
	int i;

	for (i = 0; i < MP_SHORTCUT_BOX_COUNT; i++) {
		char *thumbpath = NULL;
		char *title = NULL;

		int res = 0;
		res = mp_media_info_list_count(track_type[i], NULL, NULL, NULL, 0, &count[i]);

		if (!count[i]) {
			continue;
		}

		if (0 == i) {
			favourite_count = count[i];
			mp_media_info_list_create(&list, track_type[i], NULL, NULL, NULL, 0, 0, favourite_count);
			favourite_index = index;

		} else {
			mp_media_info_list_create(&list, track_type[i], NULL, NULL, NULL, 0, 0, 1);
		}
		if (!list) {
			continue;
		}
		if (0 == i) {
			item = mp_media_info_list_nth_item(list, index);
		} else {
			item = mp_media_info_list_nth_item(list, 0);
		}
		mp_media_info_get_thumbnail_path(item, &thumbpath);
		mp_media_info_get_title(item, &title);

		IF_FREE(g_cache_data->thumb_path[i]);
		g_cache_data->thumb_path[i] = g_strdup(thumbpath);
		IF_FREE(g_cache_data->playlist_title[i]);
		g_cache_data->playlist_title[i] = g_strdup(title);

		mp_media_info_list_destroy(list);
		list = NULL;
	}

	for (i = 0; i < MP_SHORTCUT_BOX_COUNT; i++) {
		char *thumbpath = NULL;
		char *title = NULL;
		char *part = NULL;

		if (g_cache_data->thumb_path[i]) {
			thumbpath = g_cache_data->thumb_path[i];
			title = g_cache_data->playlist_title[i];
		} else {
			thumbpath = IMAGE_EDJ_NAME;
			if (0 == count[i]) {
				part = default_thumbnail[i];
			}
		}

		bool landscape = mp_util_is_landscape();
		if (landscape)
			box = mp_widget_shorcut_box_add(layout, label[i], thumbpath, part,
			                                MP_SHORTCUT_BOX_LD_SIZE, MP_SHORTCUT_BOX_LD_SIZE,
			                                _mp_shortcut_box_click_cb, (void *)track_type[i]);
		else if (i == 0)
			box = mp_widget_shorcut_box_add(layout, label[i], thumbpath, part,
			                                MP_SHORTCUT_LARGE_BOX_SIZE, MP_SHORTCUT_LARGE_BOX_SIZE,
			                                _mp_shortcut_box_click_cb, (void *)track_type[i]);
		else
			box = mp_widget_shorcut_box_add(layout, label[i], thumbpath, part,
			                                MP_SHORT_CUT_SMALL_BOX_SIZE, MP_SHORT_CUT_SMALL_BOX_SIZE,
			                                _mp_shortcut_box_click_cb, (void *)track_type[i]);

		part = g_strdup_printf("box%d", i);
		elm_object_part_content_set(layout, part, box);
		IF_FREE(part);
	}

}

