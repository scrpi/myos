#ifndef _STRING_H
#define _STRING_H 1

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

int memcmp(const void*, const void*, size_t);
void* memcpy(void* __restrict, const void* __restrict, size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);

int strcmp(const char *a, const char *b);
int strncmp(const char *a, const char *b, size_t num);
size_t strlen(const char*);

#ifdef __cplusplus
}
#endif

#endif
