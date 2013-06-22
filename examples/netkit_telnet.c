#include "../include/netkit.h"
#include <pthread.h>

const char *http_msg = "GET / HTTP/1.1\r\n\r\n";
const size_t resp_len = 50000;
const size_t MAX_LINE = 50000;

void telnet_send(connection_t *con, const char *msg) {
	char *buf = (char *)malloc(strlen(msg) + 2);
	sprintf(buf, "%s\r\n", msg);
	nk_send(con, buf);
	free(buf);
}

struct loop_args {
	connection_t *con;
};

void *telnet_write_loop(void *args) {
	connection_t *con = ((struct loop_args *)args)->con;
	char *out_buf = (char *)calloc(1, MAX_LINE), c;
	size_t i;
	while (1) {
		i = 0;
		printf("> ");
		while ((c = getchar()) != '\n') {
			out_buf[i++] = c;
		}
		out_buf[i] = '\0';
		if (!strcmp(out_buf, "exit")) break;
		telnet_send(con, out_buf);
	}
	printf("Connection closed by client\n");
	free(out_buf);
	return NULL;
}

void *telnet_read_loop(void *args) {
	connection_t *con = ((struct loop_args *)args)->con;
	char *in_buf = (char *)calloc(1, resp_len);
	while (1) {
		size_t len = nk_recv_crlf(con, in_buf, resp_len);
		printf("%.*s\n", (int)len, in_buf);
		// printf("received %lu bytes\n", len);
		if (len == 0) break;
	}
	printf("Connection closed by foreign host\n");
	free(in_buf);
	return NULL;
}

int main(int argc, char const *argv[]) {
	
	pthread_t write_thread, read_thread;

	if (argc < 3) {
		printf("please supply server and port number\n");
		exit(0);
	}

	printf("attempting to connect to %s, port %s\n", argv[1], argv[2]);

	connection_t *con = nk_connect_to(argv[1], argv[2]);

	if (con) 
		printf("connected, file descriptor %d, ip %s\n", con->fd, con->ip);
	else 
		return 1;

	struct loop_args args;
	args.con = con;
	
	pthread_create(&write_thread, NULL, telnet_write_loop, &args);
	pthread_create(&read_thread, NULL, telnet_read_loop, &args);

	pthread_join(read_thread, NULL);
	pthread_join(write_thread, NULL);

	nk_close(con);
	printf("Thanks for using telnet today.\n");
	return 0;
}