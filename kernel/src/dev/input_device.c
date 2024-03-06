#include <kernel/dev/input_device.h>

static input_device_t *input_devices[MAX_INPUT_DEVICES];
static uint32_t num_input_devices = 0;

void register_input_device(input_device_t *idev)
{
    input_devices[num_input_devices++] = idev;
}

uint32_t input_device_get_event(input_device_t *idev, input_device_event_t *event)
{
    return idev->get_event(idev, event);
}

input_device_t **get_input_devices()
{
    return input_devices;
}

uint32_t get_num_input_devices()
{
    return num_input_devices;
}
