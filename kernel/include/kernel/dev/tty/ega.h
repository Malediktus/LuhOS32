#ifndef __KERNEL_EGA_H
#define __KERNEL_EGA_H

#include <kernel/types.h>
#include <kernel/dev/char_device.h>
#include <kernel/multiboot2.h>

uint32_t ega_driver_init(char_device_t *cdev, struct multiboot_tag_framebuffer *framebuffer);
uint32_t ega_driver_get_width(char_device_t *cdev);
uint32_t ega_driver_get_height(char_device_t *cdev);
uint32_t ega_driver_write_tty(char_device_t *cdev, uint32_t x, uint32_t y, uint8_t color, const char c);
uint32_t ega_driver_scroll_line(char_device_t *cdev);

#endif