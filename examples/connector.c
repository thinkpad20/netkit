#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	char *buf = (char *)malloc(MAX_BUF);
	const char *hostname = "www.google.com";
	connection_t *con;
	if (argc == 2) { hostname = argv[1]; }

	/* connect_to4 ensures an IPV4 connection */
	if ((con = nk_connect_to4(hostname, "80")) == NULL) { 
		printf("Error connecting\n"); return 0; 
	}

	printf("Connected!\n");
	nk_print_connection(con);

	nk_send(con, "GET / HTTP/1.0\r\n\r\n"); /* 1.0 so they close after response */

	while (1) {
		size_t res = nk_recv_crlf(con, buf, MAX_BUF);
		if (res == 0) break;
		printf("%.*s\n", ((int)res), buf);
	}

	nk_close(con);
	return 0;
}