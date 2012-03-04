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
#include <hexdump.h>
#include <vx7if/vx7if.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static void print_memory_entry(const struct vx_mem_entry *e)
{
	printf("%s : %s", e->name, e->tag);
	if(e->flag_status == MEMFLAG_STATUS_MASKED)
		printf(" [HIDDEN]");
	if(e->flag_preferential)
		printf(" [PREFERENTIAL]");
	if(e->flag_skip)
		printf(" [SKIP]");
	printf("\n");
	printf("  Freq:      %u\n", e->freq_hz);
	printf("  Tx Freq:   %u\n", e->tx_freq_hz);
	printf("  Freq Step: %u\n", e->freq_step);
	printf("  Tx Mode:   %s\n", e->tx_mode);
	printf("  Power:     %s\n", e->tx_pwr);
	printf("  Rx Mode:   %s\n", e->rx_mode);
	printf("  Squelch:   %s\n", e->squelch);
	printf("  CTCSS:     %u (tenth Hz)\n", e->ctcss_tenth_hz);
	printf("  DCS:       %03u\n", e->dcs);
	printf("\n");
}

CMDHANDLER(cloneinfo)
{
	int ret = 1;
	struct vx7_clone_data *clone;
	FILE *in;
	int i;
	struct vx_mem_entry e;

	if(argc < 1) {
		logerror("must specify input filename\n");
		return -1;
	}

	/* Open input file */
	if((in = fopen(argv[0], "r")) == NULL) {
		logerror("could not open input file: %s\n", strerror(errno));
		return -1;
	}
	/* Allocate clone data buffer */
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

	/* Size */
	printf("Clone size: %u (0x%x) bytes\n",
			(uint32_t)sizeof(*clone),
			(uint32_t)sizeof(*clone));

	/* Checksum */
	if(vx7if_checksum(clone) != clone->checksum) {
		printf("Checksum is invalid (%02x != %02x)\n", clone->checksum,
				vx7if_checksum(clone));
		goto exit;
	} else {
		printf("Checksum is valid: %02x\n", clone->checksum);
	}

	/* Memory Locations */
	/* Regular */
	for(i = 0; i < ARRAY_SIZE(clone->regular); i++) {
		if(vx7if_mem_entry_valid(clone, i, VX7_MEM_REGULAR)) {
			if(vx7if_mem_entry(clone, &e, i, VX7_MEM_REGULAR) != 0)
				continue;
			print_memory_entry(&e);
		}
	}
	/* PMS */
	for(i = 0; i < ARRAY_SIZE(clone->pms); i++) {
		if(vx7if_mem_entry_valid(clone, i, VX7_MEM_PMS)) {
			if(vx7if_mem_entry(clone, &e, i, VX7_MEM_PMS) != 0)
				continue;
			print_memory_entry(&e);
		}
	}
	/* One Touch */
	for(i = 0; i < ARRAY_SIZE(clone->one_touch); i++) {
		if(vx7if_mem_entry_valid(clone, i, VX7_MEM_ONETOUCH)) {
			if(vx7if_mem_entry(clone, &e, i, VX7_MEM_ONETOUCH) != 0)
				continue;
			print_memory_entry(&e);
		}
	}

exit:
	free(clone);
	fclose(in);
	return ret;
}

START_CMD_OPTS(cloneinfo_opts)
END_CMD_OPTS;

APPCMD_OPT(info, &cloneinfo, "display information for a clone file",
		"usage: info [OPTIONS] <input file>",
		NULL, cloneinfo_opts);
