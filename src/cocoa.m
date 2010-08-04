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

/*
 * Application Data Structure
 * 	This data structure is passed around to all commands and pre/post
 * 	command functions.
 */
static struct appdata_priv apppriv;

/*
 * Default Commands
 * 	These commands get run if no arguments are passed to the program, and
 * 	the program name == TARGET.
 */
const char *default_cmds[] = {
	"version",
	"help",
};

/*
 * Pre-command Functions
 * 	These functions get run before any commands are processed. They should
 * 	return 0 on success.
 */
int (*precmdfuncs[])(struct appdata_priv *priv) = {
};

/*
 * Post-command Functions
 * 	These functions get run after all commands are processed. They should
 * 	return 0 on success.
 */
int (*postcmdfuncs[])(struct appdata_priv *priv) = {
};

int main(int argc, const char * argv[])
{
	NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
	int status = 0;
	int i;
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

	/* run the pre-command functions */
	for(i = 0; i < ARRAY_SIZE(precmdfuncs); i++) {
		logverbose("running pre-command function %d\n", i);
		if(precmdfuncs[i](&apppriv) != 0) {
			logerror("pre-command function %d returned error\n", i);
			status = 1;
			goto exit2;
		}
	}

	/* process commands */
	if(strcmp(__TARGET__, cmdname) == 0) {
		if(argc == 1) {
			for(i = 0; i < ARRAY_SIZE(default_cmds); i++) {
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

	/* run the post-command functions */
	for(i = 0; i < ARRAY_SIZE(postcmdfuncs); i++) {
		logverbose("running post-command function %d\n", i);
		if(postcmdfuncs[i](&apppriv) != 0) {
			logerror("post-command function %d returned error\n",
					i);
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
