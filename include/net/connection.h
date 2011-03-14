/*
 * connection.h
 *
 * Copyright (C) 2011 Robert C. Curtis
 *
 * connection is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * connection is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with connection.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <ll.h>
#include <pthread.h>

#include <sys/socket.h>

#ifndef I__NETCONNECTION_H__
	#define I__NETCONNECTION_H__

/*
 * Net Connection Data Structure
 * 	This data structure contains all of the required context for a
 * 	handled network connection.
 */
struct net_connection {
	/* 
	 * Receive Data Handler
	 *
	 * + DESCRIPTION:
	 *   - called when the connection receives data
	 *
	 * + PARAMETERS:
	 *   + struct net_connection *c
	 *     - connection data structure
	 *   + size_t len
	 *     - length of received data
	 */
	void (*rx_data)(struct net_connection *c, size_t len);
	/*
	 * Recieve Buffer
	 */
	void *rx_buf;
	size_t rx_buf_sz;
	pthread_mutex_t rx_buf_lock;

	/* 
	 * Disconnect Callback
	 *
	 * + DESCRIPTION:
	 *   - called when the connection is closed. the socket and rx buffer
	 *   are no longer valid.
	 *
	 * + PARAMETERS:
	 *   + struct net_connection *c
	 *     - connection data structure
	 */
	void (*disconnect)(struct net_connection *c);

	/*
	 * Socket
	 */
	int sock;
	struct sockaddr addr;

	/*
	 * List Connectivity
	 */
	struct list_head connection_node;
	struct list_head work_node;
};

int net_init_connection(struct net_connection *c, size_t rx_buf_sz);
int net_open_connection(struct net_connection *c);
int net_connect_connection(struct net_connection *c);
void net_close_connection(struct net_connection *c);
void net_deinit_connection(struct net_connection *c);

#endif /* I__NETCONNECTION_H__ */
