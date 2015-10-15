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

#include "mp-square-view-new.h"
#include "mp-square-mgr.h"
#include "mp-widget.h"
#include "mp-util.h"
#include "mp-popup.h"
#include "mp-setting-ctrl.h"
#include "mp-play.h"
#include "mp-player-view.h"
#include "mp-square-playlist-view.h"
#include "mp-common.h"
#include <player.h>
#include "mp-player-mgr.h"


typedef struct {
	Elm_Object_Item *it;
	MpSquareView_t *view;
	mp_square_position_t position;
	bool b_seleted;
} mp_square_gengrid_item_data_t;

typedef struct {
	Elm_Object_Item *it;
	MpSquareView_t *view;

	Evas_Object *radio_main;
	int index;
} mp_square_popup_item_data_t;

static int square_normal_color[MP_SQUARE_CELLS_COUNT] = {250, 238, 67, 255,
														 247, 200, 69, 255,
														 237, 140, 66, 255,
														 232, 104, 65, 255,
														 232, 81, 81, 255,
														 227, 230, 64, 255,
														 235, 217, 164, 255,
														 240, 182, 105, 255,
														 240, 137, 105, 255,
														 229, 90, 99, 255,
														 187, 222, 113, 255,
														 182, 235, 164, 255,
														 152, 217, 157, 255,
														 227, 150, 179, 255,
														 209, 92, 145, 255,
														 144, 222, 102, 255,
														 129, 222, 220, 255,
														 144, 196, 224, 255,
														 134, 151, 217, 255,
														 135, 113, 191, 255,
														 90, 199, 97, 255,
														 111, 203, 217, 255,
														 75, 166, 189, 255,
														 76, 125, 153, 255,
														 90, 115, 156, 255};

int popup_flag = 1;
static void _mp_square_view_library_update_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_square_view_reader_on_mouse_down(void *data, Evas_Object *obj, Elm_Object_Item *item);
static void _mp_square_view_help_btn_cb(void *data, Evas_Object *obj, void *event_info);

static void
_mp_square_view_position_list_free(GList *list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(list);

	mp_square_position_t *item = NULL;
	int count = g_list_length(list);
	int i = 0;
	for (i = 0; i < count; i++) {
		item = g_list_nth_data(list, i);
		if (item != NULL)
			free(item);
	}
}

static void
_mp_square_view_music_list_free(GList *list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(list);

	mp_square_item_t *item = NULL;
	int count = g_list_length(list);
	int i = 0;
	for (i = 0; i < count; i++) {
		item = g_list_nth_data(list, i);
		if (item != NULL)
			free(item);
	}
}

static void
_mp_square_view_position_list_clear(GList **list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(list);

	_mp_square_view_position_list_free(*list);
	*list = NULL;
}

static void
_mp_square_view_music_list_clear(GList **list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(list);

	_mp_square_view_music_list_free(*list);
	*list = NULL;
}

static inline void
_mp_square_view_new_set_upper_text(Evas_Object *layout, const char *part, const char *text)
{
	MP_CHECK(layout);

	char *upper_text = g_utf8_strup(GET_STR(text), -1);
	mp_util_domain_translatable_part_text_set(layout, part, (const char *)upper_text);
	SAFE_FREE(upper_text);
}

static void
_mp_square_view_new_gengrid_title_set(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(view);
	MP_CHECK(view->square_layout);

	const char *top = NULL;
	const char *bottom = NULL;
	const char *left = NULL;
	const char *right = NULL;

	if (view->type == MP_SQUARE_TYPE_MOOD) {
		top = STR_EXCITING;
		bottom = STR_CALM;
		left = STR_PASSION;
		right = STR_JOY;
	} else if (view->type == MP_SQUARE_TYPE_YEAR) {
		top = STR_EXCITING;
		bottom = STR_CALM;
		left = STR_OLD;
		right = STR_NEW_FOR_SQUARE;
	} else if (view->type == MP_SQUARE_TYPE_ADDED) {
		top = STR_EXCITING;
		bottom = STR_CALM;
		left = STR_PREVIOUSLY_ADDED;
		right = STR_RECENTLY_ADDED;
	} else {
		top = STR_PREVIOUSLY_ADDED;
		bottom = STR_RECENTLY_ADDED;
		left = STR_OLD;
		right = STR_NEW_FOR_SQUARE;
	}

	_mp_square_view_new_set_upper_text(view->square_layout, "title_top", top);
	_mp_square_view_new_set_upper_text(view->square_layout, "title_bottom", bottom);
	_mp_square_view_new_set_upper_text(view->square_layout, "title_left", left);
	_mp_square_view_new_set_upper_text(view->square_layout, "title_right", right);
}

static mp_square_item_t*
_mp_square_view_current_playing_music_item_get(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_NULL(view);
	MP_CHECK_NULL(view->music_list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	mp_plst_item *cur_playing_music = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_NULL(cur_playing_music);

	mp_square_item_t *item = NULL;
	int count = g_list_length(view->music_list);
	int i = 0;
	for (i = 0; i < count; i++) {
		item = g_list_nth_data(view->music_list, i);
		if (item != NULL) {
			if (!strcmp(item->path, cur_playing_music->uri))
				return item;
		}
	}

	return NULL;
}

static void
_mp_square_view_current_play_list_create(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();

	int index;
	MP_CHECK(view);
	MP_CHECK(view->music_list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	GList *list_music = view->music_list;
	for (index = 0; index < g_list_length(list_music); index++) {
		mp_square_item_t *item = (mp_square_item_t *)g_list_nth_data(list_music, index);
		if (!item)	continue;

		char *media_id = NULL;
		char *title = NULL;
		char *artist = NULL;

		mp_media_info_h media_info = NULL;
		mp_media_info_create_by_path(&media_info, item->path);
		mp_media_info_get_media_id(media_info, &media_id);
		mp_media_info_get_title(media_info, &title);
		mp_media_info_get_artist(media_info, &artist);

		mp_playlist_mgr_item_append(ad->playlist_mgr, item->path, media_id, title, artist, MP_TRACK_URI);

		if (media_info) {
			mp_media_info_destroy(media_info);
			media_info = NULL;
		}
	}

	mp_playlist_mgr_set_current(ad->playlist_mgr, mp_playlist_mgr_get_current(ad->playlist_mgr));
	mp_playlist_mgr_set_list_type(ad->playlist_mgr, MP_PLST_TYPE_MUSIC_SQUARE);
}

static void
_mp_square_view_selected_musics_get(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(view);
	MP_CHECK(view->pos_list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* clear selected musics of last time */
	if (view->music_list != NULL) {
		_mp_square_view_music_list_free(view->music_list);
		view->music_list = NULL;
	}

	mp_square_mgr_records_get_by_type_and_positions(
											ad->square_mgr,
											view->type,
											view->pos_list,
											&view->music_list);
}

static void
_mp_square_view_library_empty_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(obj);
	Evas_Object *popup = obj;
	mp_evas_object_del(popup);
}

static void
_mp_square_view_library_empty_popup_create(MpSquareView_t *view)
{
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* create popup */
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main,
							MP_POPUP_NORMAL,
							NULL,
							view,
							_mp_square_view_library_empty_popup_response_cb, ad);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	char *text = g_strconcat("<align=center>", GET_STR("IDS_MUSIC_POP_NO_MUSIC_FOUND_FOR_SELECTED_CELL"), "</align>", NULL);
	mp_util_domain_translatable_text_set(popup, text);
	mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_SK_OK", MP_POPUP_YES);

	evas_object_show(popup);
	IF_FREE(text);
}

static bool
_mp_square_view_gengrid_item_is_selected(MpSquareView_t *view, mp_square_position_t *position)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_FALSE(view);
	MP_CHECK_FALSE(view->pos_list);

	GList *list = view->pos_list;
	mp_square_position_t *pos = NULL;

	int count = g_list_length(list);
	int i = 0;

	for (i = 0; i < count; i++) {
		pos = g_list_nth_data(list, i);
		if (pos != NULL) {
			if (pos->x == position->x && pos->y == position->y)
				/*mp_debug("[%d,%d] selected", pos->x, pos->y);*/
				return true;
		}
	}

	return false;
}

static Evas_Object*
_mp_square_view_gengrid_item_content_get(void *data, Evas_Object *obj, const char *part)
{
	MP_CHECK_NULL(data);
	mp_square_gengrid_item_data_t *item_data = (mp_square_gengrid_item_data_t *)data;
	MpSquareView_t *view = item_data->view;
	char *title = NULL;

	if (!g_strcmp0(part, "elm.swallow.icon")) {
		Evas_Object *icon = NULL;

		bool show_thumnail = false;
		if (view->now_playing_position.x == item_data->position.x
				&& view->now_playing_position.y == item_data->position.y) {
			mp_debug("now playing cell = [%d, %d]", item_data->position.x, item_data->position.y);
			show_thumnail = true;
		} else {
			mp_square_item_t *item = view->current_item;
			if (item) {
				if (item->pos.x == item_data->position.x && item->pos.y == item_data->position.y)
					show_thumnail = true;
			}
		}

		/*ERROR_TRACE("show_thumnail %d, item_data->b_seleted &d",show_thumnail, item_data->b_seleted);*/

		if (show_thumnail) {
			icon = mp_common_load_edj(obj, MP_EDJ_NAME, "square_focused_cell");
			Evas_Object *img = elm_image_add(icon);
			elm_object_part_content_set(icon, "elm.swallow.content", img);

			char *albumart = NULL;
			struct appdata *ad = mp_util_get_appdata();
			MP_CHECK_NULL(ad);
			mp_track_info_t *cur_playing_music = ad->current_track_info;
			MP_CHECK_NULL(cur_playing_music);

			title = cur_playing_music->title;
			albumart = cur_playing_music->thumbnail_path;

			if (mp_util_is_image_valid(ad->evas, albumart)) {

				elm_image_file_set(img, albumart, NULL);
			} else {
				elm_image_file_set(img, DEFAULT_THUMBNAIL, NULL);
			}
		} else if (item_data->b_seleted) {
			icon = elm_image_add(obj);
#ifdef MP_FEATURE_LANDSCAPE
			if (mp_util_is_landscape()) {
				elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SQUARE_CELL_SELECTED_LD);
			} else
#endif
				elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SQUARE_CELL_SELECTED);

		} else {
			icon = elm_image_add(obj);
#ifdef MP_FEATURE_LANDSCAPE
			if (mp_util_is_landscape())
				elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SQUARE_CELL_NORMAL_LD);
			else
#endif
				elm_image_file_set(icon, IMAGE_EDJ_NAME, MP_ICON_SQUARE_CELL_NORMAL);
			int item_pos = MP_SQUARE_AXIS_X_LEN * (item_data->position.y-1) + item_data->position.x - 1;
			evas_object_color_set(icon, square_normal_color[item_pos * 1], square_normal_color[item_pos * 2], square_normal_color[item_pos * 3], square_normal_color[item_pos * 4]);
		}

		mp_screen_reader_set_list_item_info(item_data->it, NULL, NULL, NULL, _mp_square_view_reader_on_mouse_down, item_data);

		/* Set the property of gengrid */
		Evas_Coord icon_w = 0;
		Evas_Coord icon_h = 0;

		elm_gengrid_item_size_get(view->gengrid, &icon_w, &icon_h);
		evas_object_size_hint_max_set(icon, icon_w, icon_h);
		evas_object_size_hint_aspect_set(icon, EVAS_ASPECT_CONTROL_VERTICAL, 1, 1);
		evas_object_size_hint_weight_set(icon, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
		evas_object_size_hint_align_set(icon, EVAS_HINT_FILL, EVAS_HINT_FILL);
		evas_object_show(icon);
		return icon;
	}

	return NULL;
}

static void
_mp_square_view_gengrid_item_del_cb(void *data, Evas_Object *obj)
{
	DEBUG_TRACE_FUNC();

	mp_square_gengrid_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static void
_mp_square_view_gengrid_item_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(data);
	mp_square_gengrid_item_data_t *item_data = (mp_square_gengrid_item_data_t *)data;

	MpSquareView_t *view = item_data->view;
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		_mp_square_view_library_empty_popup_create(view);
		return;
	}

	item_data->b_seleted = !item_data->b_seleted;
	elm_gengrid_item_update(item_data->it);
}


static void
_mp_square_view_gengrid_items_load(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(view);
	MP_CHECK(view->gengrid);

	static Elm_Gengrid_Item_Class gic = { 0, };

	gic.item_style = "music/grid_square";
	gic.func.content_get = _mp_square_view_gengrid_item_content_get;
	gic.func.del = _mp_square_view_gengrid_item_del_cb;

	int index = 0;
	for (index = 0; index < MP_SQUARE_CELLS_COUNT; index++) {
		mp_square_gengrid_item_data_t *item_data = calloc(1, sizeof(mp_square_gengrid_item_data_t));
		item_data->view = view;
		item_data->position.x = index%MP_SQUARE_AXIS_Y_LEN+1;
		item_data->position.y = index/MP_SQUARE_AXIS_X_LEN+1;
		item_data->b_seleted = _mp_square_view_gengrid_item_is_selected(view, &(item_data->position));

		item_data->it = elm_gengrid_item_append(view->gengrid,
												&gic,
												item_data,
												_mp_square_view_gengrid_item_sel_cb,
												item_data);
	}
}

static void
_mp_square_view_gengrid_items_state_reset(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(view);
	MP_CHECK(view->gengrid);

	Elm_Object_Item *it;
	mp_square_gengrid_item_data_t *data = NULL;

	it = elm_gengrid_first_item_get(view->gengrid);
	while (it) {
		data = (mp_square_gengrid_item_data_t *)elm_object_item_data_get(it);
		data->b_seleted = false;
		it = elm_gengrid_item_next_get(it);
	}
	elm_gengrid_realized_items_update(view->gengrid);
}

static void
_mp_square_view_gengrid_reset(MpSquareView_t *view)
{
	startfunc;
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	_mp_square_view_gengrid_items_state_reset(view);

	if (view->pos_list != NULL)
		_mp_square_view_position_list_clear(&(view->pos_list));

	if (view->music_list != NULL)
		_mp_square_view_music_list_clear(&(view->music_list));

	endfunc;
}

static void
_mp_square_view_update_alarm_popup_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	MpSquareView_t *view = data;
	MP_CHECK(view);
	mp_evas_object_del(view->popup);
	view->popup_status = FALSE;
	popup_flag = 0;

	int response = (int)event_info;
	mp_debug("response = %d", response);

	if (response) {
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);

		int db_count = 0;
		int square_count = 0;
		mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &db_count);
		mp_square_mgr_records_count_get(ad->square_mgr, &square_count);
		if (db_count != square_count)
			_mp_square_view_library_update_btn_cb(view, NULL, NULL);
	}
}

static void
_mp_square_view_alarm_check_clicked_cb(void *data, Evas_Object *obj, void *event_info)
{
	Eina_Bool state = elm_check_state_get(obj);
	DEBUG_TRACE("Check [%d]", state);

	if (preference_set_boolean(KEY_MUSIC_SQUARE_ASKED, state) != 0) {
		mp_error("fail to set Preference");
	}
}

static void
_mp_square_view_show_update_alarm_popup(MpSquareView_t *view)
{
	startfunc;
	bool asked = true;
	preference_get_boolean(KEY_MUSIC_SQUARE_ASKED, &asked);
	if (asked)
		return;

	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *check = NULL;
	Evas_Object *layout = NULL;
	Evas_Object *label = NULL;

	mp_evas_object_del(view->popup);

	view->popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, STR_MP_LIBRARY_UPDATE,
			view, _mp_square_view_update_alarm_popup_response_cb, ad);
	view->popup_status = TRUE;

	label = elm_label_add(view->popup);
	/*elm_object_style_set(label, "popup/default");*/
	elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
	elm_object_text_set(label, GET_STR(STR_MP_SQUARE_LIBRARY_UPDATED_NEED));
	evas_object_size_hint_weight_set(label, EVAS_HINT_EXPAND, 0.0);
	evas_object_size_hint_align_set(label, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_show(label);

	layout = elm_layout_add(view->popup);
	elm_layout_file_set(layout, MP_EDJ_NAME, "popup_library_update");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	check = elm_check_add(view->popup);
	evas_object_size_hint_align_set(check, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(check, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_show(check);
	evas_object_smart_callback_add(check, "changed", _mp_square_view_alarm_check_clicked_cb, NULL);

	elm_object_text_set(check, GET_STR(STR_MP_DO_NOT_SHOW_AGAIN));
	elm_object_part_content_set(layout, "elm.swallow.content", label);
	elm_object_part_content_set(layout, "elm.swallow.check", check);

	evas_object_show(layout);
	elm_object_content_set(view->popup, layout);

	mp_popup_button_set(view->popup, MP_POPUP_BTN_1, "IDS_COM_SK_CANCEL", MP_POPUP_NO);
	mp_popup_button_set(view->popup, MP_POPUP_BTN_2, "IDS_COM_SK_OK", MP_POPUP_YES);

	evas_object_show(view->popup);
}

static void
_mp_square_view_song_not_enough_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(obj);
	Evas_Object *popup = obj;
	mp_evas_object_del(popup);
}

static void
_mp_square_view_song_not_enough_popup_create(MpSquareView_t *view)
{
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* create popup */
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main,
				MP_POPUP_NORMAL,
				NULL,
				view,
				_mp_square_view_song_not_enough_popup_response_cb, ad);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	char *text = g_strdup_printf(GET_STR(STR_MP_SQUARE_NOT_ENOUGH_SONG_PD), 25);
	if (text) {
		elm_object_text_set(popup, text);
		mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_SK_OK", MP_POPUP_YES);
		evas_object_show(popup);
	}
	IF_FREE(text);
}


static void
_mp_square_view_check_possible(MpSquareView_t *view)
{
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->square_mgr);

	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	DEBUG_TRACE("count=%d", count);
	if (count < 25) {
		DEBUG_TRACE("need update library");
		_mp_square_view_song_not_enough_popup_create(view);

	} else if (view->popup_status) {
		_mp_square_view_show_update_alarm_popup(view);
	}
}

static Eina_Bool
_mp_square_view_update_square(void *data)
{
	startfunc;
	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK_VAL(view, ECORE_CALLBACK_CANCEL);
	MP_CHECK_VAL(view->square_layout, ECORE_CALLBACK_CANCEL);

	int w = 0;
	int h = 0;
	edje_object_part_geometry_get(_EDJ(view->square_layout), "gengrid", NULL, NULL, &w, &h);

	if (view->gengrid) {
		if (elm_gengrid_items_count(view->gengrid)) {
			DEBUG_TRACE("Already done");
			return ECORE_CALLBACK_DONE;
		}
	}

	double scale_factor = 0.0;
	scale_factor = elm_config_scale_get();
	mp_debug("elm_config_scale_get =%f", scale_factor);
	mp_debug("w: %d, h: %d, item_size: %d", w, h, (w/MP_SQUARE_AXIS_X_LEN));

	elm_gengrid_item_size_set(view->gengrid, (w/MP_SQUARE_AXIS_X_LEN), (h/MP_SQUARE_AXIS_Y_LEN));

	/* load square data */
	_mp_square_view_gengrid_items_load(view);


	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, ECORE_CALLBACK_CANCEL);

	if (ad->square_mgr == NULL) {
		int ret = mp_square_mgr_create(ad);
		if (ret != 0 || ad->square_mgr == NULL) {
			mp_error("fail to create square mgr");
			return ECORE_CALLBACK_CANCEL;
		}
		mp_view_update((MpView_t *)view);
	}

	if (popup_flag) {
		_mp_square_view_check_possible(view);
	}

	endfunc;
	return ECORE_CALLBACK_DONE;
}

static void
_mp_square_view_gengrid_destroy_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK(view);
	view->gengrid = NULL;
	endfunc;
}

static mp_square_gengrid_item_data_t*
_mp_square_view_gengrid_item_get_by_mouse_position(MpSquareView_t *view, Evas_Position *pos)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_NULL(view);
	MP_CHECK_NULL(view->gengrid);
	MP_CHECK_NULL(pos);

	Elm_Object_Item *it = NULL;
	mp_square_gengrid_item_data_t *item_data = NULL;
	Evas_Object *item_content = NULL;

	it = elm_gengrid_first_item_get(view->gengrid);
	while (it) {
		item_data = (mp_square_gengrid_item_data_t *)elm_object_item_data_get(it);
		if (item_data != NULL) {
			item_content = elm_object_item_part_content_get(it, "elm.swallow.icon");
			if (item_content != NULL) {
				int x, y, w, h;
				evas_object_geometry_get(item_content, &x, &y, &w, &h);
				if ((pos->output.x >= x) &&
					(pos->output.x) <= (x + w) &&
					(pos->output.y >= y) &&
					(pos->output.y) <= (y + h)) {
					return item_data;
				}
			}
		}
		it = elm_gengrid_item_next_get(it);
	}

	return NULL;
}


static void
_mp_square_view_reader_on_mouse_down(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	DEBUG_TRACE_FUNC();

	mp_square_gengrid_item_data_t *item_data = (mp_square_gengrid_item_data_t *)data;
	MpSquareView_t *view = item_data->view;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		mp_square_mgr_create(ad);
		MP_CHECK(ad->square_mgr);
	}

	view->b_mouse_down = true;

	/* clear the selected cells position of last time */
	if (view->pos_list != NULL) {
		_mp_square_view_position_list_free(view->pos_list);
		view->pos_list = NULL;
	}

	_mp_square_view_gengrid_items_state_reset(view);
	mp_view_update((MpView_t *)view);

	if (item_data->b_seleted)
		return;

	mp_square_position_t *position = calloc(1, sizeof(mp_square_position_t));
	position->x = item_data->position.x;
	position->y = item_data->position.y;

	view->pos_list = g_list_append(view->pos_list, position);

	item_data->b_seleted = true;
	elm_gengrid_item_update(item_data->it);

	view->b_mouse_down = false;

	_mp_square_view_selected_musics_get(view);

	if (view->music_list != NULL) {
		if (view->screen_mode == MP_SCREEN_MODE_PORTRAIT) {
			/* TODO: playlist genlist*/
			/*mp_view_layout_update(view->layout_genlist);*/
		}

	if (!ad->playlist_mgr)
		mp_common_create_playlist_mgr();
	mp_playlist_mgr_clear(ad->playlist_mgr);
	_mp_square_view_current_play_list_create(view);

	mp_play_item_play_current_item(ad);
	mp_view_update((MpView_t *)view);

	} else {
		_mp_square_view_library_empty_popup_create(view);
		mp_view_update_options((MpView_t *)view);
	}


	}


static void
_mp_square_view_on_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();
	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		mp_square_mgr_create(ad);
		MP_CHECK(ad->square_mgr);
	}

	view->b_mouse_down = true;

	/* clear the selected cells position of last time */
	if (view->pos_list != NULL) {
		_mp_square_view_position_list_free(view->pos_list);
		view->pos_list = NULL;
	}

	_mp_square_view_gengrid_items_state_reset(view);

	/* get cell item by mouse position */
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;
	mp_square_gengrid_item_data_t *item_data = NULL;
	item_data = _mp_square_view_gengrid_item_get_by_mouse_position(view, &ev->cur);
	MP_CHECK(item_data);

	if (item_data->b_seleted)
		return;

	mp_square_position_t *position = calloc(1, sizeof(mp_square_position_t));
	position->x = item_data->position.x;
	position->y = item_data->position.y;

	view->pos_list = g_list_append(view->pos_list, position);

	item_data->b_seleted = true;
	elm_gengrid_item_update(item_data->it);
}

static void
_mp_square_view_on_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		return;
	}

	if (view->b_mouse_down == false) {
		return;
	}

	/* get cell item by mouse position */
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;
	mp_square_gengrid_item_data_t *item_data = NULL;
	item_data = _mp_square_view_gengrid_item_get_by_mouse_position(view, &ev->cur);
	MP_CHECK(item_data);

	if (item_data->b_seleted)
		return;

	mp_square_position_t *position = calloc(1, sizeof(mp_square_position_t));
	position->x = item_data->position.x;
	position->y = item_data->position.y;

	view->pos_list = g_list_append(view->pos_list, position);

	item_data->b_seleted = true;
	elm_gengrid_item_update(item_data->it);
}

static void
_mp_square_view_on_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		return;
	}

	view->b_mouse_down = false;

	_mp_square_view_selected_musics_get(view);
	view->current_item = _mp_square_view_current_playing_music_item_get(view);

	if (view->music_list != NULL) {
		if (view->screen_mode == MP_SCREEN_MODE_PORTRAIT) {
			/* TODO: playlist genlist*/
			/*mp_view_layout_update(view->layout_genlist);*/
		}

		if (!ad->playlist_mgr)
			mp_common_create_playlist_mgr();
		mp_playlist_mgr_clear(ad->playlist_mgr);
		_mp_square_view_current_play_list_create(view);
		mp_view_update((MpView_t *)view);

		mp_common_show_player_view(MP_PLAYER_NORMAL, false, true, true);

	} else {
		elm_gengrid_realized_items_update(view->gengrid);
		_mp_square_view_library_empty_popup_create(view);
		mp_view_update_options((MpView_t *)view);
	}
}

static void _mp_square_view_help_select_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MP_CHECK(data);
	_mp_square_view_help_btn_cb(data, NULL, NULL);
}


static Evas_Object *
_mp_square_view_new_content_create(void *thiz)
{
	DEBUG_TRACE_FUNC();
	MpSquareView_t *view = (MpSquareView_t *)thiz;
	MP_CHECK_NULL(view);

	ERROR_TRACE("mp_util_is_landscape(): %d", mp_util_is_landscape());

	if (mp_util_is_landscape()) {
		view->square_layout = mp_common_load_edj(view->layout, PLAY_VIEW_EDJ_NAME, "mp_square_view_landscape");
	} else {
		view->square_layout = mp_common_load_edj(view->layout, PLAY_VIEW_EDJ_NAME, "mp_square_view");
	}

	MP_CHECK_NULL(view->square_layout);

	mp_evas_object_del(view->gengrid);
	view->gengrid = elm_gengrid_add(view->layout);
	MP_CHECK_NULL(view->gengrid);

	evas_object_event_callback_add(view->gengrid, EVAS_CALLBACK_DEL, _mp_square_view_gengrid_destroy_cb, view);
	evas_object_event_callback_add(view->gengrid, EVAS_CALLBACK_MOUSE_DOWN, _mp_square_view_on_mouse_down, view);
	evas_object_event_callback_add(view->gengrid, EVAS_CALLBACK_MOUSE_UP, _mp_square_view_on_mouse_up, view);
	evas_object_event_callback_add(view->gengrid, EVAS_CALLBACK_MOUSE_MOVE, _mp_square_view_on_mouse_move, view);

	elm_object_signal_callback_add(view->square_layout, "clicked", "elm", _mp_square_view_help_select_cb, view);

	evas_object_show(view->gengrid);
	elm_object_part_content_set(view->square_layout, "gengrid", view->gengrid);
	return view->square_layout;
}

static void
_mp_square_view_open_playlist_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpSquareView_t *view = data;
	MP_CHECK(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	MP_CHECK(view_manager);

	MpSquareListview_t *list_view = mp_square_playlist_view_create(view_manager->navi);
	mp_view_mgr_push_view(view_manager, (MpView_t *)list_view, NULL);
	mp_view_update_options((MpView_t *)list_view);
	mp_view_set_title((MpView_t *)list_view, STR_MP_SQUARE);

	endfunc;
}

static void
_mp_square_view_help_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpSquareView_t *view = data;
	MP_CHECK(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	char *text = NULL;
	char *title = g_strconcat("<align=center>", GET_STR(STR_MP_SQUARE), "</align>", NULL);

	Evas_Object *popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, title, NULL, NULL, ad);
	MP_CHECK(popup);

	text = g_strdup_printf("%s<br>%s<br>%s", GET_STR(STR_MP_SQUARE_HELP_TEXT_1), GET_STR(STR_MP_SQUARE_HELP_TEXT_2), GET_STR(STR_MP_SQUARE_HELP_TEXT_3));
	elm_object_text_set(popup, text);

	mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_SK_OK", MP_POPUP_YES);
	evas_object_show(popup);

	endfunc;
}


static Eina_Bool
_mp_square_view_update_library_progressbar_timer(void *data)
{
	TIMER_TRACE();
	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK_VAL(view, ECORE_CALLBACK_CANCEL);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, ECORE_CALLBACK_CANCEL);
	MP_CHECK_VAL(ad->square_mgr, ECORE_CALLBACK_CANCEL);

	double value = 0.0;
	int index = ad->square_mgr->record_count;
	int total = ad->square_mgr->total_count;
	Evas_Object *layout = view->layout_update_library_progress;
	Evas_Object *progressbar = view->update_library_progressbar;

	/*mp_debug("index=%d, total=%d\n", index, total);*/

	value = elm_progressbar_value_get(progressbar);
	if (value == 1.0) {
		mp_ecore_timer_del(view->update_library_timer);

		if (view->popup_update_library_progress) {
			evas_object_del(view->popup_update_library_progress);
			view->popup_update_library_progress = NULL;
		}

		mp_view_update((MpView_t *) view);
		mp_util_post_status_message(ad, GET_STR(STR_MP_UPDATED));
		return ECORE_CALLBACK_CANCEL;
	}

	if (total == 0) {
		index = 1;
		total = 1;
	}

	value = (double)index/total;

	/*mp_debug("value=%6.2f\n", value);*/
	elm_progressbar_value_set(progressbar, value);

	char buf[255] = {'0',};

	snprintf(buf, sizeof(buf), "%d/%d", index, total);
	elm_object_part_text_set(layout, "elm.text.right", buf);

	return ECORE_CALLBACK_RENEW;
}

static void
_mp_square_view_update_library_progressbar_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK(view);

	mp_ecore_timer_del(view->update_library_timer);
	if (view->popup_update_library_progress) {
		evas_object_del(view->popup_update_library_progress);
		view->popup_update_library_progress = NULL;
	}

	mp_view_update((MpView_t *) view);

	/* cancel update library */
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	ad->square_mgr->terminal_status = true;
}

static void
_mp_square_view_update_library_progressbar_create(MpSquareView_t *view)
{
	startfunc;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *layout = NULL;
	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;
	/*Evas_Object *label = NULL;*/
	Evas_Object *btn1 = NULL;
	Ecore_Timer	*timer = NULL;

	popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, GET_STR("IDS_MUSIC_BODY_LIBRARY_UPDATE"),
			view, _mp_square_view_update_library_progressbar_response_cb, ad);
	view->popup_update_library_progress = popup;

	layout = elm_layout_add(popup);
	view->layout_update_library_progress = layout;
	elm_layout_file_set(layout, PLAY_VIEW_EDJ_NAME, "popup_update_library_progressview");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	view->update_library_progressbar = progressbar;
	elm_object_style_set(progressbar, "list_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progressbar, 0.0);
	timer = ecore_timer_add(0.1, _mp_square_view_update_library_progressbar_timer, view);
	view->update_library_timer = timer;
	evas_object_show(progressbar);

	/*elm_object_part_content_set(layout, "elm.swallow.content", label);*/
	elm_object_part_content_set(layout, "elm.swallow.content", progressbar);
	elm_object_part_text_set(layout, "elm.text.left", GET_STR(STR_MP_UPDATING_ING));
	elm_object_part_text_set(layout, "elm.text.right", "0/0");

	evas_object_show(layout);
	elm_object_content_set(popup, layout);
	btn1 = elm_button_add(popup);
	elm_object_text_set(btn1, GET_SYS_STR("IDS_COM_SK_CANCEL"));
	elm_object_part_content_set(popup, "button1", btn1);
	evas_object_smart_callback_add(btn1, "clicked", _mp_square_view_update_library_progressbar_response_cb, view);

	evas_object_show(popup);
}

static void
_mp_square_view_library_update(MpSquareView_t *view)
{
	startfunc;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (ad->square_mgr == NULL) {
		int ret = 0;
		ret = mp_square_mgr_create(ad);
		if (ret != 0 || ad->square_mgr == NULL)
			return;
	}

	_mp_square_view_gengrid_reset(view);
	mp_view_update((MpView_t *)view);

	int ret = mp_square_mgr_update_diff_only(ad);
	if (ret != 0) {
		const char *message = NULL;
		if (ad->square_mgr->total_count == 0)
			message = STR_MP_SQURE_NO_SONGS;
		else
			message = STR_MP_UPDATING_FAILED;

		if (message)
			mp_widget_text_popup(ad, GET_STR(message));
		return;
	}

	_mp_square_view_update_library_progressbar_create(view);

	endfunc;
}


static void
_mp_square_view_update_library_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpSquareView_t *view = data;
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *popup = obj;
	mp_evas_object_del(popup);
	view->popup_update_library = NULL;

	int count = 0;
	mp_media_info_list_count(MP_TRACK_ALL, NULL, NULL, NULL, 0, &count);
	DEBUG_TRACE("count=%d", count);

	int response = (int)event_info;
	if (response) {
		if (count >= 25) {
			_mp_square_view_library_update(view);
		} else {
			DEBUG_TRACE("need update library");
			_mp_square_view_song_not_enough_popup_create(view);
		}
	}

	endfunc;
}


static void
_mp_square_view_library_update_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpSquareView_t *view = data;
	MP_CHECK(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* create popup */
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main,
							MP_POPUP_NORMAL,
							GET_STR("IDS_MUSIC_BODY_LIBRARY_UPDATE"),
							view,
							_mp_square_view_update_library_popup_response_cb, ad);
	view->popup_update_library = popup;

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_object_text_set(popup, GET_STR("IDS_MUSIC_POP_UPDATE_LIBRARY_Q"));

	mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_SK_CANCEL", MP_POPUP_NO);
	mp_popup_button_set(popup, MP_POPUP_BTN_2, "IDS_COM_SK_OK", MP_POPUP_YES);

	evas_object_show(popup);

	endfunc;
}

static void
_mp_square_view_position_append_by_item(MpSquareView_t *view, mp_square_item_t *item)
{
	MP_CHECK(view);
	MP_CHECK(item);

	bool exist = false;
	GList *pos_list = view->pos_list;
	mp_square_position_t *pos = NULL;
	while (pos_list) {
		pos = pos_list->data;
		if (pos) {
			if (pos->x == item->pos.x && pos->y == item->pos.y) {
				exist = true;
				break;
			}
		}
		pos_list = pos_list->next;
	}

	if (!exist) {
		pos = calloc(1, sizeof(mp_square_position_t));
		mp_assert(pos);
		pos->x = item->pos.x;
		pos->y = item->pos.y;
		view->pos_list = g_list_append(view->pos_list, pos);
	}
}

static void
_mp_square_view_update_now_playing_postion(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->square_mgr);
	MP_CHECK(ad->current_track_info);

	view->current_item = _mp_square_view_current_playing_music_item_get(view);

	mp_track_info_t *cur_playing_music = ad->current_track_info;
	if (cur_playing_music && mp_check_file_exist(cur_playing_music->uri)) {
		mp_square_mgr_get_positon_by_type_and_path(ad->square_mgr, view->type, cur_playing_music->uri, &(view->now_playing_position));
		mp_debug("now playing pos = [%d, %d]", view->now_playing_position.x, view->now_playing_position.y);
	}
}

static void
_mp_square_view_load_playing_square_list(MpSquareView_t *view)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(view);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);
	MP_CHECK(ad->square_mgr);

	if (view->music_list)
		_mp_square_view_music_list_clear(&view->music_list);
	if (view->pos_list)
		_mp_square_view_position_list_clear(&view->pos_list);

	if (mp_playlist_mgr_get_list_type(ad->playlist_mgr) == MP_PLST_TYPE_MUSIC_SQUARE) {
		GList *selected_list = NULL;
		int type = -1;
		mp_square_mgr_selected_list_items_get(ad->square_mgr, &type, &selected_list);

		if (g_list_length(selected_list) && view->type == type) {
			view->music_list = selected_list;
			GList *music_list = view->music_list;
			while (music_list) {
				mp_square_item_t *item = music_list->data;
				if (item)
					_mp_square_view_position_append_by_item(view, item);
				music_list = music_list->next;
			}
		} else {
			if (selected_list)
				_mp_square_view_music_list_clear(&selected_list);
		}
	}
}

static void
_mp_square_view_axis_change(MpSquareView_t *view)
{
	startfunc;
	MP_CHECK(view);

	view->type = view->radio_index;
	if (!mp_setting_set_square_axis_val(view->type)) {

		_mp_square_view_new_gengrid_title_set(view);

		struct appdata *ad = mp_util_get_appdata();
		if (ad && ad->square_mgr)
			mp_square_mgr_selected_list_items_clear(ad->square_mgr);

		_mp_square_view_gengrid_reset(view);
		mp_view_update((MpView_t *)view);
	}

	endfunc;
}

static void
_mp_square_view_radio_main_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpSquareView_t *view = data;
	MP_CHECK(view);

	elm_radio_value_set(view->radio_main, view->radio_index);
	_mp_square_view_axis_change(view);
	endfunc;
}

static void
_mp_square_view_radio_main_del_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpSquareView_t *view = data;
	MP_CHECK(view);
	view->radio_main = NULL;
}

static char *
_mp_square_view_axis_change_gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *label = (char *)data;
	/*DEBUG_TRACE("%s", label);*/
	return g_strdup(GET_STR(label));
}

static Evas_Object *
_mp_square_view_axis_change_gl_content_get(void *data, Evas_Object * obj, const char *part)
{
	DEBUG_TRACE("");
	MP_CHECK_NULL(data);

	MpSquareView_t *view = evas_object_data_get(obj, "square_view_data");
	MP_CHECK_NULL(view);

	Evas_Object *radio = elm_radio_add(obj);
	elm_radio_group_add(radio, view->radio_main);

	if (!g_strcmp0(STR_MOOD, data)) {
		elm_radio_state_value_set(radio, MP_SQUARE_TYPE_MOOD);
	} else if (!g_strcmp0(STR_YEAR, data)) {
		elm_radio_state_value_set(radio, MP_SQUARE_TYPE_YEAR);
	}
	if (!g_strcmp0(STR_ADDED, data)) {
		elm_radio_state_value_set(radio, MP_SQUARE_TYPE_ADDED);
	} else if (!g_strcmp0(STR_TIME, data)) {
		elm_radio_state_value_set(radio, MP_SQUARE_TYPE_TIME);
	}

	evas_object_show(radio);

	elm_radio_value_set(view->radio_main, view->type);
	return radio;
}

static void
_mp_square_view_axis_change_popup_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareView_t *view = evas_object_data_get(obj, "square_view_data");
	MP_CHECK(view);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	char *label = (char *)data;
	DEBUG_TRACE("label=%s", label);
	if (g_strcmp0(label, STR_MOOD) == 0)
		view->radio_index = MP_SQUARE_TYPE_MOOD;
	else if (g_strcmp0(label, STR_YEAR) == 0)
		view->radio_index = MP_SQUARE_TYPE_YEAR;
	else if (g_strcmp0(label, STR_ADDED) == 0)
		view->radio_index = MP_SQUARE_TYPE_ADDED;
	else if (g_strcmp0(label, STR_TIME) == 0)
		view->radio_index = MP_SQUARE_TYPE_TIME;

	elm_radio_value_set(view->radio_main, view->radio_index);
	evas_object_smart_callback_call(view->radio_main, "changed", view);

	mp_popup_destroy(ad);
}

static void
_mp_square_view_change_axis_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpSquareView_t *view = data;
	MP_CHECK(view);
	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_CHANGE_AXIS, ad, ad);
	MP_CHECK(popup);

	if (view->radio_main == NULL) {
		view->radio_main = elm_radio_add(popup);
		elm_radio_state_value_set(view->radio_main, -1);
		evas_object_smart_callback_add(view->radio_main, "changed", _mp_square_view_radio_main_changed_cb, view);
		evas_object_event_callback_add(view->radio_main, EVAS_CALLBACK_DEL, _mp_square_view_radio_main_del_cb, view);
		evas_object_hide(view->radio_main);
	}

	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);
	evas_object_data_set(genlist, "square_view_data", view);

	static Elm_Genlist_Item_Class change_axis_itc;
	change_axis_itc.item_style = "1text.1icon.3";
	change_axis_itc.func.text_get = _mp_square_view_axis_change_gl_label_get;
	change_axis_itc.func.content_get = _mp_square_view_axis_change_gl_content_get;
	change_axis_itc.func.state_get = NULL;
	change_axis_itc.func.del = NULL;

	elm_genlist_item_append(genlist, &change_axis_itc, STR_MOOD, NULL, ELM_GENLIST_ITEM_NONE,
				_mp_square_view_axis_change_popup_gl_sel, STR_MOOD);
	elm_genlist_item_append(genlist, &change_axis_itc, STR_YEAR, NULL, ELM_GENLIST_ITEM_NONE,
				_mp_square_view_axis_change_popup_gl_sel, STR_YEAR);

	endfunc;
}

static void _mp_square_view_normal_more_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpSquareView_t *view = (MpSquareView_t *)data;
	MP_CHECK(view);

	mp_evas_object_del(view->more_btn_ctxpopup);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	view->more_btn_ctxpopup = mp_common_create_more_ctxpopup(view);
	MP_CHECK(view->more_btn_ctxpopup);

	if (view->pos_list && g_list_length(view->pos_list)) {
		mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_OPEN_PLAYLIST, MP_PLAYER_MORE_BTN_OPEN_PLAYLIST, _mp_square_view_open_playlist_btn_cb, view);
	}

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				"IDS_MUSIC_BODY_LIBRARY_UPDATE", MP_PLAYER_MORE_BTN_REFRESH, _mp_square_view_update_library_popup_response_cb, view);

	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				"IDS_MUSIC_BODY_CHANGE_AXIS", MP_PLAYER_MORE_BTN_CHANGE_AXIS, _mp_square_view_change_axis_btn_cb, view);

	/*search*/
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
		STR_MP_SEARCH, NULL, mp_common_create_search_view_cb, view);

	/*settings*/
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_SETTINGS, MP_PLAYER_MORE_BTN_SETTING, mp_common_ctxpopup_setting_cb, view);
#ifndef MP_FEATURE_NO_END
	mp_util_ctxpopup_item_append(view->more_btn_ctxpopup,
				STR_MP_END, MP_PLAYER_MORE_BTN_VIEW_END, mp_common_ctxpopup_end_cb, view);
#endif

	mp_util_more_btn_move_ctxpopup(view->more_btn_ctxpopup, obj);

	evas_object_show(view->more_btn_ctxpopup);

	endfunc;
}

static Eina_Bool
_mp_square_view_back_btn_cb(void *data, Elm_Object_Item *it)
{
	eventfunc;
	MpSquareView_t *view = (MpSquareView_t *) data;
	MP_CHECK_FALSE(view);

	MpViewMgr_t *view_manager = mp_view_mgr_get_view_manager();
	mp_view_mgr_pop_view(view_manager, false);

	endfunc;
	return EINA_TRUE;
}

static int _mp_square_view_new_update_options(void *thiz)
{
	startfunc;
	MpSquareView_t *view = (MpSquareView_t *)thiz;
	MP_CHECK_VAL(view, -1);

	Evas_Object *btn = NULL;


	mp_view_clear_options((MpView_t *)view);

	btn = mp_widget_create_toolbar_btn(view->square_layout, MP_TOOLBAR_BTN_MORE, NULL, _mp_square_view_normal_more_btn_cb, view);
	elm_object_item_part_content_set(view->navi_it, "toolbar_more_btn", btn);

	bool landscape = mp_util_is_landscape();
	if (landscape)
		elm_naviframe_item_style_set(view->navi_it, MP_NAVI_ITEM_STYLE_TOPLINE/*"miniplayer/music/landscape"*/);
	else
		elm_naviframe_item_style_set(view->navi_it, MP_NAVI_ITEM_STYLE_TOPLINE);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_square_view_back_btn_cb, view);

	endfunc;
	return 0;
}

static int
_mp_square_view_new_update(void *thiz)
{
	startfunc;
	MpSquareView_t *view = (MpSquareView_t *)thiz;
	MP_CHECK_VAL(view, -1);
	MP_CHECK_VAL(view->gengrid, -1);

	_mp_square_view_update_now_playing_postion(view);
	elm_gengrid_realized_items_update(view->gengrid);
	mp_view_update_options((MpView_t *)view);
	endfunc;
	return 0;
}

static int
_mp_square_view_playlist_position_update(void *thiz)
{
	startfunc;
	MpSquareView_t *view = (MpSquareView_t *)thiz;
	MP_CHECK_VAL(view, -1);
	MP_CHECK_VAL(view->gengrid, -1);

	view->current_item = _mp_square_view_current_playing_music_item_get(view);
	int count = g_list_length(view->music_list);
	DEBUG_TRACE("playinglist count:%d", count);
	if (count > 1) {
		_mp_square_view_update_now_playing_postion(view);
		elm_gengrid_realized_items_update(view->gengrid);
	}
	endfunc;
	return 0;
}

static void
_mp_square_view_new_destory_cb(void *thiz)
{
	startfunc;
	MpSquareView_t *view = thiz;
	MP_CHECK(view);
	mp_view_fini((MpView_t *)view);

	/* TODO: release resource..*/

	mp_evas_object_del(view->gengrid);
	mp_evas_object_del(view->more_btn_ctxpopup);
	mp_evas_object_del(view->popup_update_library);
	mp_evas_object_del(view->popup_update_library_progress);

	mp_ecore_timer_del(view->update_library_timer);

	if (view->pos_list != NULL)
		_mp_square_view_position_list_clear(&(view->pos_list));
	if (view->music_list != NULL)
		_mp_square_view_music_list_clear(&(view->music_list));

	free(view);
}

static void
_mp_square_view_on_event(void *thiz, MpViewEvent_e event)
{
	DEBUG_TRACE("event; %d", event);
	MpSquareView_t *view = thiz;
	MP_CHECK(view);
	switch (event) {
		case MP_UPDATE_PLAYING_LIST:
		_mp_square_view_playlist_position_update(view);
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);
		if (ad->square_mgr) {
			mp_view_update((MpView_t *)view);
		}
		break;
		case MP_PLAYLIST_MGR_ITEM_CHANGED:
		{
			_mp_square_view_gengrid_items_state_reset(view);
			_mp_square_view_playlist_position_update(view);
			break;
		}
	case MP_VIEW_TRANSITION_FINISHED:
		_mp_square_view_update_square(view);
		break;
	default:
		break;
	}
}

static void _mp_square_view_resume(void *thiz)
{
	startfunc;
	MpSquareView_t *view = (MpSquareView_t *)thiz;
	MP_CHECK(view);

	_mp_square_view_new_gengrid_title_set(view);
	#ifndef MP_SOUND_PLAYER
	if (mp_player_mgr_get_state() != PLAYER_STATE_NONE)
		mp_view_set_nowplaying((MpView_t *)view);

	mp_view_freeze_nowplaying((MpView_t *)view, 0);
#endif

	endfunc;
}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_square_view_rotate(void *thiz, int randscape)
{
	startfunc;

	MpSquareView_t *view = thiz;
	MP_CHECK(view);

	mp_evas_object_del(view->gengrid);

	Evas_Object *content = elm_object_part_content_unset(view->square_layout, "list_content");
	evas_object_del(content);

	popup_flag = 0;
	view->square_layout = _mp_square_view_new_content_create(view);
	elm_object_part_content_set(view->layout, "list_content", view->square_layout);

	_mp_square_view_new_update(thiz);
	_mp_square_view_new_gengrid_title_set(view);

	_mp_square_view_update_square(view);

	endfunc;

}
#endif

static int
_mp_square_view_new_init(Evas_Object *parent, MpSquareView_t *view)
{
	startfunc;
	int ret = 0;
	ret =  mp_view_init(parent, (MpView_t *)view, MP_VIEW_SQUARE);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_square_view_new_update;
	view->update_options = _mp_square_view_new_update_options;
	view->update_options_edit = NULL;
	view->popup_status = TRUE;
	view->view_destroy_cb = _mp_square_view_new_destory_cb;
	view->view_resume = _mp_square_view_resume;

	/* init data */
	int square_type = MP_SQUARE_TYPE_MOOD;
	mp_setting_get_square_axis_val(&square_type);
	view->type = square_type;
	view->radio_index = square_type;

	view->on_event = _mp_square_view_on_event;

	popup_flag = 1;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_square_view_rotate;
#endif
	view->square_layout = _mp_square_view_new_content_create(view);
	MP_CHECK_VAL(view->square_layout, -1);
	_mp_square_view_new_gengrid_title_set(view);

	elm_object_part_content_set(view->layout, "list_content", view->square_layout);


	return ret;
}

MpSquareView_t *mp_square_view_new_create(Evas_Object *parent)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpSquareView_t *view = calloc(1, sizeof(MpSquareView_t));
	MP_CHECK_NULL(view);

	ret = _mp_square_view_new_init(parent, view);
	if (ret) goto Error;

	_mp_square_view_update_now_playing_postion(view);
	_mp_square_view_load_playing_square_list(view);

	return view;

Error:
	ERROR_TRACE("Error: mp_square_view_new_create()");
	IF_FREE(view);
	return NULL;
}

int mp_square_view_new_destory(MpSquareView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	return 0;
}

