#include "strlist.c"

int main(int argc, char const *argv[])
{
	char *str = strdup("hey how are you?\r\nyoyoyo");
	strlist_t list;
	sl_init(&list);
	printf("contains %lu\n", sl_split_reset(&list, str, "\r\n", strlen(str)));
	printf("buffer has '%s'\n", str);
	return 0;
}