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

#include "mc-group-list.h"
#include "mp-media-info.h"
#include "mc-common.h"
#include "mc-track-list.h"

typedef struct {
	struct app_data *ad;

	Evas_Object *no_content;
	Evas_Object *genlist;

	Elm_Genlist_Item_Class itc;

	mp_group_type_e type;
	char *type_str;
	int playlist_id;

	Elm_Object_Item *win_navi_it;
	mp_media_list_h group_list;
	mp_media_list_h playlists_auto;
} group_list_data_t;

#define GET_LIST_DATA(obj)	evas_object_data_get(obj, "list_data")

static Evas_Object *
_mc_create_genlist(Evas_Object *parent)
{
	Evas_Object *genlist = NULL;
	MP_CHECK_NULL(parent);

	genlist = elm_genlist_add(parent);
	elm_genlist_select_mode_set(genlist, ELM_OBJECT_SELECT_MODE_ALWAYS);
	elm_genlist_homogeneous_set(genlist, EINA_TRUE);
	elm_genlist_mode_set(genlist, ELM_LIST_COMPRESS);

	return genlist;
}

static void _gl_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	group_list_data_t *ld  = data;
	char *name = NULL;
	char *folder = NULL;
	char *folder_name = NULL;
	int track_type = MP_TRACK_BY_ALBUM;
	int playlist_id = 0;

	elm_genlist_item_selected_set(event_info, EINA_FALSE);
	MP_CHECK(ld);

	list_item_data_t *item_data = (list_item_data_t*)elm_object_item_data_get(event_info);
	mp_media_info_h media = item_data->media;
	MP_CHECK(media);
	mp_media_info_group_get_main_info(media, &name);

	if (ld->type == MP_GROUP_BY_PLAYLIST) {
		mp_media_info_group_get_playlist_id(media, &playlist_id);
		if (playlist_id == MP_SYS_PLST_MOST_PLAYED) {
			track_type = MP_TRACK_BY_PLAYED_COUNT;
		} else if (playlist_id == MP_SYS_PLST_RECENTELY_ADDED) {
			track_type = MP_TRACK_BY_ADDED_TIME;
		} else if (playlist_id == MP_SYS_PLST_RECENTELY_PLAYED) {
			track_type = MP_TRACK_BY_PLAYED_TIME;
		} else if (playlist_id == MP_SYS_PLST_QUICK_LIST) {
			track_type = MP_TRACK_BY_FAVORITE;
		} else {
			track_type = MP_TRACK_BY_PLAYLIST;
		}
	} else if (ld->type == MP_GROUP_BY_ARTIST) {
		track_type = MP_TRACK_BY_ARTIST;
	} else if (ld->type == MP_GROUP_BY_FOLDER) {
		track_type = MP_TRACK_BY_FOLDER;
		mp_media_info_group_get_folder_id(media, &folder);
		mp_media_info_group_get_main_info(media, &folder_name);
	}

	if (ld->type == MP_GROUP_BY_FOLDER) {
		mc_common_push_track_view_by_group_name(ld->ad, track_type, folder, playlist_id, folder_name);
	} else {
		mc_common_push_track_view_by_group_name(ld->ad, track_type, name, playlist_id, NULL);
	}
	free(folder);
}

static void
_layout_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	group_list_data_t *ld  = data;
	MP_CHECK(ld);

	if (ld->group_list) {
		mp_media_info_group_list_destroy(ld->group_list);
		ld->group_list = NULL;
	}
	if (ld->playlists_auto) {
		mp_media_info_group_list_destroy(ld->playlists_auto);
		ld->playlists_auto = NULL;
	}

	IF_FREE(ld->type_str);

	free(ld);
}

static void _mc_group_list_gl_del(void *data, Evas_Object *obj)
{
	list_item_data_t *it_data = data;
	IF_FREE(it_data);
}


static void
_mc_itc_init(int type, group_list_data_t *ld)
{
	MP_CHECK(ld);

	ld->itc.func.content_get = mc_group_content_get;
	ld->itc.func.del = _mc_group_list_gl_del;
	switch (type) {
	case MP_GROUP_BY_ALBUM:
		ld->itc.item_style = "2line.top";
		ld->itc.func.text_get = mc_album_text_get;
		ld->type = MP_GROUP_BY_ALBUM;
		break;
	case MP_GROUP_BY_ARTIST:
		ld->itc.item_style = "2line.top";
		ld->itc.func.text_get = mc_artist_text_get;
		ld->type = MP_GROUP_BY_ARTIST;
		break;
	case MP_GROUP_BY_PLAYLIST:
		ld->itc.item_style = "2line.top";
		ld->itc.func.text_get = mc_playlist_text_get;
		ld->type = MP_GROUP_BY_PLAYLIST;
		break;
	case MP_GROUP_BY_FOLDER:
		ld->itc.item_style = "2line.top";
		ld->itc.func.text_get = mc_folder_list_label_get;
		ld->type = MP_GROUP_BY_FOLDER;
		break;
	default:
		ERROR_TRACE("Invalid vd->type: %d", type);
		break;
	}
}


Evas_Object *mc_group_list_create(Evas_Object *parent, struct app_data *ad, Elm_Object_Item *navi_it)
{
	startfunc;
	Evas_Object *layout ;
	group_list_data_t *ld = NULL;

	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(ad);

	layout = mc_common_load_edj(parent, MC_EDJ_FILE, "list_layout");
	MP_CHECK_NULL(layout);

	ld = calloc(1, sizeof(group_list_data_t));
	MP_CHECK_NULL(ld);

	ld->ad = ad;
	ld->win_navi_it = navi_it;

	/*
	Evas_Object *title_btn = elm_object_item_part_content_unset(ld->win_navi_it, "title_right_btn");
	if (title_btn)
		evas_object_del(title_btn);
	*/

	Evas_Object *done_btn = elm_object_item_part_content_unset(ld->win_navi_it, "toolbar");
	if (done_btn) {
		evas_object_del(done_btn);
	}

	evas_object_data_set(layout, "list_data", ld);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, _layout_del_cb, ld);

	/*elm_naviframe_item_pop_cb_set(navi_it, mc_quit_cb, ad);*/
	return layout;
}

int mc_group_list_update(Evas_Object *list)
{
	startfunc;
	Evas_Object *content;

	int count = 0;
	group_list_data_t *ld  = GET_LIST_DATA(list);
	MP_CHECK_VAL(ld, -1);

	if (ld->group_list) {
		mp_media_info_group_list_destroy(ld->group_list);
		ld->group_list = NULL;
	}

	if (ld->playlists_auto) {
		mp_media_info_group_list_destroy(ld->playlists_auto);
		ld->playlists_auto = NULL;
	}

	content = elm_layout_content_get(list, "list_content");
	evas_object_del(content);

	mp_media_info_group_list_count(ld->type, ld->type_str, NULL, &count);
	if (count || ld->type == MP_GROUP_BY_PLAYLIST) {
		content = _mc_create_genlist(list);

		/*if playlist, add auto playlist firstly*/
		if (ld->type == MP_GROUP_BY_PLAYLIST) {
			mp_media_list_h playlists_auto = NULL;
			mp_media_info_group_list_create(&playlists_auto, MP_GROUP_BY_SYS_PLAYLIST, NULL, NULL, 0, 0);
			ld->playlists_auto = playlists_auto;
			int i = 0;
			for (i = 0; i < MP_SYS_PLST_COUNT; i++) {
				mp_media_info_h media = mp_media_info_group_list_nth_item(playlists_auto, i);
				if (!media) {
					continue;
				}

				list_item_data_t *item_data = calloc(1, sizeof(list_item_data_t));
				if (!item_data) {
					break;
				}

				item_data->media = media;
				item_data->index = i;

				item_data->it = elm_genlist_item_append(content, &ld->itc, item_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_cb, ld);
				elm_object_item_data_set(item_data->it, item_data);

			}
		}

		mp_media_info_group_list_create(&ld->group_list, ld->type, ld->type_str, NULL, 0, count);
		int i = 0;
		for (i = 0; i < count; i++) {
			mp_media_info_h media =  mp_media_info_group_list_nth_item(ld->group_list, i);
			if (!media) {
				continue;
			}

			list_item_data_t *item_data = calloc(1, sizeof(list_item_data_t));
			MP_CHECK_VAL(item_data, -1);
			item_data->media = media;
			item_data->index = i;
			if (ld->type == MP_GROUP_BY_ALBUM) {
				item_data->list_type = MC_ALBUM;
			} else if (ld->type == MP_GROUP_BY_ARTIST) {
				item_data->list_type = MC_ARTIST;
			} else if (ld->type == MP_GROUP_BY_FOLDER) {
				item_data->list_type = MC_FOLDER;
			}

			item_data->it = elm_genlist_item_append(content, &ld->itc, item_data, NULL, ELM_GENLIST_ITEM_NONE, _gl_sel_cb, ld);
			elm_object_item_data_set(item_data->it, item_data);

			Evas_Object* right_button = elm_object_item_part_content_get(ld->win_navi_it, "title_right_btn");
			mc_evas_object_del(right_button);
			Evas_Object* left_button = elm_object_item_part_content_get(ld->win_navi_it, "title_left_btn");
			mc_evas_object_del(left_button);
		}
	} else {
		NoContentType_e type = NO_CONTENT_SONG;
		if (ld->type == MP_GROUP_BY_ARTIST) {
			type = NO_CONTENT_ARTIST;
		} else if (ld->type == MP_GROUP_BY_ALBUM) {
			type = NO_CONTENT_ALBUM;
		}

		content = mc_widget_no_content_add(list, type);
	}

	if (count > 0) {
		/*create fastscroller*/
		if (ld->ad->select_type == MC_SELECT_MULTI) {
			mc_common_create_fastscroller(list, EINA_TRUE, content);
		} else {
			mc_common_create_fastscroller(list, EINA_FALSE, content);
		}
	} else {
		/*hide fastscroller*/
		elm_object_signal_emit(list, "hide.fastscroll", "*");
	}

	elm_layout_content_set(list, "list_content", content);
	return 0;
}

int mc_group_list_set_data(Evas_Object *list, int group_type, const char *type_str)
{
	startfunc;
	group_list_data_t *ld  = GET_LIST_DATA(list);
	MP_CHECK_VAL(ld, -1);

	ld->type = group_type;
	ld->type_str = g_strdup(type_str);

	_mc_itc_init(group_type, ld);

	return 0;
}


