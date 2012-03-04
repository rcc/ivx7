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

/* NAME TAG HIDDEN NORMAL/PREFERENTIAL/SKIP FREQ TXFREQ TXMODE POWER RXMODE SQUELCH CTCSS DCS FREQSTEP*/
static void print_memory_entry(const struct vx_mem_entry *e)
{
	printf("%s\t%s", e->name, e->tag);
	printf("\t%c", (e->flag_status == MEMFLAG_STATUS_MASKED) ? 'Y' : 'N');
	if(e->flag_preferential)
		printf("\tP");
	else if(e->flag_skip)
		printf("\tS");
	else
		printf("\tN");
	printf("\t%u", e->freq_hz);
	printf("\t%u", e->tx_freq_hz);
	printf("\t%s", e->tx_mode);
	printf("\t%s", e->tx_pwr);
	printf("\t%s", e->rx_mode);
	printf("\t%s", e->squelch);
	printf("\t%u", e->ctcss_tenth_hz);
	printf("\t%u", e->dcs);
	printf("\t%u", e->freq_step);
	printf("\n");
}

CMDHANDLER(memtsv)
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

	/* Checksum */
	if(vx7if_checksum(clone) != clone->checksum) {
		logerror("checksum is invalid (%02x != %02x)\n",
				clone->checksum, vx7if_checksum(clone));
		goto exit;
	}

	/* Memory Locations */
	printf("# NAME\tTAG\tHIDDEN\tNORM/PREFER/SKIP\tFREQ\tTX FREQ\tTX MODE\tPOWER\tRX MODE\tSQUELCH\tCTCSS (tenth Hz)\tDCS\tFREQ STEP\n");
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

START_CMD_OPTS(memtsv_opts)
END_CMD_OPTS;

APPCMD_OPT(memtsv, &memtsv, "generate a sparse tsv for memory entries",
		"usage: memtsv [OPTIONS] <clone file>",
		NULL, memtsv_opts);
