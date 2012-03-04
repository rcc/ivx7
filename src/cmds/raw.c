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
#include <vx7if/vx7if.h>
#include <hexdump.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


CMDHANDLER(dump)
{
	if(APPDATA->clone == NULL) {
		logerror("no loaded clone\n");
		return -1;
	}

	hexdump(stdout, APPDATA->clone, sizeof(*APPDATA->clone));

	return 0;
}

APPCMD(dump, &dump, "hexdump a clone", "usage: dump", NULL);

CMDHANDLER(raw)
{
	int ret = 1;

	if(APPDATA->clone == NULL) {
		logerror("no loaded clone\n");
		return -1;
	}

	if(argc < 1) {
		logerror("invalid usage\n");
		return -1;
	}

	/* Read */
	if(strcmp("read", argv[0]) == 0) {
		uint32_t addr;

		if(argc < 2) {
			logerror("invalid usage\n");
			return -1;
		}
		ret = 2;

		if((addr = (uint32_t)strtoul(argv[1], NULL, 0)) >=
				sizeof(*APPDATA->clone)) {
			logerror("address out of range\n");
			return -1;
		}

		printf("0x%02X\n", ((uint8_t *)(APPDATA->clone))[addr]);

	/* Write */
	} else if(strcmp("write", argv[0]) == 0) {
		uint32_t addr;

		if(argc < 3) {
			logerror("invalid usage\n");
			return -1;
		}
		ret = 3;

		if((addr = (uint32_t)strtoul(argv[1], NULL, 0)) >=
				sizeof(*APPDATA->clone)) {
			logerror("address out of range\n");
			return -1;
		}

		((uint8_t *)(APPDATA->clone))[addr] =
			(uint8_t)strtoul(argv[2], NULL, 0);

	} else {
		logerror("invalid operation\n");
		return -1;
	}

	return ret;
}

APPCMD(raw, &raw, "raw read and write operations",
		"usage: raw <read> <addr>\n"
		"       raw <write> <addr> <val>", NULL);
