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
#include <logging.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

/*
 * Connection Management Routines
 */
int net_init_connection(struct net_connection *c, size_t rx_buf_sz)
{
	memset(c, 0, sizeof(*c));

	if(rx_buf_sz) {
		if((c->rx_buf = malloc(rx_buf_sz)) == NULL) {
			logerror("could not allocate new receive buffer: "
					"%s\n", strerror(errno));
			return errno;
		}
		c->rx_buf_sz = rx_buf_sz;
	}

	pthread_mutex_init(&c->rx_buf_lock, NULL);

	c->sock = -1;

	return 0;
}

int net_open_connection(struct net_connection *c)
{
	if((c->sock = socket(c->addr.sa_family, SOCK_STREAM, 0)) < 0) {
		logerror("could not open socket: %s\n", strerror(errno));
		return errno;
	}

	return 0;
}

int net_connect_connection(struct net_connection *c)
{
	if(connect(c->sock, &c->addr, c->addr.sa_len) != 0) {
		logerror("could not open socket: %s\n", strerror(errno));
		return errno;
	}

	return 0;
}

void net_close_connection(struct net_connection *c)
{
	/* Shutdown the socket */
	shutdown(c->sock, SHUT_RDWR);
	close(c->sock);
	c->sock = -1;
}

void net_deinit_connection(struct net_connection *c)
{
	net_close_connection(c);

	pthread_mutex_lock(&c->rx_buf_lock);
	if(c->rx_buf) {
		free(c->rx_buf);
		c->rx_buf = NULL;
	}
	c->rx_buf_sz = 0; 
	pthread_mutex_destroy(&c->rx_buf_lock);
}

#ifdef NETCONNECTIONTESTCMD
#include <cmds.h>

void test_rx_data_handler(struct net_connection *c, size_t len)
{
	loginfo("RX Data: %u bytes\n", (unsigned int)len);
}

void test_disconnect_handler(struct net_connection *c)
{
	loginfo("Disconnected\n");
}

static struct net_connection test_connection;

CMDHANDLER(connection_test)
{
#if MAX_LOGLEVEL >= LOGLEVEL_INFO
	char addr_str[SOCKADDR_ADDR_STR_LEN];
#endif
	uint16_t port;

	if(argc < 2) {
		pcmderr("invalid usage\n");
		return -1;
	}

	port = atoi(argv[1]);

	if(net_init_connection(&test_connection, (4<<10)) != 0) {
		pcmderr("could not init connection\n");
		return -1;
	}
	test_connection.rx_data = &test_rx_data_handler;
	test_connection.disconnect = &test_disconnect_handler;

	sockaddr_by_hostname(&test_connection.addr, argv[0], port);
	loginfo("Hostaddr Found: %s\n", sockaddr_addr_str(&test_connection.addr,
				addr_str));

	if(net_open_connection(&test_connection) != 0) {
		return -1;
	}
	if(net_connect_connection(&test_connection) != 0) {
		return -1;
	}

	send(test_connection.sock, "Test\n", 5, 0);

	net_deinit_connection(&test_connection);

	return 2;
}
APPCMD(connect, &connection_test, "test net connection",
		"usage: connect <hostname> <port>",
		NULL);
#endif /* NETCONNECTIONTESTCMD */
