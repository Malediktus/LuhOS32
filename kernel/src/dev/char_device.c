#include <kernel/dev/char_device.h>

static char_device_t *char_devices[MAX_CHAR_DEVICES];
static uint32_t num_char_devices = 0;

void register_char_device(char_device_t *cdev)
{
    char_devices[num_char_devices++] = cdev;
}

uint32_t char_device_get_width(char_device_t *cdev)
{
    return cdev->get_width(cdev);
}

uint32_t char_device_get_height(char_device_t *cdev)
{
    return cdev->get_height(cdev);
}

uint32_t char_device_write_char(char_device_t *cdev, uint32_t x, uint32_t y, uint8_t color, const char c)
{
    return cdev->write_char(cdev, x, y, color, c);
}

uint32_t char_device_scroll_line(char_device_t *cdev)
{
    return cdev->scroll_line(cdev);
}

char_device_t **get_char_devices()
{
    return char_devices;
}

uint32_t get_num_char_devices()
{
    return num_char_devices;
}
