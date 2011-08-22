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

#include <net/connection.h>
#include <threadpool.h>

#include <stdint.h>
#include <pthread.h>
#include <sys/socket.h>

#ifndef I__NETLISTEN_H__
	#define I__NETLISTEN_H__

/*
 * Net Listener Data Structure
 * 	This data structure contains all of the required context for a
 * 	network listener.
 */
struct net_listener {
	/*
	 * Connection Source
	 *
	 * + DESCRIPTION:
	 *   - Provides new initialized net_connection data structures.
	 *
	 * + PARAMETERS:
	 *   + struct net_listener *
	 *     - the listener requesting a new connection data structure
	 *
	 * + RETURNS: struct net_connection *
	 *   - NULL on failure
	 */
	struct net_connection *(*new_cx)(struct net_listener *);

	/*
	 * New Connection Notification
	 *
	 * + DESCRIPTION:
	 *   - Called after a new client connection is setup.
	 *
	 * + PARAMETERS:
	 *   + struct net_listener *
	 *     - the listener that received the client
	 *   + struct net_connection *
	 *     - the new client connection
	 */
	void (*new_client)(struct net_listener *, struct net_connection *);

	/*
	 * Socket
	 */
	int sock;
	struct sockaddr addr;
	socklen_t addr_len;

	/*
	 * Threadpool
	 */
	struct net_connection_handler *handler;
	struct ctrlthread *thread;
	volatile int shutdown;
	pthread_mutex_t listener_lock;
};

int net_init_listener(struct net_listener *l,
		struct net_connection_handler *h,
		struct net_connection *(*new_cx)(struct net_listener *));
void net_deinit_listener(struct net_listener *l);

int net_start_listener(struct net_listener *l);
void net_stop_listener(struct net_listener *l);

#endif /* I__NETLISTEN_H__ */
