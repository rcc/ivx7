/*
 * logging.h
 *
 * Copyright (C) 2010 Robert C. Curtis
 *
 * logging is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * logging is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with logging.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <prjutil.h>

#ifndef I__LOGGING_H__
	#define I__LOGGING_H__

#define LOGLEVEL_NONE		0
#define LOGLEVEL_ERROR		1
#define LOGLEVEL_WARNING	2
#define LOGLEVEL_INFO		3
#define LOGLEVEL_DEBUG		4
#define LOGLEVEL_VERBOSE	5

#ifndef MAX_LOGLEVEL
#define MAX_LOGLEVEL		LOGLEVEL_VERBOSE
#endif /* MAX_LOGLEVEL */

#ifndef DEFAULT_LOGLEVEL
#define DEFAULT_LOGLEVEL	LOGLEVEL_WARNING
#endif /* DEFAULT_LOGLEVEL */


/* LOGERROR */
#if MAX_LOGLEVEL >= LOGLEVEL_ERROR
#define logerror(fmt, args...)		_log(LOGLEVEL_ERROR, \
						"[E] %s(%d): " fmt, \
						__FUNCTION__, __LINE__, ## args)
#else
#define logerror(fmt, args...)
#endif

/* LOGWARN */
#if MAX_LOGLEVEL >= LOGLEVEL_WARNING
#define logwarn(fmt, args...)		_log(LOGLEVEL_WARNING, \
						"[W] %s(%d): " fmt, \
						__FUNCTION__, __LINE__, ## args)
#else
#define logwarn(fmt, args...)
#endif

/* LOGINFO */
#if MAX_LOGLEVEL >= LOGLEVEL_INFO
#define loginfo(fmt, args...)		_log(LOGLEVEL_INFO, fmt, ## args)
#else
#define loginfo(fmt, args...)
#endif

/* LOGDEBUG */
#if MAX_LOGLEVEL >= LOGLEVEL_DEBUG
#define logdebug(fmt, args...)		_log(LOGLEVEL_DEBUG, \
						"[D] %s(%d): " fmt, \
						__FUNCTION__, __LINE__, ## args)
#else
#define logdebug(fmt, args...)
#endif

/* LOGVERBOSE */
#if MAX_LOGLEVEL >= LOGLEVEL_VERBOSE
#define logverbose(fmt, args...)	_log(LOGLEVEL_VERBOSE, \
						"[V] %s(%d): " fmt, \
						__FUNCTION__, __LINE__, ## args)
#else
#define logverbose(fmt, args...)
#endif

extern FILE *logfds[];

void _log(int level, const char *fmt, ...) __printf_check(2, 3);
void set_loglevel(int level);

#endif /* I__LOGGING_H__ */
