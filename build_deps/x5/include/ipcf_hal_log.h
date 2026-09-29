/***
 *                     COPYRIGHT NOTICE
 *            Copyright (C) 2019 -2023, Horizon Robotics Co., Ltd.
 *                   All rights reserved.
 ***/

/**
 * @file ipcf_hal_log.h
 *
 * @NO{S17E09C01I}
 * @ASIL{B}
 */

#ifndef IPCF_HAL_LOG_H
#define IPCF_HAL_LOG_H
#include <linux/types.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/time.h>
#include <libgen.h>
#include <pthread.h>
#include <log.h>

#ifdef LOG_TAG
#undef LOG_TAG
#define LOG_TAG "IPCF_HAL"
#endif
#define LOGLEVEL_ENV "IPCF_HAL_DEBUG_LEVEL"/**< setup by json*/
#define STRINGIZE_NO_EXPANSION(x) #x
#define STRINGIZE(x) STRINGIZE_NO_EXPANSION(x)
#define FILENAME basename((char *)__FILE__)
#define HERE "%s" ":" STRINGIZE(__LINE__)
#define L_INFO "[INFO][" HERE "] "/**< information*/
#define L_WARNING "[WARNING][" HERE "] "/**< warning*/
#define L_ERROR "[ERROR][" HERE "] "/**< error*/
#define L_DEBUG "[DEBUG][" HERE "] "/**< debug*/
#define L_FILE "[FILE][" HERE "] "/**< file*/
#define FILE_NAME_HEAD "/userdata/ipcf_hal_log"/**< file name head*/

#ifndef pr_fmt
#define pr_fmt(fmt) fmt
#endif

#define LIMIT_ERR (1)/**< limit error code*/

/**
 * @enum log_level_e
 * Define the descriptor of log level.
 * @NO{S17E09C01}
 */
enum log_level_e {
	LOG_FILE = 0,/**< file*/
	LOG_VERBOSE,/**< verbose*/
	LOG_DEBUG,/**< debug*/
	LOG_INFO,/**< info*/
	LOG_WARNING,/**< warning*/
	LOG_ERROR,/**< error*/
	LOG_LEVEL_MAX/**< max level*/
};

/**
 * @enum log_debug_e
 * Define the descriptor of log debug.
 * @NO{S17E09C01}
 */
enum log_debug_e {
	LOG_DEBUG_TIMESTAMP = 2,/**< debug timestamp*/
	LOG_DEBUG_INFO,/**< debug information*/
	LOG_DEBUG_DUMP,/**< debug dump*/
	LOG_DEBUG_MAX/**< max debug number*/
};

void IPCF_HAL_PRINT(const char *fmt, ...);
void IPCF_HAL_LOGERR(const char *fmt, ...);
void IPCF_HAL_LOGWARN(const char *fmt, ...);
void IPCF_HAL_LOGINFO(const char *fmt, ...);
void IPCF_HAL_LOGFILE(const char *fmt, ...);
void IPCF_HAL_LOGDEBUG(const char *fmt, ...);
void IPCF_HAL_print(const char *buf, int32_t len, const char *fmt, ...);
void IPCF_HAL_print_timestamp(const char *fmt, ...);
void IPCF_HAL_LOGRatelimited(int32_t level, const char *fmt, ...);

static inline int32_t check_debug_level(void)
{
	static int32_t debug_flag = -1;
	const char *dbg_env;
	int32_t ret;

	if (debug_flag >= 0) {
		return debug_flag;
	} else {
		dbg_env = getenv(LOGLEVEL_ENV);
		if (dbg_env != NULL) {
			ret = atoi(dbg_env);
		if (ret <= 0) {
			debug_flag = 0;
		} else {
			debug_flag = ret;
		}
		} else {
			debug_flag = 0;
		}
	}

	return debug_flag;
}

inline int64_t get_current_time_us(void)
{
	int64_t usec = 0;
	struct timeval tv;

	if (0 == gettimeofday(&tv, NULL)) {
		usec = tv.tv_sec * 1000000 + tv.tv_usec;
	}

	return usec;
}

inline int64_t get_current_time_ms(void)
{
	int64_t msec = 0;

	msec = get_current_time_us() / 1000;

	return msec;
}

#define IPCF_HAL_LOGERR(fmt, ...)\
	do {\
		fprintf(stderr, L_ERROR "" pr_fmt(fmt), FILENAME, ##__VA_ARGS__);\
		ALOGE(L_ERROR "" fmt, FILENAME, ##__VA_ARGS__);\
	} while (0);

#define IPCF_HAL_LOGWARN(fmt, ...)\
	do {\
		fprintf(stdout, L_WARNING "" pr_fmt(fmt), FILENAME, ##__VA_ARGS__);\
		ALOGW(L_WARNING "" fmt, FILENAME, ##__VA_ARGS__);\
	} while (0);

#define IPCF_HAL_LOGINFO(fmt, ...)\
	do {\
		fprintf(stdout, L_INFO "" pr_fmt(fmt), FILENAME, ##__VA_ARGS__);\
		ALOGI(L_INFO "" fmt, FILENAME, ##__VA_ARGS__);\
	} while (0);

#define IPCF_HAL_LOGDEBUG(fmt, ...)\
	do {\
		int32_t loglevel = check_debug_level();\
							\
		if (loglevel == LOG_DEBUG_INFO) {\
			fprintf(stdout, L_DEBUG "" pr_fmt(fmt), FILENAME, ##__VA_ARGS__);\
			ALOGD(L_DEBUG "" fmt, FILENAME, ##__VA_ARGS__);\
		}\
	} while (0);

#define IPCF_HAL_LOGFILE(fmt, ...)\
	do {\
		FILE *fp = fopen(FILE_NAME_HEAD, "a+");\
							\
		if (fp == NULL) break;\
		fprintf(fp, L_FILE "" pr_fmt(fmt), FILENAME, ##__VA_ARGS__);\
		fprintf(stdout, L_FILE "" pr_fmt(fmt), FILENAME, ##__VA_ARGS__);\
		ALOGI(L_FILE "" fmt, FILENAME, ##__VA_ARGS__);\
		fclose(fp);\
	} while (0);

#define IPCF_HAL_print(buf, len, fmt, ...)\
	do {\
		int32_t loglevel = check_debug_level();\
		int32_t i = 0, j = 0;\
		int32_t m = 32;\
		int32_t mul = len / m;\
		int32_t remain = len % m;\
		char tmp_buf[256];\
		int32_t tmp_len = 0;\
					\
		if ((loglevel >= LOG_DEBUG_INFO)) {\
			fprintf(stdout, L_INFO "" pr_fmt(fmt), FILENAME, ##__VA_ARGS__);\
			ALOGI(L_INFO "" fmt, FILENAME, ##__VA_ARGS__);\
			if (loglevel == LOG_DEBUG_DUMP) {\
				for (i = 0; i < mul; i++) {\
					IPCF_HAL_LOGINFO(\
						"0x%04x: %02X %02X %02X %02X %02X %02X %02X %02X "\
						"%02X %02X %02X %02X %02X %02X %02X %02X "\
						"%02X %02X %02X %02X %02X %02X %02X %02X "\
						"%02X %02X %02X %02X %02X %02X %02X %02X\n", i * m,\
						buf[i * m + 0], buf[i * m + 1], buf[i * m + 2],\
						buf[i * m + 3], buf[i * m + 4], buf[i * m + 5], buf[i * m + 6],\
						buf[i * m + 7], buf[i * m + 8], buf[i * m + 9], buf[i * m + 10],\
						buf[i * m + 11], buf[i * m + 12], buf[i * m + 13],\
						buf[i * m + 14], buf[i * m + 15], buf[i * m + 16],\
						buf[i * m + 17], buf[i * m + 18], buf[i * m + 19],\
						buf[i * m + 20], buf[i * m + 21], buf[i * m + 22],\
						buf[i * m + 23], buf[i * m + 24], buf[i * m + 25],\
						buf[i * m + 26], buf[i * m + 27], buf[i * m + 28],\
						buf[i * m + 29], buf[i * m + 30], buf[i * m + 31]);\
				}\
				if (remain > 0) {\
					for (j = 0; j < remain; j++) {\
						tmp_len += snprintf(&tmp_buf[tmp_len], sizeof(tmp_buf) - tmp_len,\
						"%02X ", buf[mul * m + j]);\
					}\
					IPCF_HAL_LOGINFO("0x%04x: %s\n", (mul * m), tmp_buf);\
				}\
			}\
		}\
	} while (0);

#define IPCF_HAL_print_timestamp(fmt, ...)\
	do {\
		int32_t loglevel = check_debug_level();\
		int64_t timestamp = get_current_time_ms();\
		if (loglevel >= LOG_DEBUG_TIMESTAMP) {\
			IPCF_HAL_LOGINFO(pr_fmt(fmt) "timestamp[%ld]\n", ##__VA_ARGS__, timestamp);\
		}\
	} while (0);

/**
 * @struct ratelimit_state_user
 * @brief Define the descriptor of ratelimit state.
 * @NO{S17E09C01}
 */
struct ratelimit_state_user {
	pthread_mutex_t lock;/**< protect the state */
	int32_t interval;/**< interval*/
	int32_t burst;/**< burst*/
	int32_t printed;/**< printed*/
	int32_t missed;/**< missed*/
	int64_t begin;/**< begin*/
};

inline int32_t __ratelimit_user(struct ratelimit_state_user *rs, const char *file,
				const char *func, const int32_t line)
{
	int32_t ret = 0;

	if (!rs->interval || !file || !func)
		return LIMIT_ERR;

	if (0 != pthread_mutex_trylock(&rs->lock))
		return 0;
	if (!rs->begin)
		rs->begin = get_current_time_ms();
	if ((rs->begin + rs->interval) < get_current_time_ms()) {
		if (rs->missed) {
			fprintf(stdout, "[WARNING]" "[%s:%d] %s %d callbacks suppressed.\n",
				file, line, func, rs->missed);
			ALOGW("[WARNING]" "[%s:%d] %s %d callbacks suppressed.\n",
				file, line, func, rs->missed);
			rs->missed = 0;
		}
		rs->begin   = get_current_time_ms();
		rs->printed = 0;
	}
	if (rs->burst && rs->burst > rs->printed) {
		rs->printed++;
		ret = LIMIT_ERR;
	} else {
		rs->missed++;
		ret = 0;
	}
	pthread_mutex_unlock(&rs->lock);

	return ret;
}

#define RATELIMIT_INTERVAL_DEFAULT (10 * 1000)/**< Interval Time (ms)*/
#define RATELIMIT_BURST_DEFAULT (5)/**< Limit Times 5times/10s*/
#define RATELIMIT_STATE_INIT_USER(name, interval_init, burst_init) {\
	.lock = PTHREAD_MUTEX_INITIALIZER,\
	.interval = interval_init,\
	.burst = burst_init,\
}

#define DEFINE_RATELIMIT_STATE_USER(name, interval_init, burst_init)\
	struct ratelimit_state_user name =\
		RATELIMIT_STATE_INIT_USER(name, interval_init, burst_init)\

/* IPCF log ratelimited print */
#define IPCF_HAL_LOGRatelimited(level, fmt, ...) ({\
	static DEFINE_RATELIMIT_STATE_USER(_rs,\
		RATELIMIT_INTERVAL_DEFAULT,\
		RATELIMIT_BURST_DEFAULT);\
	if (__ratelimit_user(&_rs, FILENAME, __func__, __LINE__)) {\
		switch (level) {\
		case LOG_FILE:\
		case LOG_VERBOSE: IPCF_HAL_LOGFILE(fmt, ##__VA_ARGS__); break;\
		case LOG_DEBUG: IPCF_HAL_LOGDEBUG(fmt, ##__VA_ARGS__); break;\
		case LOG_INFO: IPCF_HAL_LOGINFO(fmt, ##__VA_ARGS__); break;\
		case LOG_WARNING: IPCF_HAL_LOGWARN(fmt, ##__VA_ARGS__); break;\
		case LOG_ERROR: IPCF_HAL_LOGERR(fmt, ##__VA_ARGS__); break;\
		default: break;\
		}\
	}\
})

#endif/*IPCF_HAL_LOG_H*/
