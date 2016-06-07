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

#ifndef __MUSIC_CHOOSER_H__
#define __MUSIC_CHOOSER_H__

#include <Elementary.h>
#include <glib.h>
#include <libintl.h>
#include <app_control.h>
#include <app.h>
#include <notification.h>

#include "mc-debug.h"
#include "mp-common-defs.h"
#include "mc-text.h"
#include "mc-widget.h"
#include "mc-common.h"
#include "mp-images.h"
#include "mp-resource.h"
#include "mp-media-info.h"

#define ICON_SIZE 64*elm_config_scale_get()

#define LOCALE_DIR  "locale"
#define IMAGE_EDJ_NAME "mp-images.edj"
#define SYS_DOMAIN_NAME "sys_string"
#define MC_EDJ_FILE		"music-chooser.edj"

#define _EDJ(o)			elm_layout_edje_get(o)
#define GET_STR(s)			dgettext(DOMAIN_NAME, s)
#define dgettext_noop(s)	(s)
#define N_(s)			dgettext_noop(s)
#define GET_SYS_STR(str) dgettext(DOMAIN_NAME, str)

#define IF_FREE(p) ({if(p){free(p);p=NULL;}})

#define MC_SELECT_MODE_KEY	"http://tizen.org/appcontrol/data/selection_mode"
#define MC_SELECT_MULTIPLE	"multiple"

typedef enum{
	MC_SELECT_SINGLE,
	MC_SELECT_SINGLE_RINGTONE,
	MC_SELECT_MULTI,
	MC_SELECT_VOICE_CLIP,
	MC_SELECT_GROUP_PLAY,
	MC_AUTO_TEMPLATE,
	MC_SHORTCUT_ALBUM,
	MC_SHORTCUT_ARTIST,
	MC_SHORTCUT_PLAYLIST,
}ug_type;

typedef enum{
	MC_TRACK,
	MC_ALBUM,
	MC_ALBUM_TRACK,
	MC_ARTIST,
	MC_ARTIST_TRACK,
	MC_FOLDER,
	MC_FOLDER_TRACK,
}mc_list_type_e;

typedef struct
{
	int index;
	mc_list_type_e item_type;
	Elm_Object_Item *it;
	Eina_Bool checked;
	mp_group_type_e group_type;
	mp_media_info_h handle;
	bool unregister_lang_mgr;
	int artist_album_page;
	void *plst_item;
} mc_list_item_data_t;

struct app_data
{
	Evas_Object *base_layout;
	Evas_Object *navi_bar;
	int max_count;
	notification_h noti;

	char *select_uri;
	ug_type select_type;
	int track_type;

	//main window;
	Evas_Object *win;
	Evas_Object *conformant;

	int win_angle;

	//support light theme
	Elm_Theme * th;

	int auto_temp_max;

	Evas_Object *track_list;

	bool auto_recommended_show;
	Ecore_Pipe *smat_pipe;
	bool auto_recommended_on;
	app_control_h service;
	long long int limitsize;
};

#endif /* __msmc_efl_H__ */
