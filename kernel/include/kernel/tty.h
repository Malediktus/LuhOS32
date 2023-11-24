#ifndef __KERNEL_TTY_H
#define __KERNEL_TTY_H

#include <kernel/types.h>
#include <kernel/dev/driver_manager.h>

void clear_tty(void);
void kprint_at_color(const char *message, uint32_t col, uint32_t row, uint8_t color);
void kprint_color(const char *message, uint8_t color);
void kprint_at(const char *message, uint32_t col, uint32_t row);
void kprint(const char *message);

#endif