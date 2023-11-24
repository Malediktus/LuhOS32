#ifndef __KERNEL_DRIVER_MANAGER_H
#define __KERNEL_DRIVER_MANAGER_H

#include <kernel/types.h>
#include <kernel/multiboot2.h>

#define TTY_WHITE_ON_BLACK 0x0f
#define TTY_RED_ON_WHITE 0xf4

#define MAX_DRIVERS 50

typedef enum
{
  DRIVER_TYPE_NONE = 0,
  DRIVER_TYPE_TTY = 1,
} driver_type_t;

typedef struct
{
  driver_type_t type;
  void *private_data;
} driver_t;

uint32_t driver_manager_init(struct multiboot_tag_framebuffer *framebuffer);
uint32_t driver_manager_deinit();

uint32_t driver_manager_get_width();
uint32_t driver_manager_get_height();
uint32_t driver_manager_write_tty(uint32_t x, uint32_t y, uint8_t color, const char c);

#endif