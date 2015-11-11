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


#ifndef __MC_DEBUG_H_
#define __MC_DEBUG_H_


#include <stdio.h>
#include <unistd.h>
#include "assert.h"
#include <linux/unistd.h>

#ifdef MP_DEBUG_MODE
#define ENABLE_CHECK_START_END_FUNCTION	// support enter leave debug message
#endif

#define ENABLE_LOG_SYSTEM

#ifdef ENABLE_LOG_SYSTEM

#define USE_DLOG_SYSTEM

#define gettid() syscall(__NR_gettid)

#ifdef USE_DLOG_SYSTEM
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif //LOG_TAG

#define LOG_TAG "MUSIC_CHOOSER"

#ifndef TRUE
#define TRUE 1
#endif

#define LOG_COLOR_RESET    "\033[0m"
#define LOG_COLOR_RED      "\033[31m"
#define LOG_COLOR_YELLOW   "\033[33m"
#define LOG_COLOR_GREEN  	"\033[32m"
#define LOG_COLOR_BLUE		"\033[36m"

#ifndef LOGD_IF
#define LOGD_IF(fmt, arg...) dlog_print(DLOG_DEBUG, LOG_TAG, ##arg)
#endif
#ifndef LOGI_IF
#define LOGI_IF(fmt, arg...) dlog_print(DLOG_INFO, LOG_TAG, ##arg)
#endif
#ifndef LOGW_IF
#define LOGW_IF(fmt, arg...) dlog_print(DLOG_ERROR, LOG_TAG, ##arg)
#endif

#define DEBUG_TRACE(fmt, arg...)	LOGD_IF(TRUE,  LOG_COLOR_GREEN"[TID:%d]   "fmt""LOG_COLOR_RESET, gettid(), ##arg)
#define INFO_TRACE(fmt, arg...) 	LOGI_IF(TRUE,  LOG_COLOR_GREEN"[TID:%d]    "fmt""LOG_COLOR_RESET, gettid(), ##arg)
#define WARN_TRACE(fmt, arg...) 	LOGW_IF(TRUE,  LOG_COLOR_YELLOW"[TID:%d]   "fmt""LOG_COLOR_RESET, gettid(), ##arg)
#define ERROR_TRACE(fmt, arg...)	LOGW_IF(TRUE,  LOG_COLOR_RED"[TID:%d]   "fmt""LOG_COLOR_RESET, gettid(), ##arg)
#define PARAM_CHECK(fmt, arg...) 	LOGD_IF(TRUE,  LOG_COLOR_YELLOW"[TID:%d]   "fmt""LOG_COLOR_RESET, gettid(), ##arg)
#define EVENT_TRACE(fmt, arg...) 	LOGW_IF(TRUE,  LOG_COLOR_BLUE"[TID:%d]   [MUSIC_PLAYER_EVENT]"fmt""LOG_COLOR_RESET, gettid(), ##arg)

#define SECURE_DEBUG(fmt, args...)	SECURE_LOGD("[T:%d] " fmt, gettid(), ##args)
#define SECURE_INFO(fmt, args...)	SECURE_LOGI("[T:%d] " fmt, gettid(), ##args)
#define SECURE_ERROR(fmt, args...)	SECURE_LOGD("[T:%d] " fmt, gettid(), ##args)

#else // use USE_DLOG_SYSTEM

#define DEBUG_TRACE(fmt, arg...) do{fprintf(stderr, "[%s : %s-%d]\t - \n", __FILE__, __func__, __LINE__);\
		fprintf (stderr, __VA_ARGS__);}while(0)
#define INFO_TRACE(fmt, arg...) do{fprintf(stderr, "[%s : %s-%d]\t - \n", __FILE__, __func__, __LINE__);\
		fprintf (stderr, __VA_ARGS__);}while(0)
#define WARN_TRACE(fmt, arg...) do{fprintf(stderr, "[%s : %s-%d]\t - \n", __FILE__, __func__, __LINE__);\
		fprintf (stderr, __VA_ARGS__);}while(0)
#define ERROR_TRACE(fmt, arg...) do{fprintf(stderr, "[%s : %s-%d]\t - \n", __FILE__, __func__, __LINE__);\
		fprintf (stderr, __VA_ARGS__);}while(0)
#define EVENT_TRACE(fmt, arg...) do{fprintf(stderr, "[%s : %s-%d]\t - \n", __FILE__, __func__, __LINE__);\
		fprintf (stderr, __VA_ARGS__);}while(0)
#endif //USE_DLOG_SYSTEM

#define DEBUG_TRACE_FUNC() DEBUG_TRACE("")

#else //ENABLE_LOG_SYSTEM
#define DEBUG_TRACE(fmt, arg...) ;
#define INFO_TRACE(fmt, arg...)	;
#define WARN_TRACE(fmt, arg...)	;
#define ERROR_TRACE(fmt, arg...)	;
#define EVENT_TRACE(fmt, arg...)	;
#endif //ENABLE_LOG_SYSTEM

#ifdef ENABLE_CHECK_START_END_FUNCTION
#ifndef startfunc
#define startfunc   		DEBUG_TRACE("+-  START -------------------------");
#endif
#ifndef endfunc
#define endfunc     		DEBUG_TRACE("+-  END  --------------------------");
#endif

#define exceptionfunc	ERROR_TRACE("### CRITICAL ERROR   ###");
#else
#define startfunc			DEBUG_TRACE("+-  START -------------------------");
#define endfunc				DEBUG_TRACE("+-  END  --------------------------");
#define exceptionfunc
#endif

#define eventfunc		EVENT_TRACE("");

#define mp_ret_if(expr) do { \
		if(expr) { \
			PARAM_CHECK("");\
			return; \
		} \
	} while (0)
#define mp_retv_if(expr, val) do { \
		if(expr) { \
			PARAM_CHECK("");\
			return (val); \
		} \
	} while (0)

#define mp_retm_if(expr, fmt, arg...) do { \
		if(expr) { \
			PARAM_CHECK(fmt, ##arg); \
			return; \
		} \
	} while (0)

#define mp_retvm_if(expr, val, fmt, arg...) do { \
		if(expr) { \
			PARAM_CHECK(fmt, ##arg); \
			return (val); \
		} \
	} while (0)

#define CHECK_EXCEP(expr) do { \
		if(!(expr)) { \
			ERROR_TRACE("CRITICAL ERROR ## CHECK BELOW ITEM");\
			goto mp_exception;\
		} \
	} while (0)

#define mp_ecore_timer_del(timer) do { \
		if(timer) { \
			ecore_timer_del(timer);\
			timer = NULL; \
		} \
	} while (0)

#define MP_CHECK_VAL(expr, val) 		mp_retvm_if(!(expr),val,"INVALID PARM RETURN VAL: 0x%x", val)
#define MP_CHECK_NULL(expr) 		mp_retvm_if(!(expr),NULL,"INVALID PARM RETURN NULL")
#define MP_CHECK_FALSE(expr) 		mp_retvm_if(!(expr),FALSE,"INVALID PARM RETURN FALSE")
#define CHECK_CANCEL(expr) 		mp_retvm_if(!(expr), ECORE_CALLBACK_CANCEL, "INVALID PARAM RETURN")
#define MP_CHECK(expr) 				mp_retm_if(!(expr),"INVALID PARAM RETURN")

#define MC_LIST_ICON_SIZE 70 * elm_config_scale_get()

#define SAFE_FREE(x)       if(x) {free(x); x = NULL;}
#undef IF_G_FREE
#define IF_G_FREE(p) ({g_free(p);p=NULL;})

#define mp_assert(expr) do { \
		if(!(expr)) { \
			ERROR_TRACE("CRITICAL ERROR ## CHECK BELOW ITEM");\
			assert(FALSE); \
		} \
	} while (0)


#define mc_evas_object_del(object) do { \
		if(object) { \
			evas_object_del(object); \
			object = NULL; \
		} \
	} while (0)

#endif // __MC_DEBUG_H_
