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


#ifndef __MP_DEFINE_H_
#define __MP_DEFINE_H_

#include <Elementary.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <glib.h>
#include <Ecore_IMF.h>
#include <Edje.h>
#include <errno.h>
#include <libintl.h>
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
#include <sys/times.h>
#include <app.h>

#ifndef EXPORT_API
#define EXPORT_API __attribute__((__visibility__("default")))
#endif

#ifndef MP_IMAGE_ROTATE_FEATURE
//efine MP_IMAGE_ROTATE_FEATURE
#endif

#ifndef bool
#define bool Eina_Bool
#endif

#ifndef GET_SYS_STR
#define GET_SYS_STR(str) (char *)mp_util_get_text(str)
#endif

#define DOMAIN_NAME "music-player"
#define SYS_DOMAIN_NAME "sys_string"
#ifndef LOCALE_DIR
#define LOCALE_DIR LOCALEDIR
#endif
#ifndef GET_STR
#define GET_STR(str) (char *)mp_util_get_text(str)
#endif

#ifndef PACKAGE
#define PACKAGE "music-player"
#endif

#define DATA_DIR	DATA_PREFIX"/data"

#ifndef MP_INI_DIR
#define MP_INI_DIR DATA_DIR
#endif

#define PKGNAME_FOR_SHORTCUT	PKG_NAME

#define EDJ_PATH EDJDIR
#define EDJ_NAME "mp-library.edj"
#define MP_EDJ_NAME "music.edj"
#define MINICON_EDJ_NAME "mp-minicontroller.edj"
#define LOCKSCREENMINI_EDJ_NAME "mp-lockscreenmini.edj"
#define IMAGE_EDJ_NAME "mp-images.edj"
#define GRP_MAIN "main"

#define THEME_NAME "mp-custom-winset-style.edj"

#define TITLE_H 90
#define START_Y_POSITION	94

#define MP_THUMB_DOWNLOAD_TEMP_DIR	DATA_DIR"/.thumb"

#ifdef MP_FEATURE_PERSONAL_PAGE
#define MP_PERSONAL_PAGE_DIR	"/opt/storage/PersonalStorage"
#endif
#define MP_MMC_DIR	"/opt/storage/sdcard"
#define MP_MUSIC_DIR	"/opt/usr/media"
#define MP_HTTP_DIR "http://"

#ifdef PATH_MAX
#	define MAX_NAM_LEN   PATH_MAX
#else
#	define MAX_NAM_LEN   4096
#endif


#ifndef FALSE
#define FALSE  0
#endif
#ifndef TRUE
#define TRUE   1
#endif

#define SIGNAL_MAIN_MUTE_SHOW					"signal.main.mute.show"
#define SIGNAL_MAIN_MUTE_HIDE					"signal.main.mute.hide"

#define CHECK(x) if(!x)	ERROR_TRACE("RETURN NULL!!\n", x);
#define SAFE_FREE(x)       if(x) {free(x); x = NULL;}
#define SAFE_STRDUP(destptr,sourceptr)\
	do{\
		if( destptr != NULL ){\
			free( destptr );\
			destptr	= NULL;\
		}\
		if(sourceptr == NULL)\
			destptr = NULL; \
		else \
			destptr = strdup(sourceptr); \
	}while(0);

#define MAX_STR_LEN				MAX_NAM_LEN
#define MAX_URL_LEN				MAX_NAM_LEN
#define DEF_STR_LEN				512

#define PLAY_VIEW_EDJ_NAME 		"music.edj"

//2 EVAS_OBJECT_TYPE define
#define MP_FAST_SCROLLER_TYPE 					"mp_data_fast_scroller_type"

/* options */
#define SIGNAL_NOW_PLAYING_CLICKED     "now_playing_clicked"
#define SIGNAL_VOLUME					"volume"
#define SIGNAL_SHUFFLE_ON				"shuffle_on"
#define SIGNAL_SHUFFLE_OFF				"shuffle_off"
#define SIGNAL_REP_ALL					"rep_all"
#define SIGNAL_FAVORITE					"options_favorite"
//add favourite begin
#define SIGNAL_FAVOURITE_ON				"favourite_on"
#define SIGNAL_FAVOURITE_OFF				"favourite_off"
//add favourite end
#define SIGNAL_REP_OFF					"rep_A"
#define SIGNAL_REP_1					"rep_1"

#define SIGNAL_LIKE_ON					"like_on"
#define SIGNAL_LIKE_OFF					"like_off"
#define SIGNAL_BLOCK_ON					"block_on"
#define SIGNAL_BLOCK_OFF				"block_off"

#define SIGNAL_INFO					"info"
#define SIGNAL_PAUSE					"control_pause"
#define SIGNAL_PLAY					"control_play"
#define SIGNAL_PREVIOUS					"control_previous_clicked"
#define SIGNAL_NP_PAUSE					"control_pause_clicked"
#define SIGNAL_NP_PLAY					"control_play_clicked"
#define SIGNAL_NEXT					"control_next_clicked"
#define SIGNAL_MOUSE_DOWN				"mouse,down,1"
#define SIGNAL_MOUSE_UP					"mouse,up,1"
#define SIGNAL_MOUSE_CLICK				"mouse,clicked,1"
#define SIGNAL_CONTROL_VOLUME			"control_volume"
#define SIGNAL_CONTROL_VOLUME_MUTE		"control_volume_mute"
#define SIGNAL_CONTROL_QUEUE			"control_playlist_queue"


//2 FEATURE define
#define MP_FEATURE_SUPPORT_ID3_TAG

#define MP_FEATURE_PLAY_SPEED
#undef  MP_ALLSHARE_DISCONNECT_WHEN_MIRRORING

#ifndef MP_SOUND_PLAYER	/* music player only*/
//#define MP_FEATURE_ADD_TO_INCLUDE_PLAYLIST_TAB
//#define MP_FEATURE_ADD_TO_HOME
#define MP_FEATURE_AUTO_OFF
//#define MP_FEATURE_AVRCP_13
//#define MP_FEATURE_SPLIT_WINDOW
#endif //MP_SOUND_PLAYER

#ifdef MP_SOUND_PLAYER
#undef MP_3D_FEATURE
#define MP_FEATURE_EXIT_ON_BACK
#define MP_FEATURE_USB_OTG
#endif
//#define MP_FEATURE_WIFI_SHARE
//#define MP_FEATURE_VOICE_CONTROL
//#define MP_FEATURE_PALM_TOUCH

#ifdef MP_DEBUG_MODE
//#define MP_CREATE_FAKE_IMAGE
#endif

#define MP_FUNC_ALLSHARE_PLAYLIST			"music-player:allshare_"

#define MP_POPUP_YES	1
#define MP_POPUP_NO	0
#define MP_POPUP_TIMEOUT	(2.0)

#define MP_STR_UNKNOWN	"Unknown"
#define MP_YEAR_S		"%03u0s"

#define HIGHLIGHT_COLOR "2A71E7FF"

typedef int SLP_Bool;
typedef void (*MpHttpOpenRspCb) (gpointer user_data);
typedef void (*MpGetShazamSigCb) (char *signature, int size, void *data);

#define TIME_FORMAT_LEN	15

#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#undef IF_FREE
#define IF_FREE(ptr) if (ptr) {free(ptr); ptr = NULL;}

#undef IF_G_FREE
#define IF_G_FREE(p) ({g_free(p);p=NULL;})

#define mp_evas_object_del(object) do { \
	if(object) { \
		evas_object_del(object); \
		object = NULL; \
	} \
} while (0)

#define mp_elm_object_item_del(object) do { \
	if(object) { \
		elm_object_item_del(object); \
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

#define mp_ecore_timer_del(timer) do { \
	if(timer) { \
		ecore_timer_del(timer);\
		timer = NULL; \
	} \
} while (0)

#define mp_ecore_animator_del(animator) do { \
	if(animator) { \
		ecore_animator_del(animator);\
		animator = NULL; \
	} \
} while (0)

#define mp_ecore_idler_del(idler) do { \
	if(idler) { \
		ecore_idler_del(idler);\
		idler = NULL; \
	} \
} while (0)

#define MP_TIMER_FREEZE(timer) do { \
	if(timer) { \
		ecore_timer_freeze(timer);\
	} \
} while (0)

#define MP_TIMER_THAW(timer) do { \
	if(timer) { \
		ecore_timer_thaw(timer);\
	} \
} while (0)


#define SAFE_STRCPY(dest, src) \
	do{if(!dest||!src)break;\
		strncpy (dest , src, sizeof(dest)-1);\
		dest[sizeof(dest)-1] = 0;	}while(0)

#define mp_evas_object_response_set(obj, response) do { \
	if (obj) { \
		evas_object_data_set((obj), "response", (void *)(response)); \
	} \
} while (0)

#define mp_evas_object_response_get(obj) (int)evas_object_data_get((obj), "response")

#define SCALED_SIZE(x)	((x) * elm_config_scale_get())

typedef enum
{
	MP_SCREEN_MODE_PORTRAIT = 0,
	MP_SCREEN_MODE_LANDSCAPE,
} mp_screen_mode;


typedef enum
{
	MP_SND_PATH_SPEAKER = 0x01,
	MP_SND_PATH_EARPHONE= 0x02,
	MP_SND_PATH_BT= 0x04,
	MP_SND_PATH_HDMI= 0x10,
	MP_SND_PATH_MIRRORING= 0x20, //WIFI display
	MP_SND_PATH_USB_AUDIO= 0x40,
	MP_SND_PATH_MAX,
} mp_snd_path;

typedef enum
{
	MP_VIEW_MODE_DEFAULT,
	MP_VIEW_MODE_EDIT,
	MP_VIEW_MODE_SEARCH,
} mp_view_mode_t;

typedef enum
{
	MP_VIEW_TYPE_SONGS,
	MP_VIEW_TYPE_PLAYLIST,
	MP_VIEW_TYPE_ALBUM,
	MP_VIEW_TYPE_GENRE,
	MP_VIEW_TYPE_ARTIST,
	MP_VIEW_TYPE_YEAR,
	MP_VIEW_TYPE_COMPOSER,
	MP_VIEW_TYPE_FOLDER,
	MP_VIEW_TYPE_ALLSHARE,
	MP_VIEW_TYPE_PLAYVIEW,

	MP_VIEW_TYPE_MAX,
} mp_view_type_t;

typedef enum
{
	MP_DONE_DELETE_TYPE = 0x01,
	MP_DONE_REMOVED_TYPE,
	MP_DONE_ADD_TO_TYPE,
	MP_DONE_REORDER_TYPE,
	MP_DONE_ADD_TRACK_TYPE,
	MP_DONE_SELECT_ADD_TRACK_TYPE,
	MP_DONE_MAX,
} mp_done_operator_type_t;

enum
{
	MP_PLAY_STATE_NONE,
	MP_PLAY_STATE_PLAYING,
	MP_PLAY_STATE_PAUSED,
	MP_PLAY_STATE_STOP,
	MP_PLAY_STATE_MAX,
} mp_play_state;

typedef enum
{
	MP_TAB_PLAYLISTS,
	MP_TAB_SONGS,
	MP_TAB_ALBUMS,
	MP_TAB_ARTISTS,
	MP_TAB_MAX,
} MpTab_e;


#define MP_GENLIST_CHECK_FOREACH_SAFE(first, current, next, data) \
	for (current = first,                                      \
		next = elm_genlist_item_next_get(current),                    \
		data = elm_object_item_data_get(current);                  \
		current;                                             \
		current = next,                                    \
		next = elm_genlist_item_next_get(current),                    \
		data = elm_object_item_data_get(current))

#define 	MP_PLAYLIST_MAX_ITEM_COUNT 1000
#define 	MP_NOW_PLAYING_ICON_SIZE 74 * elm_config_scale_get()
#define 	MP_LIST_ICON_SIZE 90 * elm_config_scale_get()
#define 	MP_LIST_ALBUM_ICON_SIZE 120 * elm_config_scale_get()
#define 	MP_ALBUM_LIST_ICON_SIZE 48 * elm_config_scale_get()
#define 	MP_ARTIST_ALBUM_LIST_ICON_SIZE (64 * elm_config_scale_get())
#define 	MP_PLAY_VIEW_ARTWORK_SIZE 480 * elm_config_scale_get()
#define	        MP_LIST_SHUFFLE_ICON_SIZE (36* elm_config_scale_get())

#define 	MP_ARTIST_THUMB_ICON_SIZE	        162
#define 	MP_LANDSCAPE_ARTIST_THUMB_ICON_SIZE	166
#define 	MP_ALBUM_THUMB_ICON_SIZE	        312
#define 	MP_LANDSCAPE_ALBUM_THUMB_ICON_SIZE	230

#define MP_FILE_PREFIX "file://"

#define MP_NOW_PLAYING_ID_INI		DATA_PREFIX"/shared/data/NowPlayingId.ini"	//playing thread id
#ifdef MP_SOUND_PLAYER
#define MP_SHARED_PLAYING_STATUS_INI	DATA_PREFIX"/shared/data/nowplaying.ini"	//share track info of sound-player
#else
#define MP_SHARED_PLAYING_STATUS_INI	DATA_PREFIX"/shared/data/NowPlayingStatus" //share track info of music-player
#endif
//@@ MP_PLAYING_INI_FILE_NAME
//this hard corded value is used for music player & sound player.
//if this is need to be changed, you need to inform about it to lockscreen.
#define MP_PLAYING_INI_FILE_NAME_MUSIC		DATA_PREFIX"/data/playing_track_music.ini"
#define MP_PLAYING_INI_FILE_NAME_SOUND		DATA_PREFIX"/data/playing_track_sound.ini"

/* music play state for livebox and music player and sound player */
#define MP_PLAY_STATE				DATA_PREFIX"/shared/data/MusicPlayStatus.ini"	//player state


#ifndef MP_SOUND_PLAYER
#define MP_LSCR_CONTROL			        DATA_PREFIX"/data/lock_music_ctrl"
#else
#define MP_LSCR_CONTROL				DATA_PREFIX"/data/lock_sound_ctrl"
#endif


#define MP_SHORTCUT_INI_FILE_NAME_0		MP_INI_DIR"/shortcut_0.ini"
#define MP_SHORTCUT_INI_FILE_NAME_1		MP_INI_DIR"/shortcut_1.ini"
#define MP_SHORTCUT_INI_FILE_NAME_2		MP_INI_DIR"/shortcut_2.ini"
#define MP_SHORTCUT_INI_FILE_NAME_3		MP_INI_DIR"/shortcut_3.ini"

#define SINGLE_BYTE_MAX 0x7F

typedef enum
{
	MP_UG_MESSAGE_BACK,
	MP_UG_MESSAGE_DEL,
	MP_UG_MESSAGE_LOAD,
#ifdef MP_FEATURE_INNER_SETTINGS
	MP_UG_MESSAGE_SETTINGS_BACK,
#endif
}mp_ug_message_t;

#define MP_POPUP_TITLE_H 72
#define MP_POPUP_MAX_H	752
#define MP_POPUP_MAX_H_LD 432
#define MP_POPUP_GENLIST_ITEM_H 82
#define MP_POPUP_GENLIST_ITEM_H_MAX_LD (MP_POPUP_MAX_H_LD-MP_POPUP_TITLE_H)
#define MP_POPUP_GENLIST_ITEM_H_MAX     4*MP_POPUP_GENLIST_ITEM_H
#define MP_POPUP_GENLIST_ITEM_W 614

#define ELM_NAVIFRAME_ITEM_CONTENT				"default"
#define ELM_NAVIFRAME_ITEM_ICON					"icon"
#define ELM_NAVIFRAME_ITEM_OPTIONHEADER			"optionheader"
#define ELM_NAVIFRAME_ITEM_TITLE_LABEL			"title"
#define ELM_NAVIFRAME_ITEM_PREV_BTN				"prev_btn"
#define ELM_NAVIFRAME_ITEM_TITLE_LEFT_BTN		"title_left_btn"
#define ELM_NAVIFRAME_ITEM_TITLE_RIGHT_BTN		"title_right_btn"
#define ELM_NAVIFRAME_ITEM_TITLE_MORE_BTN		"title_more_btn"
#define ELM_NAVIFRAME_ITEM_CONTROLBAR			"controlbar"
#define ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_CLOSE		"elm,state,optionheader,close", ""
#define ELM_NAVIFRAME_ITEM_SIGNAL_OPTIONHEADER_OPEN			"elm,state,optionheader,open", ""

#define MP_PLAYLIST_NAME_SIZE			50
#define MP_METADATA_LEN_MAX	193
#define MP_SEARCH_MAX_CHAR_COUNT			50

//#define MP_NAVI_ITEM_STYLE_TOPLINE "music/topline"
#define MP_NAVI_ITEM_STYLE_TOPLINE NULL

#include "mp-player-debug.h"
#include "mp-ta.h"
#include "mp-vconf-private-keys.h"
#include "mp-images.h"
#include "mp-common-defs.h"
#include "mp-resource.h"

#define USER_AGENT_KEY		"db/browser/UserAgent"
#define MP_FEATURE_ALWAYS_ON

#endif /* __MP_DEFINE_H_ */
