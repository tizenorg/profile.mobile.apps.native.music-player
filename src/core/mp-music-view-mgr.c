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

#include "mp-music-view-mgr.h"

#include "mp-player-debug.h"
#include "MusicView.h"
#include "SmartVolume.h"
#include "stdbool.h"

#ifdef MP_FEATURE_MUSIC_VIEW
static bool g_music_view_init;

int mp_music_view_mgr_init(void)
{
	startfunc;
	int res = MusicViewInit();

	if (res)
		WARN_TRACE("Fail to init music view res: 0x%x", res);
	else
		g_music_view_init = 1;

	return res;
}

int mp_music_view_mgr_release(void)
{
	startfunc;
	MP_CHECK_VAL(g_music_view_init, -1);
	int res = MusicViewRelease();

	if (res)
		WARN_TRACE("Fail to init music view");

	g_music_view_init = 0;

	return res;
}

char *mp_music_view_mgr_exe(char *filename, int *length)
{
	MP_CHECK_NULL(g_music_view_init);

	char *wave_pcm = MusicViewEXE(filename, length);
	MP_CHECK_NULL(wave_pcm);

	return wave_pcm;
}
#endif

