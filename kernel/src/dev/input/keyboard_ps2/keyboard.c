#include <kernel/dev/input/keyboard_ps2.h>
#include <kernel/ports.h>
#include <kernel/interrupts.h>
#include <kernel/heap.h>
#include <kernel/lib/string.h>

uint8_t *keycache = 0;
uint16_t key_loc = 0;

void keyboard_irq(int_registers_t)
{
    if (key_loc >= 255)
    {
        return;
    }
    keycache[key_loc++] = port_byte_in(0x60);
}

uint32_t keyboard_ps2_init(input_device_t *idev)
{
    keycache = kmalloc(256);
    memset(keycache, 0, 256);
    register_interrupt_handler(33, keyboard_irq);

    idev->get_event = keyboard_ps2_get_event;

    return EOK;
}

static char c = 0;
uint32_t keyboard_ps2_get_event(input_device_t *idev, input_device_event_t *event)
{
    c = 0;
    if (key_loc == 0)
    {
        event->type = INPUT_EVENT_NONE;
        return EOK;
    }

    c = *keycache;
    key_loc--;
    for (uint32_t i = 0; i < 256; i++)
    {
        keycache[i] = keycache[i + 1];
    }

    event->type = INPUT_EVENT_KEY;
    event->data[0] = (uint64_t)c;

    return EOK;
}
