/*
 * util.h
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

#include <stdint.h>
#include <netinet/in.h>

#ifndef I__NETUTIL_H__
	#define I__NETUTIL_H__

int sockaddr_by_hostname(struct sockaddr *addr, const char *name,
		uint32_t port);

int sockaddr_port(struct sockaddr *addr);

#define SOCKADDR_ADDR_STR_LEN	20	/* XXXX:XXXX:XXXX:XXXX\0 */
char *sockaddr_addr_str(struct sockaddr *addr, char *str);

#endif /* I__NETUTIL_H__ */
