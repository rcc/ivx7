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
