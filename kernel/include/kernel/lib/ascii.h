#ifndef __KERNEL_ASCII_H
#define __KERNEL_ASCII_H

#include <kernel/types.h>

void int_to_ascii(int n, char str[]);
uint8_t key_code_to_ascii(uint32_t key);

#endif