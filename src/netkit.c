#include "../include/netkit.h"

void printLiteralChar(char c) {
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

void printLiteral(const char *str, int len) {
	int i;
    for (i=0; i<len; ++i) {
        printLiteralChar(str[i]);
    }
}

int nk_listen_on(const char *port) {
	int status, socket_res = -1, yes = 1;
	struct addrinfo hints, *res, *p;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(NULL, port, &hints, &res)) != 0) {
		perror("getting address info");
		return status;
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

	freeaddrinfo(res);

	if (p == NULL) {
		fprintf(stderr, "Could not find a socket to bind to.\n");
	}

	return socket_res;
}

int nk_connect_to(const char *hostname, const char *port) {
	int status, socket_res, yes = 1;
	struct addrinfo hints, *res, *p;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(hostname, port, &hints, &res)) != 0) {
		perror("getting address info");
		return status;
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

        break;
	}

	freeaddrinfo(res);

	if (p == NULL) {
		perror("could not find a valid socket");
	}

	return socket_res;
}

size_t nk_send(int socket, const char *msg) {
	size_t len = strlen(msg), bytes_sent = 0;
	int res;
	while (bytes_sent < len) {
		res = send(socket, msg + bytes_sent, len - bytes_sent, 0);
		if (res >= 0) {
			printf("sent %d bytes\n", res);
			bytes_sent += res;
		}
		else {
			perror("writing to socket");
			return res;
		}
	}
	return bytes_sent;
}

size_t nk_recv(int socket, char *buf, size_t len) {
	int res;
	size_t bytes_recvd = 0;
	while (1) {
		res = recv(socket, buf + bytes_recvd, len - bytes_recvd, 0);
		if (res < 0) {
			perror("reading from socket");
			return res;
		}
		if (res == 0) {
			printf("end of transmission\n");
			break;
		}
		bytes_recvd += res;
	}
	return bytes_recvd;
}

size_t nk_recv_with_delim(int socket, char *buf, size_t len, const char *delim) {
	int res;
	size_t bytes_recvd = 0, delim_len = strlen(delim), to_recv;
	char *in_buf = (char *)malloc(delim_len + 1);
	while (1) {
		if (bytes_recvd >= len) break;
		to_recv = (len - bytes_recvd < delim_len) ? len - bytes_recvd : delim_len;

		res = recv(socket, in_buf, to_recv, 0);
		in_buf[res] = '\0';

		if (res < 0) {
			perror("reading from socket");
			return res;
		}

		if (res == 0) {
			// printf("end of transmission\n");
			break;
		}
		memcpy(buf + bytes_recvd, in_buf, res);
		bytes_recvd += res;
		if (!strncmp(in_buf, delim, res)) {
			// printf("found delimiter\n");
			break;
		}
	}
	free(in_buf);
	return bytes_recvd;
}

int nk_accept(int fd, connection_t *con) {
	struct sockaddr_in saddr;
	socklen_t slen = sizeof(struct sockaddr_in);
	int newfd = accept(fd, (struct sockaddr *) &saddr, &slen);
	con->file_dest = newfd;
	inet_ntop(AF_INET, &saddr.sin_addr.s_addr, con->ip, INET_ADDRSTRLEN);
	return newfd;
}