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

#ifndef __MP_PLAYLIST_DETAILVIEW_H__
#define __MP_PLAYLIST_DETAILVIEW_H__

#include "mp-list-view.h"
#include "mp-track-list.h"
#include "mp-edit-callback.h"

#include "music.h"

typedef struct {
	INHERIT_MP_LIST_VIEW;

	Evas_Object *playlist_detail_view_layout;

	/* external objects*/

	/* selected content */
	mp_track_type_e list_type;
	char *name;
	int p_id;

	/* extention functions */
	void (*content_set)(void *view);
} MpPlaylistDetailView_t;

MpPlaylistDetailView_t *mp_playlist_detail_view_create(Evas_Object *parent, mp_track_type_e list_type, char *name, int p_id);
int mp_playlist_detail_view_destory(MpPlaylistDetailView_t *view);
void mp_playlist_detail_view_tracklist_edit_cb(void *data, Evas_Object * obj, void *event_info);
#endif
