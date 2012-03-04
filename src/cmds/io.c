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
#include <errno.h>

CMDHANDLER(load)
{
	int ret = 1;
	FILE *in;

	if(argc < 1) {
		logerror("must specify input filename\n");
		return -1;
	}

	/* Handle previously open clone */
	if(APPDATA->clone) {
		free(APPDATA->clone);
		APPDATA->clone = NULL;
	}

	/* Open input file */
	if((in = fopen(argv[0], "r")) == NULL) {
		logerror("could not open input file: %s\n", strerror(errno));
		return -1;
	}
	/* Allocate clone data buffer */
	if((APPDATA->clone = malloc(sizeof(*(APPDATA->clone)))) == NULL) {
		logerror("could not allocate buffer: %s\n", strerror(errno));
		fclose(in);
		return -1;
	}

	/* Read file */
	if(fread(APPDATA->clone, sizeof(*(APPDATA->clone)), 1, in) != 1) {
		logerror("invalid read size\n");
		ret = -1;
		goto exit;
	}

	/* Checksum */
	if(vx7if_checksum(APPDATA->clone) != APPDATA->clone->checksum) {
		logwarn("checksum is invalid (%02x != %02x)\n",
				APPDATA->clone->checksum,
				vx7if_checksum(APPDATA->clone));
	}

exit:
	fclose(in);
	return ret;
}

START_CMD_OPTS(load_opts)
END_CMD_OPTS;

APPCMD_OPT(load, &load, "load a clone file",
		"usage: load [OPTIONS] <clone file>",
		NULL, load_opts);

CMDHANDLER(save)
{
	int ret = 1;
	FILE *out;

	if(argc < 1) {
		logerror("must specify output filename\n");
		return -1;
	}

	if(APPDATA->clone == NULL) {
		logerror("no loaded clone to save\n");
		return -1;
	}

	/* Open output file */
	if((out = fopen(argv[0], "w")) == NULL) {
		logerror("could not open output file: %s\n", strerror(errno));
		return -1;
	}

	/* Set checksum */
	APPDATA->clone->checksum = vx7if_checksum(APPDATA->clone);
	logdebug("set checksum: 0x%02x\n", APPDATA->clone->checksum);

	/* Write file */
	if(fwrite(APPDATA->clone, sizeof(*(APPDATA->clone)), 1, out) != 1) {
		logerror("error writing file: %s\n", strerror(errno));
		ret = -1;
		goto exit;
	}

exit:
	fclose(out);
	return ret;
}

START_CMD_OPTS(save_opts)
END_CMD_OPTS;

APPCMD_OPT(save, &save, "save a clone file",
		"usage: save [OPTIONS] <clone file>",
		NULL, save_opts);
