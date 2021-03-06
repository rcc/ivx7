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

CMDHANDLER(clonerx)
{
	int ret = 1;
	struct vx7_clone_data *clone;
	ssize_t l;
	FILE *out;

	if(APPDATA->dev == NULL) {
		logerror("no open device\n");
		return -1;
	}

	if(argc < 1) {
		logerror("must specify output filename\n");
		return -1;
	}

	/* Open output file */
	if((out = fopen(argv[0], "w")) == NULL) {
		logerror("could not open output file: %s\n", strerror(errno));
		return -1;
	}
	/* Allocate clone buffer */
	if((clone = malloc(sizeof(*clone))) == NULL) {
		logerror("could not allocate buffer: %s\n", strerror(errno));
		fclose(out);
		return -1;
	}

	printf("Power on device while holding [MON]\n");
	printf("Screen should read \"CLONE\"\n");
	printf("Press [BAND]\n");

	if((l = vx7if_clone_receive(APPDATA->dev, clone)) > 0) {
		printf("Writing file...\n");
		if(fwrite(clone, 1, l, out) != l) {
			logerror("error writing file: %s\n", strerror(errno));
			ret = -1;
			goto exit;
		}
		printf("Complete\n");
	} else {
		logerror("error receiving from device\n");
		ret = -1;
		goto exit;
	}

exit:
	free(clone);
	fclose(out);
	return ret;
}

APPCMD(clonerx, &clonerx, "read configuration from device",
		"usage: clonerx <output file>",
		NULL);

CMDHANDLER(clonetx)
{
	int ret = 1;
	struct vx7_clone_data *clone;
	FILE *in;

	if(APPDATA->dev == NULL) {
		logerror("no open device\n");
		return -1;
	}

	if(argc < 1) {
		logerror("must specify input filename\n");
		return -1;
	}

	/* Open input file */
	if((in = fopen(argv[0], "r")) == NULL) {
		logerror("could not open input file: %s\n", strerror(errno));
		return -1;
	}
	/* Allocate clone buffer */
	if((clone = malloc(sizeof(*clone))) == NULL) {
		logerror("could not allocate buffer: %s\n", strerror(errno));
		fclose(in);
		return -1;
	}

	/* Read file */
	if(fread(clone, 1, sizeof(*clone), in) != sizeof(*clone)) {
		logerror("invalid read size\n");
		ret = -1;
		goto exit;
	}

	printf("Power on device while holding [MON]\n");
	printf("Screen should read \"CLONE\"\n");
	printf("Press [V/M]\n");
	printf("Screen should read \"CLONE WAIT\"\n");
	printf("press 'p' then enter: ");
	while(getc(stdin) != 'p');

	if(vx7if_clone_send(APPDATA->dev, clone) != 0) {
		logerror("clone error\n");
		ret = -1;
		goto exit;
	} else {
		printf("Complete\n");
	}

exit:
	free(clone);
	fclose(in);
	return ret;
}

APPCMD(clonetx, &clonetx, "send configuration to device",
		"usage: clonetx <input file>",
		NULL);
