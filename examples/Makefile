all: connector hws telnet

connector: connector.c
	$(CC) -o connector connector.c ../bin/netkit.o

hws: hello_world_server.c
	$(CC) -o hello_world_server hello_world_server.c ../bin/netkit.o

telnet: netkit_telnet.c
	$(CC) -o nk_telnet netkit_telnet.c ../bin/netkit.o

clean:
	rm connector
	rm hello_world_server
	rm nk_telnet