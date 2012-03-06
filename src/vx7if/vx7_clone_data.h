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

#include <prjutil.h>
#include <stdint.h>
#include <bitfield.h>

#ifndef I__VX7_CLONE_DATA_H__
	#define I__VX7_CLONE_DATA_H__

/* Memory Entry */
struct vx7_mem_entry {
	uint8_t unknown0;
	uint8_t pwr__step;
	uint8_t freq_100M_10M;
	uint8_t freq_1M_100K;
	uint8_t freq_10K_1K;
	uint8_t mode;
	uint8_t tag[8];
	uint8_t unknown14;
	uint8_t tx_freq_100M_10M;
	uint8_t tx_freq_1M_100K;
	uint8_t tx_freq_10K_1K;
	uint8_t tone_freq;
	uint8_t dcs_code;
	uint8_t ctcss_dcs;
	uint8_t charset;
} __packed;

/* Abstracted type */
struct vx_mem_entry {
	char name[8];
	char tag[9];
	uint8_t flag_status; /* Valid, Masked, or Invalid */
	int flag_skip;
	int flag_preferential;
	uint32_t freq_hz;
	uint32_t tx_freq_hz;
	uint32_t freq_step;
	char tx_pwr[3];
	char tx_mode[6];
	char rx_mode[5];
	char squelch[9];
	uint16_t ctcss_tenth_hz;
	uint16_t dcs;
};

#define MEMORY_REGULAR_COUNT		450
#define MEMORY_ONETOUCH_COUNT		10
#define MEMORY_PMS_COUNT		40

/* Main Clone Data Structure */
struct vx7_clone_data {
	uint8_t _resv0[0x1202];
	uint8_t mem_flag_table[(MEMORY_REGULAR_COUNT + MEMORY_ONETOUCH_COUNT +
			MEMORY_PMS_COUNT) / 2];
	uint8_t _resv1[38];
	struct vx7_mem_entry regular[MEMORY_REGULAR_COUNT];
	struct vx7_mem_entry one_touch[MEMORY_ONETOUCH_COUNT];
	struct vx7_mem_entry pms[MEMORY_PMS_COUNT];
	uint8_t _resv2[312];
	uint8_t checksum;
} __packed;

/* Stop myself from making stupid mistakes when editing vx7_clone_data
 * structure.
 */
static int __unused
_vx7_clone_data_size_check[((sizeof(struct vx7_clone_data) == 16211) ? 0 : -1)];

/********************************** Memory **********************************/

/* Memory Flag Table */
#define MEMFLAG_STATUS							2 : 0
#	define MEMFLAG_STATUS_INVALID		0x0
#	define MEMFLAG_STATUS_MASKED		0x1
#	define MEMFLAG_STATUS_UNKNOWN		0x2
#	define MEMFLAG_STATUS_VALID		0x3
#define MEMFLAG_SKIP							1 : 2
#define MEMFLAG_PREFERRED						1 : 3

/* Memory Entries */
#define MEM_TXPOWER							2 : 6
#	define MEM_TXPOWER_L1			0x0
#	define MEM_TXPOWER_L2			0x1
#	define MEM_TXPOWER_L3			0x2
#	define MEM_TXPOWER_HI			0x3
#define MEM_TXMODE							2 : 4
#	define MEM_TXMODE_SIMP			0x0
#	define MEM_TXMODE_NRPT			0x1
#	define MEM_TXMODE_PRPT			0x2
#	define MEM_TXMODE_INDTX			0x3
#define MEM_FREQSTEP							4 : 0
#	define MEM_FREQSTEP_5K			0x0
#	define MEM_FREQSTEP_10K			0x1
#	define MEM_FREQSTEP_12_5K		0x2
#	define MEM_FREQSTEP_15K			0x3
#	define MEM_FREQSTEP_20K			0x4
#	define MEM_FREQSTEP_25K			0x5
#	define MEM_FREQSTEP_50K			0x6
#	define MEM_FREQSTEP_100K		0x7
#	define MEM_FREQSTEP_9K			0x8

#define MEM_RXMODE							2 : 0
#	define MEM_RXMODE_NFM			0x0
#	define MEM_RXMODE_AM			0x1
#	define MEM_RXMODE_WFM			0x2

#define MEM_SQUELCH							2 : 0
#	define MEM_SQUELCH_NONE			0x0
#	define MEM_SQUELCH_TONE			0x1
#	define MEM_SQUELCH_TONE_SQL		0x2
#	define MEM_SQUELCH_DCS			0x3

#endif /* I__VX7_CLONE_DATA_H__ */
