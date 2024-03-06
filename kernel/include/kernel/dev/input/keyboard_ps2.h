#ifndef __KERNEL_KEYBOARD_H
#define __KERNEL_KEYBOARD_H

#include <kernel/types.h>
#include <kernel/dev/input_device.h>

uint32_t keyboard_ps2_init(input_device_t *idev);
uint32_t keyboard_ps2_get_event(input_device_t *idev, input_device_event_t *event);

#endif