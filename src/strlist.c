#include "../include/strlist.h"

static void 
free_str(void *strp) { 
	free(strp); 
}

void sl_init(strlist_t *sl) { 
	list_init(&sl->list, free_str); 
}

void sl_enqueue(strlist_t *sl, const char *msg) { 
	list_addBack(&sl->list, strdup(msg)); 
}

char *sl_dequeue(strlist_t *sl) { 
	return (char *)list_removeFront(&sl->list); 
}

bool sl_isEmpty(strlist_t *sl) { 
	return sl->list.size == 0; 
}

char *charPtoStr(void *charp) { 
	return (char *)charp; 
}

void sl_print(strlist_t *sl) { 
	list_printCustom(&sl->list, charPtoStr, false); 
}

void sl_delete(strlist_t *sl) { 
	list_destroy(&sl->list); 
}

char *sl_removeBack(strlist_t *sl) { 
	return (char *)list_removeBack(&sl->list); 
}

void loadString(char *buf, void *charp) { 
	strcpy(buf, (char *) charp); 
}

bool sl_contains(strlist_t *sl, const char *str) { 
	return list_findByString(&sl->list, loadString, str);
}

void printLiteralChar(char c) {
	switch (c) {
		case '\0':
			printf("\\0");
			break;
		case '\n':
			printf("\\n");
			break;
		case '\r':
			printf("\\r");
			break;
		case '\t':
			printf("\\t");
			break;
		default:
			printf("%c", c);
			break;
	}    
}

void printLiteral(const char *str, int len) {
	int i;
	for (i=0; i<len; ++i) {
		printLiteralChar(str[i]);
	}
}

strlist_t
sl_split(const char *string, const char *delim, size_t len) {
	size_t i, j=0, dlen = strlen(delim);
	strlist_t res;
	sl_init(&res);
	for (i=0; i<len; ++i) {
		if (!strncmp(string + i, delim, dlen) || i == len - 1) {
			if (i != j)
				sl_enqueue(&res, strndup(string + j, (i != len-1) ? i-j : i-j+1));
			j = i + dlen;
		}
	}
	return res;
}

size_t
sl_split_reset(strlist_t *res, char *string, const char *delim, size_t len) {
	size_t i, j=0, dlen = strlen(delim);
	for (i=0; i<len; ++i) {
		if (!strncmp(string + i, delim, dlen)) {
			if (i != j)
				sl_enqueue(res, strndup(string + j, i-j));
			j = i + dlen;
		}
	}
	/* shift any remaining content to the beginning of the buffer */
	if (j < len) {
		memmove(string, string+j, len-j);
		memset(string + len - j, 0, j);
	}
	return sl_size(res);
}


size_t sl_size(strlist_t *sl) {
	return sl->list.size;
}