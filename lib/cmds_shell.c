/*
 * cmds_shell.c - Shell interface for cmds
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>

/* shell colors */
#define COLOR_NO	"\x1b[00;00m"
#define COLOR_BR 	"\x1b[00;01m"
#define COLOR_RD 	"\x1b[00;31m"
#define COLOR_GR 	"\x1b[00;32m"
#define COLOR_YL 	"\x1b[00;33m"
#define COLOR_BL 	"\x1b[00;34m"
#define COLOR_MG 	"\x1b[00;35m"
#define COLOR_CY 	"\x1b[00;36m"

/* shell prompt */
#ifndef CMDS_SHELL_PROMPT
#define CMDS_SHELL_PROMPT	COLOR_CY __TARGET__ COLOR_YL " $ " COLOR_NO
#endif /* CMDS_SHELL_PROMPT */

CMDHANDLER(shell_handler)
{
	int status = 0;
	char *line;

	/* loop through lines */
	while((line = readline(CMDS_SHELL_PROMPT)) != NULL) {
		if(strlen(line)) {
			run_cmd_line(line, appdata);
			add_history(line);
		}
		free(line);
	}

	printf("\n");

	return status;
}
APPCMD(shell, &shell_handler,
	"enter a command shell",
	"usage: script",
	NULL);
