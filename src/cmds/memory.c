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

static int get_uint_opt(const char *opt, uint32_t *val, struct dictionary *opts)
{
	const char *str = dict_str_for_key(opts, opt);
	if((str == NULL) || (strlen(str) == 0)) {
		return -1;
	}
	// Assume all numbers are base 10
	*val = (uint32_t)strtol(str, NULL, 10);
	return 0;
}

CMDHANDLER(memedit)
{
	struct vx7_clone_data *clone;
	struct vx7_mem_entry *e;
	uint32_t memidx;
	enum vx7_mem_type memtype;
	int new = 0;

	if(APPDATA->clone == NULL) {
		logerror("no loaded clone\n");
		return -1;
	}
	clone = APPDATA->clone;

	if(argc < 1) {
		logerror("invalid usage\n");
		return -1;
	}

	if(vx7if_mem_entry_with_name(argv[0], &memidx, &memtype) != 0) {
		logerror("invalid memory name\n");
		return -1;
	}
	logdebug("Memory Type: %d, Memory Index: %u\n", (int)memtype, memidx);
	if((e = vx7if_mem_entry(clone, memidx, memtype)) == NULL) {
		return -1;
	}

	/* If it's a new entry, enable it */
	if(!vx7if_mem_entry_valid(clone, memidx, memtype)) {
		new = 1;
		vx7if_mem_entry_set_status(clone, memidx, memtype,
				VX7_MEMSTATUS_VALID);
		vx7if_mem_entry_set_flag(clone, memidx, memtype,
				VX7_MEMFLAG_NORMAL);
	}

	/* Handle frequency */
	if(dict_has_key(opts, "freq")) {
		uint32_t freq;
		if(get_uint_opt("freq", &freq, opts) != 0) {
			logerror("must specify frequency value\n");
			return -1;
		}
		if(vx7if_mem_entry_set_freq(e, freq) != 0)
			return -1;
	}

	/* If it's a new entry, set the defaults now that the frequency is
	 * set.
	 */
	if(new) {
		if(vx7if_mem_entry_set_defaults(e) != 0)
			return -1;
	}

	/* Handle flags */
	if(dict_has_key(opts, "skip")) {
		if(vx7if_mem_entry_set_flag(clone, memidx, memtype,
					VX7_MEMFLAG_SKIP) != 0)
			return -1;
	} else if(dict_has_key(opts, "prefer")) {
		if(vx7if_mem_entry_set_flag(clone, memidx, memtype,
					VX7_MEMFLAG_PREFERRED) != 0)
			return -1;
	} else if(dict_has_key(opts, "normal")) {
		if(vx7if_mem_entry_set_flag(clone, memidx, memtype,
					VX7_MEMFLAG_NORMAL) != 0)
			return -1;
	}

	/* Handle Tag */
	if(dict_has_key(opts, "tag")) {
		if(vx7if_mem_entry_set_tag(e, dict_str_for_key(opts, "tag"))
				!= 0)
			return -1;
	}

	/* Handle Rx Mode */
	if(dict_has_key(opts, "rxmode")) {
		if(vx7if_mem_entry_set_rxmode(e,
					dict_str_for_key(opts, "rxmode")) != 0)
			return -1;
	}

	/* Handle Tx Power */
	if(dict_has_key(opts, "txpwr")) {
		if(vx7if_mem_entry_set_txpwr(e, dict_str_for_key(opts, "txpwr"))
				!= 0)
			return -1;
	}

	/* Handle Tx Mode */
	if(dict_has_key(opts, "txmode")) {
		if(vx7if_mem_entry_set_txmode(e,
					dict_str_for_key(opts, "txmode")) != 0)
			return -1;
	}

	/* Handle Tx Frequency */
	if(dict_has_key(opts, "txfreq")) {
		uint32_t freq;
		if(get_uint_opt("txfreq", &freq, opts) != 0) {
			logerror("must specify tx frequency value\n");
			return -1;
		}
		if(vx7if_mem_entry_set_txfreq(e, freq) != 0)
			return -1;
	}

	/* Handle Squelch Mode */
	if(dict_has_key(opts, "squelch")) {
		if(vx7if_mem_entry_set_squelch(e,
					dict_str_for_key(opts, "squelch")) != 0)
			return -1;
	}

	/* Handle CTCSS */
	if(dict_has_key(opts, "ctcss")) {
		uint32_t v;
		if(get_uint_opt("ctcss", &v, opts) != 0) {
			logerror("must specify ctcss tone value\n");
			return -1;
		}
		if(vx7if_mem_entry_set_ctcss(e, v) != 0)
			return -1;
	}

	/* Handle DCS */
	if(dict_has_key(opts, "dcs")) {
		uint32_t v;
		if(get_uint_opt("dcs", &v, opts) != 0) {
			logerror("must specify dcs code\n");
			return -1;
		}
		if(vx7if_mem_entry_set_dcs(e, v) != 0)
			return -1;
	}

	/* Handle Frequency Step*/
	if(dict_has_key(opts, "freqstep")) {
		uint32_t freq;
		if(get_uint_opt("freqstep", &freq, opts) != 0) {
			logerror("must specify frequency step value\n");
			return -1;
		}
		if(vx7if_mem_entry_set_freq_step(e, freq) != 0)
			return -1;
	}

	return 1;
}

START_CMD_OPTS(memedit_opts)
	CMD_OPT(tag, '\0', "tag",
			"set station name")
	CMD_OPT(freq, '\0', "freq",
			"set station frequency in Hz")
	CMD_OPT(normal, '\0', "normal",
			"make station flags normal")
	CMD_OPT(skip, '\0', "skip",
			"skip station during scan")
	CMD_OPT(prefer, '\0', "prefer",
			"make station preferred")
	CMD_OPT(rxmode, '\0', "rxmode",
			"set receive mode (N-FM, W-FM, AM)")
	CMD_OPT(txpwr, '\0', "txpwr",
			"set transmit power (L1, L2, L3, HI)")
	CMD_OPT(txmode, '\0', "txmode",
			"set transmit mode (SIMPLEX, +RPT, -RPT, INDEP_TX)")
	CMD_OPT(txfreq, '\0', "txfreq",
			"set transmit frequency in Hz (offset for RPT modes)")
	CMD_OPT(squelch, '\0', "squelch",
			"set squelch mode (NONE, TONE, TONE_SQL, DCS)")
	CMD_OPT(ctcss, '\0', "ctcss",
			"set ctcss tone frequency in tenth Hz")
	CMD_OPT(dcs, '\0', "dcs",
			"set dcs code")
	CMD_OPT(freqstep, '\0', "freqstep",
			"set frequency step in Hz")
END_CMD_OPTS;

APPCMD_OPT(memedit, &memedit, "edit a memory location",
		"usage: memedit [options] <memory location>\n"
		"  Memory Locations:\n"
		"    M001, M002, ..., M449, M450\n"
		"    OTM1, OTM2, ..., OTM9, OTM0\n"
		"    PMS_L01, PMS_U01, ..., PMS_L20, PMS_U20",
		NULL, memedit_opts);

CMDHANDLER(memcopy)
{
	struct vx7_clone_data *clone;
	struct vx7_mem_entry *e1, *e2;
	uint32_t memidx1, memidx2;
	enum vx7_mem_type memtype1, memtype2;

	if(APPDATA->clone == NULL) {
		logerror("no loaded clone\n");
		return -1;
	}
	clone = APPDATA->clone;

	if(argc < 2) {
		logerror("invalid usage\n");
		return -1;
	}

	/* Get Source Entry */
	if(vx7if_mem_entry_with_name(argv[0], &memidx1, &memtype1) != 0) {
		logerror("invalid memory name: %s\n", argv[0]);
		return -1;
	}
	if((e1 = vx7if_mem_entry(clone, memidx1, memtype1)) == NULL) {
		return -1;
	}

	/* Get Destination Entry */
	if(vx7if_mem_entry_with_name(argv[1], &memidx2, &memtype2) != 0) {
		logerror("invalid memory name: %s\n", argv[1]);
		return -1;
	}
	if((e2 = vx7if_mem_entry(clone, memidx2, memtype2)) == NULL) {
		return -1;
	}

	vx7if_mem_entry_set_status(clone, memidx2, memtype2,
			vx7if_mem_entry_get_status(clone, memidx1, memtype1));
	vx7if_mem_entry_set_flag(clone, memidx2, memtype2,
			vx7if_mem_entry_get_flag(clone, memidx1, memtype1));
	memcpy(e2, e1, sizeof(*e2));

	return 2;
}

APPCMD(memcopy, &memcopy, "copy a memory location",
		"usage: memcopy <source> <dest>\n"
		"  Memory Locations:\n"
		"    M001, M002, ..., M449, M450\n"
		"    OTM1, OTM2, ..., OTM9, OTM0\n"
		"    PMS_L01, PMS_U01, ..., PMS_L20, PMS_U20",
		NULL);


CMDHANDLER(meminvalidate)
{
	uint32_t i;

	if(APPDATA->clone == NULL) {
		logerror("no loaded clone\n");
		return -1;
	}

	/* Regular */
	for(i = 0; i < VX7_MEM_REGULAR_COUNT; i++) {
		if(vx7if_mem_entry_set_status(APPDATA->clone, i,
					VX7_MEM_REGULAR, VX7_MEMSTATUS_INVALID)
				!= 0) {
			return -1;
		}
		if(vx7if_mem_entry_set_flag(APPDATA->clone, i,
					VX7_MEM_REGULAR, VX7_MEMFLAG_NORMAL)
				!= 0) {
			return -1;
		}
	}
	/* PMS */
	for(i = 0; i < VX7_MEM_PMS_COUNT; i++) {
		if(vx7if_mem_entry_set_status(APPDATA->clone, i,
					VX7_MEM_PMS, VX7_MEMSTATUS_INVALID)
				!= 0) {
			return -1;
		}
		if(vx7if_mem_entry_set_flag(APPDATA->clone, i,
					VX7_MEM_PMS, VX7_MEMFLAG_NORMAL)
				!= 0) {
			return -1;
		}
	}
	/* One Touch */
	for(i = 0; i < VX7_MEM_ONETOUCH_COUNT; i++) {
		if(vx7if_mem_entry_set_status(APPDATA->clone, i,
					VX7_MEM_ONETOUCH, VX7_MEMSTATUS_INVALID)
				!= 0) {
			return -1;
		}
		if(vx7if_mem_entry_set_flag(APPDATA->clone, i,
					VX7_MEM_ONETOUCH, VX7_MEMFLAG_NORMAL)
				!= 0) {
			return -1;
		}
	}

	return 0;
}

APPCMD(meminvalidate, &meminvalidate, "invalidate all memory locations",
		"usage: meminvalidate", NULL);
