#ifndef __KERNEL_TTY_H
#define __KERNEL_TTY_H

#include <kernel/types.h>
#include <kernel/dev/char_device.h>

#define TTY_BLACK 0x00
#define TTY_BLUE 0x01
#define TTY_GREEN 0x02
#define TTY_CYAN 0x03
#define TTY_RED 0x04
#define TTY_MAGENTA 0x05
#define TTY_BROWN 0x06
#define TTY_LIGHT_GRAY 0x07
#define TTY_DARK_GRAY 0x08
#define TTY_LIGHT_BLUE 0x09
#define TTY_LIGHT_GREEN 0x0A
#define TTY_LIGHT_CYAN 0x0B
#define TTY_LIGHT_RED 0x0C
#define TTY_LIGHT_MAGENTA 0x0D
#define TTY_YELLOW 0x0E
#define TTY_WHITE 0x0F

#define TTY_BG_BLACK 0x00
#define TTY_BG_BLUE 0x10
#define TTY_BG_GREEN 0x20
#define TTY_BG_CYAN 0x30
#define TTY_BG_RED 0x40
#define TTY_BG_MAGENTA 0x50
#define TTY_BG_BROWN 0x60
#define TTY_BG_LIGHT_GRAY 0x70
#define TTY_BG_GRAY 0x80
#define TTY_BG_LIGHT_BLUE 0x90
#define TTY_BG_LIGHT_GREEN 0xA0
#define TTY_BG_LIGHT_CYAN 0xB0
#define TTY_BG_LIGHT_RED 0xC0
#define TTY_BG_LIGHT_MAGENTA 0xD0
#define TTY_BG_YELLOW 0xE0
#define TTY_BG_WHITE 0xF0

#define TTY_COLOR(fg, bg) ((fg) | (bg))

void init_tty(char_device_t *cdev);
void clear_tty(void);
void kprintf(const char *fmt, ...);

#endif