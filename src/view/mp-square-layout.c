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

#include "mp-square-layout.h"
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
	MpSquareAllList_t *list;
	mp_square_position_t position;
	bool b_seleted;
} mp_square_gengrid_item_data_t;

typedef struct {
	Elm_Object_Item *it;
	MpSquareAllList_t *list;

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

static void _mp_square_reader_on_mouse_down(void *data, Evas_Object *obj, Elm_Object_Item *item);
static void _mp_square_help_btn_cb(void *data, Evas_Object *obj, void *event_info);
static void _mp_square_help_tts_double_click_cb(void *data, Evas_Object *part_obj, Elm_Object_Item *item);

static void
_mp_square_position_list_free(GList *list)
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
_mp_square_music_list_free(GList *list)
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
_mp_square_position_list_clear(GList **list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(list);

	_mp_square_position_list_free(*list);
	*list = NULL;
}

static void
_mp_square_music_list_clear(GList **list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(list);

	_mp_square_music_list_free(*list);
	*list = NULL;
}

static inline void
*_mp_square_set_upper_text(Evas_Object *layout, const char *part, const char *text)
{
	MP_CHECK(layout);

	char *upper_text = g_utf8_strup(GET_STR(text), -1);
	mp_util_domain_translatable_part_text_set(layout, part, text);

	SAFE_FREE(upper_text);
}

void
mp_square_list_gengrid_title_set(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(list);
	MP_CHECK(list->square_layout);

	const char *top = NULL;
	const char *bottom = NULL;
	const char *left = NULL;
	const char *right = NULL;

	if (list->type == MP_SQUARE_TYPE_MOOD) {
		top = STR_EXCITING;
		bottom = STR_CALM;
		left = STR_PASSION;
		right = STR_JOY;
	} else if (list->type == MP_SQUARE_TYPE_YEAR) {
		top = STR_EXCITING;
		bottom = STR_CALM;
		left = STR_OLD;
		right = STR_NEW_FOR_SQUARE;
	} else if (list->type == MP_SQUARE_TYPE_ADDED) {
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

	/* Set TTS order "top title"/"help button"/"left title"/"gengrid"/"right title"/"buttom title" */
	_mp_square_set_upper_text(list->square_layout, "title_top", top);

}

static mp_square_item_t*
_mp_square_current_playing_music_item_get(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->music_list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	mp_plst_item *cur_playing_music = mp_playlist_mgr_get_current(ad->playlist_mgr);
	MP_CHECK_NULL(cur_playing_music);

	mp_square_item_t *item = NULL;
	int count = g_list_length(list->music_list);
	int i = 0;
	for (i = 0; i < count; i++) {
		item = g_list_nth_data(list->music_list, i);

		if (item != NULL) {
			if (!strcmp(item->path, cur_playing_music->uri))
				return item;
		}
	}

	return NULL;
}

static void
_mp_square_current_play_list_create(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();

	int index;
	MP_CHECK(list);
	MP_CHECK(list->music_list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	GList *list_music = list->music_list;
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
_mp_square_selected_musics_get(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(list);
	MP_CHECK(list->pos_list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* clear selected musics of last time */
	if (list->music_list != NULL) {
		_mp_square_music_list_free(list->music_list);
		list->music_list = NULL;
	}

	mp_square_mgr_records_get_by_type_and_positions(
											ad->square_mgr,
											list->type,
											list->pos_list,
											&list->music_list);
}

static void
_mp_square_library_empty_popup_response_cb(void *data, Evas_Object * obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK(obj);
	Evas_Object *popup = obj;
	mp_evas_object_del(popup);
}

static void
_mp_square_library_empty_popup_create(MpSquareAllList_t *list)
{
	MP_CHECK(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* create popup */
	Evas_Object *popup = NULL;
	popup = mp_popup_create(ad->win_main,
							MP_POPUP_NORMAL,
							NULL,
							list,
							_mp_square_library_empty_popup_response_cb, ad);

	evas_object_size_hint_weight_set(popup, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	char *text = g_strconcat("<align=center>", GET_STR("IDS_MUSIC_POP_NO_MUSIC_FOUND_FOR_SELECTED_CELL"), "</align>", NULL);
	mp_util_domain_translatable_text_set(popup, text);
	mp_popup_button_set(popup, MP_POPUP_BTN_1, "IDS_COM_SK_OK", MP_POPUP_YES);

	evas_object_show(popup);
	IF_FREE(text);
}

static bool
_mp_square_gengrid_item_is_selected(MpSquareAllList_t *list, mp_square_position_t *position)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK_FALSE(list);
	MP_CHECK_FALSE(list->pos_list);

	GList *pos_list = list->pos_list;
	mp_square_position_t *pos = NULL;

	int count = g_list_length(pos_list);
	int i = 0;

	for (i = 0; i < count; i++) {
		pos = g_list_nth_data(pos_list, i);
		if (pos != NULL) {
			if (pos->x == position->x && pos->y == position->y)
				/*mp_debug("[%d,%d] selected", pos->x, pos->y);*/
				return true;
		}
	}

	return false;
}

static Evas_Object*
_mp_square_gengrid_item_content_get(void *data, Evas_Object *obj, const char *part)
{
	MP_CHECK_NULL(data);
	mp_square_gengrid_item_data_t *item_data = (mp_square_gengrid_item_data_t *)data;
	MpSquareAllList_t *list = item_data->list;
	char *title = NULL;

	if (!g_strcmp0(part, "elm.swallow.icon")) {
		Evas_Object *icon = NULL;

		bool show_thumnail = false;
		if (list->now_playing_position.x == item_data->position.x
				&& list->now_playing_position.y == item_data->position.y) {
			mp_debug("now playing cell = [%d, %d]", item_data->position.x, item_data->position.y);
			show_thumnail = true;
		} else {
			mp_square_item_t *item = list->current_item;
			if (item) {
				if (item->pos.x == item_data->position.x && item->pos.y == item_data->position.y)
					show_thumnail = true;
			}
		}

		/*ERROR_TRACE("show_thumnail %d, item_data->b_seleted &d",show_thumnail, item_data->b_seleted);*/

		icon = mp_common_load_edj(obj, MP_EDJ_NAME, "square_focused_cell");

		if (show_thumnail) {
			edje_object_signal_emit(_EDJ(icon), "set_focused", "*");

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
			edje_object_signal_emit(_EDJ(icon), "set_pressed", "*");

			Evas_Object *img = elm_image_add(icon);
			elm_object_part_content_set(icon, "cell_selected_bg", img);

			elm_image_file_set(img, IMAGE_EDJ_NAME, MP_ICON_SQUARE_CELL_SELECTED);

			int item_pos = MP_SQUARE_AXIS_X_LEN * (item_data->position.y-1) + item_data->position.x - 1;
			evas_object_color_set(img, square_normal_color[item_pos * 1], square_normal_color[item_pos * 2], square_normal_color[item_pos * 3], square_normal_color[item_pos * 4]);

		} else {
			edje_object_signal_emit(_EDJ(icon), "set_normal", "*");
		}

		mp_screen_reader_set_list_item_info(item_data->it, NULL, NULL, NULL, _mp_square_reader_on_mouse_down, item_data);

		/* Set the property of gengrid */
		Evas_Coord icon_w = 0;
		Evas_Coord icon_h = 0;

		elm_gengrid_item_size_get(list->gengrid, &icon_w, &icon_h);
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
_mp_square_gengrid_item_del_cb(void *data, Evas_Object *obj)
{
	DEBUG_TRACE_FUNC();

	mp_square_gengrid_item_data_t *item_data = data;
	SAFE_FREE(item_data);
}

static void
_mp_square_gengrid_item_sel_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(data);
	mp_square_gengrid_item_data_t *item_data = (mp_square_gengrid_item_data_t *)data;

	MpSquareAllList_t *list = item_data->list;
	MP_CHECK(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		_mp_square_library_empty_popup_create(list);
		return;
	}

	item_data->b_seleted = !item_data->b_seleted;
	elm_gengrid_item_update(item_data->it);
}


static void
_mp_square_gengrid_items_load(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(list);
	MP_CHECK(list->gengrid);

	static Elm_Gengrid_Item_Class gic = { 0, };

	gic.item_style = "music/grid_square";
	gic.func.content_get = _mp_square_gengrid_item_content_get;
	gic.func.del = _mp_square_gengrid_item_del_cb;

	int index = 0;
	for (index = 0; index < MP_SQUARE_CELLS_COUNT; index++) {
		mp_square_gengrid_item_data_t *item_data = calloc(1, sizeof(mp_square_gengrid_item_data_t));
		item_data->list = list;
		item_data->position.x = index%MP_SQUARE_AXIS_Y_LEN+1;
		item_data->position.y = index/MP_SQUARE_AXIS_X_LEN+1;
		item_data->b_seleted = _mp_square_gengrid_item_is_selected(list, &(item_data->position));

		item_data->it = elm_gengrid_item_append(list->gengrid,
												&gic,
												item_data,
												_mp_square_gengrid_item_sel_cb,
												item_data);
	}
}

void
mp_square_gengrid_items_state_reset(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(list);
	MP_CHECK(list->gengrid);

	Elm_Object_Item *it;
	mp_square_gengrid_item_data_t *data = NULL;

	it = elm_gengrid_first_item_get(list->gengrid);
	while (it) {
		data = (mp_square_gengrid_item_data_t *)elm_object_item_data_get(it);
		data->b_seleted = false;
		it = elm_gengrid_item_next_get(it);
	}
	elm_gengrid_realized_items_update(list->gengrid);
}

static void
_mp_square_gengrid_reset(MpSquareAllList_t *list)
{
	startfunc;
	MP_CHECK(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	mp_square_gengrid_items_state_reset(list);

	if (list->pos_list != NULL)
		_mp_square_position_list_clear(&(list->pos_list));

	if (list->music_list != NULL)
		_mp_square_music_list_clear(&(list->music_list));

	endfunc;
}


Eina_Bool
mp_square_list_update_square(void *data)
{
	startfunc;
	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK_VAL(list, ECORE_CALLBACK_CANCEL);
	MP_CHECK_VAL(list->square_layout, ECORE_CALLBACK_CANCEL);

	int w = 0;
	int h = 0;
	edje_object_part_geometry_get(_EDJ(list->square_layout), "gengrid", NULL, NULL, &w, &h);

	if (list->gengrid) {
		if (elm_gengrid_items_count(list->gengrid)) {
			DEBUG_TRACE("Already done");
			return ECORE_CALLBACK_DONE;
		}
	}

	double scale_factor = 0.0;
	scale_factor = elm_config_scale_get();
	mp_debug("elm_config_scale_get =%f", scale_factor);
	mp_debug("w: %d, h: %d, item_size: %d", w, h, (w/MP_SQUARE_AXIS_X_LEN));

	elm_gengrid_item_size_set(list->gengrid, (w/MP_SQUARE_AXIS_X_LEN), (h/MP_SQUARE_AXIS_Y_LEN));

	/* load square data */
	_mp_square_gengrid_items_load(list);


	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, ECORE_CALLBACK_CANCEL);

	if (ad->square_mgr == NULL) {
		int ret = mp_square_mgr_create(ad);
		if (ret != 0 || ad->square_mgr == NULL) {
			mp_error("fail to create square mgr");
			return ECORE_CALLBACK_CANCEL;
		}
		mp_list_update((MpList_t *)list);
	}

	endfunc;
	return ECORE_CALLBACK_DONE;
}

static Eina_Bool
_mp_square_update_timer(void *data)
{
	TIMER_TRACE();
	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK_FALSE(list);

	mp_square_list_update_square(list);
	mp_ecore_timer_del(list->init_timer);

	return ECORE_CALLBACK_DONE;
}


static void
_mp_square_gengrid_destroy_cb(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK(list);
	list->gengrid = NULL;
	endfunc;
}

static mp_square_gengrid_item_data_t*
_mp_square_gengrid_item_get_by_mouse_position(MpSquareAllList_t *list, Evas_Position *pos)
{
	DEBUG_TRACE_FUNC();

	MP_CHECK_NULL(list);
	MP_CHECK_NULL(list->gengrid);
	MP_CHECK_NULL(pos);

	Elm_Object_Item *it = NULL;
	mp_square_gengrid_item_data_t *item_data = NULL;
	Evas_Object *item_content = NULL;

	it = elm_gengrid_first_item_get(list->gengrid);
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
_mp_square_reader_on_mouse_down(void *data, Evas_Object *obj, Elm_Object_Item *item)
{
	DEBUG_TRACE_FUNC();

	mp_square_gengrid_item_data_t *item_data = (mp_square_gengrid_item_data_t *)data;
	MpSquareAllList_t *list = item_data->list;

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		mp_square_mgr_create(ad);
		MP_CHECK(ad->square_mgr);
	}

	list->b_mouse_down = true;

	/* clear the selected cells position of last time */
	if (list->pos_list != NULL) {
		_mp_square_position_list_free(list->pos_list);
		list->pos_list = NULL;
	}

	mp_square_gengrid_items_state_reset(list);
	mp_list_update((MpList_t *)list);

	if (item_data->b_seleted)
		return;

	mp_square_position_t *position = calloc(1, sizeof(mp_square_position_t));
	position->x = item_data->position.x;
	position->y = item_data->position.y;

	list->pos_list = g_list_append(list->pos_list, position);

	item_data->b_seleted = true;
	elm_gengrid_item_update(item_data->it);

	list->b_mouse_down = false;

	_mp_square_selected_musics_get(list);

	if (list->music_list != NULL) {
		if (list->screen_mode == MP_SCREEN_MODE_PORTRAIT) {
		/* TODO: playlist genlist */
		/*mp_layout_update(list->layout_genlist);*/
		}

	if (!ad->playlist_mgr)
		mp_common_create_playlist_mgr();
		mp_playlist_mgr_clear(ad->playlist_mgr);
		_mp_square_current_play_list_create(list);

		mp_play_item_play_current_item(ad);
		mp_list_update((MpList_t *)list);

	} else {
		_mp_square_library_empty_popup_create(list);
		/*mp_square_update_options((MpSquareAllList_t *)list);*/
}


}


static void
_mp_square_on_mouse_down(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();
	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		mp_square_mgr_create(ad);
		MP_CHECK(ad->square_mgr);
	}

	list->b_mouse_down = true;

	/* clear the selected cells position of last time */
	if (list->pos_list != NULL) {
		_mp_square_position_list_free(list->pos_list);
		list->pos_list = NULL;
	}

	mp_square_gengrid_items_state_reset(list);

	/* get cell item by mouse position */
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;
	mp_square_gengrid_item_data_t *item_data = NULL;
	item_data = _mp_square_gengrid_item_get_by_mouse_position(list, &ev->cur);
	MP_CHECK(item_data);

	if (item_data->b_seleted)
		return;

	mp_square_position_t *position = calloc(1, sizeof(mp_square_position_t));
	position->x = item_data->position.x;
	position->y = item_data->position.y;

	list->pos_list = g_list_append(list->pos_list, position);

	item_data->b_seleted = true;
	elm_gengrid_item_update(item_data->it);
}

static void
_mp_square_on_mouse_move(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		return;
	}

	if (list->b_mouse_down == false) {
		return;
	}

	/* get cell item by mouse position */
	Evas_Event_Mouse_Move *ev = (Evas_Event_Mouse_Move *)event_info;
	mp_square_gengrid_item_data_t *item_data = NULL;
	item_data = _mp_square_gengrid_item_get_by_mouse_position(list, &ev->cur);
	MP_CHECK(item_data);

	if (item_data->b_seleted)
		return;

	mp_square_position_t *position = calloc(1, sizeof(mp_square_position_t));
	position->x = item_data->position.x;
	position->y = item_data->position.y;

	list->pos_list = g_list_append(list->pos_list, position);

	item_data->b_seleted = true;
	elm_gengrid_item_update(item_data->it);
}

static void
_mp_square_on_mouse_up(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	/* square library is not inited */
	if (ad->square_mgr == NULL) {
		return;
	}

	list->b_mouse_down = false;

	_mp_square_selected_musics_get(list);
	list->current_item = _mp_square_current_playing_music_item_get(list);

	if (list->music_list != NULL) {
		if (list->screen_mode == MP_SCREEN_MODE_PORTRAIT) {
			/* TODO: playlist genlist
			mp_layout_update(list->layout_genlist);*/
		}

		if (!ad->playlist_mgr)
			mp_common_create_playlist_mgr();
		mp_playlist_mgr_clear(ad->playlist_mgr);
		_mp_square_current_play_list_create(list);
		mp_list_update((MpList_t *)list);

		mp_common_show_player_view(MP_PLAYER_NORMAL, false, true, true);

	} else {
		elm_gengrid_realized_items_update(list->gengrid);
		_mp_square_library_empty_popup_create(list);
		/*mp_square_update_options((MpSquareAllList_t *)list);*/
	}
}

static void _mp_square_help_select_cb(void *data, Evas_Object *obj, const char *emission, const char *source)
{
	MP_CHECK(data);
	_mp_square_help_btn_cb(data, NULL, NULL);
}

static void _mp_square_help_tts_double_click_cb(void *data, Evas_Object *part_obj, Elm_Object_Item *item)
{
	MP_CHECK(data);
	_mp_square_help_btn_cb(data, NULL, NULL);
}

Evas_Object *
mp_square_list_get_content(void *thiz)
{
	DEBUG_TRACE_FUNC();
	MpSquareAllList_t *list = (MpSquareAllList_t *)thiz;
	MP_CHECK_NULL(list);

	return list->square_layout;
}

Evas_Object *
mp_square_content_create(void *thiz)
{
	DEBUG_TRACE_FUNC();
	MpSquareAllList_t *list = (MpSquareAllList_t *)thiz;
	MP_CHECK_NULL(list);

	Evas_Object *layout = NULL;

	ERROR_TRACE("mp_util_is_landscape(): %d", mp_util_is_landscape());

	if (mp_util_is_landscape()) {
		layout = mp_common_load_edj(list->layout, PLAY_VIEW_EDJ_NAME, "mp_square_view_landscape");
	} else {
		layout = mp_common_load_edj(list->layout, PLAY_VIEW_EDJ_NAME, "mp_square_view");
	}

	MP_CHECK_NULL(layout);

	mp_evas_object_del(list->gengrid);
	list->gengrid = elm_gengrid_add(layout);
	MP_CHECK_NULL(list->gengrid);

	evas_object_event_callback_add(list->gengrid, EVAS_CALLBACK_DEL, _mp_square_gengrid_destroy_cb, list);
	evas_object_event_callback_add(list->gengrid, EVAS_CALLBACK_MOUSE_DOWN, _mp_square_on_mouse_down, list);
	evas_object_event_callback_add(list->gengrid, EVAS_CALLBACK_MOUSE_UP, _mp_square_on_mouse_up, list);
	evas_object_event_callback_add(list->gengrid, EVAS_CALLBACK_MOUSE_MOVE, _mp_square_on_mouse_move, list);

	elm_object_signal_callback_add(layout, "clicked", "elm", _mp_square_help_select_cb, list);

	evas_object_show(list->gengrid);
	elm_object_part_content_set(layout, "gengrid", list->gengrid);

	return layout;
}


static void
_mp_square_help_btn_cb(void *data, Evas_Object *obj, void *event_info)
{
	eventfunc;
	MpSquareAllList_t *list = data;
	MP_CHECK(list);
	/*mp_evas_object_del(list->more_btn_ctxpopup);*/

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
_mp_square_update_library_progressbar_timer(void *data)
{
	TIMER_TRACE();
	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK_VAL(list, ECORE_CALLBACK_CANCEL);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_VAL(ad, ECORE_CALLBACK_CANCEL);
	MP_CHECK_VAL(ad->square_mgr, ECORE_CALLBACK_CANCEL);

	double value = 0.0;
	int index = ad->square_mgr->record_count;
	int total = ad->square_mgr->total_count;
	Evas_Object *layout = list->layout_update_library_progress;
	Evas_Object *progressbar = list->update_library_progressbar;

	/*mp_debug("index=%d, total=%d\n", index, total);*/

	value = elm_progressbar_value_get(progressbar);
	if (value == 1.0) {
		mp_ecore_timer_del(list->update_library_timer);

		if (list->popup_update_library_progress) {
			evas_object_del(list->popup_update_library_progress);
			list->popup_update_library_progress = NULL;
		}

		mp_list_update((MpList_t *) list);
		mp_util_post_status_message(ad, GET_STR(STR_MP_UPDATED));
		return ECORE_CALLBACK_CANCEL;
	}

	if (total == 0) {
		index = 1;
		total = 1;
	}

	value = (double)index / total;

	/*mp_debug("value=%6.2f\n", value);*/
	elm_progressbar_value_set(progressbar, value);

	char buf[255] = {'0',};

	snprintf(buf, sizeof(buf), "%d/%d", index, total);
	elm_object_part_text_set(layout, "elm.text.right", buf);

	return ECORE_CALLBACK_RENEW;
}

static void
_mp_square_update_library_progressbar_response_cb(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareAllList_t *list = (MpSquareAllList_t *)data;
	MP_CHECK(list);

	mp_ecore_timer_del(list->update_library_timer);
	if (list->popup_update_library_progress) {
		evas_object_del(list->popup_update_library_progress);
		list->popup_update_library_progress = NULL;
	}

	mp_list_update((MpList_t *) list);

	/* cancel update library */
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	ad->square_mgr->terminal_status = true;
}

static void
_mp_square_update_library_progressbar_create(MpSquareAllList_t *list)
{
	startfunc;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *layout = NULL;
	Evas_Object *popup = NULL;
	Evas_Object *progressbar = NULL;
	/*Evas_Object *label = NULL;*/
	Evas_Object *btn1 = NULL;
	Ecore_Timer	*timer = NULL;

	popup = mp_popup_create(ad->win_main, MP_POPUP_NORMAL, GET_STR("IDS_MUSIC_BODY_LIBRARY_UPDATE"),
			list, _mp_square_update_library_progressbar_response_cb, ad);
	list->popup_update_library_progress = popup;

	layout = elm_layout_add(popup);
	list->layout_update_library_progress = layout;
	elm_layout_file_set(layout, PLAY_VIEW_EDJ_NAME, "popup_update_library_progresssquare");
	evas_object_size_hint_weight_set(layout, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);

	progressbar = elm_progressbar_add(popup);
	list->update_library_progressbar = progressbar;
	elm_object_style_set(progressbar, "list_progress");
	elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
	evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
	evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	elm_progressbar_value_set(progressbar, 0.0);
	timer = ecore_timer_add(0.1, _mp_square_update_library_progressbar_timer, list);
	list->update_library_timer = timer;
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
	evas_object_smart_callback_add(btn1, "clicked", _mp_square_update_library_progressbar_response_cb, list);

	evas_object_show(popup);
}

void
mp_square_library_update(MpSquareAllList_t *list)
{
	startfunc;
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	if (ad->square_mgr == NULL) {
		int ret = 0;
		ret = mp_square_mgr_create(ad);
		if (ret != 0 || ad->square_mgr == NULL)
			return;
	}

	_mp_square_gengrid_reset(list);
	mp_list_update((MpList_t *)list);

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

	_mp_square_update_library_progressbar_create(list);

	endfunc;
}

static void
_mp_square_position_append_by_item(MpSquareAllList_t *list, mp_square_item_t *item)
{
	MP_CHECK(list);
	MP_CHECK(item);

	bool exist = false;
	GList *pos_list = list->pos_list;
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
		list->pos_list = g_list_append(list->pos_list, pos);
	}
}

static void
_mp_square_update_now_playing_postion(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->square_mgr);
	MP_CHECK(ad->current_track_info);

	list->current_item = _mp_square_current_playing_music_item_get(list);

	mp_track_info_t *cur_playing_music = ad->current_track_info;
	if (cur_playing_music && mp_check_file_exist(cur_playing_music->uri)) {
		mp_square_mgr_get_positon_by_type_and_path(ad->square_mgr, list->type, cur_playing_music->uri, &(list->now_playing_position));
		mp_debug("now playing pos = [%d, %d]", list->now_playing_position.x, list->now_playing_position.y);
	}
}

static void
_mp_square_load_playing_square_list(MpSquareAllList_t *list)
{
	DEBUG_TRACE_FUNC();
	MP_CHECK(list);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	MP_CHECK(ad->playlist_mgr);
	MP_CHECK(ad->square_mgr);

	if (list->music_list)
		_mp_square_music_list_clear(&list->music_list);
	if (list->pos_list)
		_mp_square_position_list_clear(&list->pos_list);

	if (mp_playlist_mgr_get_list_type(ad->playlist_mgr) == MP_PLST_TYPE_MUSIC_SQUARE) {
		GList *selected_list = NULL;
		int type = -1;
		mp_square_mgr_selected_list_items_get(ad->square_mgr, &type, &selected_list);

		if (g_list_length(selected_list) && list->type == type) {
			list->music_list = selected_list;
			GList *music_list = list->music_list;
			while (music_list) {
				mp_square_item_t *item = music_list->data;
				if (item)
					_mp_square_position_append_by_item(list, item);
				music_list = music_list->next;
			}
		} else {
			if (selected_list)
				_mp_square_music_list_clear(&selected_list);
		}
	}
}

static void
_mp_square_axis_change(MpSquareAllList_t *list)
{
	startfunc;
	MP_CHECK(list);

	list->type = list->radio_index;
	if (!mp_setting_set_square_axis_val(list->type)) {

		mp_square_list_gengrid_title_set(list);

		struct appdata *ad = mp_util_get_appdata();
		if (ad && ad->square_mgr)
			mp_square_mgr_selected_list_items_clear(ad->square_mgr);

		_mp_square_gengrid_reset(list);
		mp_list_update((MpList_t *)list);
	}

	endfunc;
}

static void
_mp_square_radio_main_changed_cb(void *data, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpSquareAllList_t *list = data;
	MP_CHECK(list);

	elm_radio_value_set(list->radio_main, list->radio_index);
	_mp_square_axis_change(list);
	endfunc;
}

static void
_mp_square_radio_main_del_cb(void *data, Evas * evas, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpSquareAllList_t *list = data;
	MP_CHECK(list);
	list->radio_main = NULL;
}

static char *
_mp_square_axis_change_gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	char *label = (char *)data;
	/*DEBUG_TRACE("%s", label);*/
	return g_strdup(GET_STR(label));
}

static Evas_Object *
_mp_square_axis_change_gl_content_get(void *data, Evas_Object * obj, const char *part)
{
	DEBUG_TRACE("");
	MP_CHECK_NULL(data);

	MpSquareAllList_t *list = evas_object_data_get(obj, "square_square_data");
	MP_CHECK_NULL(list);

	Evas_Object *radio = elm_radio_add(obj);
	elm_radio_group_add(radio, list->radio_main);

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

	elm_radio_value_set(list->radio_main, list->type);

	return radio;
}

static void
_mp_square_axis_change_popup_gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	DEBUG_TRACE_FUNC();

	MpSquareAllList_t *list = evas_object_data_get(obj, "square_square_data");
	MP_CHECK(list);
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	char *label = (char *)data;
	DEBUG_TRACE("label=%s", label);
	if (g_strcmp0(label, STR_MOOD) == 0)
		list->radio_index = MP_SQUARE_TYPE_MOOD;
	else if (g_strcmp0(label, STR_YEAR) == 0)
		list->radio_index = MP_SQUARE_TYPE_YEAR;
	else if (g_strcmp0(label, STR_ADDED) == 0)
		list->radio_index = MP_SQUARE_TYPE_ADDED;
	else if (g_strcmp0(label, STR_TIME) == 0)
		list->radio_index = MP_SQUARE_TYPE_TIME;

	elm_radio_value_set(list->radio_main, list->radio_index);
	evas_object_smart_callback_call(list->radio_main, "changed", list);

	mp_popup_destroy(ad);
}

void
mp_square_change_axis_popup(void *data)
{
	eventfunc;
	MpSquareAllList_t *list = data;
	MP_CHECK(list);
	/*mp_evas_object_del(list->more_btn_ctxpopup);*/

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *popup = mp_genlist_popup_create(ad->win_main, MP_POPUP_CHANGE_AXIS, ad, ad);
	MP_CHECK(popup);

	if (list->radio_main == NULL) {
		list->radio_main = elm_radio_add(popup);
		elm_radio_state_value_set(list->radio_main, -1);
		evas_object_smart_callback_add(list->radio_main, "changed", _mp_square_radio_main_changed_cb, list);
		evas_object_event_callback_add(list->radio_main, EVAS_CALLBACK_DEL, _mp_square_radio_main_del_cb, list);
		evas_object_hide(list->radio_main);
	}

	Evas_Object *genlist = evas_object_data_get(popup, "genlist");
	MP_CHECK(genlist);
	evas_object_data_set(genlist, "square_square_data", list);

	static Elm_Genlist_Item_Class change_axis_itc;
	change_axis_itc.item_style = "1text.1icon.3";
	change_axis_itc.func.text_get = _mp_square_axis_change_gl_label_get;
	change_axis_itc.func.content_get = _mp_square_axis_change_gl_content_get;
	change_axis_itc.func.state_get = NULL;
	change_axis_itc.func.del = NULL;

	elm_genlist_item_append(genlist, &change_axis_itc, STR_MOOD, NULL, ELM_GENLIST_ITEM_NONE,
				_mp_square_axis_change_popup_gl_sel, STR_MOOD);
	elm_genlist_item_append(genlist, &change_axis_itc, STR_YEAR, NULL, ELM_GENLIST_ITEM_NONE,
				_mp_square_axis_change_popup_gl_sel, STR_YEAR);

	endfunc;
}

static void
_mp_square_all_list_update(void *thiz)
{
	startfunc;
	MpSquareAllList_t *list = (MpSquareAllList_t *)thiz;
	MP_CHECK(list);
	MP_CHECK(list->gengrid);

	_mp_square_update_now_playing_postion(list);
	elm_gengrid_realized_items_update(list->gengrid);
	endfunc;
}

int
mp_square_playlist_position_update(void *thiz)
{
	startfunc;
	MpSquareAllList_t *list = (MpSquareAllList_t *)thiz;
	MP_CHECK_VAL(list, -1);
	MP_CHECK_VAL(list->gengrid, -1);

	list->current_item = _mp_square_current_playing_music_item_get(list);
	int count = g_list_length(list->music_list);
	DEBUG_TRACE("playinglist count:%d", count);
	if (count > 1) {
		_mp_square_update_now_playing_postion(list);
		elm_gengrid_realized_items_update(list->gengrid);
	}
	endfunc;
	return 0;
}

static mp_track_type_e _mp_square_all_list_get_track_type(void *thiz)
{
	return MP_TRACK_BY_SQUARE;
}

static void
_mp_square_all_list_destory(void *thiz)
{
	startfunc;
	MpSquareAllList_t *list = thiz;
	MP_CHECK(list);

	/* TODO: release resource..*/

	mp_evas_object_del(list->gengrid);
	/*mp_evas_object_del(list->more_btn_ctxpopup);*/
	mp_evas_object_del(list->popup_update_library);
	mp_evas_object_del(list->popup_update_library_progress);
	mp_ecore_timer_del(list->init_timer);

	mp_ecore_timer_del(list->update_library_timer);

	if (list->pos_list != NULL)
		_mp_square_position_list_clear(&(list->pos_list));
	if (list->music_list != NULL)
		_mp_square_music_list_clear(&(list->music_list));

	free(list);
}

MpSquareAllList_t *
mp_square_all_list_create(Evas_Object *parent)
{
	startfunc;
	MP_CHECK_NULL(parent);

	MpSquareAllList_t *list = calloc(1, sizeof(MpSquareAllList_t));
	MP_CHECK_NULL(list);

	mp_list_init((MpList_t *)list, parent, MP_LIST_TYPE_SQUARE);

	list->update = _mp_square_all_list_update;
	list->destory_cb = _mp_square_all_list_destory;
	list->get_track_type = _mp_square_all_list_get_track_type;
	/*list->rotate = _mp_square_all_list_rotate;*/

	int square_type = MP_SQUARE_TYPE_MOOD;
	mp_setting_get_square_axis_val(&square_type);
	list->type = (mp_square_type_t)square_type;

	list->square_layout = mp_square_content_create(list);

	mp_square_list_gengrid_title_set(list);
	_mp_square_update_now_playing_postion(list);
	_mp_square_load_playing_square_list(list);

	list->init_timer = ecore_timer_add(0.1, _mp_square_update_timer, list);

	return list;
}
