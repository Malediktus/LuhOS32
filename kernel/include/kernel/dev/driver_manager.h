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
  DRIVER_TYPE_INPUT = 3,
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

enum KEYCODE
{
  NULL_KEY = 0,
  Q_PRESSED = 0x10,
  Q_RELEASED = 0x90,
  W_PRESSED = 0x11,
  W_RELEASED = 0x91,
  E_PRESSED = 0x12,
  E_RELEASED = 0x92,
  R_PRESSED = 0x13,
  R_RELEASED = 0x93,
  T_PRESSED = 0x14,
  T_RELEASED = 0x94,
  Z_PRESSED = 0x15,
  Z_RELEASED = 0x95,
  U_PRESSED = 0x16,
  U_RELEASED = 0x96,
  I_PRESSED = 0x17,
  I_RELEASED = 0x97,
  O_PRESSED = 0x18,
  O_RELEASED = 0x98,
  P_PRESSED = 0x19,
  P_RELEASED = 0x99,
  A_PRESSED = 0x1E,
  A_RELEASED = 0x9E,
  S_PRESSED = 0x1F,
  S_RELEASED = 0x9F,
  D_PRESSED = 0x20,
  D_RELEASED = 0xA0,
  F_PRESSED = 0x21,
  F_RELEASED = 0xA1,
  G_PRESSED = 0x22,
  G_RELEASED = 0xA2,
  H_PRESSED = 0x23,
  H_RELEASED = 0xA3,
  J_PRESSED = 0x24,
  J_RELEASED = 0xA4,
  K_PRESSED = 0x25,
  K_RELEASED = 0xA5,
  L_PRESSED = 0x26,
  L_RELEASED = 0xA6,
  Y_PRESSED = 0x2C,
  Y_RELEASED = 0xAC,
  X_PRESSED = 0x2D,
  X_RELEASED = 0xAD,
  C_PRESSED = 0x2E,
  C_RELEASED = 0xAE,
  V_PRESSED = 0x2F,
  V_RELEASED = 0xAF,
  B_PRESSED = 0x30,
  B_RELEASED = 0xB0,
  N_PRESSED = 0x31,
  N_RELEASED = 0xB1,
  M_PRESSED = 0x32,
  M_RELEASED = 0xB2,

  ZERO_PRESSED = 0x29,
  ONE_PRESSED = 0x2,
  NINE_PRESSED = 0xA,

  POINT_PRESSED = 0x34,
  POINT_RELEASED = 0xB4,

  SLASH_RELEASED = 0xB5,

  BACKSPACE_PRESSED = 0xE,
  BACKSPACE_RELEASED = 0x8E,
  SPACE_PRESSED = 0x39,
  SPACE_RELEASED = 0xB9,
  ENTER_PRESSED = 0x1C,
  ENTER_RELEASED = 0x9C,
};

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

uint32_t driver_manager_get_key();
uint32_t driver_manager_get_key_modifiers();

#endif