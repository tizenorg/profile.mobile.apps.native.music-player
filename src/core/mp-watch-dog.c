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

#include "mp-watch-dog.h"
#include <Elementary.h>
#include "mp-player-debug.h"
#include "mp-define.h"
#include "mp-file-util.h"

#ifdef MP_WATCH_DOG

Ecore_Thread *g_WathDogThread;
static int g_MainThreadLockup;
static int g_write_lock;

#define LOCKUP_TIMEOUT 60

static void
__mp_watchdog_thread_function(void *data, Ecore_Thread *thread)
{
	startfunc;
	while (1) {
		if (ecore_thread_check(thread)) {	/* pending cancellation */
			WARN_TRACE("Thread canceled");
			goto END;
		}

		if (g_MainThreadLockup) {
			ERROR_TRACE("Main thread does not response for %d sec.", LOCKUP_TIMEOUT);
			ERROR_TRACE("Make coredump");
			assert(0);
		}

		if (!g_write_lock)
			g_MainThreadLockup = 1;
		DEBUG_TRACE("Send Notify to Main thread");
		ecore_thread_feedback(thread, NULL);

		struct timeval tv;
		tv.tv_sec = LOCKUP_TIMEOUT;
		tv.tv_usec = 0;
		select(0, NULL, NULL, NULL, &tv);

	}

	END:
	endfunc;
}

static void
_mp_watch_dog_thread_end_cb(void *data, Ecore_Thread *thread)
{
	DEBUG_TRACE("Watch dog thread end");
	g_WathDogThread = NULL;
}

static void
_mp_watch_dog_thread_cancel_cb(void *data, Ecore_Thread *thread)
{
	DEBUG_TRACE("");
}

void _mp_watch_dog_thread_notify_cb(void *data, Ecore_Thread *thread, void *msg_data)
{
	DEBUG_TRACE("");
	g_write_lock = 1;
	g_MainThreadLockup = 0;
	g_write_lock = 0;
}

void mp_watch_dog_init(void)
{
#ifdef MP_DEBUG_MODE
	return;
#endif

	if (mp_file_exists("/tmp/mp_disable_watchdog"))
		return;

	if (g_WathDogThread)
		return;

	g_WathDogThread = ecore_thread_feedback_run(__mp_watchdog_thread_function,
		_mp_watch_dog_thread_notify_cb,
		_mp_watch_dog_thread_end_cb,
		_mp_watch_dog_thread_cancel_cb,  NULL, EINA_TRUE);
}

void mp_watch_dog_finalize(void)
{
	if (g_WathDogThread)
		ecore_thread_cancel(g_WathDogThread);
}

void mp_watch_dog_reset(void)
{
	g_write_lock = 1;
	if (g_MainThreadLockup)
		g_MainThreadLockup = EINA_TRUE;
	g_write_lock = 0;
}

#endif

