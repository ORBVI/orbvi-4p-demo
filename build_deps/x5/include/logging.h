/*
 * Horizon Robotics
 *
 *  Copyright (C) 2019 Horizon Robotics Inc.
 *  All rights reserved.
 *  Author: leye.wang<leye.wang@horizon.ai>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifndef INCLUDE_LOGGING_H_
#define INCLUDE_LOGGING_H_

#define ALOG_SUPPORT 1

#include <stdio.h>
#include <stdlib.h>
#ifdef ALOG_SUPPORT
#include <log.h>
#endif

#define STRINGIZE_NO_EXPANSION(x) #x
#define STRINGIZE(x) STRINGIZE_NO_EXPANSION(x)
#define HERE __FILE__ ":" STRINGIZE(__LINE__)

#ifndef SUBSYS_NAME
#define SUBSYS_NAME
#endif
#define SUBSYS STRINGIZE(SUBSYS_NAME)

#define L_FATAL "[FATAL][" SUBSYS "][" HERE "] "
#define L_INFO "[INFO][" SUBSYS "][" HERE "] "
#define L_WARNING "[WARNING][" SUBSYS "][" HERE "] "
#define L_ERROR "[ERROR][" SUBSYS "][" HERE "] "
#define L_DEBUG "[DEBUG][" SUBSYS "][" HERE "] "
#define L_VERBOSE "[VERBOSE][" SUBSYS "][" HERE "] "

/* output log by console */
#define CONSOLE_VERBOSE_LEVEL 	15
#define CONSOLE_DEBUG_LEVEL		14
#define CONSOLE_INFO_LEVEL		13
#define CONSOLE_WARNING_LEVEL	12
#define CONSOLE_ERROR_LEVEL		11
#define CONSOLE_FATAL_LEVEL		10

/* output log by ALOG */
#ifdef ALOG_SUPPORT
#define ALOG_VERBOSE_LEVEL		5
#define ALOG_DEBUG_LEVEL		4
#define ALOG_INFO_LEVEL			3
#define ALOG_WARNING_LEVEL		2
#define ALOG_ERROR_LEVEL		1
#define ALOG_FATAL_LEVEL		0
#else
#define ALOG_VERBOSE_LEVEL		CONSOLE_VERBOSE_LEVEL
#define ALOG_DEBUG_LEVEL		CONSOLE_DEBUG_LEVEL
#define ALOG_INFO_LEVEL			CONSOLE_INFO_LEVEL
#define ALOG_WARNING_LEVEL		CONSOLE_WARNING_LEVEL
#define ALOG_ERROR_LEVEL		CONSOLE_ERROR_LEVEL
#define ALOG_FATAL_LEVEL		CONSOLE_FATAL_LEVEL
#endif

#ifndef pr_fmt
#define pr_fmt(fmt)		fmt
#endif

#ifndef here_fmt
#define here_fmt(fmt)		"[%s:%d]" fmt, __func__, __LINE__
#endif

#ifndef ALOGV_TAG
#define ALOGV_TAG(tag, ...) ((void)ALOG(LOG_VERBOSE, tag, __VA_ARGS__))
#endif
#ifndef ALOGD_TAG
#define ALOGD_TAG(tag, ...) ((void)ALOG(LOG_DEBUG, tag, __VA_ARGS__))
#endif
#ifndef ALOGI_TAG
#define ALOGI_TAG(tag, ...) ((void)ALOG(LOG_INFO, tag, __VA_ARGS__))
#endif
#ifndef ALOGW_TAG
#define ALOGW_TAG(tag, ...) ((void)ALOG(LOG_WARN, tag, __VA_ARGS__))
#endif
#ifndef ALOGE_TAG
#define ALOGE_TAG(tag, ...) ((void)ALOG(LOG_ERROR, tag, __VA_ARGS__))
#endif
#ifndef ALOGF_TAG
#define ALOGF_TAG(tag, ...) ((void)ALOG(LOG_FATAL, tag, __VA_ARGS__))
#endif
#ifndef ALOGF
#define ALOGF(...) ((void)ALOG(LOG_FATAL, LOG_TAG, __VA_ARGS__))
#endif

/* get log level from environment variable */
/* we use console debug level in default */
#define LOGLEVEL_ENV	"LOGLEVEL"

static inline int get_loglevel(void)
{
	char *loglevel_env = NULL;
	int loglevel_value = CONSOLE_ERROR_LEVEL;

	loglevel_env = getenv(LOGLEVEL_ENV);
	if (loglevel_env != NULL) {
		loglevel_value = atoi(loglevel_env);
		/* loglevel value should in the configuration area */
		if (((loglevel_value >= CONSOLE_FATAL_LEVEL) &&
					(loglevel_value <= CONSOLE_VERBOSE_LEVEL)) ||
				((loglevel_value >= ALOG_FATAL_LEVEL) &&
				 (loglevel_value <= ALOG_VERBOSE_LEVEL)))
			return loglevel_value;
	}

	/* default log level */
	loglevel_value = CONSOLE_ERROR_LEVEL;

	return loglevel_value;
}

/* pr_info defintion */
#ifdef ALOG_SUPPORT
#define pr_info(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_INFO_LEVEL)		\
				fprintf(stdout, L_INFO "" pr_fmt(fmt), ##__VA_ARGS__);		\
			else if (loglevel >= ALOG_INFO_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL)	\
				ALOGI(fmt, ##__VA_ARGS__);		\
	} while (0);
#define pr_info_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_INFO_LEVEL) {\
				fprintf(stdout, "[INFO][" tag "]" here_fmt(fmt), ##__VA_ARGS__);		\
				fprintf(stdout, "\n");	\
			} else if ( loglevel >= ALOG_INFO_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL ) {\
				ALOGI_TAG(tag, here_fmt(fmt), ##__VA_ARGS__);		\
			}	\
	} while (0);
#else
#define pr_info(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_INFO_LEVEL)		\
				fprintf(stdout, L_INFO "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#define pr_info_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_INFO_LEVEL)		\
				fprintf(stdout, L_INFO "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#endif

/* pr_warn defintion */
#ifdef ALOG_SUPPORT
#define pr_warn(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_WARNING_LEVEL)		\
				fprintf(stdout, L_WARNING "" pr_fmt(fmt), ##__VA_ARGS__);		\
			else if (loglevel >= ALOG_WARNING_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL)	\
				ALOGW(fmt, ##__VA_ARGS__);		\
	} while (0);
#define pr_warn_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_WARNING_LEVEL) {		\
				fprintf(stdout, "[WARNING][" tag "]" here_fmt(fmt), ##__VA_ARGS__);		\
				fprintf(stdout, "\n");	\
			} else if (loglevel >= ALOG_WARNING_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL) { \
				ALOGW_TAG(tag, here_fmt(fmt), ##__VA_ARGS__);		\
			}	\
	} while (0);
#else
#define pr_warn(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_WARNING_LEVEL)		\
				fprintf(stdout, L_WARNING "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#define pr_warn_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_WARNING_LEVEL)		\
				fprintf(stdout, L_WARNING "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#endif

/* pr_err defintion */
#ifdef ALOG_SUPPORT
#define pr_err(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_ERROR_LEVEL)		\
				fprintf(stdout, L_ERROR "" pr_fmt(fmt), ##__VA_ARGS__);		\
			else if (loglevel >= ALOG_ERROR_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL)	\
				ALOGE(fmt, ##__VA_ARGS__);		\
	} while (0);
#define pr_err_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_ERROR_LEVEL) {		\
				fprintf(stdout, "[ERROR][" tag "]" here_fmt(fmt), ##__VA_ARGS__);		\
				fprintf(stdout, "\n");	\
			} else if (loglevel >= ALOG_ERROR_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL) {	\
				ALOGE_TAG(tag, here_fmt(fmt), ##__VA_ARGS__);	\
			}	\
	} while (0);
#else
#define pr_err(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_ERROR_LEVEL)		\
				fprintf(stdout, L_ERROR "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#define pr_err_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_ERROR_LEVEL)		\
				fprintf(stdout, L_ERROR "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#endif

/* pr_debug defintion */
#ifdef ALOG_SUPPORT
#define pr_debug(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_DEBUG_LEVEL)		\
				fprintf(stdout, L_DEBUG "" pr_fmt(fmt), ##__VA_ARGS__);		\
			else if (loglevel >= ALOG_DEBUG_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL)	\
				ALOGD(fmt, ##__VA_ARGS__);		\
	} while (0);
#define pr_debug_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_DEBUG_LEVEL) {		\
				fprintf(stdout, "[DEBUG][" tag "]" here_fmt(fmt), ##__VA_ARGS__);		\
				fprintf(stdout, "\n");	\
			} else if (loglevel >= ALOG_DEBUG_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL) {	\
				ALOGD_TAG(tag, here_fmt(fmt), ##__VA_ARGS__);		\
			}	\
	} while (0);
#else
#define pr_debug(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_DEBUG_LEVEL)		\
				fprintf(stdout, L_DEBUG "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#define pr_debug_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_DEBUG_LEVEL)		\
				fprintf(stdout, L_DEBUG "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#endif

/* pr_verbose defintion */
#ifdef ALOG_SUPPORT
#define pr_verbose(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_VERBOSE_LEVEL)		\
				fprintf(stdout, L_VERBOSE "" pr_fmt(fmt), ##__VA_ARGS__);		\
			else if (loglevel == ALOG_VERBOSE_LEVEL)		\
				ALOGV(fmt, ##__VA_ARGS__);		\
	} while (0);
#define pr_verbose_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_VERBOSE_LEVEL) {		\
				fprintf(stdout, "[VERBOSE][" tag "]" here_fmt(fmt), ##__VA_ARGS__);		\
				fprintf(stdout, "\n");	\
			} else if (loglevel == ALOG_VERBOSE_LEVEL) {	\
				ALOGV_TAG(tag, here_fmt(fmt), ##__VA_ARGS__);		\
			}	\
	} while (0);
#else
#define pr_verbose(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_VERBOSE_LEVEL)		\
				fprintf(stdout, L_VERBOSE "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#define pr_verbose_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_VERBOSE_LEVEL)		\
				fprintf(stdout, L_VERBOSE "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#endif

/* pr_fatal defintion */
#ifdef ALOG_SUPPORT
#define pr_fatal(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_FATAL_LEVEL)		\
				fprintf(stdout, L_FATAL "" pr_fmt(fmt), ##__VA_ARGS__);		\
			else if (loglevel >= ALOG_FATAL_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL)	\
				ALOGF(fmt, ##__VA_ARGS__);		\
	} while (0);
#define pr_fatal_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_FATAL_LEVEL) {		\
				fprintf(stdout, "[FATAL][" tag "]" here_fmt(fmt), ##__VA_ARGS__);		\
				fprintf(stdout, "\n");	\
			} else if (loglevel >= ALOG_FATAL_LEVEL && loglevel <= ALOG_VERBOSE_LEVEL) {	\
				ALOGF_TAG(tag, here_fmt(fmt), ##__VA_ARGS__);		\
			}	\
	} while (0);
#else
#define pr_fatal(fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_FATAL_LEVEL)		\
				fprintf(stdout, L_FATAL "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#define pr_fatal_with_tag(tag, fmt, ...)				\
	do {								\
			int loglevel = get_loglevel();		\
			if (loglevel >= CONSOLE_FATAL_LEVEL)		\
				fprintf(stdout, L_FATAL "" pr_fmt(fmt), ##__VA_ARGS__);		\
	} while (0);
#endif

#endif /* INCLUDE_LOGGING_H_ */
