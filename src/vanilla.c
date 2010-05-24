/*
 * vanilla.c
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
#include <prjutil.h>
#include <cmds.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

int main(int argc, const char * argv[])
{
	int status = 0;
	int ret;
	char *arg0, *cmdname;

	/* create the command string with basename() */
	if((arg0 = malloc(strlen(argv[0]))) == NULL) {
		perror("Could not allocate arg0");
		status = 1;
		goto exit1;
	}
	strcpy(arg0, argv[0]);
	if((cmdname = basename(arg0)) == NULL) {
		perror("Could not get basename of command");
		status = 1;
		goto exit2;
	}

	pdebug("Command: %s\n", cmdname);
	pdebug("Version: %s\n", SCM_HASH);

	if(strcmp(__TARGET__, cmdname) == 0) {
		if((ret = run_cmds(argc-1, &argv[1], NULL)) != 0) {
			if(ret < 0) {
				printe("An error occurred with command: %s\n",
					argv[-ret]);
			} else {
				printe("Could not find command: %s\n",
					argv[ret]);
			}
			status = 1;
			goto exit2;
		}
	} else {
		/* treat the argv[0] command name as a command */
		if((ret = run_cmd(cmdname, argc - 1, &argv[1], NULL)) != 0) {
			if(ret < 0) {
				printe("An error occurred with command: %s\n",
					cmdname);
			} else {
				printe("Could not find command: %s\n", cmdname);
			}
			status = 1;
			goto exit2;
		}
	}

exit2:
	free(arg0);
exit1:
	return status;
}

CMDHANDLER(version)
{
	printf("Version: %s\n", SCM_HASH);
	return 0;
}
APPCMD(version, &version, "print the version", "usage: version", NULL);
