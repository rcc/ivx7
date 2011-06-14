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

#include <ll.h>
#include <threadpool.h>

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
	 *   - called when the connection is closed. the socket is no
	 *   longer valid. should take care of any memory management
	 *   (e.g. free()).
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
	pthread_mutex_t cx_lock;

	/*
	 * List Connectivity
	 */
	struct list_head connection_node;
	struct list_head work_node;
	pthread_mutex_t handled_lock;
};

int net_init_connection(struct net_connection *c, size_t rx_buf_sz);
int net_open_connection(struct net_connection *c);
int net_connect_connection(struct net_connection *c);
void net_close_connection(struct net_connection *c);
void net_deinit_connection(struct net_connection *c);

/*
 * Net Connection Handler
 * 	This data structure defines a connection handler (thread pool).
 */
struct net_connection_handler {
	struct threadpool pool;
	struct list_head connections;
	pthread_mutex_t connections_lock;
};

int net_connection_handler_init(struct net_connection_handler *h);
void net_connection_handler_shutdown(struct net_connection_handler *h);
void net_handle_connection(struct net_connection_handler *h,
		struct net_connection *c);
void net_unhandle_connection(struct net_connection_handler *h,
		struct net_connection *c);
void net_close_handled_connection(struct net_connection_handler *h,
		struct net_connection *c);

#endif /* I__NETCONNECTION_H__ */
