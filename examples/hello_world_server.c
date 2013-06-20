#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	connection_t *my_con = nk_listen_on4("7890"), *their_con = nk_accept(my_con);
	nk_send(their_con, "hello, world!\r\n");
	nk_close(their_con);
	nk_close(my_con);
}