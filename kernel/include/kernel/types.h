#ifndef __KERNEL_TYPES_H
#define __KERNEL_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void kprint_color(const char *message, uint8_t color);

#define PANIC_PRINT(msg)                                                \
  kprint_color("kernel panic... the system can not continue:\n", 0xf4); \
  kprint_color(msg, 0xf4);                                              \
  while (true)                                                          \
    ;

#define PANIC_CODE(code)                                                \
  kprint_color("kernel panic... the system can not continue:\n", 0xf4); \
  code;                                                                 \
  while (true)                                                          \
    ;

#define PAGE_SIZE 4096

#define EOK 0
#define EIO 1
#define EINVARG 2
#define ENOMEM 3
#define EBADPATH 4
#define EFSNOTUS 5
#define ERDONLY 6
#define EUNIMP 7
#define EISTKN 8
#define EINFORMAT 9
#define EHRDWRE 10

const char *string_error(uint32_t error); // defined in ascii.c

#endif