#include <kernel/dev/input/keyboard.h>
#include <kernel/ports.h>
#include <kernel/interrupts.h>
#include <kernel/heap.h>
#include <kernel/lib/string.h>

uint8_t *keycache = 0;
uint16_t key_loc = 0;

void keyboard_irq(registers_t regs)
{
    if (key_loc >= 255)
    {
        return;
    }
    keycache[key_loc++] = port_byte_in(0x60);
}

uint32_t keyboard_init(driver_t *driver)
{
    driver->type = DRIVER_TYPE_INPUT;

    keycache = kmalloc(256);
    memset(keycache, 0, 256);
    register_interrupt_handler(33, keyboard_irq);

    return EOK;
}

static char c = 0;
uint32_t keyboard_get_key()
{
    c = 0;
    if (key_loc == 0)
    {
        goto out;
    }

    c = *keycache;
    key_loc--;
    for (uint32_t i = 0; i < 256; i++)
    {
        keycache[i] = keycache[i + 1];
    }

out:
    return c;
}

uint32_t keyboard_get_key_modifiers()
{
}
