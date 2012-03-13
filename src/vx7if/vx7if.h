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

#include "vxcharset.h"

#include <serial/serial.h>
#include <prjutil.h>
#include <stdint.h>
#include <stddef.h>

#ifndef I__VX7IF_H__
	#define I__VX7IF_H__


#define VX7_MEM_REGULAR_COUNT		450
#define VX7_MEM_ONETOUCH_COUNT		10
#define VX7_MEM_PMS_COUNT		40


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
	char tx_mode[9];
	char rx_mode[5];
	char squelch[9];
	uint16_t ctcss_tenth_hz;
	uint16_t dcs;
};

/* Main Clone Data Structure */
struct vx7_clone_data {
	uint8_t _resv0[0x1202];
	uint8_t mem_flag_table[(VX7_MEM_REGULAR_COUNT + VX7_MEM_ONETOUCH_COUNT +
			VX7_MEM_PMS_COUNT) / 2];
	uint8_t _resv1[38];
	struct vx7_mem_entry regular[VX7_MEM_REGULAR_COUNT];
	struct vx7_mem_entry one_touch[VX7_MEM_ONETOUCH_COUNT];
	struct vx7_mem_entry pms[VX7_MEM_PMS_COUNT];
	uint8_t _resv2[312];
	uint8_t checksum;
} __packed;

/* Stop myself from making stupid mistakes when editing vx7_clone_data
 * structure.
 */
static int __unused
_vx7_clone_data_size_check[((sizeof(struct vx7_clone_data) == 16211) ? 0 : -1)];


/********************************* Data API *********************************/

/* Abstracted Enums for API */
enum vx7_mem_type {
	VX7_MEM_REGULAR = 0,
	VX7_MEM_ONETOUCH,
	VX7_MEM_PMS,
};

enum vx7_mem_status {
	VX7_MEMSTATUS_INVALID = 0,
	VX7_MEMSTATUS_HIDDEN,
	VX7_MEMSTATUS_VALID,
};

enum vx7_mem_flag {
	VX7_MEMFLAG_NORMAL = 0,
	VX7_MEMFLAG_PREFERRED,
	VX7_MEMFLAG_SKIP,
};

/* FUNCTION:    vx7if_checksum
 *
 * + DESCRIPTION:
 *   - calculate clone checksum
 *
 * + PARAMETERS:
 *   + const struct vx7_clone_data *clone
 *     - clone data
 *
 * + RETURNS: uint8_t
 *   - checksum
 */
uint8_t vx7if_checksum(const struct vx7_clone_data *clone);

/* FUNCTION:    vx7if_mem_entry_valid
 *
 * + DESCRIPTION:
 *   - check if a memory entry is valid
 *
 * + PARAMETERS:
 *   + struct vx7_clone_data *clone
 *     - clone data
 *   + uint32_t index
 *     - memory index
 *   + enum vx7_mem_type type
 *     - memory type
 *
 * + RETURNS: int
 *   - boolean
 */
int vx7if_mem_entry_valid(const struct vx7_clone_data *clone, uint32_t index,
		enum vx7_mem_type type);

/* FUNCTION:    vx7if_mem_entry_info
 *
 * + DESCRIPTION:
 *   - read a memory entry into the abstracted data type
 *
 * + PARAMETERS:
 *   + struct vx7_clone_data *clone
 *     - clone data
 *   + struct vx_mem_entry *entry
 *     - memory entry abstracted data type
 *   + uint32_t index
 *     - memory index
 *   + enum vx7_mem_type type
 *     - memory type
 *
 * + RETURNS: int
 *   - 0 on success, -1 otherwise
 */
int vx7if_mem_entry_info(const struct vx7_clone_data *clone,
		struct vx_mem_entry *entry,
		uint32_t index, enum vx7_mem_type type);

int vx7if_mem_entry_with_name(const char *name, uint32_t *index,
		enum vx7_mem_type *type);

struct vx7_mem_entry *vx7if_mem_entry(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type);

/* Status */
int vx7if_mem_entry_set_status(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type,
		enum vx7_mem_status status);
enum vx7_mem_status vx7if_mem_entry_get_status(
		const struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type);

/* Flags */
int vx7if_mem_entry_set_flag(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type, enum vx7_mem_flag flag);
enum vx7_mem_flag vx7if_mem_entry_get_flag(const struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type);

/* Frequency */
int vx7if_mem_entry_set_freq(struct vx7_mem_entry *e, uint32_t freq);
uint32_t vx7if_mem_entry_get_freq(const struct vx7_mem_entry *e);

/* Tx Frequency */
int vx7if_mem_entry_set_txfreq(struct vx7_mem_entry *e, uint32_t freq);
uint32_t vx7if_mem_entry_get_txfreq(const struct vx7_mem_entry *e);

/* Frequency Step */
int vx7if_mem_entry_set_freq_step(struct vx7_mem_entry *e, uint32_t freq);
uint32_t vx7if_mem_entry_get_freq_step(const struct vx7_mem_entry *e);

/* Tx Mode */
int vx7if_mem_entry_set_txmode(struct vx7_mem_entry *e, const char *mode);
const char *vx7if_mem_entry_get_txmode(const struct vx7_mem_entry *e);

/* Tx Power */
int vx7if_mem_entry_set_txpwr(struct vx7_mem_entry *e, const char *pwr);
const char *vx7if_mem_entry_get_txpwr(const struct vx7_mem_entry *e);

/* Rx Mode */
int vx7if_mem_entry_set_rxmode(struct vx7_mem_entry *e, const char *mode);
const char *vx7if_mem_entry_get_rxmode(const struct vx7_mem_entry *e);

/* Squelch */
int vx7if_mem_entry_set_squelch(struct vx7_mem_entry *e, const char *squelch);
const char *vx7if_mem_entry_get_squelch(const struct vx7_mem_entry *e);

/* TODO */
/* CTCSS */
int vx7if_mem_entry_set_ctcss(struct vx7_mem_entry *e, uint16_t tenth_hz);
uint16_t vx7if_mem_entry_get_ctcss(const struct vx7_mem_entry *e);

/* DCS */
int vx7if_mem_entry_set_dcs(struct vx7_mem_entry *e, uint16_t dcs);
uint16_t vx7if_mem_entry_get_dcs(const struct vx7_mem_entry *e);
/* END TODO */

/* Tag */
int vx7if_mem_entry_set_tag(struct vx7_mem_entry *e, const char *tag);
int vx7if_mem_entry_get_tag(const struct vx7_mem_entry *e, char *tag);

/* Defaults */
int vx7if_mem_entry_set_defaults(struct vx7_mem_entry *e);

/***************************** Communication API ****************************/
/* FUNCTION:    vx7if_clone_receive
 *
 * + DESCRIPTION:
 *   - receives a clone dump from a device
 *
 * + PARAMETERS:
 *   + struct serial_device *dev
 *     - open serial device
 *   + struct vx7_clone_data *clone
 *     - buffer to store clone data
 *
 * + RETURNS: ssize_t
 *   - size of data, -1 on error
 */
ssize_t vx7if_clone_receive(struct serial_device *dev,
		struct vx7_clone_data *clone);

/* FUNCTION:    vx7if_clone_send
 *
 * + DESCRIPTION:
 *   - send a clone file to a device
 *
 * + PARAMETERS:
 *   + struct serial_device *dev
 *     - open serial device
 *   + struct vx7_clone_data *clone
 *     - clone data
 *
 * + RETURNS: int
 *   - 0 on success, -1 on failure
 */
int vx7if_clone_send(struct serial_device *dev,
		const struct vx7_clone_data *clone);

#endif /* I__VX7IF_H__ */
