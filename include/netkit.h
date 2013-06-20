#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>

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
	char *port;
} connection_t;

connection_t *nk_listen_on(connection_t *con, const char *port);
connection_t *nk_connect_to(connection_t *con, 
							const char *hostname, 
							const char *port);
connection_t *nk_accept(connection_t *server_con, connection_t *in_con);
size_t nk_send(connection_t *con, const char *msg);
size_t nk_recv(connection_t *con, char *buf, size_t len);
size_t nk_recv_with_delim(connection_t *con, char *buf, size_t len, const char *delim);
void nk_close(connection_t *con);