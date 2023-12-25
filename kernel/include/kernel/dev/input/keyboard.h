#ifndef __KERNEL_KEYBOARD_H
#define __KERNEL_KEYBOARD_H

#include <kernel/types.h>
#include <kernel/dev/driver_manager.h>

uint32_t keyboard_init(driver_t *driver);
uint32_t keyboard_get_key();
uint32_t keyboard_get_key_modifiers();

#endif