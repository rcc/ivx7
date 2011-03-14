/*
 * connection.c - connection handler
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
