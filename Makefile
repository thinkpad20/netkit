NK=bin/netkit.o

all: netkit

netkit: src/netkit.c
	gcc -c src/netkit.c -o $(NK)
