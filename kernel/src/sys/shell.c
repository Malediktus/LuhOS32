#include <kernel/shell.h>
#include <kernel/dev/driver_manager.h>
#include <kernel/dev/block_device.h>
#include <kernel/heap.h>
#include <kernel/page_allocator.h>
#include <kernel/lib/string.h>
#include <kernel/lib/ascii.h>

struct command_info
{
    char *command;
    char **arguments;
    uint32_t num_arguments;
};

struct command_info parse_command(char *line, uint32_t len)
{
    struct command_info result = {NULL, NULL, 0};

    if (len == 0 || line == NULL)
    {
        return result;
    }

    uint32_t command_end = strcspn(line, " \t\n");
    result.command = strndup(line, command_end);

    uint32_t max_arguments = len / 2;
    result.arguments = kmalloc(sizeof(char *) * max_arguments);
    if (!result.arguments)
    {
        kfree(result.command);
        result.command = NULL;
        return result;
    }

    char *line_copy = strdup(line);
    char *token = strtok(line_copy, " \t\n");
    uint32_t arg_index = 0;
    while (token != NULL)
    {
        if (token[0] == '"')
        {
            result.arguments[arg_index++] = strdup(token + 1);
            token = strtok(NULL, "\"");
            if (token != NULL)
            {
                result.arguments[arg_index - 1] = krealloc(result.arguments[arg_index - 1], strlen(result.arguments[arg_index - 1]) + strlen(token) + 1);
                strcat(result.arguments[arg_index - 1], " ");
                strcat(result.arguments[arg_index - 1], token);
            }
        }
        else
        {
            result.arguments[arg_index++] = strdup(token);
        }

        if (arg_index >= max_arguments)
        {
            break;
        }

        token = strtok(NULL, " \t\n");
    }

    result.num_arguments = arg_index;
    kfree(line_copy);

    return result;
}

void free_command_info(struct command_info *info)
{
    kfree(info->command);
    for (uint32_t i = 0; i < info->num_arguments; i++)
    {
        kfree(info->arguments[i]);
    }
    kfree(info->arguments);
}

void process_command(char *line, uint32_t len)
{
    if (len <= 0)
    {
        return;
    }

    struct command_info command_info = parse_command(line, len);

    if (strcmp(command_info.command, "help") == 0)
    {
        kprintf("list of commands:\n - help: prints this message\n - sysinfo: gives you info about the kernel version\n - page: dynamicly allocates a page and prints it\n - dumpdisks: prints information about scanned block devices\n - echo: echoes arguments\n");
    }
    else if (strcmp(command_info.command, "sysinfo") == 0)
    {
        kprintf("LuhOS 32Bit - i386\nkernel version 0.1\ncopyright Nico Grundei 2023\n");
    }
    else if (strcmp(command_info.command, "page") == 0)
    {
        kprintf("new page allocated at 0x%p\n", page_alloc());
    }
    else if (strcmp(command_info.command, "dumpdisks") == 0)
    {
        kprintf("scanned disks:\n");

        block_device_t **block_devices = get_block_devices();
        for (uint32_t i = 0; i < get_num_block_devices(); i++)
        {
            const block_device_t *block_device = block_devices[i];
            kprintf("\t%s: id %d, block size %d, num blocks %d\n", block_device->device_name, block_device->device_id, block_device->block_size, block_device->total_blocks);
        }
    }
    else if (strcmp(command_info.command, "echo") == 0)
    {
        for (uint32_t i = 1; i < command_info.num_arguments; i++)
        {
            kprintf("%s\n", command_info.arguments[i]);
        }
    }
    else
    {
        kprintf("unknown command: \"%s\"\n", line);
    }

    free_command_info(&command_info);
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
