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

#include <logging.h>

#include <net/util.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

int sockaddr_by_hostname(struct sockaddr *addr, socklen_t *addr_len,
		const char *name, uint32_t port)
{
	struct hostent *h;

	if((h = gethostbyname(name)) == NULL) {
		logerror("%s\n", hstrerror(h_errno));
		return h_errno;
	}

	memset(addr, 0, sizeof(*addr));
	addr->sa_family = h->h_addrtype;

	switch(addr->sa_family) {
	case AF_INET: {
		struct sockaddr_in *sin = (struct sockaddr_in *)addr;
		*addr_len = sizeof(struct sockaddr_in);
#ifdef __HOST_DARWIN__
		sin->sin_len = sizeof(struct sockaddr_in);
#endif
		sin->sin_port = htons(port);
		memcpy(&sin->sin_addr, h->h_addr_list[0], h->h_length);
		break;
	      }
	case AF_INET6: {
		struct sockaddr_in6 *sin = (struct sockaddr_in6 *)addr;
		*addr_len = sizeof(struct sockaddr_in6);
#ifdef __HOST_DARWIN__
		sin->sin6_len = sizeof(struct sockaddr_in6);
#endif
		sin->sin6_port = htons(port);
		memcpy(&sin->sin6_addr, h->h_addr_list[0], h->h_length);
		break;
	      }
	default:
		logerror("unsupported address type: %d\n",
				h->h_addrtype);
		return NO_RECOVERY;
	}

	return 0;
}

int sockaddr_port(struct sockaddr *addr)
{
	int port;

	switch(addr->sa_family) {
	case AF_INET:
		port = (int)ntohs(((struct sockaddr_in *)addr)->sin_port);
		break;
	case AF_INET6:
		port = (int)ntohs(((struct sockaddr_in6 *)addr)->sin6_port);
		break;
	default:
		port = -1;
	}

	return port;
}

char *sockaddr_addr_str(struct sockaddr *addr, char *str)
{
	switch(addr->sa_family) {
	case AF_INET: {
		uint32_t ip;
		ip = ntohl(((struct sockaddr_in *)addr)->sin_addr.s_addr);
		snprintf(str, SOCKADDR_ADDR_STR_LEN, "%d.%d.%d.%d",
				((ip >> 24) & 0xFF),
				((ip >> 16) & 0xFF),
				((ip >> 8) & 0xFF),
				((ip >> 0) & 0xFF));
		break;
	}
	case AF_INET6: {
		unsigned char *ip;
		ip = &(((struct sockaddr_in6 *)addr)->sin6_addr.s6_addr[0]);
		snprintf(str, SOCKADDR_ADDR_STR_LEN,
				"%02x%02x%02x%02x:%02x%02x%02x%02x:"
				"%02x%02x%02x%02x:%02x%02x%02x%02x",
				ip[0], ip[1], ip[2], ip[3],
				ip[4], ip[5], ip[6], ip[7],
				ip[8], ip[9], ip[10], ip[11],
				ip[12], ip[13], ip[14], ip[15]);
		break;
	}
	default:
		snprintf(str, SOCKADDR_ADDR_STR_LEN, "<unknown>");
	}

	return str;
}
