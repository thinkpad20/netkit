CC=clang
LIB=../bin/netkit.o ../bin/netkit_http.o ../bin/list.o

all: connector telnet mini_http one_liner

connector: connector.c
	$(CC) -o connector connector.c $(LIB)

telnet: netkit_telnet.c
	$(CC) -o nk_telnet netkit_telnet.c $(LIB)

mini_http: mini_http.c
	$(CC) -o mini_http mini_http.c $(LIB)

one_liner: one_liner_helloworld.c
	$(CC) -o one_liner one_liner_helloworld.c $(LIB)

clean:
	rm connector
	rm nk_telnet
	rm mini_http
	rm one_liner