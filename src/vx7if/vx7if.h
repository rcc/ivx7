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

#ifndef VX7_INTER_BYTE_DELAY
#define VX7_INTER_BYTE_DELAY	8000
#endif

/*********************************** Data ***********************************/
struct vx7_mem_entry {
	uint8_t unknown0;
	uint8_t power_level__step;
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

struct vx7_clone_data {
	uint8_t _resv0[0x1202];
	uint8_t mem_flag_table[250];
	uint8_t _resv1[38];
	struct vx7_mem_entry regular[450];
	struct vx7_mem_entry one_touch[10];
	struct vx7_mem_entry pms[40];
	uint8_t _resv2[312];
	uint8_t checksum;
} __packed;

/* Stop myself from making stupid mistakes when editing vx7_clone_data
 * structure.
 */
static int __unused
_vx7_clone_data_size_check[((sizeof(struct vx7_clone_data) == 16211) ? 0 : -1)];

/* Memory type enum. */
enum vx7_mem_type {
	VX7_MEM_REGULAR = 0,
	VX7_MEM_ONETOUCH = 1,
	VX7_MEM_PMS = 2,
};
enum vx7_mem_status {
	VX7_MEM_INVALID = 0,
	VX7_MEM_MASKED,
	VX7_MEM_VALID,
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

enum vx7_mem_status vx7if_mem_entry_status(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type);

/******************************* Communication ******************************/
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
