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

#import <prjutil.h>
#import <cmds.h>

#import <stdio.h>

int main(int argc, const char * argv[])
{
	NSAutoreleasePool * pool = [[NSAutoreleasePool alloc] init];
	int ret;
	int status = 0;

	logdebug("Version: %s\n", SCM_HASH);

	if((ret = run_cmds(argc-1, &argv[1], NULL)) < 0) {
		fprintf(stderr,
			"An error occurred with command at position %d\n",
			-ret);
		status = -1;
		goto exit;
	}

exit:
	[pool drain];
	return status;
}

CMDHANDLER(version)
{
	printf("Version: %s\n", SCM_HASH);
	return 0;
}
APPCMD(version, &version, "print the version", "usage: version", NULL);
