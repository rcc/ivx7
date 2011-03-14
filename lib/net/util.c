/*
 * util.c - net utilities
 *
 * Copyright (C) 2010 Robert C. Curtis
 *
 * netutil is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * netutil is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with netutil.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <logging.h>

#include <net/util.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <netdb.h>

int sockaddr_by_hostname(struct sockaddr *addr, const char *name,
		uint32_t port)
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
		sin->sin_len = sizeof(struct sockaddr_in);
		sin->sin_port = htons(port);
		memcpy(&sin->sin_addr, h->h_addr_list[0], h->h_length);
		break;
	      }
	case AF_INET6: {
		struct sockaddr_in6 *sin = (struct sockaddr_in6 *)addr;
		sin->sin6_len = sizeof(struct sockaddr_in6);
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
