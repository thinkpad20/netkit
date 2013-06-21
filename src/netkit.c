#include "../include/netkit.h"
#include <assert.h>

typedef enum connection_options {
	FREE_HOSTNAME_OPT = 1,
	FREE_IP_OPT = 2,
	FREE_PORT_OPT = 4,
	IPV6_OPT = 8,
	UDP_OPT = 16,
} conopt;

typedef enum connection_type {
	SERVER_TYPE,
	CLIENT_TYPE,
	INCOMING_TYPE,
} contype;

typedef uint32_t bool_t;

/*
connection_t->options bits:

1: 0/1->don't free/free hostname (default 1)
2: 0/1-> don't free/free ip (default 1)
3: 0/1-> don't free/free port (default 1)
4: 0->ipv4, 1->ipv6 (default 0)
5: 0->TCP, 1->UDP (default 0)

*/

static void set_type(connection_t *con, contype type);
static bool_t get_option(connection_t *con, conopt option);
static void set_option(connection_t *con, conopt option, bool_t plus);
static void make_default_con(connection_t *con, contype type);
static size_t addr_len(connection_t *con);
static connection_t *_nk_listen_on(const char *port, int family);
static connection_t *_nk_connect_to(const char *, const char *, int);

static void
set_type(connection_t *con, contype type) {
	con->type = type;
}

static bool_t 
get_option(connection_t *con, conopt option) {
	return option & con->options;
}

static void 
set_option(connection_t *con, conopt option, bool_t plus) {
	if (plus)
		con->options |= option;
	else {
		con->options = ~con->options;
		con->options |= option;
		con->options = ~con->options;
	}
}

void 
printLiteralChar(char c) {
	switch (c) {
		case '\0':
			printf("\\0");
			break;
		case '\n':
			printf("\\n");
			break;
		case '\r':
			printf("\\r");
			break;
		case '\t':
			printf("\\t");
			break;
		default:
			printf("%c", c);
			break;
	}    
}

void 
printLiteral(const char *str, int len) {
	int i;
	for (i=0; i<len; ++i) {
		printLiteralChar(str[i]);
	}
}

static void
make_default_con(connection_t *con, contype type) {
	memset(con, 0, sizeof(connection_t));
	set_type(con, type);
	if (type == CLIENT_TYPE)
		set_option(con, FREE_HOSTNAME_OPT, 1);
	if (type != INCOMING_TYPE)
		set_option(con, FREE_PORT_OPT, 1);
	set_option(con, FREE_IP_OPT, 1);
}

static size_t 
addr_len(connection_t *con) {
	if (get_option(con, IPV6_OPT))
		return INET6_ADDRSTRLEN;
	else
		return INET_ADDRSTRLEN;
}

static connection_t *
_nk_listen_on(const char *port, int family) {
	int status, socket_res = -1, yes = 1;
	struct addrinfo hints, *res, *p;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;
	connection_t *con;

	if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
		perror("getting address info");
		return NULL;
	}

	for(p = res; p != NULL; p = p->ai_next)  {
		if ((socket_res = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Could not open socket");
			continue;
		}

		if (setsockopt(socket_res, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("Socket setsockopt() failed");
			close(socket_res);
			continue;
		}

		if (bind(socket_res, p->ai_addr, p->ai_addrlen) == -1) {
			perror("Socket bind() failed");
			close(socket_res);
			continue;
		}

		if (listen(socket_res, 5) == -1) {
			perror("Socket listen() failed");
			close(socket_res);
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "Could not find a socket to bind to.\n");
		return NULL;
	}

	con = (connection_t *)malloc(sizeof(connection_t));

	make_default_con(con, 0);

	/* set if this is an ipv6 or ipv4 connection */
	set_option(con, IPV6_OPT, (p->ai_family == AF_INET) ? 0 : 1);	

	con->fd = socket_res;
	con->port = strdup(port);
	con->ip = (char *)malloc(addr_len(con));

	/* put in the IP address */
	if (AF_INET == p->ai_family) {
		con->ip_int = malloc(4); /* 32 bits (4*8) in IPv4 addr */
		memcpy(con->ip_int, 
				 &((struct sockaddr_in *)p->ai_addr)->sin_addr.s_addr, 
				 4);
	} else {
		con->ip_int = malloc(16); /* 128 bits (16*8) in IPv6 addr */
		memcpy(con->ip_int,
				 &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr, 
				 16);
	}
	inet_ntop(p->ai_family, 
				 con->ip_int, 
				 con->ip, 
				 addr_len(con));

	freeaddrinfo(res);
	return con;
}

connection_t *
nk_listen_on(const char *port) {
	return _nk_listen_on(port, AF_UNSPEC);
}

connection_t *
nk_listen_on4(const char *port) {
	return _nk_listen_on(port, AF_INET);
}

connection_t *
nk_listen_on6(const char *port) {
	return _nk_listen_on(port, AF_INET6);
}

static connection_t *
_nk_connect_to(const char *hostname, const char *port, int family) {
	int status, socket_res, yes = 1;
	struct addrinfo hints, *res, *p;
	connection_t *con;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = family;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
		perror("getting address info");
		return NULL;
	}

	for (p = res; p != NULL; p = p->ai_next) {
		if ((socket_res = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("Could not open socket");
			continue;
		}

		if (setsockopt(socket_res, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("Socket setsockopt() failed");
			close(socket_res);
			continue;
		}

		if (connect(socket_res, p->ai_addr, p->ai_addrlen) == -1) {
			perror("connect() failed");
			close(socket_res);
			continue;
		}
		/* if we get this far, we can stop */
		break;
	}

	if (p == NULL) {
		perror("could not find a valid socket");
		return NULL;
	}

	con = (connection_t *)malloc(sizeof(connection_t));
	make_default_con(con, 1);

	/* set if this is an ipv6 or ipv4 connection */
	set_option(con, IPV6_OPT, (p->ai_family == AF_INET6));

	con->fd = socket_res;
	con->hostname = strdup(hostname);
	con->port = strdup(port);
	con->ip = (char *)malloc(addr_len(con));

	/* put in the IP address */
	if (AF_INET == p->ai_family) {
		con->ip_int = malloc(4); /* 32 bits (4*8) in IPv4 addr */
		memcpy(con->ip_int, 
				 &((struct sockaddr_in *)p->ai_addr)->sin_addr.s_addr, 
				 4);
	} else {
		con->ip_int = malloc(16); /* 128 bits (16*8) in IPv6 addr */
		memcpy(con->ip_int,
				 &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr, 
				 16);
	}
	inet_ntop(p->ai_family, 
				 con->ip_int, 
				 con->ip, 
				 addr_len(con));
	freeaddrinfo(res);
	return con;
}

connection_t *
nk_connect_to(const char *hostname, const char *port) {
	return _nk_connect_to(hostname, port, AF_UNSPEC);
}

connection_t *
nk_connect_to4(const char *hostname, const char *port) {
	return _nk_connect_to(hostname, port, AF_INET);
}

connection_t *
nk_connect_to6(const char *hostname, const char *port) {
	return _nk_connect_to(hostname, port, AF_INET6);
}

size_t 
nk_send_len(connection_t *con, const char *msg, size_t len) {
	size_t bytes_sent = 0;
	int res;
	while (bytes_sent < len) {
		res = send(con->fd, msg + bytes_sent, len - bytes_sent, 0);
		if (res >= 0) {
			#ifdef __VERBOSE__
			fprintf(stderr, "sent %d bytes\n", res);
			#endif
			bytes_sent += res;
		}
		else {
			perror("writing to socket");
			return res;
		}
	}
	return bytes_sent;
}

size_t
nk_send(connection_t *con, const char *msg) {
	return nk_send_len(con, msg, strlen(msg));
}

size_t 
nk_recv(connection_t *con, char *buf, size_t len) {
	int res;
	size_t bytes_recvd = 0;
	while (1) {
		res = recv(con->fd, buf + bytes_recvd, len - bytes_recvd, 0);
		if (res < 0) {
			perror("reading from socket");
			return res;
		}
		if (res == 0) {
			#ifdef __VERBOSE__
			printf("end of transmission\n");
			#endif
			break;
		}
		bytes_recvd += res;
	}
	return bytes_recvd;
}

size_t 
nk_recv_with_delim(connection_t *con, 
						 char *buf, 
						 size_t len,
						 const char *delim) {
	int res;
	size_t bytes_recvd = 0, delim_len = strlen(delim), to_recv;
	char *in_buf = (char *)malloc(delim_len + 1);
	while (1) {
		if (bytes_recvd >= len) break;
		to_recv = (len - bytes_recvd < delim_len) ? len - bytes_recvd : delim_len;

		res = recv(con->fd, in_buf, to_recv, 0);
		in_buf[res] = '\0';

		if (res < 0) {
			perror("reading from socket");
			return res;
		}

		if (res == 0) {
			break;
		}
		memcpy(buf + bytes_recvd, in_buf, res);
		bytes_recvd += res;
		if (!strncmp(in_buf, delim, res)) {
			break;
		}
	}
	free(in_buf);
	return bytes_recvd;
}

size_t
nk_recv_crlf(connection_t *con, char *buf, size_t len) {
	return nk_recv_with_delim(con, buf, len, "\r\n");
}

connection_t *
nk_accept(connection_t *server_con) {
	struct sockaddr_in saddr;
	struct sockaddr_in6 saddr6;
	socklen_t slen = sizeof(struct sockaddr_in);
	socklen_t slen6 = sizeof(struct sockaddr_in6);
	connection_t *in_con;
	int fd;
	if (!server_con) {
		fprintf(stderr, "Must supply a server-side connection\n");
		return NULL;
	}

	/* depending on if we're ipv6 or v4, we'll accept one or the other */
	if (get_option(server_con, IPV6_OPT)) {
		fd = accept(server_con->fd, (struct sockaddr *) &saddr6, &slen6);
	} else {
		fd = accept(server_con->fd, (struct sockaddr *) &saddr, &slen);
	}

	in_con = (connection_t *)malloc(sizeof(connection_t));
	if (!in_con) return NULL;
	make_default_con(in_con, INCOMING_TYPE);

	/* set the incoming connection's ip version to match the server's */
	set_option(in_con, IPV6_OPT, get_option(server_con, IPV6_OPT));

	/* store the file descriptor */
	in_con->fd = fd;

	/* store the IP address string */
	in_con->ip = (char *)malloc(addr_len(in_con));
	assert(in_con->ip && "Error in memory allocation");

	/* put in the IP address */
	if (get_option(server_con, IPV6_OPT)) {
		in_con->ip_int = malloc(16); /* 128 bits (16*8) in IPv6 addr */
		assert(in_con->ip_int && "Error in memory allocation");
		memcpy(in_con->ip_int, &saddr6.sin6_addr, 16);
	} else {
		in_con->ip_int = malloc(4); /* 32 bits (4*8) in IPv4 addr */
		assert(in_con->ip_int && "Error in memory allocation");
		memcpy(in_con->ip_int, &saddr.sin_addr.s_addr, 4);
	}
	/* put in the IP address string */
	inet_ntop(get_option(server_con, IPV6_OPT) ? AF_INET6 : AF_INET,
				 in_con->ip_int, 
				 in_con->ip, 
				 addr_len(in_con));
	return in_con;
}

void 
nk_close(connection_t *con) {
	if (get_option(con, FREE_HOSTNAME_OPT)) { free(con->hostname); }
	if (get_option(con, FREE_IP_OPT)) {	free(con->ip);	free(con->ip_int); }
	if (get_option(con, FREE_PORT_OPT)) { free(con->port); }
	close(con->fd);
	free(con);
}

void
nk_print_connection(connection_t *con) {
	fprintf(stderr, "Connection:\n\tType: ");
	switch (con->type) {
		case SERVER_TYPE:
			fprintf(stderr, "Server (listening) type\n");
			break;
		case CLIENT_TYPE:
			fprintf(stderr, "Client (seeking) type\n");
			break;
		case INCOMING_TYPE:
			fprintf(stderr, "Incoming (received) type\n");
			break;
		default:
			fprintf(stderr, "Unknown type\n");
	}
	if (get_option(con, IPV6_OPT)) {
		fprintf(stderr, "\tVersion: IPV6\n");
	} else {
		fprintf(stderr, "\tVersion: IPV4\n");
	}
	fprintf(stderr, "\tIP address: %s\n", con->ip);
	fprintf(stderr, "\tHostname: %s\n", con->hostname);
	fprintf(stderr, "\tPort: %s\n", con->port);
}