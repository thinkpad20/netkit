#include "../include/netkit.h"
#include "../include/netkit_http.h"

const char *response_fmt = "HTTP/1.1 200 OK\r\n"
					   "Server: Netkit Mini_HTTP\r\n"
					   "Content-length: %lu\r\n"
					   "Content-Type: text/html\r\n"
					   "\r\n"
					   "%s\r\n",
		   *port = "7890", 
		   *response_body = "<html><body>"
		   					"<b>Hello</b>, world!<br>"
		   					"<a href=\"https://github.com/thinkpad20/netkit\">"
		   					"Try netkit!</a></body></html>";

int main(int argc, char const *argv[]) {
	connection_t *my_con, *their_con;
	char *response = malloc(MAX_BUF);
	if (argc == 2) port = argv[1];
	my_con = nk_listen_on4(port);
	if (!my_con) { puts("Error! :("); return 1; }
	printf("Server established on port %s!\n", port);
	nk_print_connection(my_con);

	their_con = nk_accept(my_con);
	printf("Client connected!\n");
	nk_print_connection(their_con);

	printf("Client's message:\n");
	http_request_t *req = nk_parse_request(their_con);
	print_http_request(req);
	printf("Url given was: %s\n", nk_lookup_key(req, "URL"));

	sprintf(response, response_fmt, strlen(response_body), response_body);

	printf("Server response:\n%s", response);
	nk_send(their_con, response);
	puts("finished sending response");
	nk_close(their_con);
	nk_close(my_con);
	return 0;
}