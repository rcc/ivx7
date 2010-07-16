/*
 * logging.c
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
#include <logging.h>
#include <stdarg.h>

#ifdef LOG_WITH_NSLOG
#import <Foundation/Foundation.h>
#endif


FILE *logfd[MAX_LOGLEVEL + 1];

static int loglevel = min_of(DEFAULT_LOGLEVEL, MAX_LOGLEVEL);


void set_loglevel(int level)
{
	loglevel = min_of(level, MAX_LOGLEVEL);
}

void _log(int level, const char *fmt, ...)
{
	va_list argp;
	va_start(argp, fmt);

	if(level && (level <= loglevel)) {
#ifdef LOG_WITH_NSLOG
		NSLogv(fmt, argp);
#else
		vfprintf(logfd[level], fmt, argp);
#endif
	}

	va_end(argp);
}

static void __constructor _init_logging(void)
{
	int i;
	for(i = 0; i < num_elements(logfd); i++) {
		logfd[i] = stderr;
	}
}

