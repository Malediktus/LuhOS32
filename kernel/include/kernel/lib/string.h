#ifndef __KERNEL_STRING_H
#define __KERNEL_STRING_H

#include <kernel/types.h>

void *memcpy(void *dest, const void *src, uint32_t n);
void *memset(void *s, int c, uint32_t n);
void *memmove(void *dest, const void *src, uint32_t n);
int memcmp(const void *s1, const void *s2, uint32_t n);
void *memchr(const void *s, unsigned char c, uint32_t n);
void *rawmemchr(const void *s, int c);
int strcmp(const char *s1, const char *s2);
int strncmp(const char *s1, const char *s2, uint32_t n);
char *strchr(const char *str, int c);
uint32_t strspn(const char *str, const char *chars);
uint32_t strcspn(const char *s1, const char *s2);
uint32_t strlen(const char *str);
char *strdup(const char *src);
char *strndup(char *src, uint32_t size);
char *strcat(char *dest, const char *src);
char *strpbrk(char *s1, const char *s2);
char *strtok(char *s, const char *delim);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, uint32_t n);
char *strtoupper(char *str);

#endif