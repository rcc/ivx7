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

#include "vx7if.h"
#include "vx7_clone_data.h"

#include <prjutil.h>
#include <logging.h>
#include <serial/serial.h>
#include <timelib.h>

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifndef VX7_INTER_BYTE_DELAY
#define VX7_INTER_BYTE_DELAY	8000
#endif

/* CTCSS Table in tenths of Hz */
static const uint16_t ctcss_table[] = {
	670,	693,	719,	744,	770,	797,	825,	854,	885,
	915,	948,	974,	1000,	1035,	1072,	1109,	1148,	1188,
	1230,	1273,	1318,	1365,	1413,	1462,	1514,	1567,	1598,
	1622,	1655,	1679,	1713,	1738,	1773,	1799,	1835,	1862,
	1899,	1928,	1966,	1995,	2035,	2065,	2107,	2181,	2257,
	2291,	2336,	2418,	2503,	2541,
};

static const uint16_t dcs_table[] = {
	23,	25,	26,	31,	32,	36,	43,	47,	51,
	53,	54,	65,	71,	72,	73,	74,	114,	115,
	116,	122,	125,	131,	132,	134,	143,	145,	152,
	155,	156,	162,	165,	172,	174,	205,	212,	223,
	225,	226,	243,	244,	245,	246,	251,	252,	255,
	261,	263,	265,	266,	271,	274,	306,	311,	315,
	325,	331,	332,	343,	346,	351,	356,	364,	365,
	371,	411,	412,	413,	423,	431,	432,	445,	446,
	452,	454,	455,	462,	464,	465,	466,	503,	506,
	516,	523,	526,	532,	546,	565,	606,	612,	624,
	627,	631,	632,	654,	662,	664,	703,	712,	723,
	731,	732,	734,	743,	754,
};

const char *txpwr_table[] = {
	"L1", "L2", "L3", "HI",
};

const char *txmode_table[] = {
	"SIMPLEX", "-RPT", "+RPT", "INDEP_TX",
};

static const uint32_t freqstep_table[] = {
	5000, 10000, 12500, 15000, 20000, 25000, 50000, 100000, 9000,
};

const char *rxmode_table[] = {
	"N-FM", "AM", "W-FM",
};

const char *squelch_table[] = {
	"NONE", "TONE", "TONE_SQL", "DCS",
};

/*********************************** Data ***********************************/
uint8_t vx7if_checksum(const struct vx7_clone_data *clone)
{
	return vx7if_checksum_bytes((uint8_t *)clone, offsetof(struct vx7_clone_data, checksum));
}

uint8_t vx7if_checksum_bytes(uint8_t *bytes, size_t len)
{
	size_t i;
	uint8_t sum;

	for(i = 0, sum = 0; i < len; sum += bytes[i++]);

	return sum;
}

static int32_t vx7if_mem_entry_flag_index(uint32_t index,
		enum vx7_mem_type type)
{
	switch(type) {
	case VX7_MEM_REGULAR:
		if(index >= VX7_MEM_REGULAR_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		return index;

	case VX7_MEM_ONETOUCH:
		if(index >= VX7_MEM_ONETOUCH_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		return index + VX7_MEM_REGULAR_COUNT;

	case VX7_MEM_PMS:
		if(index >= VX7_MEM_PMS_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		return index + VX7_MEM_REGULAR_COUNT + VX7_MEM_ONETOUCH_COUNT;

	default:
		logerror("invalid memory type\n");
		return -1;
	}
}

static uint8_t vx7if_mem_entry_flag(const struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type)
{
	uint8_t f;
	int32_t i;

	if((i = vx7if_mem_entry_flag_index(index, type)) < 0) {
		return BF(MEMFLAG_STATUS, MEMFLAG_STATUS_INVALID);
	}

	/* Two entries per byte */
	f = clone->mem_flag_table[i / 2];

	/* Odd indices are in upper nibble */
	if(i & 1)
		f >>= 4;

	return (f & 0xF);
}

int vx7if_mem_entry_valid(const struct vx7_clone_data *clone, uint32_t index,
		enum vx7_mem_type type)
{
	uint8_t f = vx7if_mem_entry_flag(clone, index, type);

	/* A memory location is valid if it's valid or masked (hidden). */
	if(GETBF(MEMFLAG_STATUS, f) == MEMFLAG_STATUS_VALID)
		return 1;
	if(GETBF(MEMFLAG_STATUS, f) == MEMFLAG_STATUS_MASKED)
		return 1;

	/* Invalid memory location */
	return 0;
}

int vx7if_mem_entry_info(const struct vx7_clone_data *clone,
		struct vx_mem_entry *entry,
		uint32_t index, enum vx7_mem_type type)
{
	int i;
	uint8_t f = vx7if_mem_entry_flag(clone, index, type);
	const struct vx7_mem_entry *m;

	memset(entry, 0, sizeof(*entry));

	/* Flags */
	entry->flag_status = GETBF(MEMFLAG_STATUS, f);
	entry->flag_skip = GETBF(MEMFLAG_SKIP, f);
	entry->flag_preferential = GETBF(MEMFLAG_PREFERRED, f);

	/* Name */
	switch(type) {
	case VX7_MEM_REGULAR:
		if(index >= VX7_MEM_REGULAR_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		m = &clone->regular[index];
		snprintf(&entry->name[0], sizeof(entry->name), "M%03u",
				index + 1);
		break;
	case VX7_MEM_ONETOUCH:
		if(index >= VX7_MEM_ONETOUCH_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		m = &clone->one_touch[index];
		snprintf(&entry->name[0], sizeof(entry->name), "OTM%u",
				(index < 9) ? (index + 1) : 0);
		break;
	case VX7_MEM_PMS:
		if(index >= VX7_MEM_PMS_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		m = &clone->pms[index];
		snprintf(&entry->name[0], sizeof(entry->name), "PMS_%c%02u",
				(index & 1) ? 'U' : 'L', index / 2 + 1);
		break;
	default:
		return -1;
	}

	/* Tag */
	for(i = 0; i < sizeof(m->tag); i++) {
		entry->tag[i] = vx2ascii(m->tag[i], m->charset);
	}

	/* Frequency */
	entry->freq_hz = 100000000 * ((m->freq_100M_10M >> 4) & 0xF);
	entry->freq_hz += 10000000 * ((m->freq_100M_10M >> 0) & 0xF);
	entry->freq_hz +=  1000000 * ((m->freq_1M_100K >> 4) & 0xF);
	entry->freq_hz +=   100000 * ((m->freq_1M_100K >> 0) & 0xF);
	entry->freq_hz +=    10000 * ((m->freq_10K_1K >> 4) & 0xF);
	entry->freq_hz +=     1000 * ((m->freq_10K_1K >> 0) & 0xF);
	if(m->freq_10K_1K == 0x12)
		entry->freq_hz += 500;

	entry->tx_freq_hz = 100000000 * ((m->tx_freq_100M_10M >> 4) & 0xF);
	entry->tx_freq_hz += 10000000 * ((m->tx_freq_100M_10M >> 0) & 0xF);
	entry->tx_freq_hz +=  1000000 * ((m->tx_freq_1M_100K >> 4) & 0xF);
	entry->tx_freq_hz +=   100000 * ((m->tx_freq_1M_100K >> 0) & 0xF);
	entry->tx_freq_hz +=    10000 * ((m->tx_freq_10K_1K >> 4) & 0xF);
	entry->tx_freq_hz +=     1000 * ((m->tx_freq_10K_1K >> 0) & 0xF);
	if(m->tx_freq_10K_1K == 0x12)
		entry->tx_freq_hz += 500;

	/* Frequency Step */
	if(GETBF(MEM_FREQSTEP, m->pwr__step) < ARRAY_SIZE(freqstep_table)) {
		entry->freq_step = freqstep_table[
			GETBF(MEM_FREQSTEP, m->pwr__step)];
	}

	/* Tx Power */
	if(GETBF(MEM_TXPOWER, m->pwr__step) < ARRAY_SIZE(txpwr_table)) {
		strncpy(&entry->tx_pwr[0],
				txpwr_table[GETBF(MEM_TXPOWER, m->pwr__step)],
				sizeof(entry->tx_pwr));
	}

	/* Tx Mode */
	if(GETBF(MEM_TXMODE, m->pwr__step) < ARRAY_SIZE(txmode_table)) {
		strncpy(&entry->tx_mode[0],
				txmode_table[GETBF(MEM_TXMODE, m->pwr__step)],
				sizeof(entry->tx_mode));
	}

	/* Rx Mode */
	if(GETBF(MEM_RXMODE, m->mode) < ARRAY_SIZE(rxmode_table)) {
		strncpy(&entry->rx_mode[0],
				rxmode_table[GETBF(MEM_RXMODE, m->mode)],
				sizeof(entry->rx_mode));
	}

	/* Squelch */
	if(GETBF(MEM_SQUELCH, m->ctcss_dcs) < ARRAY_SIZE(squelch_table)) {
		strncpy(&entry->squelch[0],
				squelch_table[GETBF(MEM_SQUELCH, m->ctcss_dcs)],
				sizeof(entry->squelch));
	}

	/* CTCSS */
	if(m->tone_freq < ARRAY_SIZE(ctcss_table)) {
		entry->ctcss_tenth_hz = ctcss_table[m->tone_freq];
	}

	/* DCS */
	if(m->dcs_code < ARRAY_SIZE(dcs_table)) {
		entry->dcs = dcs_table[m->dcs_code];
	}

	return 0;
}

int vx7if_mem_entry_with_name(const char *name, uint32_t *index,
		enum vx7_mem_type *type)
{
	switch(name[0]) {
	/* Regular: M### */
	case 'M':
		*type = VX7_MEM_REGULAR;
		*index = (uint32_t)strtoul(&name[1], NULL, 10);
		if(*index > VX7_MEM_REGULAR_COUNT || *index < 1) {
			logerror("index out of range\n");
			return -1;
		}
		/* Zero based index */
		(*index)--;
		break;
	/* One Touch: OTM# */
	case 'O':
		*type = VX7_MEM_ONETOUCH;
		if(strlen(name) != 4) {
			logerror("invalid memory entry name\n");
			return -1;
		}
		*index = (uint32_t)strtoul(&name[3], NULL, 10);
		if(*index >= VX7_MEM_ONETOUCH_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		/* 1 = 0, 2 = 1, ..., 9 = 8, 0 = 9 */
		if(*index == 0)
			*index = 9;
		else
			(*index)--;
		break;
	/* PMS: PMS_[LU]## */
	case 'P':
		*type = VX7_MEM_PMS;
		if(strlen(name) < 6) {
			logerror("invalid memory entry name\n");
			return -1;
		}
		*index = (uint32_t)strtoul(&name[5], NULL, 10);
		*index = (*index - 1) * 2;
		if(name[4] == 'L') {
			*index += 0;
		} else if(name[4] == 'U') {
			*index += 1;
		} else {
			logerror("invalid memory entry name\n");
			return -1;
		}
		if(*index >= VX7_MEM_PMS_COUNT) {
			logerror("index out of range\n");
			return -1;
		}
		break;
	default:
		logerror("invalid memory entry name\n");
		return -1;
	}

	return 0;
}

struct vx7_mem_entry *vx7if_mem_entry(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type)
{
	struct vx7_mem_entry *e = NULL;

	switch(type) {
	case VX7_MEM_REGULAR:
		if(index >= VX7_MEM_REGULAR_COUNT) {
			logerror("index out of range\n");
		} else {
			e = &clone->regular[index];
		}
		break;
	case VX7_MEM_ONETOUCH:
		if(index >= VX7_MEM_ONETOUCH_COUNT) {
			logerror("index out of range\n");
		} else {
			e = &clone->one_touch[index];
		}
		break;
	case VX7_MEM_PMS:
		if(index >= VX7_MEM_PMS_COUNT) {
			logerror("index out of range\n");
		} else {
			e = &clone->pms[index];
		}
		break;
	default:
		break;
	}

	return e;
}

/* Status */
int vx7if_mem_entry_set_status(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type,
		enum vx7_mem_status status)
{
	int32_t i;
	uint8_t s;

	if((i = vx7if_mem_entry_flag_index(index, type)) < 0) {
		return -1;
	}

	switch(status) {
	case VX7_MEMSTATUS_INVALID:
		s = MEMFLAG_STATUS_INVALID;
		break;
	case VX7_MEMSTATUS_HIDDEN:
		s = MEMFLAG_STATUS_MASKED;
		break;
	case VX7_MEMSTATUS_VALID:
		s = MEMFLAG_STATUS_VALID;
		break;
	default:
		return -1;
	}

	if(i & 1) {
		INSERTBF(MEMFLAG_STATUS_ODD, s, clone->mem_flag_table[i / 2]);
	} else {
		INSERTBF(MEMFLAG_STATUS, s, clone->mem_flag_table[i / 2]);
	}

	return 0;
}

enum vx7_mem_status vx7if_mem_entry_get_status(
		const struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type)
{
	uint8_t f = vx7if_mem_entry_flag(clone, index, type);

	switch(GETBF(MEMFLAG_STATUS, f)) {
		case MEMFLAG_STATUS_MASKED:
			return VX7_MEMSTATUS_HIDDEN;
		case MEMFLAG_STATUS_VALID:
			return VX7_MEMSTATUS_VALID;
		default:
			return VX7_MEMSTATUS_INVALID;
	}
}

/* Flags */
int vx7if_mem_entry_set_flag(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type, enum vx7_mem_flag flag)
{
	int32_t i;

	if((i = vx7if_mem_entry_flag_index(index, type)) < 0) {
		return -1;
	}

	if(i & 1) {
		INSERTBF(MEMFLAG_SKIP_ODD, 0,
				clone->mem_flag_table[i / 2]);
		INSERTBF(MEMFLAG_PREFERRED_ODD, 0,
				clone->mem_flag_table[i / 2]);
		if(flag == VX7_MEMFLAG_PREFERRED) {
			INSERTBF(MEMFLAG_PREFERRED_ODD, 1,
					clone->mem_flag_table[i / 2]);
		} else if(flag == VX7_MEMFLAG_SKIP) {
			INSERTBF(MEMFLAG_SKIP_ODD, 1,
					clone->mem_flag_table[i / 2]);
		} else if(flag != VX7_MEMFLAG_NORMAL) {
			logerror("invalid flag\n");
			return -1;
		}
	} else {
		INSERTBF(MEMFLAG_SKIP, 0, clone->mem_flag_table[i / 2]);
		INSERTBF(MEMFLAG_PREFERRED, 0, clone->mem_flag_table[i / 2]);
		if(flag == VX7_MEMFLAG_PREFERRED) {
			INSERTBF(MEMFLAG_PREFERRED, 1,
					clone->mem_flag_table[i / 2]);
		} else if(flag == VX7_MEMFLAG_SKIP) {
			INSERTBF(MEMFLAG_SKIP, 1,
					clone->mem_flag_table[i / 2]);
		} else if(flag != VX7_MEMFLAG_NORMAL) {
			logerror("invalid flag\n");
			return -1;
		}
	}

	return 0;
}

enum vx7_mem_flag vx7if_mem_entry_get_flag(const struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type)
{
	uint8_t f = vx7if_mem_entry_flag(clone, index, type);

	if(GETBF(MEMFLAG_PREFERRED, f)) {
		return VX7_MEMFLAG_PREFERRED;
	} else if(GETBF(MEMFLAG_SKIP, f)) {
		return VX7_MEMFLAG_SKIP;
	}

	return VX7_MEMFLAG_NORMAL;
}

/* Frequency */
int vx7if_mem_entry_set_freq(struct vx7_mem_entry *e, uint32_t freq)
{
	if(e == NULL)
		return -1;

	e->freq_100M_10M = 	((freq / 100000000) & 0xF) << 4;
	freq %= 100000000;
	e->freq_100M_10M |=	((freq / 10000000) & 0xF);
	freq %= 10000000;
	e->freq_1M_100K = 	((freq / 1000000) & 0xF) << 4;
	freq %= 1000000;
	e->freq_1M_100K |=	((freq / 100000) & 0xF);
	freq %= 100000;
	e->freq_10K_1K = 	((freq / 10000) & 0xF) << 4;
	freq %= 10000;
	e->freq_10K_1K |=	((freq / 1000) & 0xF);
	/* freq %= 1000; */

	return 0;
}

uint32_t vx7if_mem_entry_get_freq(const struct vx7_mem_entry *e)
{
	uint32_t freq = 0;

	if(e == NULL)
		return 0;

	freq = 100000000 * ((e->freq_100M_10M >> 4) & 0xF);
	freq += 10000000 * ((e->freq_100M_10M >> 0) & 0xF);
	freq +=  1000000 * ((e->freq_1M_100K >> 4) & 0xF);
	freq +=   100000 * ((e->freq_1M_100K >> 0) & 0xF);
	freq +=    10000 * ((e->freq_10K_1K >> 4) & 0xF);
	freq +=     1000 * ((e->freq_10K_1K >> 0) & 0xF);
	if(e->freq_10K_1K == 0x12)
		freq += 500;

	return freq;
}

/* Tx Frequency */
int vx7if_mem_entry_set_txfreq(struct vx7_mem_entry *e, uint32_t freq)
{
	if(e == NULL)
		return -1;

	e->tx_freq_100M_10M = 	((freq / 100000000) & 0xF) << 4;
	freq %= 100000000;
	e->tx_freq_100M_10M |=	((freq / 10000000) & 0xF);
	freq %= 10000000;
	e->tx_freq_1M_100K = 	((freq / 1000000) & 0xF) << 4;
	freq %= 1000000;
	e->tx_freq_1M_100K |=	((freq / 100000) & 0xF);
	freq %= 100000;
	e->tx_freq_10K_1K = 	((freq / 10000) & 0xF) << 4;
	freq %= 10000;
	e->tx_freq_10K_1K |=	((freq / 1000) & 0xF);
	/* freq %= 1000; */

	return 0;
}

uint32_t vx7if_mem_entry_get_txfreq(const struct vx7_mem_entry *e)
{
	uint32_t freq = 0;

	if(e == NULL)
		return 0;

	freq = 100000000 * ((e->tx_freq_100M_10M >> 4) & 0xF);
	freq += 10000000 * ((e->tx_freq_100M_10M >> 0) & 0xF);
	freq +=  1000000 * ((e->tx_freq_1M_100K >> 4) & 0xF);
	freq +=   100000 * ((e->tx_freq_1M_100K >> 0) & 0xF);
	freq +=    10000 * ((e->tx_freq_10K_1K >> 4) & 0xF);
	freq +=     1000 * ((e->tx_freq_10K_1K >> 0) & 0xF);
	if(e->tx_freq_10K_1K == 0x12)
		freq += 500;

	return freq;
}

/* Frequency Step */
int vx7if_mem_entry_set_freq_step(struct vx7_mem_entry *e, uint32_t freq)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(freqstep_table); i++) {
		if(freqstep_table[i] == freq) {
			INSERTBF(MEM_FREQSTEP, i, e->pwr__step);
			return 0;
		}
	}

	logerror("invalid frequency step: %u\n", freq);
	return -1;
}

uint32_t vx7if_mem_entry_get_freq_step(const struct vx7_mem_entry *e)
{
	uint32_t step = 0;

	if(GETBF(MEM_FREQSTEP, e->pwr__step) < ARRAY_SIZE(freqstep_table)) {
		step = freqstep_table[GETBF(MEM_FREQSTEP, e->pwr__step)];
	} else {
		logerror("invalid frequency step in memory entry\n");
	}

	return step;
}

/* Tx Mode */
int vx7if_mem_entry_set_txmode(struct vx7_mem_entry *e, const char *mode)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(txmode_table); i++) {
		if(strcmp(txmode_table[i], mode) == 0) {
			INSERTBF(MEM_TXMODE, i, e->pwr__step);
			return 0;
		}
	}

	logerror("invalid tx mode: %s\n", mode);
	return -1;
}

const char *vx7if_mem_entry_get_txmode(const struct vx7_mem_entry *e)
{
	const char *mode = NULL;

	if(GETBF(MEM_TXMODE, e->pwr__step) < ARRAY_SIZE(txmode_table)) {
		mode = txmode_table[GETBF(MEM_TXMODE, e->pwr__step)];
	} else {
		logerror("invalid tx mode in memory entry\n");
	}

	return mode;
}

/* Tx Power */
int vx7if_mem_entry_set_txpwr(struct vx7_mem_entry *e, const char *pwr)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(txpwr_table); i++) {
		if(strcmp(txpwr_table[i], pwr) == 0) {
			INSERTBF(MEM_TXPOWER, i, e->pwr__step);
			return 0;
		}
	}

	logerror("invalid tx power: %s\n", pwr);
	return -1;
}

const char *vx7if_mem_entry_get_txpwr(const struct vx7_mem_entry *e)
{
	const char *pwr = NULL;

	if(GETBF(MEM_TXPOWER, e->pwr__step) < ARRAY_SIZE(txpwr_table)) {
		pwr = txpwr_table[GETBF(MEM_TXPOWER, e->pwr__step)];
	} else {
		logerror("invalid tx power in memory entry\n");
	}

	return pwr;
}

/* Rx Mode */
int vx7if_mem_entry_set_rxmode(struct vx7_mem_entry *e, const char *mode)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(rxmode_table); i++) {
		if(strcmp(rxmode_table[i], mode) == 0) {
			e->mode = 0x30 | BF(MEM_RXMODE, i);
			return 0;
		}
	}

	logerror("invalid rx mode: %s\n", mode);
	return -1;
}

const char *vx7if_mem_entry_get_rxmode(const struct vx7_mem_entry *e)
{
	const char *mode = NULL;

	if(GETBF(MEM_RXMODE, e->mode) < ARRAY_SIZE(rxmode_table)) {
		mode = rxmode_table[GETBF(MEM_RXMODE, e->mode)];
	} else {
		logerror("invalid rx mode in memory entry\n");
	}

	return mode;
}

/* Squelch */
int vx7if_mem_entry_set_squelch(struct vx7_mem_entry *e, const char *squelch)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(squelch_table); i++) {
		if(strcmp(squelch_table[i], squelch) == 0) {
			e->ctcss_dcs = BF(MEM_SQUELCH, i);
			return 0;
		}
	}

	logerror("invalid squelch mode: %s\n", squelch);
	return -1;
}

const char *vx7if_mem_entry_get_squelch(const struct vx7_mem_entry *e)
{
	const char *sql = NULL;

	if(GETBF(MEM_SQUELCH, e->ctcss_dcs) < ARRAY_SIZE(squelch_table)) {
		sql = squelch_table[GETBF(MEM_SQUELCH, e->ctcss_dcs)];
	} else {
		logerror("invalid squelch mode in memory entry\n");
	}

	return sql;
}

/* CTCSS */
int vx7if_mem_entry_set_ctcss(struct vx7_mem_entry *e, uint16_t tenth_hz)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(ctcss_table); i++) {
		if(ctcss_table[i] == tenth_hz) {
			e->tone_freq = i;
			return 0;
		}
	}

	logerror("invalid ctcss frequency: %u\n", tenth_hz);
	return -1;
}

uint16_t vx7if_mem_entry_get_ctcss(const struct vx7_mem_entry *e)
{
	uint16_t tenth_hz = 0;

	if(e->tone_freq < ARRAY_SIZE(ctcss_table)) {
		tenth_hz = ctcss_table[e->tone_freq];
	} else {
		logerror("invalid ctcss frequency in memory entry\n");
	}

	return tenth_hz;
}

/* DCS */
int vx7if_mem_entry_set_dcs(struct vx7_mem_entry *e, uint16_t dcs)
{
	int i;

	for(i = 0; i < ARRAY_SIZE(dcs_table); i++) {
		if(dcs_table[i] == dcs) {
			e->dcs_code = i;
			return 0;
		}
	}

	logerror("invalid dcs code: %u\n", dcs);
	return -1;
}

uint16_t vx7if_mem_entry_get_dcs(const struct vx7_mem_entry *e)
{
	uint16_t dcs = 0;

	if(e->dcs_code < ARRAY_SIZE(dcs_table)) {
		dcs = dcs_table[e->dcs_code];
	} else {
		logerror("invalid dcs code in memory entry\n");
	}

	return dcs;
}

/* Tag */
int vx7if_mem_entry_set_tag(struct vx7_mem_entry *e, const char *tag)
{
	int i;

	e->charset = 0;

	for(i = 0; i < sizeof(e->tag) && tag && tag[i]; i++) {
		e->tag[i] = ascii2vx(tag[i], e->charset);
	}
	for(; i < sizeof(e->tag); i++) {
		e->tag[i] = ascii2vx(' ', e->charset);
	}

	return 0;
}

int vx7if_mem_entry_get_tag(const struct vx7_mem_entry *e, char *tag)
{
	int i;

	for(i = 0; i < sizeof(e->tag); i++) {
		tag[i] = vx2ascii(e->tag[i], e->charset);
	}

	return 0;
}

/* Defaults */
int vx7if_mem_entry_set_defaults(struct vx7_mem_entry *e)
{
	uint32_t freq = 0;
	uint32_t tx_freq = 0;

	if(e == NULL)
		return -1;

	freq = vx7if_mem_entry_get_freq(e);
	if(freq < 144000000) {
		/* 50MHz */
		tx_freq = 1000000;
	} else if(freq < 222000000) {
		/* 144MHz */
		tx_freq = 600000;
	} else if(freq < 420000000) {
		/* 222MHz */
		tx_freq = 1600000;
	} else {
		/* 420MHz */
		tx_freq = 5000000;
	}
	vx7if_mem_entry_set_freq_step(e, 20000);
	vx7if_mem_entry_set_txfreq(e, tx_freq);
	e->unknown0 = 0x05;
	e->unknown14 = 0;


	e->charset = 0;
	vx7if_mem_entry_set_tag(e, "");

	vx7if_mem_entry_set_txmode(e, "SIMPLEX");
	vx7if_mem_entry_set_txpwr(e, "HI");
	vx7if_mem_entry_set_rxmode(e, "N-FM");
	vx7if_mem_entry_set_squelch(e, "NONE");
	vx7if_mem_entry_set_ctcss(e, 1000);
	vx7if_mem_entry_set_dcs(e, 255);

	return 0;
}

/******************************* Communication ******************************/
static int vx7if_wait_for_ack(struct serial_device *dev)
{
	const uint8_t ack = 0x06;
	uint8_t b = 0;
	struct timeval s;

	gettimeofday(&s, NULL);
	while(!time_has_elapsed_us(&s, 200 * 1000)) {
		if(serial_read(dev, &b, sizeof(b)) > 0) {
			break;
		}
	}
	logdebug("ack took %u ms\n", (uint32_t)(time_elapsed_us(&s) / 1000));

	return (b == ack) ? 0 : -1;
}

static int vx7if_ack(struct serial_device *dev)
{
	const uint8_t ack = 0x06;
	uint8_t b = 0;
	struct timeval s;

	serial_flush(dev);
	if(serial_write(dev, &ack, 1) != 1)
		return -1;
	gettimeofday(&s, NULL);
	while((serial_read(dev, &b, sizeof(b)) == 0) &&
			!time_has_elapsed_us(&s, 200 * 1000));

	return (b == ack) ? 0 : -1;
}

ssize_t vx7if_clone_receive(struct serial_device *dev,
		struct vx7_clone_data *clone)
{
	size_t i = 0;
	ssize_t r;
	struct timeval s;

	serial_flush(dev);

	gettimeofday(&s, NULL);

	while(i < sizeof(*clone) &&
			(i == 0 || !time_has_elapsed_us(&s, 500 * 1000))) {

		/* Handle the two acks that happen after the first 10
		 * then 8 bytes.
		 */
		if(i == 10 || i == 18) {
			if(vx7if_ack(dev) != 0) {
				logerror("ack failed\n");
				return -1;
			}
		}

		/* Rx bytes */
		if((r = serial_read(dev, (uint8_t *)clone + i, 1)) > 0) {
			gettimeofday(&s, NULL);
			i++;
		} else if(r < 0) {
			logerror("a serial error occurred\n");
			return -1;
		}

		if(((i % 1000) == 0 && i) || i == 1) {
			loginfo("%u%% complete\n", (uint32_t)(i * 100 /
					(sizeof(*clone) - 1)));
		}
	}

	loginfo("%u%% complete\n", (uint32_t)(i * 100 / (sizeof(*clone) - 1)));
	loginfo("received %u bytes from device\n", (uint32_t)i);

	if(i != sizeof(*clone)) {
		logerror("did not receive enough bytes from the device\n");
		return -1;
	}

	if(vx7if_checksum(clone) != clone->checksum) {
		logerror("invalid checksum\n");
		return -1;
	} else {
		logdebug("checksum valid\n");
	}

	return (ssize_t)i;
}

int vx7if_clone_send(struct serial_device *dev,
		const struct vx7_clone_data *clone)
{
	size_t i;
	struct timeval s;
	uint8_t *buf = (uint8_t *)clone;

	serial_flush(dev);

	if(vx7if_checksum(clone) != clone->checksum) {
		logerror("invalid checksum\n");
		return -1;
	}

	for(i = 0; i < sizeof(*clone); i++) {
		uint8_t b;

		/* Write bytes */
		if(serial_write(dev, (uint8_t *)clone + i, 1) != 1) {
			logerror("send error\n");
			return -1;
		}
		/* Handle echoed bytes */
		b = ~buf[i];
		gettimeofday(&s, NULL);
		while((serial_read(dev, &b, 1) == 0) &&
				!time_has_elapsed_us(&s, 200 * 1000));
		if(b != buf[i]) {
			logwarn("echo byte mismatch (%02x != %02x)\n",
					b, buf[i]);
		}

		/* Handle the two acks that happen after the first 10
		 * then 8 bytes.
		 */
		if(i == 9 || i == 17) {
			if(vx7if_wait_for_ack(dev) != 0) {
				logerror("ack failed\n");
				return -1;
			}
		}

		/* Progress */
		if((i % 1000) == 0) {
			loginfo("%u%% complete\n", (uint32_t)(i * 100 /
					(sizeof(*clone) - 1)));
		}

		usleep(VX7_INTER_BYTE_DELAY);
	}

	loginfo("%u%% complete\n", (uint32_t)(i * 100 / (sizeof(*clone) - 1)));

	return 0;
}
