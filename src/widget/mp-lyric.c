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
#include "mp-lyric.h"
#include "mp-lyric-mgr.h"
#include "mp-common.h"
#include "mp-player-mgr.h"
#include "mp-player-view.h"
#include "mp-widget.h"
#include "mp-util.h"
#include <metadata_extractor.h>

typedef struct {
	bool b_drag;
	int win_w;
	int win_h;
	mp_lyric_mgr_t *lyric_mgr;
	Evas_Object *layout;
	Evas_Object *scroller;
	Evas_Object *box;
	Evas_Object *cur_line;
	Evas_Object *prev_line;
	int prev_line_index;
	int cur_line_index;
	char *path;
}MpLyricData_t;

#define MP_LRC_STR_NO_LYRIC ("IDS_MUSIC_SK3_NO_LYRICS")
#define MP_LRC_FOLDER_PATH "/opt/usr/media/Sounds/Lyric/"
#define MP_LRC_SIFFIX_NAME ".lrc"
#define MP_LRC_FILE_PATH_LEN (int)255 /* The max length of file path */

#define MP_LRC_LINE_COLOR_DEFAULT "#FFFFFF" /* 62:62:62 -> 707070*/
#define MP_LRC_LINE_COLOR_PLAYING "#277EE6" /* 0:0:0 -> #3b7352*/
#define MP_LRC_FONT_CURRENT_SIZE (int)30 /* The size of current playing line */
#define MP_LRC_FONT_NORMAL_SIZE (int)30 /* The size of normal line */

#define HD_MAIN_W 720
#define HD_MAIN_H 1280

/* Portrait mode */
#define MP_LRC_VIEW_H_SCALE 720/HD_MAIN_H /* The hight of lyric view */
#define MP_LRC_VIEW_W_SCALE 608/HD_MAIN_W /* The wigth of lyric view */

/* Landscape mode */
#define MP_LRC_VIEW_H_SCALE_LS 104/HD_MAIN_W
#define MP_LRC_VIEW_W_SCALE_LS 400/HD_MAIN_W

#define MP_LRC_HEAD_H_SCALE 15/HD_MAIN_H
#define MP_LRC_HEAD_H_SCALE_LS 10/HD_MAIN_W

static void
_mp_lyric_view_drag_start(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpLyricData_t *wd = data;
	MP_CHECK(wd);

	wd->b_drag = true;
}

static void
_mp_lyric_view_drag_stop(void *data, Evas *e, Evas_Object *obj, void *event_info)
{
	startfunc;
	MpLyricData_t *wd = data;
	MP_CHECK(wd);

	wd->b_drag = false;
}

static void
_mp_lyric_view_layout_del_cb(void *data, Evas * e, Evas_Object * obj, void *event_info)
{
	startfunc;
	MpLyricData_t *wd = data;
	MP_CHECK(wd );
	mp_lyric_mgr_destory(wd->lyric_mgr);
	IF_FREE(wd->path);
	IF_FREE(wd);
}

static Evas_Object *
_create_lyric(Evas_Object *parent, MpLyricData_t *wd)
{
	startfunc;
	Evas_Object *layout = NULL;

	/* Create layout */
	layout = mp_common_load_edj(parent, PLAY_VIEW_EDJ_NAME, "mp_lrc_layout");
	MP_CHECK_NULL(layout);
	evas_object_event_callback_add(layout, EVAS_CALLBACK_FREE, _mp_lyric_view_layout_del_cb,
				       wd);
	wd->layout = layout;

	/* Create scroller */
	Evas_Object *scroller = elm_scroller_add(layout);
	elm_scroller_bounce_set(scroller, EINA_FALSE, EINA_TRUE);
	elm_scroller_policy_set(scroller,ELM_SCROLLER_POLICY_OFF,ELM_SCROLLER_POLICY_AUTO);
	evas_object_show(scroller);
	wd->scroller = scroller;

	/* Create box */
	Evas_Object *box = elm_box_add(scroller);
	evas_object_show(box);
	evas_object_size_hint_weight_set(box, EVAS_HINT_EXPAND, EVAS_HINT_EXPAND);
	evas_object_size_hint_align_set(box, EVAS_HINT_FILL, EVAS_HINT_FILL);
	wd->box = box;

	/* Set layout content */
	elm_object_content_set(scroller, box);
	elm_object_part_content_set(layout, "lyric", scroller);

	/* Set event callback */
	evas_object_event_callback_add(scroller, EVAS_CALLBACK_MOUSE_DOWN, _mp_lyric_view_drag_start, wd);
	evas_object_event_callback_add(scroller, EVAS_CALLBACK_MOUSE_UP, _mp_lyric_view_drag_stop, wd);

	elm_win_screen_size_get(parent, NULL, NULL, &wd->win_w, &wd->win_h);
	evas_object_data_set(layout, "wd", wd);

	return layout;
}

static void
_mp_lyric_view_load_lyric(MpLyricData_t *wd)
{
	startfunc;
	MP_CHECK(wd);
	MP_CHECK(wd->box);

	MP_CHECK(wd->lyric_mgr);

	/* Create lyric line */
	int index = 0;
	Eina_List *list = wd->lyric_mgr->synclrc_list;
	int count = eina_list_count(list);

	if (count <= 0)
	{
		list = wd->lyric_mgr->unsynclrc_list;
		count = eina_list_count(list);
	}

	elm_box_clear(wd->box);
	for (index = 0; index < count; index++)
	{
		mp_lrc_node_t *lrc_node = (mp_lrc_node_t*)eina_list_nth(list, index);
		MP_CHECK(lrc_node);
		Evas_Object* label = elm_label_add(wd->box);
		MP_CHECK(label);
		evas_object_size_hint_fill_set(label, EVAS_HINT_FILL, 0.5);
		elm_label_line_wrap_set(label, ELM_WRAP_MIXED);
		char *label_text = g_strdup_printf("<font_size=30><color=#FFFFFF><align=center>%s</align></color></font_size>", lrc_node->lyric);
		elm_object_text_set(label, label_text);
		IF_FREE(label_text);
		evas_object_show(label);
		evas_object_data_set(label, "time", &lrc_node->time);
		elm_box_pack_end(wd->box, label);
	}

	/* Add gap for head and tail */
	Evas_Object* head = mp_common_load_edj(wd->box, PLAY_VIEW_EDJ_NAME, "mp_lrc_head");
	elm_box_pack_start(wd->box, head);
}

static void
_mp_lyric_set_line_color(MpLyricData_t *wd, Evas_Object *obj, int index, const char *color)
{
	MP_CHECK(obj);
	MP_CHECK(color);
	MP_CHECK(index >= 0);
	MP_CHECK(wd->lyric_mgr);

	Eina_List *list = wd->lyric_mgr->synclrc_list;
	MP_CHECK(list);
	int count = eina_list_count(list);

	if (count <= 0)
		list = wd->lyric_mgr->unsynclrc_list;

	mp_lrc_node_t *lrc_node = (mp_lrc_node_t*)eina_list_nth(list, index);
	MP_CHECK(lrc_node);
	char *text_old = lrc_node->lyric;
	MP_CHECK(text_old);

	char *text_new = (char*)malloc(sizeof(char)*(strlen(text_old)+50));
	MP_CHECK(text_new);
	char * text = "<color=";
	strncpy(text_new, text, strlen(text));
	strncat(text_new, color ,strlen(color));
	text = ">";
	strncat(text_new, text, strlen(text));
	strncat(text_new, text_old ,strlen(text_old));
	text = "</color>";
	strncat(text_new, text, strlen(text));

	edje_object_part_text_set(_EDJ(obj), "txt", text_new);

	free(text_new);

}

static Eina_List*
_mp_lyric_get_line_list(MpLyricData_t *wd)
{
	MP_CHECK_NULL(wd);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK_NULL(ad);

	Eina_List *list = NULL;
	MpLyricData_t *lyric = wd;

	Evas_Object *scroller =  lyric->scroller;
	Evas_Object *content = elm_object_content_get(scroller);
	list = elm_box_children_get(content);
	return list;
}

static Evas_Object*
_mp_lyric_get_line(MpLyricData_t *wd, int index)
{
	MP_CHECK_NULL(index >= 0);
	Eina_List *list = _mp_lyric_get_line_list(wd);
	Evas_Object *line = NULL;
	if (list) {
		line = eina_list_nth(list, index+1);

		eina_list_free(list);
		list = NULL;
	}
	return line;
}

static void
_mp_lyric_set_current_line(MpLyricData_t *wd, int index)
{
	MP_CHECK(index >= 0);
	MP_CHECK(wd);

	struct appdata *ad = mp_util_get_appdata();
	MP_CHECK(ad);

	Evas_Object *line = _mp_lyric_get_line(wd, index);
	MP_CHECK(line);

	/* No change */
	if (wd->cur_line == line)
		return;

	/* Update current line */
	wd->prev_line = wd->cur_line;
	wd->cur_line = line;

	wd->prev_line_index = wd->cur_line_index;
	wd->cur_line_index = index;

	int x0, y0, w0, h0;
	Evas_Object *head = _mp_lyric_get_line(wd, 0);

	evas_object_geometry_get(head, &x0, &y0, &w0, &h0);
	int head_gap = y0 + h0;

	//DEBUG_TRACE("===Get current line====i =%d, x=%d, y=%d, w=%d, h=%d\n",index, x0, y0, w0, h0);
	int x, y, w, h;
	evas_object_geometry_get(wd->cur_line, &x, &y, &w, &h);
	//DEBUG_TRACE("===Get current line====i =%d, x=%d, y=%d, w=%d, h=%d\n",index, x, y, w, h);

	int new_y = 0;
	if (ad->screen_mode == MP_SCREEN_MODE_LANDSCAPE)
	{
		int scale_h_ls = wd->win_w*MP_LRC_VIEW_H_SCALE_LS;
		int scale_w_ls = wd->win_h*MP_LRC_VIEW_W_SCALE_LS;
		int scale_head_h_ls = wd->win_w*MP_LRC_HEAD_H_SCALE_LS;

		if ((y-y0) > (scale_h_ls/2 - scale_head_h_ls))
		{
			new_y = y-head_gap+(h)-(scale_h_ls/2 - scale_head_h_ls);
		}
		else
		{
			new_y = y-head_gap+(h/2)-(y-y0);
		}

		elm_scroller_region_bring_in(wd->scroller, x, new_y, scale_w_ls, scale_h_ls);
	}
	else
	{
		int scale_h = wd->win_h*MP_LRC_VIEW_H_SCALE;
		int scale_w = wd->win_w*MP_LRC_VIEW_W_SCALE;
		int scale_head_h = wd->win_h*MP_LRC_HEAD_H_SCALE;

		if ((y-y0) > (scale_h/2 - scale_head_h))
		{
			new_y = y-head_gap+(h)-(scale_h/2 - scale_head_h);
		}
		else
		{
			new_y = y-head_gap+(h/2)-(y-y0);
		}

		elm_scroller_region_bring_in(wd->scroller, x, new_y, scale_w, scale_h);
	}

	_mp_lyric_set_line_color(wd, wd->cur_line, wd->cur_line_index, MP_LRC_LINE_COLOR_PLAYING);

	if (wd->prev_line != NULL)
	{
		_mp_lyric_set_line_color(wd, wd->prev_line, wd->prev_line_index, MP_LRC_LINE_COLOR_DEFAULT);
	}

	evas_object_show(wd->scroller);

}

Evas_Object *mp_lyric_create(Evas_Object *parent, const char *path)
{
	startfunc;
	MP_CHECK_NULL(parent);
	MP_CHECK_NULL(path);

	Evas_Object *lyric_layout = NULL;
	MpLyricData_t *wd = NULL;

	int sync_num = 0;
	DEBUG_TRACE("sync_num=%d", sync_num);

	mp_lyric_mgr_t *lyric_mgr = mp_lyric_mgr_create(path);
	MP_CHECK_NULL(lyric_mgr);

	wd = calloc(1, sizeof(MpLyricData_t));
	if (!wd) goto ERROR;

	lyric_layout = _create_lyric(parent, wd);
	if (!lyric_layout) goto ERROR;

	wd->lyric_mgr = lyric_mgr;

	_mp_lyric_view_load_lyric(wd);

	wd->path = strdup(path);

	return lyric_layout;

	ERROR:
	ERROR_TRACE("Unable to create lyric");
	IF_FREE(wd);

	mp_lyric_mgr_destory(lyric_mgr);
	return NULL;
}

void mp_lyric_sync_update(Evas_Object *lyric)
{
	MP_CHECK(lyric);
	MpLyricData_t *wd =  evas_object_data_get(lyric, "wd");
	MP_CHECK(wd);

	if (wd->b_drag)
		return;

	if (wd->lyric_mgr)
	{
		Eina_List *list = wd->lyric_mgr->synclrc_list;
		if (!list) return;

		int count = eina_list_count(list);
		int pos = mp_player_mgr_get_position();
		int index = 0;
		//DEBUG_TRACE("pos=%d, count=%d", pos, count);
		/* Get the current playing line */
		for (index = 0; index < count-1; index++)
		{
			mp_lrc_node_t *node1 = (mp_lrc_node_t*)eina_list_nth(list, index);
			//mp_lrc_node_t *node2 = (mp_lrc_node_t*)eina_list_nth(list, index+1);
			MP_CHECK(node1);
			//MP_CHECK(node2);
			if ((pos <= node1->time))
			{
				//DEBUG_TRACE("node1->time=%ld", node1->time);
				_mp_lyric_set_current_line(wd, index);
				break;
			}
		}
	}

}

const char *mp_lyric_get_path(Evas_Object *lyric)
{
	MP_CHECK_NULL(lyric);
	MpLyricData_t *wd =  evas_object_data_get(lyric, "wd");
	MP_CHECK_NULL(wd);

	return wd->path;

}

