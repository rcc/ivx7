/*
 * cmds.h - Modular Command Support.
 *
 * Copyright (C) 2008 Robert C. Curtis
 *
 * cmds.h is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * cmds.h is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with cmds.h.  If not, see <http://www.gnu.org/licenses/>.
 */

/****************************************************************************/

/* DESCRIPTION: This header file describes data structures and functions for a
 * modular command system.
 */

#include <ll.h>

#ifndef I__CMDS_H__
	#define I__CMDS_H__

/* Command Data Structure */
typedef struct cmd_t {
	/* The name of the command */
	const char *name;

	/* The handler function for the command.
	 * 	int argc	- number of arguments passed to handler
	 * 	char **argv	- arguments passed to handler
	 * 	cmd_t *cmd	- pointer to command structure
	 * 	void *appdata	- optional data passed by the the application
	 */
	int (*handler)(int argc, const char **argv, const struct cmd_t *cmd,
			void *appdata);

	/* A _short_ summary of the command. shown with list of commands */
	const char *summary;

	/* Text shown when help for a command is requested. */
	const char *help;

	/* pointer to private data structure for command */
	void *priv;
} cmd_t;

typedef struct reg_cmd_t {
	ll_t node;
	const cmd_t *cmd;
} reg_cmd_t;

/* Add a Command to Section - use this for defining commands */
#define APPCMD(name,handler,summary,help,priv) \
	static const cmd_t cmd_entry_ ## name = { \
		#name, \
		handler, \
		summary, \
		help, \
		priv }; \
	static void __constructor REGFUNC__ ## name(void) { \
		static reg_cmd_t c; \
		c.cmd = &cmd_entry_ ## name; \
		_register_cmd(&c); \
	}


/* Command handler prototype
 * 	RETURN -1 for error, otherwise number of arguments used
 */
#define CMDHANDLER(name) \
		int name(int argc, const char **argv, const cmd_t *cmd, \
				void *appdata)

#define THISCMD cmd->name

/* Lookup Command
 * 	name		- name of command to lookup
 * 	cmd_list	- list of reg_cmd_t structures
 *
 * 	RETURNS pointer to matched command on success, NULL of failure
 */
const cmd_t *lookup_cmd(const char *name, ll_t *cmd_list);

/* Run Commands
 * 	argc		- number of arguments in argv
 * 	argv		- list of commands and arguments
 * 	appdata		- data to pass to each command
 */
int run_cmds(int argc, const char **argv, void *appdata);

void _register_cmd(reg_cmd_t *rcmd);

#endif /* I__CMDS_H__ */
