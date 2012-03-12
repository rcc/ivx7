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

/* Memory Flag Table */
#define MEMFLAG_STATUS							2 : 0
#define MEMFLAG_STATUS_ODD						2 : 4
#	define MEMFLAG_STATUS_INVALID		0x0
#	define MEMFLAG_STATUS_MASKED		0x1
#	define MEMFLAG_STATUS_UNKNOWN		0x2
#	define MEMFLAG_STATUS_VALID		0x3
#define MEMFLAG_SKIP							1 : 2
#define MEMFLAG_SKIP_ODD						1 : 6
#define MEMFLAG_PREFERRED						1 : 3
#define MEMFLAG_PREFERRED_ODD						1 : 7

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
