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

#include <stdbool.h>
#include <service_app.h>

#include "music-service.h"
#include "mp-service-square.h"
#include "mp-service-debug.h"

struct appdata *g_ad;

static bool
_app_control_app_create(void *data)
{
	startfunc;

	struct appdata *ad = data;
	g_ad = (struct appdata *)data;
	MP_CHECK_VAL(ad, EINA_FALSE);

	ad->square_mgr = mp_app_control_square_mgr_create();
	MP_CHECK_FALSE(ad->square_mgr);

	return true;
}


static void
_app_control_app_terminate(void *data)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK(ad);

	mp_app_control_square_mgr_destory(ad->square_mgr);
}


static void
app_control_app_control(app_control_h app_control, void *data)
{
	startfunc;

	struct appdata *ad = data;
	MP_CHECK(ad);

	int ret = mp_app_control_square_mgr_update_diff_only(ad->square_mgr);

	if (ret != 0) {
		ERROR_TRACE("square mgr update diff error");
	}
}


int
main(int argc, char *argv[])
{
	startfunc;

	struct appdata ad;
	service_app_event_callback_s ops;

	memset(&ops, 0x0, sizeof(service_app_event_callback_s));
	ops.create = _app_control_app_create;
	ops.terminate = _app_control_app_terminate;
	ops..app_control = app_control_app_control;
	ops.low_memory = NULL;
	ops.low_battery = NULL;

	memset(&ad, 0x0, sizeof(struct appdata));

	return service_app_main(argc, argv, &ops, &ad);
}

