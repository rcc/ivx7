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
#include <cmds.h>
#include <stdarg.h>
#include <stdlib.h>

#ifdef LOG_WITH_NSLOG
extern void _nslog_hook(const char *fmt, va_list ap);
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
		_nslog_hook(fmt, argp);
#else
		vfprintf(logfd[level], fmt, argp);
#endif
	}

	va_end(argp);
}

/* We need an init constructor to initialize the file descriptor array with
 * non-constant values
 */
static void __constructor _init_logging(void)
{
	int i;
	for(i = 0; i < num_elements(logfd); i++) {
		logfd[i] = stderr;
	}
}

/* Some commands to handle changing logging behavior during run-time */
CMDHANDLER(setloglevel)
{
	int req_level;

	/* check arguments */
	if(!argc) {
		pcmderr("no level specified\n");
		return -1;
	}
	/* parse log level */
	req_level = atoi(argv[0]);
	if((req_level < 0) || (req_level > MAX_LOGLEVEL)) {
		pcmderr("invalid level specified. max: %d\n", MAX_LOGLEVEL);
		return -1;
	}

	loglevel = req_level;

	return 1;
}
APPCMD(loglevel, &setloglevel, "set the log level",
		"usage: loglevel <level>\n"
		"  0 - NONE\n"
		"  1 - ERROR\n"
		"  2 - WARNING\n"
		"  3 - INFO\n"
		"  4 - DEBUG\n"
		"  5 - VERBOSE",
		NULL);
