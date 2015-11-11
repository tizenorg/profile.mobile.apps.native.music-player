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

#ifndef __MP_ALBUM_DETAILVIEW_H__
#define __MP_ALBUM_DETAILVIEW_H__

#include "mp-list-view.h"
#include "mp-album-detail-list.h"
#include "mp-edit-callback.h"
#include "music.h"

typedef struct {
	INHERIT_MP_LIST_VIEW;

	Evas_Object *album_detail_view_layout;
	Evas_Object *album_index;

	/* external objects*/

	/* album name */
	char *name;
	/* artist name */
	char *artist;
	/* thumbnail */
	char *thumbnail;

	/* extention functions */
	void (*content_set)(void *view);
} MpAlbumDetailView_t;

MpAlbumDetailView_t *mp_album_detail_view_create(Evas_Object *parent, char *album, char *artist, char *thumbnail);
int mp_album_detail_view_destory(MpAlbumDetailView_t *view);

#endif
