/*
 * cmds.c - Modular Command Support.
 *
 * Copyright (C) 2008 Robert C. Curtis
 *
 * cmds.c is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * cmds.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cmds.c.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <cmds.h>
#include <string.h>
#include <stdio.h>

LIST_HEAD(registered_cmds);

static const cmd_t *lookup_cmd(const char *name, ll_t *cmd_list)
{
	reg_cmd_t *pos;

	list_for_each_entry(pos, cmd_list, node) {
		if(strcmp(name, pos->cmd->name) == 0)
			return pos->cmd;
	}

	return NULL;
}

int run_cmds(int argc, const char **argv, void *appdata)
{
	unsigned int carg = 0;
	int ret = 0;

	/* loop over all argv entries */
	while(carg < argc) {
		const cmd_t *cmd_entry =
			lookup_cmd(argv[carg++], &registered_cmds);
		if(cmd_entry == NULL)
			return -carg;

		/* call the command handler */
		if((ret = (cmd_entry->handler)(argc-carg, &argv[carg],
						cmd_entry, appdata)) < 0)
			return -carg;
		carg += ret;
	}

	return 0;
}

int run_cmd(const char *name, int argc, const char **argv, void *appdata)
{
	const cmd_t *cmd_entry = lookup_cmd(name, &registered_cmds);
	if(cmd_entry == NULL)
		return -1;

	/* call the command handler */
	if((cmd_entry->handler)(argc, argv, cmd_entry, appdata) < 0)
		return -1;

	return 0;
}

void _register_cmd(reg_cmd_t *rcmd)
{
	list_add(&rcmd->node, &registered_cmds);
}

CMDHANDLER(help)
{
	if(argc > 0) {
		const cmd_t *cmd_entry = lookup_cmd(argv[0], &registered_cmds);
		if(cmd_entry == NULL) {
			printf("%s: Command `%s' not found.\n", THISCMD,
					argv[0]);
			return -1;
		}
		printf("Help for `%s' command:\n", cmd_entry->name);
		printf("%s\n", cmd_entry->help);
	} else {
		reg_cmd_t *pos;
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
