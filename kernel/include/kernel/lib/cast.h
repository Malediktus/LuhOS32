#ifndef __KERNEL_CAST_H
#define __KERNEL_CAST_H

#include <kernel/types.h>
#include <kernel/tty.h>

void uint32_to_hex_string(uint32_t value, char *hex_string);
void uint16_to_hex_string(uint16_t value, char *hex_string);
void uint8_to_hex_string(uint8_t value, char *hex_string);

void uint32_to_string(uint32_t value, char *str);
void uint16_to_string(uint16_t value, char *str);
void uint8_to_string(uint8_t value, char *str);

void int32_to_string(int32_t value, char *str);
void int16_to_string(int16_t value, char *str);
void int8_to_string(int8_t value, char *str);

#endif