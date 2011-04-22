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

/*
 * cmds.c - Modular Command Support.
 */

#include <cmds.h>
#include <prjutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifndef CMDS_MAX_ARGUMENTS
#define CMDS_MAX_ARGUMENTS	64
#endif /* CMDS_MAX_ARGUMENTS */

LIST_HEAD(registered_cmds);

static const struct cmd *lookup_cmd(const char *name,
		struct list_head *cmd_list)
{
	struct cmd_mgr *pos;

	list_for_each_entry(pos, cmd_list, node) {
		if(strcmp(name, pos->cmd->name) == 0)
			return pos->cmd;
	}

	return NULL;
}

static int tokenize_cmd_string(char *argstr, char **argv, size_t max_args)
{
	char *tok, *brkt;
	char *sep = " \t";
	size_t i;

	for(tok = strtok_r(argstr, sep, &brkt), i = 0;
			tok && (i < (max_args - 1));
			tok = strtok_r(NULL, sep, &brkt), i++) {
		argv[i] = tok;
	}
	argv[i] = NULL;

	return (int)i;
}

int run_cmds(int argc, const char **argv, void *appdata)
{
	unsigned int carg = 0;
	int ret = 0;

	/* loop over all argv entries */
	while(carg < argc) {
		const struct cmd *cmd_entry =
			lookup_cmd(argv[carg++], &registered_cmds);
		if(cmd_entry == NULL) {
			logerror("could not find command '%s'\n",
					argv[carg-1]);
			return -ENOSYS;
		}
		logverbose("running command: %s\n", cmd_entry->name);

		/* call the command handler */
		if((ret = (cmd_entry->handler)(argc-carg, &argv[carg],
						cmd_entry, appdata)) < 0)
			return carg;
		carg += ret;
	}

	return 0;
}

int run_cmd(const char *name, int argc, const char **argv, void *appdata)
{
	const struct cmd *cmd_entry = lookup_cmd(name, &registered_cmds);
	if(cmd_entry == NULL) {
		logerror("could not find command '%s'\n", name);
		return -ENOSYS;
	}

	logverbose("running command: %s\n", cmd_entry->name);

	/* call the command handler */
	if((cmd_entry->handler)(argc, argv, cmd_entry, appdata) < 0)
		return 1;

	return 0;
}

int run_cmd_line(const char *cmd_line, void *appdata)
{
	int status = 0;
	char *s;
	int argc;
	char **argv;

	if((s = malloc(strlen(cmd_line) + 1)) == NULL) {
		logerror("could not allocate command line buffer");
		status = -ENOMEM;
		goto exit1;
	}
	if((argv = malloc(CMDS_MAX_ARGUMENTS * sizeof(argv[0]))) == NULL) {
		logerror("could not allocate argv");
		status = -ENOMEM;
		goto exit2;
	}
	strcpy(s, cmd_line);

	argc = tokenize_cmd_string(s, argv, CMDS_MAX_ARGUMENTS);

	if(argc) {
		status = run_cmd(argv[0], argc - 1, (const char **)&argv[1],
				appdata);
	} else {
		logerror("command line empty\n");
		status = -EINVAL;
	}

	free(argv);
exit2:
	free(s);
exit1:
	return status;
}

void _register_cmd(struct cmd_mgr *rcmd)
{
	struct cmd_mgr *pos;

	/* add command in alphabetical order */
	list_for_each_entry(pos, &registered_cmds, node) {
		if(strcmp(rcmd->cmd->name, pos->cmd->name) <= 0) {
			list_add_tail(&rcmd->node, &pos->node);
			return;
		}
	}
	list_add_tail(&rcmd->node, &registered_cmds);
}

CMDHANDLER(help)
{
	if(argc > 0) {
		const struct cmd *cmd_entry = lookup_cmd(argv[0],
				&registered_cmds);
		if(cmd_entry == NULL) {
			pcmderr("command `%s' not found.\n", argv[0]);
			return -1;
		}
		printf("Help for `%s' command:\n", cmd_entry->name);
		printf("%s\n", cmd_entry->help);
	} else {
		struct cmd_mgr *pos;
		int j;

		printf("Commands:\n");
		list_for_each_entry(pos, &registered_cmds, node) {
			printf("  %s ", pos->cmd->name);
			for(j = 0; j < (int)(14 - strlen(pos->cmd->name));
					j++)
				printf(" ");
			printf("%s\n", pos->cmd->summary);
		}

	}
	return argc;
}
APPCMD(help, &help, "Show this help. Also try `help <command>`",
	"usage: help [command]\n"
	"  `help' with no arguments will show a list of all commands.\n"
	"  `help <command>' will show a detailed help for `command'",
	NULL);
