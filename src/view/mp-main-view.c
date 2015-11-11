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

#include "mp-main-view.h"
#include "mp-player-view.h"
#include "mp-all-view.h"
#include "mp-util.h"
#include "mp-player-control.h"
#include "mp-widget.h"
#include "mp-http-mgr.h"
#include "mp-player-mgr.h"
#include "music.h"
#include "mp-common.h"

#define ACTIVE_VIEW mp_common_get_scroll_view(GET_VIEW_MGR)

int _mp_main_view_update_options(void *thiz);
