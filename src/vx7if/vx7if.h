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
#include "vx7_clone_data.h"

#include <serial/serial.h>
#include <prjutil.h>
#include <stdint.h>
#include <stddef.h>

#ifndef I__VX7IF_H__
	#define I__VX7IF_H__

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

/*********************************** Data ***********************************/
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

/* FUNCTION:    vx7if_mem_entry
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

int vx7if_mem_entry_set_status(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type,
		enum vx7_mem_status status);
enum vx7_mem_status vx7if_mem_entry_get_status(
		const struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type);

int vx7if_mem_entry_set_flag(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type, enum vx7_mem_flag flag);
enum vx7_mem_flag vx7if_mem_entry_get_flag(const struct vx7_clone_data *clone,
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
