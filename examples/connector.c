#include "../include/netkit.h"

#define MAX_SIZE 50000

int main(int argc, char const *argv[]) {
	int fd;
	char *buf;
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}
	fd = nk_connect_to(argv[1], argv[2]);
	nk_send(fd, "GET / HTTP/1.1\r\n\r\n");
	buf = (char *)malloc(MAX_SIZE);
	size_t total_bytes = 0;
	while (1) {
		size_t r = nk_recv_with_delim(fd, buf + total_bytes, MAX_SIZE, "\r\n");
		if (!r) break;
		printf("%.*s\n", ((int)r), buf + total_bytes);
		total_bytes += r;
	}
	free(buf);
	return 0;
}