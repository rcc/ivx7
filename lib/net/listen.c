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

#include <net/listen.h>
#include <net/util.h>
#include <threadpool.h>
#include <logging.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>


#ifndef NET_LISTEN_BACKLOG
#define NET_LISTEN_BACKLOG		5
#endif /* NET_LISTEN_BACKLOG */


/* Listener thread prototype */
static void listener_thread(struct ctrlthread *thread);

int net_init_listener(struct net_listener *l,
		struct net_connection_handler *h,
		struct net_connection *(*new_cx)(struct net_listener *))
{
	memset(l, 0, sizeof(*l));
	l->sock = -1;
	l->handler = h;
	l->new_cx = new_cx;
	pthread_mutex_init(&l->listener_lock, NULL);

	return 0;
}

void net_deinit_listener(struct net_listener *l)
{
	pthread_mutex_lock(&l->listener_lock);
	pthread_mutex_destroy(&l->listener_lock);
}

int net_open_listener_afinet(struct net_listener *l, uint16_t port)
{
	/* setup socket bind address */
	((struct sockaddr_in *)&l->addr)->sin_family = AF_INET;
	((struct sockaddr_in *)&l->addr)->sin_port = htons(port);
	((struct sockaddr_in *)&l->addr)->sin_addr.s_addr = htonl(INADDR_ANY);

	/* open the socket */
	if((l->sock = socket(l->addr.sa_family, SOCK_STREAM, 0)) < 0) {
		logerror("could not open socket: %s\n", strerror(errno));
		return errno;
	}

	/* bind the socket */
	if(bind(l->sock, &l->addr, sizeof(struct sockaddr_in)) < 0) {
		logerror("could not bind socket: %s\n", strerror(errno));
		return errno;
	}

	/* set socket to listen */
	if(listen(l->sock, NET_LISTEN_BACKLOG) < 0) {
		logerror("could not listen on socket: %s\n", strerror(errno));
		return errno;
	}

	return 0;
}

int net_start_listener(struct net_listener *l)
{
	int status = 0;

	if(!l->handler || !l->new_cx || (l->sock < 0)) {
		logerror("invalid arguments\n");
		return EINVAL;
	}

	pthread_mutex_lock(&l->listener_lock);
	if((l->thread = threadpool_start_control_thread(&l->handler->pool,
					&listener_thread, l)) == NULL) {
		logerror("could not start listener thread\n");
		pthread_mutex_unlock(&l->listener_lock);
		status = -1;
		goto exit;
	}

exit:
	return status;
}

void net_stop_listener(struct net_listener *l)
{
	l->shutdown = 1;
	/* Make sure the listener has shutdown before returning */
	pthread_mutex_lock(&l->listener_lock);
	pthread_mutex_unlock(&l->listener_lock);
}

static void listener_accept_connection(struct net_listener *l)
{
	int rsock;
	struct sockaddr addr;
	socklen_t addr_len = sizeof(addr);
	struct net_connection *cx;

	if((rsock = accept(l->sock, &addr, &addr_len)) < 0) {
		logerror("could not accept client connection\n");
		return;
	}
	if((cx = l->new_cx(l)) == NULL) {
		logerror("rejecting client connection\n");
		close(rsock);
		return;
	}
	cx->sock = rsock;
	cx->addr_len = addr_len;
	memcpy(&cx->addr, &addr, sizeof(addr));

	net_handle_connection(l->handler, cx);

	logdebug("client connected\n");
	if(l->new_client)
		l->new_client(l, cx);
}

static void listener_thread(struct ctrlthread *thread)
{
	struct net_listener *l = (struct net_listener *)thread->priv;
	int e;
	struct pollfd listenpoll;

	/* Setup the poll */
	listenpoll.fd = l->sock;
	listenpoll.events = POLLIN;
	listenpoll.revents = 0;

	e = poll(&listenpoll, 1, 200);
	if(e == 0) {
		/* nothing to do */
	} else if(e < 0) {
		/* error */
		logwarn("poll returned an error: %s\n", strerror(errno));
	} else {
		/* client is trying to connect */
		listener_accept_connection(l);
	}

	if(l->shutdown) {
		logdebug("shutting down listener\n");
		close(l->sock);
		thread->shutdown = 1;
		pthread_mutex_unlock(&l->listener_lock);
	}
}

#ifdef NETCONNECTIONTESTCMD
#include <cmds.h>

static void test_rx_data_handler(struct net_connection *c, size_t len)
{
	loginfo("RX Data: %u bytes\n", (unsigned int)len);
}

static void test_new_client_handler(struct net_listener *l,
		struct net_connection *c)
{
#if MAX_LOGLEVEL >= LOGLEVEL_DEBUG
	char addr_str[SOCKADDR_ADDR_STR_LEN];
#endif
	logdebug("New client: %s\n", sockaddr_addr_str(&c->addr, &addr_str[0]));
	send(c->sock, "Hello, World\n", 13, 0);
}

static void test_disconnect_handler(struct net_connection *c)
{
	loginfo("Disconnected\n");
	net_deinit_connection(c);
	free(c);
}

static struct net_connection *new_client_connection(struct net_listener *l)
{
	struct net_connection *c;

	if((c = malloc(sizeof(*c))) == NULL) {
		return NULL;
	}

	if(net_init_connection(c, (4<<10)) != 0) {
		logerror("could not init connection\n");
		return NULL;
	}
	c->rx_data = &test_rx_data_handler;
	c->disconnect = &test_disconnect_handler;

	return c;
}

static struct net_connection_handler test_handler;
static struct net_listener test_listener;

CMDHANDLER(listener_test)
{
	uint16_t port;

	if(argc < 1) {
		pcmderr("invalid usage\n");
		return -1;
	}

	port = atoi(argv[0]);

	if(net_connection_handler_init(&test_handler) != 0) {
		pcmderr("could not init connection handler\n");
		return -1;
	}

	if(net_init_listener(&test_listener, &test_handler,
				&new_client_connection) != 0) {
		pcmderr("could not init listener\n");
		return -1;
	}
	test_listener.new_client = &test_new_client_handler;

	if(net_open_listener_afinet(&test_listener, port) != 0) {
		pcmderr("could not open listener\n");
		return -1;
	}

	if(net_start_listener(&test_listener) != 0) {
		pcmderr("could not start listener\n");
		return -1;
	}

	sleep(20);

	net_stop_listener(&test_listener);
	net_deinit_listener(&test_listener);
	net_connection_handler_shutdown(&test_handler);

	return 1;
}
APPCMD(listen, &listener_test, "test net listener",
		"usage: listen <port>",
		NULL);
#endif /* NETCONNECTIONTESTCMD */
