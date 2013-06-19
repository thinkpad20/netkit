#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	int socketfd = nk_listen_on("7890");
	connection_t con;
	nk_accept(socketfd, &con);
	printf("client connected, IP: %s.\n", con.ip);
	nk_send(con.file_dest, "hello, world!\r\n");
	return 0;
}