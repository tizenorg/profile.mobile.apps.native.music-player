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

#ifndef __MP_COMMON_DEFS_H__
#define __MP_COMMON_DEFS_H__

#define MP_B_PATH "path"
#define MP_MM_KEY "multimedia_key"
#define MP_PLAY_RECENT "play_recent"
#define MP_REQ_TYPE "request_type"
#define MP_HOST_TYPE "host_type"

#define MP_SAMSUNG_LINK_ARTIST "http://tizen.org/appcontrol/data/artist"

// download notification (store)
#define MP_DOWNLOAD_NOTIFICATION	"download_notification"
#define MH_DOWNLOAD_QUEUE			"download_queue"
#define MP_MY_MUSIC					"my_music"

//ug-music-player request type
#define MC_REQ_TYPE_KEY 			MP_REQ_TYPE

#define MC_REQ_SHORT_ALBUM_VAL 		"SC_Album"
#define MC_REQ_SHORT_ARTIST_VAL		"SC_Artist"
#define MC_REQ_SHORT_PLAYLIST_VAL 	"SC_Playlist"
#define MC_REQ_SELECT_SINGLE	"Select"
#define MC_REQ_SELECT_SINGLE_RINGTONE        "SelectRingtone"
#define MC_REQ_SELECT_MULTI		"MultipleSelect"
#define MC_REQ_VOICE_CLIP		"voice_clip"
#define MC_REQ_GROUP_PLAY		"group_play"


#define MP_NOWPLAYING_LIST_INDEX	"NowPlayingListIndex"
#define MP_NOWPLAYING_LIST_URIS	"NowPlayingListURIs"
#define MP_REFRESH_PLAYLIST			"RefreshPlaylist"

//ug-music-player host app type
#define MC_REQ_SHOW_RECOMMENDED_KEY			MP_HOST_TYPE

#define MC_SHOW_VAL 		"show"
#define MC_HIDE_VAL 		"hide"



//1 Do not change default thumbnail path. it's shared by other apps
#define DEFAULT_THUMBNAIL				SHAREDDIR"/res/shared_images/default_album_art_120.png"
#define DEFAULT_THUMBNAIL_MIDDLE		SHAREDDIR"/res/shared_images/default_albumart_middle.png"
#define DEFAULT_THUMBNAIL_SMALL			SHAREDDIR"/res/shared_images/default_albumart_small.png"
#define DEFAULT_PLAYER_THUMBNAIL		SHAREDDIR"/res/shared_images/default_album_art_480.png"
#define BROKEN_ALBUMART_IMAGE_PATH		"/opt/usr/share/media/.thumb/thumb_default.png"

#define PLAYLIST_CREATE_THUMBNAIL		"T02_playlist_thumbnail_created.png"
//for shortcut
typedef enum {
	MP_ADD_TO_HOME_SHORTCUT_TYPE_NONE,
	MP_ADD_TO_HOME_SHORTCUT_TYPE_SYS_PLAYLIST,
	MP_ADD_TO_HOME_SHORTCUT_TYPE_USER_PLAYLIST,
	MP_ADD_TO_HOME_SHORTCUT_TYPE_ALBUM,
	MP_ADD_TO_HOME_SHORTCUT_TYPE_NUM,
}MpShortcutType_e;

#define MP_ADD_TO_HOME_SHORTCUT_PREFIX 				"_Shortcut:MusicPlayer://"
#define MP_ADD_TO_HOME_SHORTCUT_FIELD_DELIMETER		"|"
#define MP_ADD_TO_HOME_SHORTCUT_FIELD_NUM			4


//for livebox play, pause, next, prev events
#define MP_MESSAGE_PORT_LIVEBOX		"messageport_livebox"
#define MP_LB_EVENT_KEY				"LiveboxEvent"
#define MP_LB_EVENT_NEXT_PRESSED		"OnLBNextPress"
#define MP_LB_EVENT_NEXT_RELEASED		"OnLBNextRelease"
#define MP_LB_EVENT_PREV_PRESSED		"OnLBPreviousPress"
#define MP_LB_EVENT_PREV_RELEASED		"OnLBPreviousRelease"
#define MP_LB_EVENT_PLAY_CLICKED		"OnLBPlayClicked"
#define MP_LB_EVENT_PAUSE_CLICKED		"OnLBPauseClicked"
#define MP_LB_EVENT_SHUFFLE_ON_CLICKED          "OnLBShuffleOnClicked"
#define MP_LB_EVENT_SHUFFLE_OFF_CLICKED         "OnLBShuffleOffClicked"
#define MP_LB_EVENT_REPEAT_ALL_CLICKED          "OnLBRepeatAllClicked"
#define MP_LB_EVENT_REPEAT_1_CLICKED            "OnLBRepeat1Clicked"
#define MP_LB_EVENT_REPEAT_A_CLICKED            "OnLBRepeatAClicked"

//for support shortcut and launching from search app.
#define MP_REQ_TYPE_SHORTCUT_TYPE	"shortcut_type"
#define MP_SHORTCUT_ARTIST "artist"
#define MP_SHORTCUT_ALBUM "album"
#define MP_SHORTCUT_PLAYLIST "playlist"

#define MP_REQ_TYPE_SHORTCUT_DESC "shortcut_desc"

//for supprot nowplaying list in PD
#define MP_NOWPLAYING_LIST_DATA		MP_INI_DIR"/MpPlayingList.dat"

//for supprot group list in PD
#define MP_GROUP_LIST_DATA		MP_INI_DIR"/MpGroupList.dat"

#define MP_NOWPLAYING_INI_FILE_NAME	MP_INI_DIR"/now_playing.ini"

#define MP_AUTO_PLAYLIST_ITEM_MAX 	50

#define FACTORY_MUSIC "/opt/usr/media/Sounds/Over the horizon.mp3"

#define MP_PHONE_ROOT_PATH        "/opt/usr/media"
#define MP_MMC_ROOT_PATH		"/opt/storage/sdcard"

#define CONTROLLER_REW_SOURCE "control_previous"
#define CONTROLLER_FF_SOURCE "control_next"

#define PLAY_TIME_ARGS(t) \
        (((int)(t)) / 60) % 60, \
        ((int)(t)) % 60
#define PLAY_TIME_FORMAT "02u:%02u"

#define MUSIC_TIME_ARGS(t) \
        ((int)(t)) / (3600), \
        (((int)(t)) / 60) % 60, \
        ((int)(t)) % 60
#define MUSIC_TIME_FORMAT "02u:%02u:%02u"

#endif
