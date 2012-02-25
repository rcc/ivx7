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

#include <serial/serial.h>
#include <stddef.h>

#ifndef I__VX7IF_H__
	#define I__VX7IF_H__

#define VX7_CLONE_SIZE	16211
#define VX7_CHECKSUM	0x3F52

/* FUNCTION:    vx7if_device_ready
 *
 * + DESCRIPTION:
 *   - checks if a device is ready for communication
 *
 * + PARAMETERS:
 *   + struct serial_device *dev
 *     - open serial device
 *
 * + RETURNS: int
 *   - boolean
 */
int vx7if_device_ready(struct serial_device *dev);

/* FUNCTION:    vx7if_clone_receive
 *
 * + DESCRIPTION:
 *   - receives a clone dump from a device
 *
 * + PARAMETERS:
 *   + struct serial_device *dev
 *     - open serial device
 *   + uint8_t *buf
 *     - buffer to store clone data. should be at least VX_CLONE_SIZE
 *
 * + RETURNS: ssize_t
 *   - size of data, -1 on error
 */
ssize_t vx7if_clone_receive(struct serial_device *dev, uint8_t *buf);

#endif /* I__VX7IF_H__ */
