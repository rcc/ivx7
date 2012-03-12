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
	*val = (uint32_t)strtol(str, NULL, 0);
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
	}

	/* Handle Tag */
	if(dict_has_key(opts, "tag")) {
		if(vx7if_mem_entry_set_tag(e, dict_str_for_key(opts, "tag"))
				!= 0)
			return -1;
	}

	return 1;
}

START_CMD_OPTS(memedit_opts)
	CMD_OPT(tag, '\0', "tag", "set station name")
	CMD_OPT(freq, '\0', "freq", "set station frequency in Hz")
	CMD_OPT(skip, '\0', "skip", "skip station during scan")
	CMD_OPT(prefer, '\0', "prefer", "make station preferred")
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
