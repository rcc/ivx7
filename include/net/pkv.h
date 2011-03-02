/*
 * pkv.h - Simple key/value based packet protocol
 *
 * Copyright (C) 2011 Robert C. Curtis
 *
 * pkv is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * pkv is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pkv.  If not, see <http://www.gnu.org/licenses/>.
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
