/*
 * Copyright 2012 Robert C. Curtis. All rights reserved.
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

#include <app.h>
#include <cmds.h>
#include <dict.h>
#include <serial/device_chooser.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

CMDHANDLER(device)
{
	int args = 0;
	char devpath[1024] = {'\0'};
	uint32_t baud = 19200;

	/* Check for baud option */
	if(dict_has_key(opts, "baud")) {
		const char *val = dict_str_for_key(opts, "baud");
		if((val == NULL) || (strlen(val) == 0)) {
			logerror("must specify baud value\n");
			return -1;
		}
		baud = (uint32_t)strtol(val, NULL, 10);
	}

	/* Get device path */
	if(dict_has_key(opts, "choose")) {
		if(serial_device_chooser(&devpath[0], sizeof(devpath)) != 0) {
			return -1;
		}
	} else {
		args = 1;
		if(argc < 1) {
			logerror("must specify device or use choose option\n");
			return -1;
		}
		strncpy(&devpath[0], argv[0], sizeof(devpath));
	}

	logdebug("Device: %s\n", &devpath[0]);
	logdebug("Baud: %u\n", baud);

	/* Close any open device */
	if(APPDATA->dev) {
		loginfo("Closing already open device\n");
		serial_close(APPDATA->dev);
	}
	if((APPDATA->dev = serial_open(&devpath[0], baud, 1)) == NULL) {
		return -1;
	}

	return args;
}

START_CMD_OPTS(device_opts)
	CMD_OPT(choose, 'c', "choose", "choose device from list")
	CMD_OPT(baud, '\0', "baud", "set baudrate (default: 19200)")
END_CMD_OPTS;

APPCMD_OPT(device, &device, "open serial device",
		"usage: device [OPTIONS] <device>",
		NULL, device_opts);
