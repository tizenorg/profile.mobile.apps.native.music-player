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

#include "music.h"
#include "mp-streaming-mgr.h"
#include "mp-http-mgr.h"
#include "mp-player-debug.h"
#include "mp-play.h"
#include "mp-widget.h"
#include "mp-player-mgr.h"
#include "mp-util.h"
#include "mp-player-view.h"
#include "mp-player-control.h"

static bool _mp_streaming_mgr_play_streaming_real(struct appdata *ad);

bool mp_streaming_mgr_check_streaming(struct appdata *ad, const char *path)
{
	MP_CHECK_FALSE(path);

	if (!mp_check_file_exist(path) && mp_util_is_streaming(path)) {
		mp_debug("streaming....");
		return TRUE;
	}

	return FALSE;
}

bool mp_streaming_mgr_set_attribute(struct appdata *ad, player_h player)
{
	startfunc;

	return TRUE;
}

bool mp_streaming_mgr_play_new_streaming(struct appdata *ad)
{
	startfunc;
	MP_CHECK_FALSE(ad);

	bool ret = FALSE;
	{	/* connected */
		ret = _mp_streaming_mgr_play_streaming_real(ad);
		if (ret == 0)
			mp_player_view_update_buffering_progress(GET_PLAYER_VIEW, 0);
	}

	return ret;
}

static bool _mp_streaming_mgr_play_streaming_real(struct appdata *ad)
{
	startfunc;
	MP_CHECK_FALSE(ad);

	return mp_player_control_ready_new_file(ad, TRUE);
}

