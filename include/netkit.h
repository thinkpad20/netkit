#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/types.h>
#include <string.h>

typedef struct connection_s {
	int file_dest;
	const char *hostname;
	char ip[INET_ADDRSTRLEN];
	const char *port;
} connection_t;

int nk_listen_on(const char *port);
int nk_connect_to(const char *hostname, const char *port);
int nk_accept(int fd, connection_t *con);
size_t nk_send(int socket, const char *msg);
size_t nk_recv(int socket, char *buf, size_t len);
size_t nk_recv_with_delim(int socket, char *buf, size_t len, const char *delim);