#include "../include/netkit.h"

#define MAX_SIZE 50000

int main(int argc, char const *argv[]) {
	char *buf;
	connection_t con;
	if (argc < 3) {
		fprintf(stderr, "Usage: %s <hostname> <port>\n", argv[0]);
		exit(0);
	}
	nk_connect_to(&con, argv[1], argv[2]);
	nk_send(&con, "GET / HTTP/1.1\r\n\r\n");
	buf = (char *)malloc(MAX_SIZE);
	size_t total_bytes = 0;
	while (1) {
		size_t r = nk_recv_with_delim(&con, buf + total_bytes, MAX_SIZE, "\r\n");
		if (!r) break;
		printf("%.*s\n", ((int)r), buf + total_bytes);
		total_bytes += r;
	}
	free(buf);
	nk_close(&con);
	return 0;
}