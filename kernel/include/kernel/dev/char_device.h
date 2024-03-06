#ifndef __KERNEL_CHAR_DEVICE_H
#define __KERNEL_CHAR_DEVICE_H

#include <kernel/types.h>

#define MAX_CHAR_DEVICES 10

typedef struct char_device
{
    uint32_t (*get_width)(struct char_device *);
    uint32_t (*get_height)(struct char_device *);
    uint32_t (*write_char)(struct char_device *, uint32_t x, uint32_t y, uint8_t color, const char c);
    uint32_t (*scroll_line)(struct char_device *);
    void *private_data;
} char_device_t;

void register_char_device(char_device_t *cdev);
uint32_t char_device_get_width(char_device_t *cdev);
uint32_t char_device_get_height(char_device_t *cdev);
uint32_t char_device_write_char(char_device_t *cdev, uint32_t x, uint32_t y, uint8_t color, const char c);
uint32_t char_device_scroll_line(char_device_t *cdev);

char_device_t **get_char_devices();
uint32_t get_num_char_devices();

#endif