#include <stdio.h>

void fuzz() { puts ("yo"); }

int main(int argc, char const *argv[])
{
	FILE *fp = fopen("hello_world_server.c", "r");
	char c;
	if (!fp) return 0;
	while ((c = fgetc(fp)) != EOF) putchar(c);
	return 0;
}