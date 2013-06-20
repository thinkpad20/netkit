#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	connection_t *my_con, *their_con;
	my_con = nk_listen_on4("7890"),
	printf("Server established!\n");
	nk_print_connection(my_con);

	their_con = nk_accept(my_con);
	printf("Client connected!\n");
	nk_print_connection(their_con);

	nk_send(their_con, "hello, world!\r\n");

	nk_close(their_con);
	nk_close(my_con);
	return 0;
}