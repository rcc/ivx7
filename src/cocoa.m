/*
 * cocoa.m
 *
 * Copyright (C) 2010 Robert C. Curtis
 *
 * <prjstart> is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * <prjstart> is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with <prjstart>.  If not, see <http://www.gnu.org/licenses/>.
 */
#import <Foundation/Foundation.h>

#import "prjstart.h"
#import <prjutil.h>
#import <cmds.h>

#import <stdio.h>
#import <stdlib.h>
#import <string.h>
#import <libgen.h>

/* application's data structure */
static struct appdata_priv apppriv;
static void init_appdata_priv(struct appdata_priv *priv);

/* command strings that get run if program given no arguments */
const char *default_cmds[] = {
	"version",
	"help",
};

int main(int argc, const char * argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	int status = 0;
	char *arg0, *cmdname;

	/* create the command string with basename() */
	if((arg0 = malloc(strlen(argv[0]))) == NULL) {
		logerror("Could not allocate arg0");
		status = 1;
		goto exit1;
	}
	strcpy(arg0, argv[0]);
	if((cmdname = basename(arg0)) == NULL) {
		logerror("Could not get basename of command");
		status = 1;
		goto exit2;
	}

	logverbose("Command: %s\n", cmdname);

	init_appdata_priv(&apppriv);

	if(strcmp(__TARGET__, cmdname) == 0) {
		if(argc == 1) {
			int i;
			for(i = 0; i < num_elements(default_cmds); i++) {
				run_cmd_line(default_cmds[i], &apppriv);
			}

		} else if(run_cmds(argc - 1, &argv[1], &apppriv) != 0) {
			status = 1;
			goto exit2;
		}
	} else {
		/* treat the argv[0] command name as a command */
		if(run_cmd(cmdname, argc - 1, &argv[1], &apppriv) != 0) {
			status = 1;
			goto exit2;
		}
	}

exit2:
	free(arg0);
exit1:
	[pool drain];
	return status;
}

static void init_appdata_priv(struct appdata_priv *priv)
{
	logverbose("initializing application data: %p\n", priv);
}

CMDHANDLER(version)
{
	printf("Version:  %s\n", VERSION);
	return 0;
}
APPCMD(version, &version, "print the version", "usage: version", NULL);

#ifdef LOG_WITH_NSLOG
/* The logging framework needs a cocoa hook to make this work */
#import <stdarg.h>
void _nslog_hook(const char *fmt, va_list ap)
{
	NSString *NSfmt = [[NSString alloc] initWithUTF8String:fmt];
	NSLogv(NSfmt, ap);
	[NSfmt release];
}
#endif
