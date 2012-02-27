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

#include <prjutil.h>
#include <logging.h>
#include <serial/serial.h>
#include <timelib.h>
#include <unistd.h>

#include <stddef.h>

/*********************************** Data ***********************************/
uint8_t vx7if_checksum(const struct vx7_clone_data *clone)
{
	size_t i;
	uint8_t sum;

	for(i = 0, sum = 0; i < offsetof(struct vx7_clone_data, checksum);
			sum += ((uint8_t *)clone)[i++]);

	return sum;
}

enum vx7_mem_status vx7if_mem_entry_status(struct vx7_clone_data *clone,
		uint32_t index, enum vx7_mem_type type)
{
	uint8_t f;

	if(type == VX7_MEM_ONETOUCH)
		index += 450;
	else if(type == VX7_MEM_PMS)
		index += 450 + 10;

	if((index / 2) >= ARRAY_SIZE(clone->mem_flag_table))
		return VX7_MEM_INVALID;

	f = clone->mem_flag_table[index / 2];

	if(index & 1)
		f >>= 4;

	if((f & 0x3) == 1)
		return VX7_MEM_MASKED;
	else if((f & 0x3) == 3)
		return VX7_MEM_VALID;
	else
		return VX7_MEM_INVALID;
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
