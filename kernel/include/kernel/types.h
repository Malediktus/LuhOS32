#ifndef __KERNEL_TYPES_H
#define __KERNEL_TYPES_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

void kprintf(const char *message, ...);

#define PANIC_PRINT(msg)                                                      \
  kprintf("\033[41mkernel panic... the system can not continue:\n%s\n", msg); \
  while (true)                                                                \
    ;

#define PANIC_CODE(code)                                             \
  kprintf("\033[41mkernel panic... the system can not continue:\n"); \
  code;                                                              \
  kprintf("\n");                                                     \
  while (true)                                                       \
    ;

#define PAGE_SIZE 4096

#define EOK 0
#define EIO 1
#define EINVARG 2
#define ENOMEM 3
#define EBADFS 4
#define EHRDWRE 10

const char *string_error(uint32_t error); // defined in ascii.c

#endif