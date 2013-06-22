#ifndef __STRLIST_H_
#define __STRLIST_H_

#include "list.h"

typedef struct strlist_s {
    List list;
} strlist_t;

void sl_init(strlist_t *sl);
void sl_enqueue(strlist_t *sl, const char *str);
char *sl_dequeue(strlist_t *sl);
bool sl_isEmpty(strlist_t *sl);
void sl_delete(strlist_t *sl);
void sl_print(strlist_t *sl);
char *sl_removeBack(strlist_t *sl);
char *sl_find(strlist_t *sl, const char *str);
strlist_t sl_split(const char *string, const char *delim, size_t len);
size_t sl_split_reset(strlist_t *res, char *string, const char *delim, size_t len);
size_t sl_size(strlist_t *sl);

void printLiteralChar(char c);
void printLiteral(const char *str, int len);
#endif
