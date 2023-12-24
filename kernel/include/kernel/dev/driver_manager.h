#ifndef __KERNEL_DRIVER_MANAGER_H
#define __KERNEL_DRIVER_MANAGER_H

#include <kernel/types.h>
#include <kernel/multiboot2.h>

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

#define MAX_DRIVERS 50
#define MAX_DISKS 50

#define IDE_DRIVER_DISK_ID_OFFSET 0

typedef enum
{
  DRIVER_TYPE_NONE = 0,
  DRIVER_TYPE_EGA = 1,
  DRIVER_TYPE_IDE = 2,
} driver_type_t;

typedef struct
{
  driver_type_t type;
  void *private_data;
} driver_t;

typedef struct
{
  driver_type_t driver_type;
  uint32_t id;
  void *private_data;
} disk_t;

uint32_t driver_manager_init_early(struct multiboot_tag_framebuffer *framebuffer);
uint32_t driver_manager_init();
uint32_t driver_manager_deinit();

uint32_t driver_manager_disk_write_sector(uint32_t disk_id, uint32_t lba, uint8_t *buf);
uint32_t driver_manager_disk_read_sector(uint32_t disk_id, uint32_t lba, uint8_t *buf);

uint32_t get_num_disks();
disk_t **get_disks();

uint32_t driver_manager_get_width();
uint32_t driver_manager_get_height();
uint32_t driver_manager_write_tty(uint32_t x, uint32_t y, uint8_t color, const char c);

#endif