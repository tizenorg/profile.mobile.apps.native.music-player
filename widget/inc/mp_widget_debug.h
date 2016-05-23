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

#ifndef MP_WIDGET_DEBUG_H
#define MP_WIDGET_DEBUG_H

#include <stdio.h>
#include <unistd.h>
#include "assert.h"
#include <linux/unistd.h>


#define gettid() syscall(__NR_gettid)

#include <dlog.h>

#if !defined(FLOG)

#define DbgPrint(format, arg...)	dlog_print(DLOG_DEBUG, LOG_TAG, ##arg)
#define ErrPrint(format, arg...)	dlog_print(DLOG_ERROR, LOG_TAG, ##arg)
#define WarnPrint(format, arg...)	dlog_print(DLOG_INFO, LOG_TAG, ##arg)

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#define DEBUG_TRACE(fmt, arg...)	dlog_print(DLOG_DEBUG, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##arg)
#define INFO_TRACE(fmt, arg...) 	dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##arg)
#define WARN_TRACE(fmt, arg...) 	dlog_print(DLOG_WARN, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##arg)
#define ERROR_TRACE(fmt, arg...)	dlog_print(DLOG_ERROR, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##arg)
#define PARAM_CHECK(fmt, arg...) 	dlog_print(DLOG_WARN, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##arg)
#define EVENT_TRACE(fmt, arg...) 	dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##arg)

#define SECURE_DEBUG(fmt, args...)	dlog_print(DLOG_DEBUG, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##args)
#define SECURE_INFO(fmt, args...)	dlog_print(DLOG_INFO, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##args)
#define SECURE_ERROR(fmt, args...)	dlog_print(DLOG_ERROR, LOG_TAG, "[%s : %05d %lu]" fmt "\n", __func__, __LINE__, gettid(), ##args)

#else
extern FILE *__file_log_fp;
#define DbgPrint(format, arg...) do { fprintf(__file_log_fp, "[LOG] [[32m%s/%s[0m:%d] " format, util_basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)
#define ErrPrint(format, arg...) do { fprintf(__file_log_fp, "[ERR] [[32m%s/%s[0m:%d] " format, util_basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)
#define WarnPrint(format, arg...) do { fprintf(__file_log_fp, "[WRN] [[32m%s/%s[0m:%d] " format, util_basename(__FILE__), __func__, __LINE__, ##arg); fflush(__file_log_fp); } while (0)
#endif

// DbgPrint("FREE\n");
#define DbgFree(a) do { \
	free(a); \
} while (0)

#define DbgXFree(a) do { \
	DbgPrint("XFree\n"); \
	XFree(a); \
} while (0)

#define mp_retvm_if(expr, val, fmt, arg...) do { \
	if(expr) { \
		PARAM_CHECK(fmt, ##arg); \
		return (val); \
	} \
} while (0)

#if defined(_ENABLE_PERF)
#define PERF_INIT() \
	struct timeval __stv; \
	struct timeval __etv; \
	struct timeval __rtv

#define PERF_BEGIN() do { \
	if (gettimeofday(&__stv, NULL) < 0) { \
		ErrPrint("gettimeofday: %s\n", strerror(errno)); \
	} \
} while (0)

#define PERF_MARK(tag) do { \
	if (gettimeofday(&__etv, NULL) < 0) { \
		ErrPrint("gettimeofday: %s\n", strerror(errno)); \
	} \
	timersub(&__etv, &__stv, &__rtv); \
	DbgPrint("[%s] %u.%06u\n", tag, __rtv.tv_sec, __rtv.tv_usec); \
} while (0)
#else
#define PERF_INIT()
#define PERF_BEGIN()
#define PERF_MARK(tag)
#endif

#endif// MP_WIDGET_DEBUG_H
/* End of a file */
