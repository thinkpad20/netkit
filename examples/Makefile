CC=clang
LIB=../bin/netkit.o

all: connector hws telnet mini_http

connector: connector.c
	$(CC) -o connector connector.c $(LIB)

hws: hello_world_server.c
	$(CC) -o hello_world_server hello_world_server.c $(LIB)

telnet: netkit_telnet.c
	$(CC) -o nk_telnet netkit_telnet.c $(LIB)

mini_http: mini_http.c
	$(CC) -o mini_http mini_http.c $(LIB)

clean:
	rm connector
	rm hello_world_server
	rm nk_telnet
	rm mini_http