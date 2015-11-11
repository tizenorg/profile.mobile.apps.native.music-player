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

#ifndef __MP_UTIL_H_
#define __MP_UTIL_H_

#include <app_preference.h>
#include "music.h"

typedef enum {
	MP_FILE_DELETE_ERR_USING = -2,
	MP_FILE_DELETE_ERR_REMOVE_FAIL = -1,
	MP_FILE_DELETE_ERR_INVALID_FID = 0,
	MP_FILE_DELETE_ERR_NONE,
} mp_file_delete_err_t;

typedef enum {
	MP_DIR_NONE = 0,
	MP_DIR_PHONE,
	MP_DIR_MMC,
	MP_DIR_HTTP
} mp_dir_e;

enum {
	PREF_MUSIC_OFF = 0x00,
	PREF_MUSIC_PLAY,
	PREF_MUSIC_PAUSE,
	PREF_MUSIC_STOP,
	PREF_MUSIC_STATE_MAX
};

#define GET_WINDOW() ((mp_util_get_appdata()) ? mp_util_get_appdata()->win_main : NULL)
#define CHECK_STORE 	mp_util_is_store_enable()
#define PREF_MUSIC_STATE "preference/org.tizen.music-player/state"

int mp_setting_get_nowplaying_id(void);
bool			mp_util_is_streaming(const char *uri);
bool                    mp_util_text_multiline_check(Evas_Object *obj, const char*text, const char*textstyle, int text_width, int text_height);
bool 			mp_check_file_exist(const char *path);
void 			mp_util_format_duration(char *time, int ms);
void mp_util_song_format_duration(char *time, int ms);
bool				mp_util_add_to_playlist_by_key(int playlist_id, char *key_id);
bool     mp_util_delete_track_by_id(char *key_id);
Evas_Object * mp_util_create_image(Evas_Object * obj, const char *path, const char *group, int w, int h);
Evas_Object * 	mp_util_create_thumb_icon(Evas_Object *obj, const char *path, int w, int h);
Evas_Object * mp_util_create_lazy_update_thumb_icon(Evas_Object * obj, const char *path, int w, int h);
const char* 		mp_util_get_index(const char *p);
const char* 		mp_util_get_second_index(const char *p);
Evas_Object * 			mp_util_create_selectioninfo_with_count(void *data, int count);
void mp_util_post_status_message(struct appdata *ad, const char *text);
void mp_util_post_add_to_playlist_popup_message(int count);
char *			mp_util_get_new_playlist_name(void);
mp_file_delete_err_t mp_util_delete_track(void *data, char *fid, char *file_path);

int				mp_util_share_via_bt(const char *formed_path, int file_cnt);
int 			mp_util_file_is_in_phone_memory(const char *path);

char*  			mp_util_get_path_by_handle(mp_media_info_h record);
char* 			mp_util_isf_get_edited_str(Evas_Object *isf_entry, bool permit_first_blank);
int				mp_util_create_playlist(struct appdata *ad, char *name, mp_playlist_h *playlist_handle);

bool 			mp_util_set_screen_mode(void *data , int mode);

bool mp_util_launch_browser(const char *url, struct appdata *ad);


#define mp_object_free(obj)	\
	do {						\
		if(obj != NULL) {		\
			g_free(obj);		\
			obj = NULL;			\
		}						\
	}while(0)

#define MMC_PATH			MP_MMC_ROOT_PATH
#define PKGNAME_SYSTEM			"sys_string"

gunichar mp_util_get_utf8_initial_value(const char *name);
gchar * mp_get_new_playlist_name(void);
gchar *mp_parse_get_title_from_path(const gchar *path);
char *mp_util_get_title_from_path(const char *path);
bool	mp_util_is_playlist_name_valid(char *name);
void mp_util_set_library_controlbar_items(void *data);

void mp_util_reset_genlist_mode_item(Evas_Object *genlist);

bool mp_util_is_image_valid(Evas *evas, const char *path);
char *mp_util_shorten_path(char *path_info);

bool mp_util_is_earjack_inserted(void);
void mp_util_get_sound_path(mp_snd_path *snd_path);

const char * mp_util_search_markup_keyword(const char *string, char *searchword, bool *result);

bool mp_util_is_other_player_playing(void);
bool mp_util_is_now_active_player(void);

int mp_commmon_check_rotate_lock(void);
int mp_check_mass_storage_mode(void);

bool mp_util_sleep_lock_set(bool lock, bool force_unlock);
bool mp_util_is_nfc_feature_on(void);

void mp_util_strncpy_safe(char *x_dst, const char *x_src, int max_len);
#ifdef MP_IMAGE_EFFECT
bool mp_util_edit_image(Evas *evas, Evas_Object *src_image, const char *path, int mode);
#endif
void mp_util_free_track_info(mp_track_info_t *track_info);
void mp_util_load_track_info(struct appdata *ad, mp_plst_item *cur_item, mp_track_info_t **info);
void mp_util_append_media_list_item_to_playlist(mp_plst_mgr *playlist_mgr, mp_media_list_h media_list, int count, int current_index, const char *uri);
char* mp_util_get_fid_by_full_path(const char *full_path, char **title, char **artist);
EXPORT_API struct appdata *mp_util_get_appdata(void);
char *mp_util_get_text(const char *str);
void mp_util_more_btn_move_ctxpopup(Evas_Object *ctxpopup, Evas_Object *btn);
Elm_Object_Item *mp_util_ctxpopup_item_append_ext(Evas_Object *obj, const char *label, const char *file,
        const char *group, Evas_Smart_Cb func,
        const void *data);
Elm_Object_Item *mp_util_ctxpopup_item_append(Evas_Object *obj, const char *label,
        const char *group, Evas_Smart_Cb func,
        const void *data);
Elm_Object_Item *mp_util_toolbar_item_append(Evas_Object *obj, const char *icon,
        const char *label, Evas_Smart_Cb func,
        const void *data);

Elm_Object_Item *mp_util_toolbar_nth_item(Evas_Object *obj, int n);

bool mp_util_get_sip_state(void);
bool mp_util_is_landscape(void);

int mp_util_parse_device_type(const char *name);

void mp_util_set_livebox_update_timer(void);
void mp_util_print_geometry(Evas_Object *obj, const char *name);
void mp_util_hide_lock_screen(void);
bool mp_util_is_lock_screen_exist(void);
void mp_util_object_item_translate_set(Elm_Object_Item *item, const char *ID);
bool mp_util_file_playable(const char *uri);
char * mp_util_file_mime_type_get(const char *uri);
bool mp_util_app_resume(void);
bool mp_util_system_volume_popup_show(void);
bool mp_util_is_call_connected(void);
void mp_util_domain_translatable_text_set(Evas_Object *obj, const char* text);
void mp_util_domain_translatable_part_text_set(Evas_Object *obj, const char* part, const char* text);
void mp_util_item_domain_translatable_part_text_set(Elm_Object_Item *item, const char* part, const char* text);
bool mp_util_is_mmc_removed(void);

void dump_win(Evas_Object *obj, int max_depth);
void dump_obj(Evas_Object *obj, int lvl, int max_depth);
void dump_widget(Evas_Object *obj, int lvl, int max_depth);
#ifdef MP_FEATURE_PERSONAL_PAGE
bool mp_util_is_in_personal_page(const char *path);
bool mp_util_is_personal_page_on();
void mp_util_get_unique_name(char *original_file_name, char **unique_file_name);
int mp_util_is_duplicated_name(const char *dir, const char *name);
int mp_util_get_file_ext(const char *filename, char **file_ext);

#endif
bool mp_util_mirroring_is_connected(void);

int mp_util_get_root_window_angle(Evas_Object *win);
bool mp_util_is_scan_nearby_available();
bool mp_util_is_store_enable(void);
bool mp_util_free_space_check(double size);

bool mp_util_media_is_uhqa(const char *media_id);
mp_dir_e mp_util_get_file_location(const char *uri);
void mp_util_lock_cpu();
void mp_util_release_cpu();

#endif //__MP_UTIL_H_

