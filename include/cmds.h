/*
 * cmds.h - Modular Command Support.
 *
 * Copyright (C) 2008 Robert C. Curtis
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

/* DESCRIPTION: This header file describes data structures and functions for a
 * modular command system.
 */

#include <prjutil.h>
#include <logging.h>
#include <ll.h>

#ifndef I__CMDS_H__
	#define I__CMDS_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Command Data Structure */
struct cmd {
	/* The name of the command */
	const char *name;

	/* The handler function for the command.
	 * 	int argc	- number of arguments passed to handler
	 * 	char **argv	- arguments passed to handler
	 * 	struct cmd *cmd	- pointer to command structure
	 * 	void *appdata	- optional data passed by the the application
	 */
	int (*handler)(int argc, const char **argv, const struct cmd *cmd,
			void *appdata);

	/* A _short_ summary of the command. shown with list of commands */
	const char *summary;

	/* Text shown when help for a command is requested. */
	const char *help;

	/* pointer to private data structure for command */
	void *priv;
};

struct cmd_mgr {
	struct list_head node;
	const struct cmd *cmd;
};

/* Add a Command to Section - use this for defining commands */
#define APPCMD(name,handler,summary,help,priv) \
	static const struct cmd cmd_entry_ ## name = { \
		#name, \
		handler, \
		summary, \
		help, \
		priv }; \
	static void __constructor REGFUNC__ ## name(void) { \
		static struct cmd_mgr c; \
		c.cmd = &cmd_entry_ ## name; \
		_register_cmd(&c); \
	}


/* Command handler prototype
 * 	RETURN negative for error, otherwise number of arguments used
 */
#define CMDHANDLER(name) \
		static int name(int argc, const char **argv, \
				const struct cmd *cmd, void *appdata)

#define THISCMD cmd->name
/* Standard error print */
#define pcmderr(fmt, args...) logerror("%s: " fmt, THISCMD, ## args)


/* Run Command[s] Error Handling
 * 	run_cmd[s] will return 0 on success. A positive error code, n, means
 * 	that command number n encountered an error. A negative error code
 * 	means that the cmds framework encountered an error.
 * 		ENOSYS		- could not find command
 * 		ENOMEM		- could not allocate enough memory
 * 		EINVAL		- invalid argument
 * 	Note that 'command n' actually means argv[n].
 */

/* FUNCTION:    run_cmds
 *
 * + DESCRIPTION:
 *   - parses and runs a mixed argument list of commands and parameters
 *
 * + PARAMETERS:
 *   + int argc
 *     - number of arguments
 *   + const char **argv
 *     - argument list
 *   + void *appdata
 *     - appdata pointer to pass to each command run
 *
 * + RETURNS: int
 *   - see Error Handling above
 */
int run_cmds(int argc, const char **argv, void *appdata);

/* FUNCTION:    run_cmd
 *
 * + DESCRIPTION:
 *   - runs a single command with arguments
 *
 * + PARAMETERS:
 *   + const char *name
 *     - name of command
 *   + int argc
 *     - number of arguments
 *   + const char **argv
 *     - argument list
 *   + void *appdata
 *     - appdata pointer to pass to command
 *
 * + RETURNS: int
 *   - see Error Handling above
 */
int run_cmd(const char *name, int argc, const char **argv, void *appdata);

/* FUNCTION:    run_cmd_line
 *
 * + DESCRIPTION:
 *   - parse a string as a command line
 *
 * + PARAMETERS:
 *   + const char *cmd_line
 *     - command line string
 *   + void *appdata
 *     - appdata to pass to command
 *
 * + RETURNS: int
 *   - see Error Handling above
 */
int run_cmd_line(const char *cmd_line, void *appdata);


/* FUNCTION:    _register_cmd
 *
 * + DESCRIPTION:
 *   - adds a command to the command list. this is a low-level function that
 *   users should normally not need to call.
 *
 * + PARAMETERS:
 *   + struct cmd_mgr *rcmd
 *     - command structure to register
 */
void _register_cmd(struct cmd_mgr *rcmd);

#ifdef __cplusplus
}
#endif
#endif /* I__CMDS_H__ */
