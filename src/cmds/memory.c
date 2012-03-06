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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


CMDHANDLER(memedit)
{
	uint32_t memidx;
	enum vx7_mem_type memtype;

	if(APPDATA->clone == NULL) {
		logerror("no loaded clone\n");
		return -1;
	}

	if(argc < 1) {
		logerror("invalid usage\n");
		return -1;
	}

	if(vx7if_mem_entry_with_name(argv[0], &memidx, &memtype) != 0) {
		logerror("invalid memory name\n");
		return -1;
	}

	logdebug("Memory Type: %d, Memory Index: %u\n", (int)memtype, memidx);

	return 1;
}

START_CMD_OPTS(memedit_opts)
END_CMD_OPTS;

APPCMD_OPT(memedit, &memedit, "edit a memory location",
		"usage: memedit [options] <memory location>\n"
		"  Memory Locations:\n"
		"    M001, M002, ..., M449, M450\n"
		"    OTM1, OTM2, ..., OTM9, OTM0\n"
		"    PMS_L01, PMS_U01, ..., PMS_L20, PMS_U20",
		NULL, memedit_opts);


CMDHANDLER(meminvalidate)
{
	if(APPDATA->clone == NULL) {
		logerror("no loaded clone\n");
		return -1;
	}

	return 0;
}

APPCMD(meminvalidate, &meminvalidate, "invalidate all memory locations",
		"usage: meminvalidate", NULL);
