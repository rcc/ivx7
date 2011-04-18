/*
 * Copyright 2011 Robert C. Curtis. All rights reserved.
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

/*
 * pkv.h - Simple key/value based packet protocol
 */

#include <dict.h>
#include <stddef.h>
#include <stdint.h>

#ifndef I__PKV_H__
	#define I__PKV_H__

#ifdef __cplusplus
extern "C" {
#endif

#define PKV_VERSION	1

/****************************** Data Structures *****************************/
/* PKV Header */
struct pkv_header {
	/* PKV */
	uint8_t magic[3];
	/* Version Count */
	uint8_t version;
	/* Type enum - application defined */
	uint32_t type;
	/* Length of payload */
	uint32_t length;
	/* Header checksum - helps ensure a valid header */
	uint32_t checksum;
	/* Payload */
	uint8_t payload[];
} __packed;

/* PKV */
struct pkv {
	uint32_t type;
	struct dictionary *payload;

	/* Packet Buffer. NULL if it hasn't been created yet. */
	struct pkv_header *pkt;
	size_t pkt_size;
};

/******************************* API Functions ******************************/
/* FUNCTION:    new_pkv
 *
 * + DESCRIPTION:
 *   - creates a new pkv with empty payload
 *
 * + PARAMETERS:
 *   + uint32_t type
 *     - pkv type
 *
 * + RETURNS: struct pkv *
 *   - pointer to new pkv, NULL on failure
 */
struct pkv *new_pkv(uint32_t type);

/* FUNCTION:    delete_pkv
 *
 * + DESCRIPTION:
 *   - frees a pkv
 *
 * + PARAMETERS:
 *   + struct pkv *p
 *     - pkv pointer to free
 */
void delete_pkv(struct pkv *p);

/* FUNCTION:    pkv_serialize
 *
 * + DESCRIPTION:
 *   - creates a serialized pkv packet. the packet is created within the pkv
 *   structure using the pkt and pkt_size members.
 *
 * + PARAMETERS:
 *   + struct pkv *p
 *     - pkv structure pointer
 *
 * + RETURNS: ssize_t
 *   - the size of the packet, negative on error
 */
ssize_t pkv_serialize(struct pkv *p);

/* FUNCTION:    pkv_deserialize
 *
 * + DESCRIPTION:
 *   - deserializes a pkv packet into a dictionary. takes in a pkv structure
 *   with only a raw packet, and creates a dictionary payload from it.
 *
 * + PARAMETERS:
 *   + struct pkv *p
 *     - pkv structure pointer
 *
 * + RETURNS: int
 *   - 0 on success
 */
int pkv_deserialize(struct pkv *p);

#ifdef __cplusplus
}
#endif
#endif /* I__PKV_H__ */
