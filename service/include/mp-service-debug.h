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

#ifndef __MP_APP_CONTROL_DEBUG_H__
#define __MP_APP_CONTROL_DEBUG_H__


#include <stdio.h>
#include <unistd.h>
#include "assert.h"
#include <linux/unistd.h>
#include "mp-file-util.h"

//#ifdef MP_DEBUG_MODE
#define ENABLE_CHECK_START_END_FUNCTION	//101219 aramie.kim support enter leave debug message
//#endif

#define LOG_COLOR_RESET    "\033[0m"
#define LOG_COLOR_RED      "\033[31m"
#define LOG_COLOR_YELLOW   "\033[33m"
#define LOG_COLOR_GREEN  	"\033[32m"
#define LOG_COLOR_BLUE		"\033[36m"
#define LOG_COLOR_PURPLE   "\033[35m"


#define ENABLE_LOG_SYSTEM

#ifdef ENABLE_LOG_SYSTEM

#define USE_DLOG_SYSTEM

#define gettid() syscall(__NR_gettid)

#define DATA_DIR	DATA_PREFIX"/data"

#ifdef USE_DLOG_SYSTEM
#include <dlog.h>

#ifdef LOG_TAG
#undef LOG_TAG
#endif //LOG_TAG

#define LOG_TAG "SVC_MUSIC_PLAYER"

#ifndef TRUE
#define TRUE 1
#endif

#define DEBUG_TRACE(fmt, arg...)	LOGD_IF(TRUE, "[TID:%d]   "fmt"\n", gettid(), ##arg)
#define INFO_TRACE(fmt, arg...) 	LOGI_IF(TRUE, "[TID:%d]   "fmt"\n", gettid(), ##arg)
#define WARN_TRACE(fmt, arg...) 	LOGW_IF(TRUE, "[TID:%d]   "fmt"\n", gettid(), ##arg)
#define ERROR_TRACE(fmt, arg...)	LOGW_IF(TRUE, "[TID:%d]   "fmt"\n", gettid(), ##arg)
#define mp_debug_temp(fmt, arg...)	LOGD_IF(TRUE, "[TID:%d]   "fmt"\n", gettid(), ##arg)

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
#define mp_debug_temp(fmt, arg...) do{fprintf(stderr, "[%s : %s-%d]\t - \n", __FILE__, __func__, __LINE__);\
	    fprintf (stderr, __VA_ARGS__);}while(0)
#endif //USE_DLOG_SYSTEM

#define DEBUG_TRACE_FUNC() DEBUG_TRACE("")

#else //ENABLE_LOG_SYSTEM
#define mp_debug(fmt, arg...)
#define DEBUG_TRACE(fmt, arg...)
#define INFO_TRACE(fmt, arg...)
#define WARN_TRACE(fmt, arg...)
#define ERROR_TRACE(fmt, arg...)
#define mp_debug_temp(fmt, arg...)
#endif //ENABLE_LOG_SYSTEM

#ifdef ENABLE_CHECK_START_END_FUNCTION
#define startfunc   		DEBUG_TRACE("+-  START -------------------------");
#define endfunc     		DEBUG_TRACE("+-  END  --------------------------");
#define exceptionfunc	ERROR_TRACE("#################################      CRITICAL ERROR   #####################################");
#else
#define startfunc
#define endfunc
#define exceptionfunc
#endif

#define mp_debug(fmt, arg...)			LOGD_IF(TRUE,  LOG_COLOR_GREEN"[TID:%d]   "fmt""LOG_COLOR_RESET, gettid(), ##arg)
#define mp_error(fmt, arg...)			LOGW_IF(TRUE,  LOG_COLOR_RED"[TID:%d]# ERROR   CHECK  #   "fmt""LOG_COLOR_RESET, gettid(), ##arg)

#define TIMER_TRACE(fmt, arg...)	do { if (mp_file_exists("/tmp/mp_show_timer_log")) LOGD_IF(TRUE,  LOG_COLOR_PURPLE"[TID:%d]   "fmt""LOG_COLOR_RESET, gettid(), ##arg); } while (0)

#undef FREE
#define FREE(ptr) free(ptr); ptr = NULL;

#undef SAFE_FREE
#define SAFE_FREE(x)       if(x) {free(x); x = NULL;}

#undef IF_FREE
#define IF_FREE(ptr) if (ptr) {free(ptr); ptr = NULL;}

#define mp_ret_if(expr) do { \
	if(expr) { \
		DEBUG_TRACE("");\
		return; \
	} \
} while (0)
#define mp_retv_if(expr, val) do { \
	if(expr) { \
		DEBUG_TRACE("");\
		return (val); \
	} \
} while (0)

#define mp_retm_if(expr, fmt, arg...) do { \
	if(expr) { \
		DEBUG_TRACE(fmt, ##arg); \
		return; \
	} \
} while (0)

#define mp_retvm_if(expr, val, fmt, arg...) do { \
	if(expr) { \
		DEBUG_TRACE(fmt, ##arg); \
		return (val); \
	} \
} while (0)

#define MP_CHECK_EXCEP(expr) do { \
	if(!(expr)) { \
		ERROR_TRACE("CRITICAL ERROR ########################################## CHECK BELOW ITEM");\
		goto mp_exception;\
	} \
} while (0)

#define MP_CHECK_VAL(expr, val) 		mp_retvm_if(!(expr),val,"INVALID PARM RETURN val:0x%x", val)
#define MP_CHECK_NULL(expr) 		mp_retvm_if(!(expr),NULL,"INVALID PARM RETURN NULL")
#define MP_CHECK_FALSE(expr) 		mp_retvm_if(!(expr),FALSE,"INVALID PARM RETURN FALSE")
#define MP_CHECK_CANCEL(expr) 		mp_retvm_if(!(expr), ECORE_CALLBACK_CANCEL, "INVALID PARAM RETURN")
#define MP_CHECK(expr) 				mp_retm_if(!(expr),"INVALID PARAM RETURN")

#define mp_assert(expr) do { \
	if(!(expr)) { \
		ERROR_TRACE("CRITICAL ERROR ########################################## CHECK BELOW ITEM");\
		assert(FALSE); \
	} \
} while (0)



#endif	// __MP_APP_CONTROL_DEBUG_H__

