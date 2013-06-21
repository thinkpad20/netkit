#include "../include/netkit.h"

const char *response_fmt = "HTTP/1.1 200 OK\r\n"
					   "Server: Netkit Mini_HTTP\r\n"
					   "Content-length: %lu\r\n"
					   "Content-Type: text/html\r\n"
					   "\r\n"
					   "%s\r\n";

int main(int argc, char const *argv[]) {
	connection_t *my_con, *their_con;
	const char *port = "7890", 
			   *response_body = "<html><body>"
			   					"<b>Hello</b>, world!<br>"
			   					"<a href=\"https://github.com/thinkpad20/netkit\">"
			   					"Try netkit!</a></body></html>";
	char *buf = malloc(MAX_BUF), *response = malloc(MAX_BUF);
	if (argc == 2) port = argv[1];
	my_con = nk_listen_on4(port);
	int i;
	if (!my_con) { puts("Error! :("); return 1; }
	printf("Server established on port %s!\n", port);
	nk_print_connection(my_con);

	their_con = nk_accept(my_con);
	printf("Client connected!\n");
	nk_print_connection(their_con);

	printf("Client's message:\n");
	for (i=0; i<10; ++i) {
		int len = nk_recv_crlf(their_con, buf, MAX_BUF);
		if (i == 0) {
			http_header_t *hdr = nk_read_header(buf, len);
		}
		printf("%.*s", len, buf);
		if (!strncmp(buf, "\r\n", 2))
			break;
	}

	sprintf(response, response_fmt, strlen(response_body), response_body);

	printf("Server response:\n%s", response);
	nk_send(their_con, response);

	nk_close(their_con);
	nk_close(my_con);
	return 0;
}