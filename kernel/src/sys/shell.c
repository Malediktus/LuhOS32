#include <kernel/shell.h>
#include <kernel/dev/driver_manager.h>
#include <kernel/dev/block_device.h>
#include <kernel/heap.h>
#include <kernel/page_allocator.h>
#include <kernel/lib/string.h>

void process_command(const char *line, uint32_t len)
{
    if (len <= 0)
    {
        return;
    }

    if (strcmp(line, "help") == 0)
    {
        kprintf("list of commands:\n - help: prints this message\n - sysinfo: gives you info about the kernel version\n - page: dynamicly allocates a page and prints it\n - dumpdisks: prints information about scanned block devices\n");
    }
    else if (strcmp(line, "sysinfo") == 0)
    {
        kprintf("LuhOS 32Bit - i386\nkernel version 0.1\ncopyright Nico Grundei 2023\n");
    }
    else if (strcmp(line, "page") == 0)
    {
        kprintf("new page allocated at 0x%p\n", page_alloc());
    }
    else if (strcmp(line, "dumpdisks") == 0)
    {
        kprintf("scanned disks:\n");

        const block_device_t **block_devices = get_block_devices();
        for (uint32_t i = 0; i < get_num_block_devices(); i++)
        {
            const block_device_t *block_device = block_devices[i];
            kprintf("\t%s: id %d, block size %d, num blocks %d\n", block_device->device_name, block_device->device_id, block_device->block_size, block_device->total_blocks);
        }
    }
    else
    {
        kprintf("unknown command: \"%s\"\n", line);
    }
}

void run_kernel_shell()
{
    kprintf("\033[32mWelcome to LuhOS. This is a work in progress kernel shell. Have fun!\033[0m\n");
    kprintf("<\033[31mluh32\033[0m@\033[34;1mkernel\033[0m>$ ");

    char *line = kmalloc(256);
    memset(line, 0, 256);
    uint32_t line_len = 0;

    while (true)
    {
        uint32_t key = driver_manager_get_key();
        if (key != NULL_KEY)
        {
            uint8_t ascii_key = key_code_to_ascii(key);
            if (ascii_key == 0 || line_len >= 256)
            {
                continue;
            }

            if (ascii_key == '\n')
            {
                kprintf("\n");
                process_command(line, line_len);
                kprintf("<\033[31mluh32\033[0m@\033[34;1mkernel\033[0m>$ ");
                line_len = 0;
                memset(line, 0, 256);
                continue;
            }

            kprintf("%c", ascii_key);
            line[line_len++] = ascii_key;
        }
    }
}
