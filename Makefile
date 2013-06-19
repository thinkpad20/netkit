all: netkit

netkit: src/netkit.c
	gcc -c src/netkit.c -o bin/netkit.o