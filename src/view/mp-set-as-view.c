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

/* #include "smat.h" */
#include "mp-set-as-view.h"
#include "music.h"
#include "mp-player-debug.h"
#include "mp-player-mgr.h"
#include "mp-play.h"
#include "mp-minicontroller.h"
#include "app_manager.h"
#include "mp-popup.h"
#include "mp-util.h"
#include <system_settings.h>
#include "mp-ug-launch.h"

static Evas_Object *g_radio_recommend = NULL;
static Evas_Object *g_radio_set_as_type = NULL;

typedef struct {
	Elm_Object_Item *item;
	int index;
} item_data_s;

static void _mp_set_as_view_moved_recommended_time(void *data);
static void _mp_set_as_view_enable_done_btn(MpSetAsView_t *view);


/*SMAT id;
SMAT_STAT nowstat = SMAT_ERR; */

pthread_mutex_t smat_mutex;
pthread_cond_t  smat_cond;

static char *_mp_set_as_view_time_to_string(int time)
{
	int minutes = (time / 1000) / 60;
	int seconds = (time / 1000) % 60;
	char *seconds_fmt = NULL;
	char *minutes_fmt = NULL;
	DEBUG_TRACE("minute is %d\tseconds is %d", minutes, seconds);
	if (seconds < 10) {
		seconds_fmt = "0%d";
	} else {
		seconds_fmt = "%d";
	}

	if (minutes < 10) {
		minutes_fmt = "0%d";
	} else {
		minutes_fmt = "%d";
	}
	char *format = g_strconcat(minutes_fmt, ":", seconds_fmt, NULL);
	if (!format) {
		return NULL;
	}
	char *total_txt = g_strdup_printf(format, minutes, seconds);
	IF_FREE(format);
	return total_txt;
}

/****Pre-listen related****/
static void _mp_set_as_view_create_player(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);

	/*1. create player*/
	int ret = 0;
	ret = player_create(&(view->player));
	if (PLAYER_ERROR_NONE != ret) {
		DEBUG_TRACE("create player error %s", ret);
	}
}

static void _mp_set_as_view_destroy_player(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);

	/*1. create player*/
	int ret = 0;
	ret = player_destroy(view->player);
	if (PLAYER_ERROR_NONE != ret) {
		DEBUG_TRACE("destroy player error %s", ret);
	}
	view->player = NULL;
}

static int _mp_set_as_view_get_duration(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK_VAL(view, 0);
	MP_CHECK_VAL(view->player, 0);

	int duration = 0;
	int ret = player_get_duration(view->player, &duration);
	if (ret) {
		mp_error("player_get_duration() .. [0x%x]", ret);
		duration = 0;
	}

	return duration;
}

static void _mp_set_as_view_prelisten_set_uri(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	char *path = NULL;	/* do not free */
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	path = view->path;
	MP_CHECK(path);

	int ret = PLAYER_ERROR_NONE;
	ret = player_set_uri(view->player, path);
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("Set URI failed");
	}
}

static void _mp_set_as_view_prelisten_prepare(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	int ret = PLAYER_ERROR_NONE;
	ret = player_prepare(view->player);
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("prepare player failed");
	}
}

static void _mp_set_as_view_prelisten_start(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	int ret = PLAYER_ERROR_NONE;
	ret = player_start(view->player);
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("start player failed");
	}
}

static void _mp_set_as_view_prelisten_stop(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	int ret = PLAYER_ERROR_NONE;
	ret = player_stop(view->player);
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("stop player failed");
	}
}

static void _mp_set_as_view_prelisten_pause(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	int ret = PLAYER_ERROR_NONE;
	ret = player_pause(view->player);
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("pause player failed");
	}
}

static player_state_e _mp_set_as_view_prelisten_get_state(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK_VAL(view, PLAYER_STATE_NONE);
	MP_CHECK_VAL(view->player, PLAYER_STATE_NONE);

	int ret = PLAYER_ERROR_NONE;
	player_state_e state = PLAYER_STATE_NONE;
	ret = player_get_state(view->player, &state);
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("get player state failed");
	}
	return state;
}

static void __mp_set_as_view_player_seek_completed_cb(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	ERROR_TRACE("seek done");
	int millisecond = 0;
	player_get_play_position(view->player, &millisecond);
	DEBUG_TRACE("position is %d", millisecond);
	_mp_set_as_view_prelisten_start(data);

	_mp_set_as_view_moved_recommended_time(view);
}

static void _mp_set_as_view_prelisten_set_position(void *data, player_seek_completed_cb callback, void *cb_data, bool flag_position)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	DEBUG_TRACE("seek postion is %d", view->position);
	int position = flag_position ? view->position : 0;
	int ret = PLAYER_ERROR_NONE;
	ret = player_set_play_position(view->player, position, TRUE, callback, cb_data);
	if (ret != PLAYER_ERROR_NONE) {
		ERROR_TRACE("set position failed");
	}
}

/**************************/

/************SMAT related*****************/
void *observer(void* data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK_NULL(view);

	long int pos = 0;
	/*SMAT id = param;
	SMAT_INFO info;
	info.seg_infos = NULL;

	while (1)*/
	{
		/*nowstat = smat_get_stat(id);
		if (nowstat == SMAT_CLST_DONE)*/
		{
			/*smat_get_info(id,&info);
			pos = info.time / 1000;
			DEBUG_TRACE("*Selected Pos : [%ld:%ld]", (((pos)%3600)/60),(((pos)%3600)%60));
			view->position = info.time;
			break; */
		}
		/*else if (nowstat == SMAT_QUIT )*/
		{
			/*break; */
		}
	}

	ecore_pipe_write(view->smat_pipe, &pos, sizeof(pos));
	return NULL;
}

void _mp_set_as_view_smat_pipe_cb(void *data, void *buffer, unsigned int nbyte)
{
	startfunc;
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);

	long int message = (long int)buffer;

	DEBUG_TRACE("Position is %ld", message);

	/* smat_deinit(id); */

	/*set recommended text*/
	Evas_Object *genlist = view->content;
	Evas_Object *layout = elm_object_item_part_content_get(elm_genlist_nth_item_get(genlist, MP_SET_AS_RECOMMEND_PRE_LISTEN), "elm.icon");
	Evas_Object *recommended_txt = elm_object_part_content_get(layout, "recommended_text_play");
	char *recommended_time = _mp_set_as_view_time_to_string(view->position);
	mp_util_domain_translatable_text_set(recommended_txt, recommended_time);
	IF_FREE(recommended_time);
	/* evas_object_show(recommended_txt); */
	Evas_Object *progressbar = elm_object_part_content_get(layout, "progress_bar");
	elm_progressbar_value_set(progressbar, (double)(1.0 - (double)view->position / (double)view->duration));

	_mp_set_as_view_prelisten_set_position(view, __mp_set_as_view_player_seek_completed_cb, view, true);
}

void
_mp_set_as_view_smat_popup_back_cb(void *data, Evas_Object *obj, void *event_info)
{
	return;
}

/**************************/
static void _mp_set_as_view_moved_recommended_time(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	MP_CHECK(view->player);

	double ratio = 0;
	Evas_Coord px = 0, py = 0, pw = 0, ph = 0;
	Evas_Coord rx = 0, ry = 0, rw = 0, rh = 0;
	Evas_Object *genlist = view->content;
	Evas_Object *layout = elm_object_item_part_content_get(elm_genlist_nth_item_get(genlist, MP_SET_AS_RECOMMEND_PRE_LISTEN), "elm.icon");
	Evas_Object *recommended_layout = elm_object_part_content_get(layout, "recommended_text_play");
	Evas_Object *recommended_txt = elm_object_part_content_get(recommended_layout, "progressbar_playing");
	Evas_Object *progressbar = elm_object_part_content_get(layout, "progress_bar");

	char *recommended_time = _mp_set_as_view_time_to_string(view->position);
	mp_util_domain_translatable_text_set(recommended_txt, recommended_time);
	IF_FREE(recommended_time);

	if (progressbar == NULL) {
		DEBUG_TRACE("recommended_txt is NULL");
	} else {
		evas_object_geometry_get(progressbar, &px, &py, &pw, &ph);
	}
	if ((double)view->duration != 0) {
		ratio = (double)view->position / (double)view->duration;
	}
	int position = pw * ratio + px;
	if (recommended_txt == NULL) {
		DEBUG_TRACE("recommended_txt is NULL");
	} else {

		evas_object_geometry_get(recommended_txt, &rx, &ry, &rw, &rh);

		if (pw != 0) {
			edje_object_part_drag_value_set(elm_layout_edje_get(recommended_layout), "progressbar_playing", (double)(position - rw / 2) / (double)pw, 0.0);
		}
	}
	evas_object_show(recommended_txt);
	mp_evas_object_del(view->progress_popup);
}

static Eina_Bool
_move_idler(void *data)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK_FALSE(view);

	_mp_set_as_view_moved_recommended_time(view);

	view->move_idler = NULL;
	return ECORE_CALLBACK_CANCEL;
}

static Evas_Object *
_mp_set_as_view_gl_contents_get(void *data, Evas_Object * obj, const char *part)
{
	MpSetAsView_t *view = evas_object_data_get(obj, "view");
	MP_CHECK_VAL(view, NULL);

	Evas_Object *content = NULL;
	item_data_s *item_data = (item_data_s *)data;
	MP_CHECK_NULL(item_data);
	const Elm_Genlist_Item_Class *item_class = NULL;
	int index = item_data->index;

	DEBUG_TRACE("index is %d", index);
	/* if edit mode */
	if (!strcmp(part, "elm.icon")) {
		switch (index) {
		case	MP_SET_AS_FROM_START:
		case	MP_SET_AS_RECOMMEND:
		case	MP_SET_AS_PHONE_RINGTONE:
		case	MP_SET_AS_CALLER_RINGTONE:
		case	MP_SET_AS_ALARM_TONE:
			/* swallow checkbox or radio button */
			content = elm_radio_add(obj);
			elm_radio_state_value_set(content, index);
			if (index == MP_SET_AS_FROM_START || index == MP_SET_AS_RECOMMEND) {
				elm_radio_group_add(content, g_radio_recommend);
			} else if (index >= MP_SET_AS_PHONE_RINGTONE && index <= MP_SET_AS_ALARM_TONE) {
				elm_radio_group_add(content, g_radio_set_as_type);
			}

			evas_object_size_hint_weight_set(content, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
			evas_object_size_hint_align_set(content, EVAS_HINT_FILL, EVAS_HINT_FILL);
			evas_object_propagate_events_set(content, TRUE);
			/* evas_object_smart_callback_add(content, "changed", _radio_cb, view); */
			break;
		case	MP_SET_AS_RECOMMEND_PRE_LISTEN:
			item_class = elm_genlist_item_item_class_get(item_data->item);
			if (!g_strcmp0(item_class->item_style, "music/1icon/set_as_full")) {
				if (!mp_util_is_landscape()) {
					content = mp_common_load_edj(obj, MP_EDJ_NAME, "music/set_as/prelisten");
				} else {
					content = mp_common_load_edj(obj, MP_EDJ_NAME, "music/set_as/prelisten_ld");
				}
				mp_util_domain_translatable_part_text_set(content, "recommended_text", STR_MP_SET_AS_RECOMMENDED_TXT);
				Evas_Object *progressbar = elm_progressbar_add(content);
				elm_object_style_set(progressbar, "list_progress");
				elm_progressbar_inverted_set(progressbar, EINA_TRUE);
				elm_progressbar_horizontal_set(progressbar, EINA_TRUE);
				evas_object_size_hint_align_set(progressbar, EVAS_HINT_FILL, EVAS_HINT_FILL);
				evas_object_size_hint_weight_set(progressbar, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
				elm_progressbar_value_set(progressbar, (double)(1.0 - (double)view->position / (double)view->duration));
				evas_object_show(progressbar);
				elm_object_part_content_set(content, "progress_bar", progressbar);

				Evas_Object *recommended_layout = mp_common_load_edj(content, MP_EDJ_NAME, "movable_text");
				elm_object_part_content_set(content, "recommended_text_play", recommended_layout);
				Evas_Object *recommended_txt = elm_label_add(recommended_layout);
				elm_object_part_content_set(recommended_layout, "progressbar_playing", recommended_txt);
				if (view->position != -1) {
					char *recommended_time = _mp_set_as_view_time_to_string(view->position);
					mp_util_domain_translatable_text_set(recommended_txt, recommended_time);
					IF_FREE(recommended_time);

					if (view->move_idler == NULL) {
						view->move_idler = ecore_idler_add(_move_idler, view);
					}
				} else {
					evas_object_hide(recommended_layout);
				}

				/*set duration*/
				char *total_time = _mp_set_as_view_time_to_string(view->duration);
				mp_util_domain_translatable_part_text_set(content, "progress_text_total", total_time);
				IF_FREE(total_time);
			} else if (!g_strcmp0(item_class->item_style, "music/1icon/set_as_text")) {
				content = mp_common_load_edj(obj, MP_EDJ_NAME, "music/set_as/prelisten_text");
				mp_util_domain_translatable_part_text_set(content, "recommended_text", STR_MP_SET_AS_RECOMMENDED_TXT);
			}
			break;
		}
	}

	return content;
}


static char *
_mp_set_as_view_gl_label_get(void *data, Evas_Object * obj, const char *part)
{
	item_data_s *item_data = (item_data_s *)data;
	MP_CHECK_NULL(item_data);
	int index = item_data->index;

	char *txt = NULL;

	if (strcmp(part, "elm.text") == 0) {
		switch (index) {
		case MP_SET_AS_FROM_START:
			txt = STR_MP_SET_AS_FROM_BEGIN;
			break;
		case MP_SET_AS_RECOMMEND:
			txt = STR_MP_SET_AS_AUTO_RECOMMEND;
			break;
		case MP_SET_AS_RECOMMEND_PRE_LISTEN:
			txt = STR_MP_SET_AS_RECOMMENDED_TXT;
			break;
		case MP_SET_AS_TITLE:
			txt = STR_MP_SET_AS;
			break;
		case MP_SET_AS_PHONE_RINGTONE:
			txt = STR_MP_SET_AS_PHONE_RINGTONG;
			break;
		case MP_SET_AS_CALLER_RINGTONE:
			txt = STR_MP_SET_AS_CALLER_RINGTONG;
			break;
		case MP_SET_AS_ALARM_TONE:
			txt = STR_MP_SET_AS_ALARM_TONE;
			break;
		default:
			break;
		}
	}
	return g_strdup(GET_STR(txt));
}

static void _mp_set_as_view_enable_done_btn(MpSetAsView_t *view)
{
	MP_CHECK(view);

	Evas_Object *left_btn = elm_object_item_part_content_get(view->navi_it, "title_right_btn");
	elm_object_disabled_set(left_btn, false);
}

static void _gl_sel(void *data, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpSetAsView_t *view = evas_object_data_get(obj, "view");
	MP_CHECK(view);

	item_data_s *item_data = (item_data_s *)data;
	MP_CHECK(item_data);
	int index = item_data->index;
	int old_index = -1;

	elm_genlist_item_selected_set(event_info, EINA_FALSE);

	Evas_Object *radio_group = NULL;
	if (index == MP_SET_AS_FROM_START || index == MP_SET_AS_RECOMMEND) {
		radio_group = g_radio_recommend;
		old_index = view->recommended;
		view->recommended = index;

		/*pause the main player*/
		struct appdata *ad = mp_util_get_appdata();
		MP_CHECK(ad);
		if (ad->player_state == PLAY_STATE_PLAYING) {
			view->need_to_resume = TRUE;
			mp_play_control_play_pause(ad, FALSE);
		}

	} else if (index >= MP_SET_AS_PHONE_RINGTONE && index <= MP_SET_AS_ALARM_TONE) {
		radio_group = g_radio_set_as_type;
		old_index = view->set_as_type;
		view->set_as_type = index;
	}

	if (view->set_as_type != -1 && view->recommended != -1) { /*enable left_button*/
		_mp_set_as_view_enable_done_btn(view);
		view->button_enable = TRUE;
	}

	elm_radio_value_set(radio_group, index);
	evas_object_smart_callback_call(radio_group, "changed", NULL);

	if (!mp_media_info_uri_is_exist_in_db(view->path)) {
		/*the popup followed S5*/
		if (index == MP_SET_AS_FROM_START) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_PLAYER_UNSUPPORT));
		} else if (index == MP_SET_AS_RECOMMEND) {
			mp_widget_text_popup(NULL, GET_STR(STR_MP_RECOMMENDATION_UNSUPPORT));
		}
		return ;
	}

	player_state_e state = _mp_set_as_view_prelisten_get_state(view);
	if (old_index == index) {
		DEBUG_TRACE("-------->same item clicked");
		if (index == MP_SET_AS_FROM_START || index == MP_SET_AS_RECOMMEND) {
			if (state == PLAYER_STATE_PLAYING) {
				_mp_set_as_view_prelisten_pause(view);
				if (index == MP_SET_AS_FROM_START) {
					_mp_set_as_view_prelisten_set_position(view, NULL, NULL, false);
				}
				if (index == MP_SET_AS_RECOMMEND) {
					_mp_set_as_view_prelisten_set_position(view, NULL, NULL, true);
				}
			} else if (state == PLAYER_STATE_PAUSED) {
				_mp_set_as_view_prelisten_start(view);
			}
		}
	} else {
		if (old_index == MP_SET_AS_RECOMMEND) {
			/*get item */
			Elm_Object_Item *recommended_item = elm_genlist_nth_item_get(obj, MP_SET_AS_RECOMMEND_PRE_LISTEN);
			/*update item class to text*/
			elm_genlist_item_item_class_update(recommended_item, view->recommend_itc_text);
			elm_genlist_item_update(recommended_item);

			/* view->position = 0; */
		} else {
			if (index == MP_SET_AS_RECOMMEND) {
				/*get item */
				Elm_Object_Item *recommended_item = elm_genlist_nth_item_get(obj, MP_SET_AS_RECOMMEND_PRE_LISTEN);
				/*update item class to full*/
				elm_genlist_item_item_class_update(recommended_item, view->recommend_itc_full);
				elm_genlist_item_update(recommended_item);

				/*get smat
				SMAT_INFO infos;
				memset(&infos, 0, sizeof(SMAT_INFO));
				if (view->position == -1)
					_mp_set_as_view_get_recommend(view, &infos);*/
			}
		}

		if (index == MP_SET_AS_FROM_START) {
			_mp_set_as_view_prelisten_set_position(view, __mp_set_as_view_player_seek_completed_cb, view, false);
		}

		if (index == MP_SET_AS_RECOMMEND && view->position != -1) {
			_mp_set_as_view_prelisten_set_position(view, __mp_set_as_view_player_seek_completed_cb, view, true);
		}
	}
	return;

}

static void
_mp_set_as_list_item_del(void *data, Evas_Object * obj)
{
	startfunc;
	item_data_s *item_data = data;
	MP_CHECK(item_data);
	free(item_data);
}

static void
_mp_set_as_view_load_genlist_itc(MpSetAsView_t *view)
{
	startfunc;
	mp_retm_if(!view, "INVALID param");

	if (view->radio_itc == NULL) {
		view->radio_itc = elm_genlist_item_class_new();
		MP_CHECK(view->radio_itc);
		view->radio_itc->item_style = "1text.1icon.3";
		view->radio_itc->func.text_get = _mp_set_as_view_gl_label_get;
		view->radio_itc->func.content_get = _mp_set_as_view_gl_contents_get;
		view->radio_itc->func.del = _mp_set_as_list_item_del;
	}

	if (view->title_itc == NULL) {
		view->title_itc = elm_genlist_item_class_new();
		MP_CHECK(view->title_itc);
		view->title_itc->func.text_get = _mp_set_as_view_gl_label_get;
		view->title_itc->func.content_get = NULL;
		view->title_itc->item_style = "1text";
		view->title_itc->func.del = _mp_set_as_list_item_del;
	}

	if (view->recommend_itc_full == NULL) {
		view->recommend_itc_full = elm_genlist_item_class_new();
		MP_CHECK(view->recommend_itc_full);
		view->recommend_itc_full->item_style = "music/1icon/set_as_full";
		view->recommend_itc_full->func.content_get = _mp_set_as_view_gl_contents_get;
		view->recommend_itc_full->func.del = _mp_set_as_list_item_del;
	}
	if (view->recommend_itc_text == NULL) {
		view->recommend_itc_text = elm_genlist_item_class_new();
		MP_CHECK(view->recommend_itc_text);
		view->recommend_itc_text->item_style = "music/1icon/set_as_text";
		view->recommend_itc_text->func.content_get = _mp_set_as_view_gl_contents_get;
		view->recommend_itc_text->func.del = _mp_set_as_list_item_del;
	}
}


static void
_mp_set_as_view_append_genlist_items(Evas_Object *genlist, MpSetAsView_t *view)
{
	startfunc;
	int i;
	Elm_Genlist_Item_Class *itc = NULL;
	/*Elm_Object_Item *item;*/
	Elm_Genlist_Item_Type flag = ELM_GENLIST_ITEM_NONE;
	mp_retm_if(!view, "INVALID param");

	_mp_set_as_view_load_genlist_itc(view);

	for (i = MP_SET_AS_FROM_START; i < MP_SET_AS_MAX; i++) {
		switch (i) {
		case MP_SET_AS_FROM_START:
		case MP_SET_AS_RECOMMEND:
		case MP_SET_AS_PHONE_RINGTONE:
		case MP_SET_AS_CALLER_RINGTONE:
		case MP_SET_AS_ALARM_TONE:
			itc = view->radio_itc;
			break;
		case MP_SET_AS_TITLE:
			itc = view->title_itc;
			break;
		case MP_SET_AS_RECOMMEND_PRE_LISTEN:
			itc = view->recommend_itc_text;
			break;
		}

		item_data_s *item_data = calloc(1, sizeof(item_data_s));
		MP_CHECK(item_data);
		item_data->index = i;

		item_data->item = elm_genlist_item_append(genlist, itc, (void *)item_data, NULL, flag, _gl_sel, (void *)item_data);
		if (i == MP_SET_AS_TITLE || i == MP_SET_AS_RECOMMEND_PRE_LISTEN) {
			elm_genlist_item_select_mode_set(item_data->item, ELM_OBJECT_SELECT_MODE_DISPLAY_ONLY);
		}
	}
}

static Evas_Object*
_mp_set_as_view_create_list(MpSetAsView_t *view, Evas_Object *parent)
{
	startfunc;
	MP_CHECK_VAL(view, NULL);

	Evas_Object *genlist = mp_widget_genlist_create(parent);
	elm_scroller_policy_set(genlist, ELM_SCROLLER_POLICY_OFF, ELM_SCROLLER_POLICY_AUTO);
	evas_object_size_hint_weight_set(genlist, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(genlist, EVAS_HINT_FILL, EVAS_HINT_FILL);
	view->content = genlist;

	g_radio_recommend = elm_radio_add(genlist);
	elm_radio_state_value_set(g_radio_recommend, -1);
	elm_radio_value_set(g_radio_recommend, -1);

	g_radio_set_as_type = elm_radio_add(genlist);
	elm_radio_state_value_set(g_radio_set_as_type, -1);
	elm_radio_value_set(g_radio_set_as_type, -1);

	_mp_set_as_view_append_genlist_items(genlist, view);

	evas_object_show(genlist);
	evas_object_data_set(genlist, "view", view);
	return genlist;
}

static Eina_Bool
_mp_set_as_view_pop_cb(void *data, Elm_Object_Item *it)
{
	startfunc;

	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK_VAL(view, EINA_TRUE);
	mp_view_mgr_pop_view(GET_VIEW_MGR, true);

	endfunc;
	return EINA_TRUE;
}

static int
_mp_set_as_view_update(void *thiz)
{
	startfunc;
	MpSetAsView_t *view = thiz;
	MP_CHECK_VAL(view, -1);
	MP_CHECK_VAL(view->navi_it, -1);


	endfunc;
	return 0;
}

static int
_mp_set_as_view_set_caller_rington(char *path)
{
	int ret = -1;
	ret = system_settings_set_value_string(SYSTEM_SETTINGS_KEY_INCOMING_CALL_RINGTONE, path);
	if (ret != SYSTEM_SETTINGS_ERROR_NONE) {
		mp_error("system_settings_set_value_string()... [0x%x]", ret);
		return -1;
	}
	return ret;

}

static void _done_button_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);
	/*1. get selected item
	1.1 get from start or recommended */
	int recommend = elm_radio_value_get(g_radio_recommend);
	/*1.2 get what to set as*/
	int type = elm_radio_value_get(g_radio_set_as_type);
	int position = 0;

	DEBUG_TRACE("Auto recommand %d, pos: %d", recommend, view->position);
	DEBUG_TRACE("type %d", type);

	if (recommend == 1) {
		position = view->position;
	}

	int ret = 0;
	char *path = NULL;	/* do not free */
	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);
	path = view->path;
	MP_CHECK(path);
	if (!mp_media_info_uri_is_exist_in_db(view->path)) {
		/*delete set as view*/
		MpViewMgr_t *view_mgr = GET_VIEW_MGR;
		mp_view_mgr_delete_view(view_mgr, MP_VIEW_SET_AS);
		return ;
	}


	switch (type) {
	case	MP_SET_AS_PHONE_RINGTONE: {
		char *popup_txt = NULL;
		ret = _mp_set_as_view_set_caller_rington(path);

		if (!ret) {
			popup_txt = GET_SYS_STR("IDS_COM_POP_SUCCESS");
		} else {
			popup_txt = GET_SYS_STR("IDS_COM_POP_FAILED");
		}

		mp_widget_text_popup(ad, popup_txt);
		break;
	}
	case	MP_SET_AS_CALLER_RINGTONE:
		mp_ug_contact_user_sel(path, ad);
		break;
	case	MP_SET_AS_ALARM_TONE:
		mp_ug_set_as_alarm_tone(path, position);
		break;
	default:
		break;
	}

	/*delete set as view*/
	MpViewMgr_t *view_mgr = GET_VIEW_MGR;
	mp_view_mgr_delete_view(view_mgr, MP_VIEW_SET_AS);
}

static void _cancel_button_clicked_cb(void *data, Evas_Object * obj, void *event_info)
{
	MpSetAsView_t *view = (MpSetAsView_t *)data;
	MP_CHECK(view);

	MpViewMgr_t *view_mgr = GET_VIEW_MGR;
	mp_view_mgr_delete_view(view_mgr, MP_VIEW_SET_AS);
}

static int
_mp_set_as_view_update_option_cb(void *thiz)
{
	startfunc;
	MpSetAsView_t *view = thiz;
	MP_CHECK_VAL(view, -1);
	MP_CHECK_VAL(view->navi_it, -1);

	elm_naviframe_item_pop_cb_set(view->navi_it, _mp_set_as_view_pop_cb, view);

	Evas_Object *left_btn = elm_object_item_part_content_unset(view->navi_it, "title_left_btn");
	Evas_Object *right_btn = elm_object_item_part_content_unset(view->navi_it, "title_right_btn");
	mp_evas_object_del(left_btn);
	mp_evas_object_del(right_btn);

	left_btn = mp_create_title_text_btn(view->layout, STR_MP_CANCEL, NULL, NULL);
	elm_object_item_part_content_set(view->navi_it, "title_left_btn", left_btn);
	evas_object_smart_callback_add(left_btn, "clicked",  _cancel_button_clicked_cb, view);

	right_btn = mp_create_title_text_btn(view->layout, STR_MP_DONE, NULL, NULL);
	elm_object_item_part_content_set(view->navi_it, "title_right_btn", right_btn);
	evas_object_smart_callback_add(right_btn, "clicked", _done_button_clicked_cb, view);
	if (!view->button_enable) {
		elm_object_disabled_set(right_btn, true);
	}

	endfunc;
	return 0;
}

static void
_mp_set_as_view_destory_cb(void *thiz)
{
	startfunc;
	MpSetAsView_t *view = thiz;
	MP_CHECK(view);
	mp_set_as_view_destory(view);

	mp_view_fini((MpView_t *)view);

	free(view);
}

static void
_mp_set_as_view_on_event(void *thiz, MpViewEvent_e event)
{
	startfunc;
	MpSetAsView_t *view = thiz;
	DEBUG_TRACE("event is %d", event);
	switch (event) {
		/* when the track rename/move/delete should destroy the player */
	case MP_DB_UPDATED:
		if (!mp_media_info_uri_is_exist_in_db(view->path)) {
			player_state_e state = _mp_set_as_view_prelisten_get_state(view);
			if (state <= PLAYER_STATE_PAUSED && state >= PLAYER_STATE_READY) {
				_mp_set_as_view_prelisten_stop(view);
				_mp_set_as_view_destroy_player(view);
			}
		}
		break;
	default:
		break;
	}
	MP_CHECK(view);

}

static void _mp_set_as_view_resume(void *thiz)
{
	startfunc;
	MpSetAsView_t *view = (MpSetAsView_t *)thiz;
	_mp_set_as_view_update((void *)view);
}

#ifdef MP_FEATURE_LANDSCAPE
static void
_mp_set_as_view_rotate_cb(void *thiz, int randscape)
{
	startfunc;
	MpSetAsView_t *view = (MpSetAsView_t *)thiz;
	MP_CHECK(view);

	elm_genlist_realized_items_update(view->content);
	/*_mp_set_as_view_moved_recommended_time(view);
	add idler to move text */
	if (view->move_idler == NULL) {
		view->move_idler = ecore_idler_add(_move_idler, view);
	}
}
#endif

static int
_mp_set_as_view_init(Evas_Object *parent, MpSetAsView_t *view, void *data)
{
	startfunc;
	int ret = 0;

	ret =  mp_view_init(parent, (MpView_t *)view, MP_VIEW_SET_AS);
	MP_CHECK_VAL(ret == 0, -1);

	view->update = _mp_set_as_view_update;
	view->update_options = _mp_set_as_view_update_option_cb;
	view->update_options_edit = NULL;
	view->view_destroy_cb = _mp_set_as_view_destory_cb;
	view->set_nowplaying = NULL;
	view->unset_nowplaying = NULL;
	view->update_nowplaying = NULL;
	view->start_playback = NULL;
	view->pause_playback = NULL;
	view->on_event = _mp_set_as_view_on_event;
#ifdef MP_FEATURE_LANDSCAPE
	view->rotate = _mp_set_as_view_rotate_cb;
#endif
	view->view_resume = _mp_set_as_view_resume;

	view->content = _mp_set_as_view_create_list(view, parent);

	MP_CHECK_VAL(view->content, -1);
	elm_object_part_content_set(view->layout, "list_content", view->content);

	view->path = (char *)data;
	_mp_set_as_view_create_player(view);
	_mp_set_as_view_prelisten_set_uri(view);
	_mp_set_as_view_prelisten_prepare(view);

	view->duration = _mp_set_as_view_get_duration(view);

	view->set_as_type = -1;
	view->recommended = -1;
	view->position = -1;

	return ret;
}

/*param void *data is used to update previous view..
	change mp_media_info_h to path,becacuse path can deal with exception(rename...)*/
EXPORT_API MpSetAsView_t *mp_set_as_view_create(Evas_Object *parent, char* path)
{
	startfunc;
	int ret;
	MP_CHECK_NULL(parent);

	MpSetAsView_t *view = calloc(1, sizeof(MpSetAsView_t));
	MP_CHECK_NULL(view);

	ret = _mp_set_as_view_init(parent, view, (void *)path);
	if (ret) {
		goto Error;
	}

	return view;

Error:
	ERROR_TRACE("Error: mp_set_as_view_create()");
	IF_FREE(view);
	return NULL;
}

int mp_set_as_view_destory(MpSetAsView_t *view)
{
	startfunc;
	MP_CHECK_VAL(view, -1);

	if (view->radio_itc) {
		elm_genlist_item_class_free(view->radio_itc);
	}
	if (view->recommend_itc_full) {
		elm_genlist_item_class_free(view->recommend_itc_full);
	}
	if (view->recommend_itc_text) {
		elm_genlist_item_class_free(view->recommend_itc_text);
	}
	if (view->title_itc) {
		elm_genlist_item_class_free(view->title_itc);
	}


	player_state_e state = _mp_set_as_view_prelisten_get_state(view);
	if (state <= PLAYER_STATE_PAUSED && state >= PLAYER_STATE_READY) { /*player is active */
		_mp_set_as_view_prelisten_stop(view);
		_mp_set_as_view_destroy_player(view);
	}

	if (view->need_to_resume == TRUE) {
		struct appdata *ad = mp_util_get_appdata();
		mp_play_control_play_pause(ad, TRUE);
	}

	if (view->smat_pipe) {
		ecore_pipe_del(view->smat_pipe);
		view->smat_pipe = NULL;
	}

	return 0;
}



