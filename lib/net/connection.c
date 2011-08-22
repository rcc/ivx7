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

/*
 * Connection Management Routines
 */
int net_init_connection(struct net_connection *c, size_t rx_buf_sz)
{
	memset(c, 0, sizeof(*c));

	if(rx_buf_sz) {
		if((c->rxb.buf = malloc(rx_buf_sz)) == NULL) {
			logerror("could not allocate new receive buffer: "
					"%s\n", strerror(errno));
			return errno;
		}
		c->rxb.size = rx_buf_sz;
	}

	pthread_mutex_init(&c->rxb.lock, NULL);
	pthread_mutex_init(&c->cx_lock, NULL);
	pthread_mutex_init(&c->handled_lock, NULL);

	c->sock = -1;

	return 0;
}

int net_open_connection(struct net_connection *c)
{
	int status = 0;

	pthread_mutex_lock(&c->cx_lock);
	if((c->sock = socket(c->addr.sa_family, SOCK_STREAM, 0)) < 0) {
		logerror("could not open socket: %s\n", strerror(errno));
		status = errno;
		goto exit;
	}

exit:
	pthread_mutex_unlock(&c->cx_lock);
	return status;
}

int net_connect_connection(struct net_connection *c)
{
	int status = 0;

	pthread_mutex_lock(&c->cx_lock);
	if(connect(c->sock, &c->addr, c->addr_len) != 0) {
		logerror("could not open socket: %s\n", strerror(errno));
		status = errno;
		goto exit;
	}

exit:
	pthread_mutex_unlock(&c->cx_lock);
	return status;
}

void net_close_connection(struct net_connection *c)
{
	/* Shutdown the socket */
	pthread_mutex_lock(&c->cx_lock);
	if(c->sock >= 0) {
		shutdown(c->sock, SHUT_RDWR);
		close(c->sock);
		c->sock = -1;
		pthread_mutex_unlock(&c->cx_lock);
		if(c->disconnect)
			c->disconnect(c);
	} else {
		pthread_mutex_unlock(&c->cx_lock);
	}
}

void net_deinit_connection(struct net_connection *c)
{
	pthread_mutex_lock(&c->cx_lock);
	pthread_mutex_lock(&c->rxb.lock);
	if(c->rxb.buf) {
		free(c->rxb.buf);
		c->rxb.buf = NULL;
	}
	c->rxb.size = 0;
	pthread_mutex_destroy(&c->cx_lock);
	pthread_mutex_destroy(&c->rxb.lock);
}


/*
 * Connection Handler Routines
 */
static struct list_head *connection_handler_thread(struct poolthread *thread,
		struct list_head *work)
{
	int e;
	struct pollfd sockpoll;
	struct net_connection *c = container_of(work, struct net_connection,
			work_node);

	if(!c->connection_node.next || (c->sock < 0)) {
		/* Connection is unhandled or socket invalid. Drop work. */
		pthread_mutex_unlock(&c->handled_lock);
		work = NULL;
		goto exit;
	}

	/* Poll the connection */
	pthread_mutex_lock(&c->cx_lock);

	sockpoll.fd = c->sock;
	sockpoll.events = POLLIN | POLLHUP;
	sockpoll.revents = 0;

	e = poll(&sockpoll, 1, 200);

	pthread_mutex_unlock(&c->cx_lock);

	if(e == 0) {
		/* nothing to do */
		goto exit;
	} else if(e < 0) {
		/* error */
		logwarn("poll returned an error: %s\n", strerror(errno));
		goto exit;
	}

	/* Handle poll events */
	if(POLLIN & sockpoll.revents) {
		/* Data Ready */
		ssize_t rlen;
		pthread_mutex_lock(&c->rxb.lock);
		pthread_mutex_lock(&c->cx_lock);
		rlen = recvfrom(c->sock, c->rxb.buf, c->rxb.size, 0,
				NULL, NULL);
		pthread_mutex_unlock(&c->cx_lock);

		if(rlen < 0) {
			logwarn("recvfrom returned an error\n");
		} else if(rlen == 0) {
			/* Disconnected, POLLHUP takes care of it */
		} else {
			if(c->rx_data)
				c->rx_data(c, rlen);
		}
		pthread_mutex_unlock(&c->rxb.lock);
	}
	if(POLLHUP & sockpoll.revents) {
		/* Connection Closed */
		struct net_connection_handler *h = container_of(thread->pool,
				struct net_connection_handler, pool);
		/* Remove the connection from the handler */
		pthread_mutex_unlock(&c->handled_lock);
		net_unhandle_connection(h, c);
		/* Disconnect */
		net_close_connection(c);
		/* Drop the work */
		work = NULL;
		goto exit;
	}

exit:
	return work;
}

int net_connection_handler_init(struct net_connection_handler *h)
{
	int status;

	if((status = threadpool_init(&h->pool)) != 0) {
		logerror("could not initialize threadpool\n");
		return status;
	}
	h->pool.config.work_func = &connection_handler_thread;

	INIT_LIST_HEAD(&h->connections);
	pthread_mutex_init(&h->connections_lock, NULL);

	return status;
}

void net_connection_handler_shutdown(struct net_connection_handler *h)
{
	struct net_connection *c, *n;

	pthread_mutex_lock(&h->connections_lock);
	list_for_each_entry_safe(c, n, &h->connections, connection_node) {
		pthread_mutex_unlock(&h->connections_lock);
		net_close_handled_connection(h, c);
		pthread_mutex_lock(&h->connections_lock);
	}
	pthread_mutex_unlock(&h->connections_lock);

	threadpool_shutdown(&h->pool);
}

void net_handle_connection(struct net_connection_handler *h,
		struct net_connection *c)
{
	pthread_mutex_lock(&h->connections_lock);
	list_add(&c->connection_node, &h->connections);
	pthread_mutex_unlock(&h->connections_lock);

	pthread_mutex_lock(&c->handled_lock);
	threadpool_queue_work(&h->pool, &c->work_node);
}

void net_unhandle_connection(struct net_connection_handler *h,
		struct net_connection *c)
{
	pthread_mutex_lock(&h->connections_lock);
	if(c->connection_node.next) {
		list_del(&c->connection_node);
	}
	pthread_mutex_unlock(&h->connections_lock);

	/* Wait until we can acquire the handled_lock. This means the
	 * handler thread has given it up. */
	pthread_mutex_lock(&c->handled_lock);
	pthread_mutex_unlock(&c->handled_lock);
}

void net_close_handled_connection(struct net_connection_handler *h,
		struct net_connection *c)
{
	net_unhandle_connection(h, c);
	net_close_connection(c);
}

#ifdef NETCONNECTIONTESTCMD
#include <cmds.h>

static void test_rx_data_handler(struct net_connection *c, size_t len)
{
	loginfo("RX Data: %u bytes\n", (unsigned int)len);
}

static void test_disconnect_handler(struct net_connection *c)
{
	loginfo("Disconnected\n");
}

static struct net_connection_handler test_handler;
static struct net_connection test_connection;

CMDHANDLER(connection_test)
{
#if MAX_LOGLEVEL >= LOGLEVEL_DEBUG
	char addr_str[SOCKADDR_ADDR_STR_LEN];
#endif
	uint16_t port;

	if(argc < 2) {
		pcmderr("invalid usage\n");
		return -1;
	}

	port = atoi(argv[1]);

	if(net_connection_handler_init(&test_handler) != 0) {
		pcmderr("could not init connection handler\n");
		return -1;
	}

	if(net_init_connection(&test_connection, (4<<10)) != 0) {
		pcmderr("could not init connection\n");
		return -1;
	}
	test_connection.rx_data = &test_rx_data_handler;
	test_connection.disconnect = &test_disconnect_handler;

	if(sockaddr_by_hostname(&test_connection.addr,
			&test_connection.addr_len, argv[0], port) != 0) {
		pcmderr("could not lookup hostname\n");
		return -1;
	}
	logdebug("connecting to: %s:%u\n",
			sockaddr_addr_str(&test_connection.addr, addr_str),
			sockaddr_port(&test_connection.addr));

	if(net_open_connection(&test_connection) != 0) {
		pcmderr("could not open socket\n");
		return -1;
	}
	if(net_connect_connection(&test_connection) != 0) {
		pcmderr("could not connect to host\n");
		return -1;
	}
	net_handle_connection(&test_handler, &test_connection);
	logdebug("connected\n");

	sleep(1);

	send(test_connection.sock, "Test\n", 5, 0);

	sleep(5);

	logdebug("disconnecting\n");
	net_connection_handler_shutdown(&test_handler);
	net_deinit_connection(&test_connection);

	return 2;
}
APPCMD(connect, &connection_test, "test net connection",
		"usage: connect <hostname> <port>",
		NULL);
#endif /* NETCONNECTIONTESTCMD */
