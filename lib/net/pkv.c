/*
 * pkv.c
 *
 * Copyright (C) 2010 Robert C. Curtis
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
#include <pkv.h>
#include <prjutil.h>
#include <logging.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


/* calculate checksum - assumes checksum is last field and is aligned. */
static inline uint32_t pkv_calc_checksum(struct pkv_header *pkt)
{
	size_t i;
	uint32_t chksum = 0;

	for(i = 0; i < (sizeof(struct pkv_header) - sizeof(pkt->checksum));
			i += sizeof(pkt->checksum)) {
		chksum += *(uint32_t *)((void *)pkt + i);
	}

	return (0 - chksum);
}

struct pkv *new_pkv(uint32_t type)
{
	struct pkv *p;

	if((p = malloc(sizeof(*p))) == NULL) {
		logerror("could not allocate new pkv : %s\n", strerror(errno));
		return NULL;
	}
	memset(p, 0, sizeof(*p));

	p->type = type;

	if((p->payload = new_dict()) == NULL) {
		logerror("could not allocate new pkv payload\n");
		free(p);
		return NULL;
	}

	return p;
}

void delete_pkv(struct pkv *p)
{
	if(p->pkt)
		free(p->pkt);

	delete_dict(p->payload);

	free(p);
}

ssize_t pkv_serialize(struct pkv *p)
{
	size_t payload_sz = dict_serialized_size(p->payload);
	size_t sz = sizeof(*(p->pkt)) + payload_sz;

	if(p->pkt)
		logwarn("serializing over non-NULL p->pkt : %p\n", p->pkt);

	if((p->pkt = malloc(sz)) == NULL) {
		logerror("could not serialize pkv : %s\n", strerror(errno));
		return -1;
	}
	p->pkt_size = sz;

	/* fill in header */
	p->pkt->magic[0] = 'P';
	p->pkt->magic[1] = 'K';
	p->pkt->magic[2] = 'V';
	p->pkt->version = PKV_VERSION;
	p->pkt->type = p->type;
	p->pkt->length = payload_sz;
	p->pkt->checksum = pkv_calc_checksum(p->pkt);

	/* fill in payload */
	if(dict_serialize_into(p->payload, p->pkt->payload) < 0) {
		logerror("could not serialize pkv payload\n");
		free(p->pkt);
		p->pkt = NULL;
		p->pkt_size = 0;
		return -1;
	}

	return sz;
}

int pkv_deserialize(struct pkv *p)
{
	if(p->payload)
		logwarn("deserializing over non-NULL p->payload : %p\n",
				p->payload);
	/* check checksum */
	if(p->pkt->checksum != pkv_calc_checksum(p->pkt)) {
		logerror("checksum mismatch\n");
		return -1;
	}

	if((p->payload = dict_for_buf(p->pkt->payload, p->pkt->length,
					NULL)) == NULL) {
		logerror("could not deserialize pkv payload\n");
		return -1;
	}

	p->type = p->pkt->type;

	return 0;
}
