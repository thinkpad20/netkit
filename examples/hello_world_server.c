#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	connection_t *my_con = nk_listen_on(NULL, "7890"),
				 *their_con = nk_accept(my_con, NULL);
	printf("client connected, IP: %s.\n", their_con->ip);
	nk_send(their_con, "hello, world!\r\n");

	nk_close(their_con);
	nk_close(my_con);
	return 0;
}