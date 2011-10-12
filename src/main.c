/*
 * Copyright 2011 Robert C. Curtis. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *    1. Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *    2. Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials
 *       provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY ROBERT C. CURTIS ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL ROBERT C. CURTIS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * The views and conclusions contained in the software and documentation
 * are those of the authors and should not be interpreted as representing
 * official policies, either expressed or implied, of Robert C. Curtis.
 */

#include "app.h"
#include <prjutil.h>
#include <logging.h>
#include <cmds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

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
	"version --pretty",
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
	int status = 0;
	int i;
	char *arg0, *cmdname;

	/* create the command string with basename() */
	if((arg0 = malloc(strlen(argv[0]) + 1)) == NULL) {
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
	return status;
}
