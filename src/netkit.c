#include "../include/netkit.h"
#include "../include/list.h"
#include <assert.h>

#define __VERBOSE__

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
static connection_t *con_new(contype type);
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

/*static void
printLiteralChar(char *buf, char c) {
	switch (c) {
		case '\0':
			strcat(buf, "\\0");
			break;
		case '\n':
			strcat(buf, "\\n");
			break;
		case '\r':
			strcat(buf, "\\r");
			break;
		case '\t':
			strcat(buf, "\\t");
			break;
		default:
			sprintf(buf, "%s%c", buf, c);
			break;
	}    
}

static char * 
printLiteral(const char *str, char *buf, int len) {
	int i;
	for (i=0; i<len; ++i) {
		printLiteralChar(buf, str[i]);
	}
	return buf;
}*/

static connection_t *
con_new(contype type) {
	connection_t *con = (connection_t *)calloc(1, sizeof(connection_t));
	set_type(con, type);
	if (type == CLIENT_TYPE)
		set_option(con, FREE_HOSTNAME_OPT, 1);
	if (type != INCOMING_TYPE)
		set_option(con, FREE_PORT_OPT, 1);
	set_option(con, FREE_IP_OPT, 1);
	sl_init(&con->lines);
	return con;
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

	con = con_new(SERVER_TYPE);

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

	con = con_new(CLIENT_TYPE);

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

int 
nk_send_len(connection_t *con, const char *msg, size_t len) {
	size_t bytes_sent = 0;
	int res;
	#ifdef __VERBOSE__
	fprintf(stderr, "Preparing to send %lu bytes\n", len);
	#endif
	while (bytes_sent < len) {
		res = send(con->fd, msg + bytes_sent, len - bytes_sent, 0);
		if (res >= 0) {
			#ifdef __VERBOSE__
			fprintf(stderr, "sent %d bytes to file descriptor %d\n", res, con->fd);
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

int
nk_send(connection_t *con, const char *msg) {
	return nk_send_len(con, msg, strlen(msg));
}

int 
nk_recv(connection_t *con, char *buf, size_t len) {
	int res, bytes_recvd = 0;
	while (1) {
		res = recv(con->fd, buf + bytes_recvd, len - bytes_recvd, 0);
		if (res < 0) {
			perror("reading from socket");
			return res;
		}
		if (res == 0) {
			printf("wtf? res is %d?\n", res);
			#ifdef __VERBOSE__
			printf("end of transmission\n");
			#endif
			break;
		}
		bytes_recvd += res;
	}
	return bytes_recvd;
}

int 
nk_recv_with_delim(connection_t *con, 
						 strlist_t *prevs,
						 char *buf, 
						 size_t len,
						 const char *delim) {
	if (!con->buf) con->buf = (char *)calloc(1, MAX_BUF);
	/* if we have something in the queue, we can copy it in and return it */
	if (sl_size(prevs) == 0) {
		/* otherwise, pull data from the socket until we get something */
		int len2;
		while (1) {
			puts("waiting for data");
			size_t dlen = strlen(delim);
			len2 = nk_recv(con, con->buf + con->buf_in, dlen);
			if (len2 <= 0) return len2; /* connection closed or error */
			printf("received some data: '");
			printLiteral(con->buf, len2);
			printf("' (%d bytes)\n", len2);
			con->buf_in += len2;
			if (sl_split_reset(prevs, con->buf, delim, con->buf_in) > 0)
				break;
		}
	}
	const char *str = sl_dequeue(prevs);
	memcpy(buf, str, (strlen(str) < len) ? strlen(str) : len);
	return strlen(str);
}

int
nk_recv_crlf(connection_t *con, char *buf, size_t len) {
	return nk_recv_with_delim(con, &con->lines, buf, len, "\r\n");
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

	in_con = con_new(INCOMING_TYPE);

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
	if (con->buf) free(con->buf);
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
	fprintf(stderr, "\tFile descriptor: %d\n", con->fd);
}
