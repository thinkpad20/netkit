#ifndef _NETKIT_H_
#define _NETKIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>
#include <assert.h>
#include "../include/strlist.h"

#define MAX_BUF 50000

/*
connection_t->options:

first bit: free hostname (default 1)
second bit: free ip (default 1)
third bit: free port (default 1)
fourth bit: 0->ipv4, 1->ipv6 (default 0)
fifth bit: 0->TCP, 1->UDP (default 0)

*/

typedef struct connection_s {
	int fd;
	int type, options;
	char *hostname;
	char *ip;
	void *ip_int;
	char *port;
	char *buf;
	size_t buf_in;
	strlist_t lines;
} connection_t;

/* unspecified IP version */
connection_t *nk_listen_on(const char *port);
connection_t *nk_connect_to(const char *hostname, 
							const char *port);
/* IPV4 */
connection_t *nk_listen_on4(const char *port);
connection_t *nk_connect_to4(const char *hostname, 
							 const char *port);
/* IPV6 */
connection_t *nk_listen_on6(const char *port);
connection_t *nk_connect_to6(const char *hostname, 
							 const char *port);

/* accepts a connection, returns a new connection_t */
connection_t *nk_accept(connection_t *server_con);

int nk_send(connection_t *con, const char *msg);
int nk_send_len(connection_t *con, const char *msg, size_t len);
int nk_recv(connection_t *con, char *buf, size_t len);
int nk_recv_crlf(connection_t *con, char *buf, size_t len);
int nk_recv_with_delim(connection_t *con, 
						strlist_t *lines,
						  char *buf, 
						  size_t len, 
						  const char *delim);
void nk_close(connection_t *con);
void nk_print_connection(connection_t *con);

#endif /* _NETKIT_H_ */