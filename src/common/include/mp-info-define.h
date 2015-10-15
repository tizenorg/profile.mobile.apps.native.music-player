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


#ifndef __MP_INFO_DEFINE_H_
#define __MP_INFO_DEFINE_H_

#include <Elementary.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <glib.h>
#ifdef GBSBUILD
#include <vconf.h>
#endif
#include <Ecore_IMF.h>
//#include <Ecore_X.h>
#include <app.h>
#include <Edje.h>
#include <errno.h>
#include <libintl.h>
//#include <libsoup/soup.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
//#include <libsoup/soup.h>
#include <sys/times.h>
#include "openssl/aes.h"
#include "mp-define.h"

#include "mp-player-debug.h"

#define STR_DOMAIN_NAME		"music-player"

#define TITLE_H 90
#define START_Y_POSITION	94

#define MP_PHONE_ROOT_PATH        "/opt/usr/media"
#define MP_MMC_ROOT_PATH		"/opt/storage/sdcard"

#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

#define PLAY_TIME_FORMAT "02u:%02u"
#define MUSIC_TIME_FORMAT "02u:%02u:%02u"

#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#undef IF_FREE
#define IF_FREE(ptr) if (ptr) {free(ptr); ptr = NULL;}

#undef SAFE_FREE
#define SAFE_FREE(x)       if(x) {free(x); x = NULL;}

#define mp_evas_object_del(object) do { \
	if(object) { \
		evas_object_del(object); \
		object = NULL; \
	} \
} while (0)

#define mp_elm_genlist_del(list) do { \
	if(list) { \
		elm_genlist_clear(list);\
		evas_object_del(list); \
		list = NULL; \
	} \
} while (0)

#define mp_elm_genlist_item_class_free(itc) do { \
	if(itc) { \
		elm_genlist_item_class_free(itc); \
		itc = NULL; \
	} \
} while (0)

#define mp_ecore_timer_del(timer) do { \
	if(timer) { \
		ecore_timer_del(timer);\
		timer = NULL; \
	} \
} while (0)

#ifdef MUSICDIR
#undef MUSICDIR
#endif
#define MUSICDIR DATA_PREFIX
#define MP_RESDIR 			MUSICDIR"/res"
#ifdef EDJDIR
#undef EDJDIR
#endif
#define EDJDIR 			MP_RESDIR"/edje"
#ifdef RICH_INFO_EDJ
#undef RICH_INFO_EDJ
#endif
#define RICH_INFO_EDJ EDJDIR"/music.edj"
#ifdef EDJ_NAME
#undef EDJ_NAME
#endif

#define INFO_VIEW_VIDEO_PLAY_IMAGE "T02_Video_play.png"

#define MP_ICON_RATING_PD			ICON_DIRECTORY"/music_player/34_rating_0%d.png"

#define MP_STAR_ICON_ON			ICON_DIRECTORY"/music_player/T02_star_on.png"
#define MP_STAR_ICON_OFF			ICON_DIRECTORY"/music_player/T02_star_off.png"


#define MP_UG_INFO_ALBUMART_CLICKED "albumart_clicked"

#define MP_METADATA_LEN_MAX	193

#endif /* __MP_INFO_DEFINE_H_ */
