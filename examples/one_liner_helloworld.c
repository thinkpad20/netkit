#include "../include/netkit.h"

int main(int argc, char const *argv[]) {
	nk_send(nk_accept(nk_listen_on("7890")), "hello, world!\r\n");
}