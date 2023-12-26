#ifndef __KERNEL_EGA_H
#define __KERNEL_EGA_H

#include <kernel/types.h>
#include <kernel/dev/driver_manager.h>

uint32_t ega_driver_init(driver_t *driver, struct multiboot_tag_framebuffer *framebuffer);
uint32_t ega_driver_deinit(driver_t *driver);
uint32_t ega_driver_get_width(driver_t *driver);
uint32_t ega_driver_get_height(driver_t *driver);
uint32_t ega_driver_write_tty(driver_t *driver, uint32_t x, uint32_t y, uint8_t color, const char c);
uint32_t ega_driver_scroll_line(driver_t *driver);

#endif