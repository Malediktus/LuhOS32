#ifndef __KERNEL_CAST_H
#define __KERNEL_CAST_H

#include <kernel/types.h>
#include <kernel/tty.h>

void uint32_to_hex_string(uint32_t value, char *hex_string);
void uint16_to_hex_string(uint16_t value, char *hex_string);

#define PRINT_UINT16_HEX(prefix, value) \
  {                                     \
    char str[9];                        \
    uint16_to_hex_string(value, str);   \
    kprint(prefix "0x");                \
    kprint(str);                        \
    kprint("\n");                       \
  }

#define PRINT_UINT32_HEX(prefix, value) \
  {                                     \
    char str[9];                        \
    uint32_to_hex_string(value, str);   \
    kprint(prefix "0x");                \
    kprint(str);                        \
    kprint("\n");                       \
  }

#endif