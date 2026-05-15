#ifndef _MYSTRING_H
#define _MYSTRING_H
#include <stddef.h>
void *memcpy(void *dest, const void *src, size_t n);
void *memset(void *dest, int c, size_t n);
size_t strlen(const char *s);
int strcmp(const char *a, const char *b);
#endif