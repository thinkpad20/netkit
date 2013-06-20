#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	connection_t *my_con, *their_con;
	const char *port = "7890";
	if (argc == 2) port = argv[1];
	my_con = nk_listen_on4(port);
	if (!my_con) { puts("Error! :("); return 1; }
	printf("Server established on port %s!\n", port);
	nk_print_connection(my_con);
	
	their_con = nk_accept(my_con);
	printf("Client connected!\n");
	nk_print_connection(their_con);

	nk_send(their_con, "hello, world!\r\n");

	nk_close(their_con);
	nk_close(my_con);
	return 0;
}