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


#ifndef __MP_VCONF_PRIVATE_KEYS_H__
#define __MP_VCONF_PRIVATE_KEYS_H__

#ifdef GBS_BUILD
#include <vconf-keys.h>
#endif
#include <app_preference.h>

#define MP_PRIVATE			"private/"PKG_NAME"/"
#define MP_DB_PREFIX		"db/"MP_PRIVATE
#define MP_MEMORY_PREFIX	"memory/"MP_PRIVATE




/**
 * @brief playlist shuffle state
 *
 * type: bool
 *
 * 0 : off
 * 1 : on
 */
#define MP_KEY_MUSIC_SHUFFLE			MP_DB_PREFIX"shuffle"


/**
 * @brief playlist repeat state
 *
 * type: int
 *
 * 0 : repeat all
 * 1 : no repeat
 * 2 : repeat only a songs
 */
#define MP_KEY_MUSIC_REPEAT			MP_DB_PREFIX"repeat"
enum {
	MP_SETTING_REP_ALL,
	MP_SETTING_REP_NON,
	MP_SETTING_REP_1
};

/* for live-box */
#define MP_LIVE_VCONF_PREFIX				MP_MEMORY_PREFIX
/* this state is only for livebox Sound player should not affect this app*/
//#define MP_LIVE_PLAY_STATE					MP_LIVE_VCONF_PREFIX"player_state"


/****************************/
/* preference relative key  */
/****************************/



#ifdef MP_FEATURE_PERSONAL_PAGE
#define KEY_MP_PERSONAL_PAGE	"memory/setting/personal"
#endif

/**
 * @brief trigger of sound effect changes
 *
 * type: bool
 *
 * value is not meaningful
 */
#define KEY_MUSIC_SE_CHANGE			MP_DB_PREFIX"se_change"

/**
 * @brief trigger of extend sound effect changes
 *
 * type: int
 *
 * value is not meaningful
 */
#define KEY_MUSIC_SA_USER_CHANGE			MP_MEMORY_PREFIX"sa_user_change"

/**
 * @brief trigger of menu settng changes
 *
 * type: int
 *
 * value is not meaningful
 */
#define KEY_MUSIC_MENU_CHANGE			MP_DB_PREFIX"menu_change"


/**
 * @brief setting value of sound alive
 *
 * type: int
 *
 * value is not meaningful
 */

/**
 * @brief custom equalizer value
 *
 * type: double
 */
#define KEY_MUSIC_EQUALISER_CUSTOM_1		"eq_custom_1"
#define KEY_MUSIC_EQUALISER_CUSTOM_2		"eq_custom_2"
#define KEY_MUSIC_EQUALISER_CUSTOM_3		"eq_custom_3"
#define KEY_MUSIC_EQUALISER_CUSTOM_4		"eq_custom_4"
#define KEY_MUSIC_EQUALISER_CUSTOM_5		"eq_custom_5"
#define KEY_MUSIC_EQUALISER_CUSTOM_6		"eq_custom_6"
#define KEY_MUSIC_EQUALISER_CUSTOM_7		"eq_custom_7"
#define KEY_MUSIC_EQUALISER_CUSTOM_8		"eq_custom_8"


/**
 * @brief extended user audio effects
 *
 * type: double
 */
#define KEY_MUSIC_USER_AUDIO_EFFECT_3D		"user_audio_effect_3d"
#define KEY_MUSIC_USER_AUDIO_EFFECT_BASS	"user_audio_effect_bass"
#define KEY_MUSIC_USER_AUDIO_EFFECT_ROOM	"user_audio_effect_room"
#define KEY_MUSIC_USER_AUDIO_EFFECT_REVERB	"user_audio_effect_reverb"
#define KEY_MUSIC_USER_AUDIO_EFFECT_CLARITY	"user_audio_effect_clarity"


/**
 * @brief auto off
 *
 * type: int (minute)
 */
#define KEY_MUSIC_AUTO_OFF_TIME_VAL	"auto_off_time_val"

/**
 * @brief auto off type
 *
 * type: int
 */
#define KEY_MUSIC_AUTO_OFF_TYPE_VAL	"auto_off_type_val"
enum {
	KEY_MUSIC_AUTO_OFF_TIME_OFF	= 0,
	KEY_MUSIC_AUTO_OFF_TIME_15,
	KEY_MUSIC_AUTO_OFF_TIME_30,
	KEY_MUSIC_AUTO_OFF_TIME_60,
	KEY_MUSIC_AUTO_OFF_TIME_90,
	KEY_MUSIC_AUTO_OFF_TIME_120,
	KEY_MUSIC_AUTO_OFF_TIME_CUSTOM,
	KEY_MUSIC_AUTO_OFF_TIME_MAX,
};

/**
 * @brief time for custom auto off
 *
 * type: int (minute)
 */
#define KEY_MUSIC_AUTO_OFF_CUSTOM_TIME	"auto_off_custom_time"


/**
 * @brief playlist repeat state
 *
 * type: int
 *
 * 0 : repeat all
 * 1 : no repeat
 * 2 : repeat only a songs
 */
#define MP_KEY_MUSIC_SQUARE_AXIS_VAL	"square_axis_val"
enum {
	MP_KEY_MUSIC_SQUARE_AXIS_MOOD,
	MP_KEY_MUSIC_SQUARE_AXIS_YEAR,
	MP_KEY_MUSIC_SQUARE_AXIS_ADDED,
	MP_KEY_MUSIC_SQUARE_AXIS_TIME
};


/**
 * @brief check which player is playing now
 *
 * type: int
 *
 * pid of music-player of sound-player
 */
#define MP_PREFKEY_PLAYING_PID		"playing_pid"


/**
 * @brief setting value of auto created playlist
 *
 * All playlist can be selected with OR operation
 *
*/
#define MP_PREFKEY_PLAYLIST_VAL_INT	"playlist"
enum {
	MP_PLAYLIST_MOST_PLAYED = 0x0001,
	MP_PLAYLIST_RECENTLY_PLAYED = 0x0002,
	MP_PLAYLIST_RECENTLY_ADDED = 0x0004,
	MP_PLAYLIST_QUICK_LIST = 0x0008,
};

/**
 * @brief playlist order
 *
 * type: string
 *
 * range : 1~4
 */

#define MP_PREFKEY_PLAYLIST_VAL_STR	"playlist_order"


/**
 * @brief tabs order
 *
 * type: string
 *
 * range : 1~8
 */

#define MP_PREFKEY_TABS_VAL_STR	"tabs_order"


/**
 * @brief setting value of  created tabs
 *
 * All tabs can be selected
 *
*/

#define MP_PREFKEY_TABS_VAL_INT	"tabs_select"
enum {
	MP_TRACKS_TAB = 0x0001,
	MP_PLAYLISTS_TAB = 0x0002,
	MP_ALBUMS_TAB = 0x0004,
	MP_ARTISTS_TAB = 0x0008,
	MP_GENRES_TAB = 0x0010,
	MP_FOLDERS_TAB = 0x0020,
	MP_DEVICES_TAB = 0x0040,
};


/**
 * @brief play speed
 *
 * type: double
 *
 * range : 0.5 ~ 2.0
 */
#define PREFKEY_MUSIC_PLAY_SPEED		"playspeed"


/**
 * @brief motion initial popup check box
 *
 * type: bool
 *
 */
#define KEY_MUSIC_MOTION_ASKED		"motion_asked"

/**
 * @brief square initial popup check box
 *
 * type: bool
 *
 */
#define KEY_MUSIC_SQUARE_ASKED		"square_asked"

/**
 * @brief smart volume on/off
 *
 * type: bool
 *
 */
#define KEY_MUSIC_SMART_VOLUME		"smart_volume"


/**
 * @brief show lyrics on/off
 *
 * type: bool
 *
 */
#define KEY_MUSIC_LYRICS		"show_lyrics"


/*********************************** @@ removed @@  ***********************************/

/**
 * @brief elapsed time of current playing song
 *
 * type: string
 *
 * ex) "00:00"
 */
#define MP_LIVE_CUR_POS						MP_LIVE_VCONF_PREFIX"pos"


/**
 * @brief progressbar position of current playing song
 *
 * type: double
 *
 * range : 0.0 ~ 1.0
 */
#define MP_LIVE_CUR_PROGRESS_POS			MP_LIVE_VCONF_PREFIX"progress_pos"


/**
 * @brief trigger of live box button click
 *
 * type: bool
 */
#define MP_LIVE_PROGRESS_RATIO_CHANGED				MP_LIVE_VCONF_PREFIX"position_changed"	//double

#define KEY_PERSONAL_NO_ASK_AGAIN	"personal_no_ask_again" //"db/private/org.tizen.music-player/personal_no_ask_again"


#endif /* __MP_VCONF_PRIVATE_KEYS_H__ */

