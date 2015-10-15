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

#ifndef __MP_FOLDER_VIEW__
#define __MP_FOLDER_VIEW__

#include "mp-list-view.h"
#include "mp-folder-list.h"
#include "mp-edit-callback.h"

#include "music.h"

typedef struct
{
	INHERIT_MP_LIST_VIEW;
	/* extention variables */
	Evas_Object *folder_view_layout;

	/* external objects*/

	/* extention functions */
	void (*content_set)(void *view);
}MpFolderView_t;

MpFolderView_t *mp_folder_view_create(Evas_Object *parent);
int mp_folder_view_destory(MpFolderView_t *view);

#endif
