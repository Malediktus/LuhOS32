#include <kernel/types.h>
#include <kernel/ports.h>
#include <kernel/tty.h>
#include <kernel/lib/string.h>

#define PCI_DATA_PORT 0xCFC
#define PCI_COMMAND_PORT 0xCF8

enum bar_type
{
    BAR_TYPE_MEMORY_MAPPING = 0,
    BAR_TYPE_INPUT_OUTPUT = 1
};

struct base_address_register
{
    bool prefetchable;
    uint8_t *address;
    uint32_t size;
    enum bar_type type;
};

struct pci_device_descriptor
{
    uint32_t port_base;
    uint32_t interrupt;

    uint16_t bus;
    uint16_t device;
    uint16_t function;

    uint16_t vendor_id;
    uint16_t device_id;

    uint8_t class_id;
    uint8_t subclass_id;
    uint8_t interface_id;

    uint8_t revision;
};

uint32_t pci_read(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (register_offset & 0xFC);

    port_dword_out(PCI_COMMAND_PORT, id);
    uint32_t result = port_dword_in(PCI_DATA_PORT);

    return result >> (8 * (register_offset % 4));
}

void pci_write(uint16_t bus, uint16_t device, uint16_t function, uint32_t register_offset, uint32_t value)
{
    uint32_t id = 0x1 << 31 | ((bus & 0xFF) << 16) | ((device & 0x1F) << 11) | ((function & 0x07) << 8) | (register_offset & 0xFC);

    port_dword_out(PCI_COMMAND_PORT, id);
    port_dword_out(PCI_DATA_PORT, value);
}

bool device_has_functions(uint16_t bus, uint16_t device)
{
    return pci_read(bus, device, 0, 0x0E) & (1 << 7);
}

void populate_pci_device_descriptor(struct pci_device_descriptor *desc, uint16_t bus, uint16_t device, uint16_t function)
{
    desc->bus = bus;
    desc->device = device;
    desc->function = function;

    desc->vendor_id = pci_read(bus, device, function, 0x00);
    desc->device_id = pci_read(bus, device, function, 0x02);

    desc->class_id = pci_read(bus, device, function, 0x0b);
    desc->subclass_id = pci_read(bus, device, function, 0x0a);
    desc->interface_id = pci_read(bus, device, function, 0x09);

    desc->revision = pci_read(bus, device, function, 0x08);
    desc->interrupt = pci_read(bus, device, function, 0x3c);
}

void populate_base_address_register(struct base_address_register *bar, uint16_t bus, uint16_t device, uint16_t function, uint16_t bar_num)
{
    memset(bar, 0, sizeof(struct base_address_register));

    uint32_t headertype = pci_read(bus, device, function, 0x0e) & 0x7f;

    uint16_t max_bars = 6 - (4 * headertype);
    if (bar_num >= max_bars)
    {
        return;
    }

    uint32_t bar_value = pci_read(bus, device, function, 0x10 + 4 * bar_num);
    bar->type = (bar_value & 0x01) ? BAR_TYPE_INPUT_OUTPUT : BAR_TYPE_MEMORY_MAPPING;

    if (bar->type == BAR_TYPE_MEMORY_MAPPING)
    {
        switch ((bar_value >> 1) & 0x03)
        {
        case 0:
        case 1:
        case 2:
        }

        bar->prefetchable = ((bar_value >> 3) & 0x01) == 0x01;
    }
    else
    {
        bar->address = (uint8_t *)(bar_value & ~0x03);
        bar->prefetchable = false;
    }
}

void pci_instantiate_drivers(void)
{
    for (uint16_t bus = 0; bus < 8; bus++)
    {
        for (uint16_t device = 0; device < 32; device++)
        {
            uint16_t num_functions = device_has_functions(bus, device) ? 8 : 1;
            for (uint16_t function = 0; function < num_functions; function++)
            {
                struct pci_device_descriptor desc;
                populate_pci_device_descriptor(&desc, bus, device, function);

                if (desc.vendor_id == 0x0000 || desc.vendor_id == 0xFFFF)
                {
                    continue;
                }

                for (uint16_t bar_num = 0; bar_num < 6; bar_num++)
                {
                    struct base_address_register bar;
                    populate_base_address_register(&bar, bus, device, function, bar_num);
                    if (bar.address && (bar.type == BAR_TYPE_INPUT_OUTPUT))
                    {
                        desc.port_base = (uint32_t)bar.address;
                    }
                }
            }
        }
    }
}
