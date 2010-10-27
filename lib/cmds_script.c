/*
 * cmds_script.c - Script interface for cmds
 *
 * Copyright (C) 2010 Robert C. Curtis
 *
 * cmds is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * cmds is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cmds.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cmds.h>
#include <getline.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef CMDS_SCRIPT_LINE_LENGTH
#define CMDS_SCRIPT_LINE_LENGTH		1024
#endif /* CMDS_SCRIPT_LINE_LENGTH */

CMDHANDLER(script_handler)
{
	int status = 1;
	char *line;
	ssize_t len;
	int line_num = 1;
	FILE *fp = stdin;

	/* check arguments */
	if(!argc) {
		pcmderr("no file specified\n");
		status = -1;
		goto exit1;
	}
	/* try and open script file */
	if(strcmp(argv[0], "-") != 0) {
		if((fp = fopen(argv[0], "r")) == NULL) {
			perror("could not open script file");
			status = -1;
			goto exit1;
		}
	}
	/* allocate line buffer */
	if((line = malloc(CMDS_SCRIPT_LINE_LENGTH)) == NULL) {
		perror("could not allocate line buffer");
		status = -1;
		goto exit2;
	}

	/* loop through lines */
	while((len = fgetline(fp, line, CMDS_SCRIPT_LINE_LENGTH)) >= 0) {
		if((len > 0) && (line[0] != '#')) {
			if(run_cmd_line(line, appdata) != 0) {
				pcmderr("an error occured with line #%d\n",
						line_num);
				status = -1;
				break;
			}
		}
		line_num++;
	}

	/* clean up and exit */
	free(line);
exit2:
	if(fp != stdin)
		fclose(fp);
exit1:
	return status;
}
APPCMD(script, &script_handler,
	"run a command script",
	"usage: script <file>\n"
	"  a - for <file> will read from stdin",
	NULL);
