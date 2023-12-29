#include <kernel/multiboot2.h>
#include <kernel/types.h>
#include <kernel/dev/driver_manager.h>
#include <kernel/tty.h>
#include <kernel/segmentation.h>
#include <kernel/interrupts.h>
#include <kernel/lib/ascii.h>
#include <kernel/lib/string.h>
#include <kernel/lib/cast.h>
#include <kernel/ports.h>
#include <kernel/page_allocator.h>
#include <kernel/paging.h>
#include <kernel/heap.h>
#include <kernel/dev/block_device.h>
#include <kernel/dev/disk/ide.h>
#include <kernel/shell.h>

extern uint32_t _kernel_start;
extern uint32_t _kernel_end;

static uint32_t *kernel_page_directory = 0;

void kernel_main(unsigned long magic, unsigned long addr)
{
    disable_interrupts();

    struct multiboot_tag_framebuffer *framebuffer = NULL;
    struct multiboot_tag_mmap *mmap = NULL;
    uint32_t mmap_size = 0;
    char *bootloader_name;

    if (magic != MULTIBOOT2_BOOTLOADER_MAGIC)
    {
        return;
    }

    if (addr & 7)
    {
        return;
    }

    for (struct multiboot_tag *tag = (struct multiboot_tag *)(addr + 8);
         tag->type != MULTIBOOT_TAG_TYPE_END;
         tag = (struct multiboot_tag *)((multiboot_uint8_t *)tag + ((tag->size + 7) & ~7)))
    {
        switch (tag->type)
        {
        case MULTIBOOT_TAG_TYPE_BOOT_LOADER_NAME:
        {
            bootloader_name = ((struct multiboot_tag_string *)tag)->string;
            break;
        }

        case MULTIBOOT_TAG_TYPE_FRAMEBUFFER:
        {
            framebuffer = (struct multiboot_tag_framebuffer *)tag;
            break;
        }

        case MULTIBOOT_TAG_TYPE_MMAP:
        {
            mmap = (struct multiboot_tag_mmap *)tag;
            mmap_size = tag->size;
            break;
        }
        }
    }

    if (framebuffer == NULL)
    {
        return;
    }

    uint32_t result = driver_manager_init_early(framebuffer);
    if (result != EOK)
    {
        PANIC_CODE(kprintf("failed to initialize early driver manager. error: %s\n", string_error(result)));
    }

    clear_tty();
    result = segmentation_init();
    if (result != EOK)
    {
        PANIC_CODE(kprintf("failed to initialize segmentation. error: %s\n", string_error(result)));
    }

    kprintf("kernel booted from %s\n", bootloader_name);

    result = interrupts_init();
    if (result != EOK)
    {
        PANIC_CODE(kprintf("failed to initialize interrupts. error: %s\n", string_error(result)));
    }

    enable_interrupts();

    result = page_alloc_init(mmap, mmap_size);
    if (result != EOK)
    {
        PANIC_CODE(kprintf("failed to initialize page allocator. error: %s\n", string_error(result)));
    }

    for (uint32_t i = ((uint32_t)&_kernel_start / PAGE_SIZE); i <= ((uint32_t)&_kernel_end / PAGE_SIZE); i++)
    {
        page_reserve((void *)(i * PAGE_SIZE));
    }

    for (uint32_t i = ((uint32_t)addr / PAGE_SIZE); i <= ((uint32_t)(addr + *(uint32_t *)addr) / PAGE_SIZE); i++)
    {
        page_reserve((void *)(i * PAGE_SIZE));
    }

    kernel_page_directory = page_directory_create(PAGING_IS_PRESENT | PAGING_IS_WRITEABLE | PAGING_ACCESS_FROM_ALL);
    paging_switch_directory(kernel_page_directory);
    paging_enable();

    uint32_t heap_size_bytes = 0;
    void *heap_virtual_start = get_largest_memory_hole(mmap, mmap_size, &heap_size_bytes);

    heap_init(heap_virtual_start, heap_size_bytes / PAGE_SIZE, kernel_page_directory);

    result = ide_driver_init();
    if (result != EOK)
    {
        PANIC_CODE(kprintf("failed to initialize ide driver. error: %s\n", string_error(result)));
    }

    result = ide_driver_scan_disks();
    if (result != EOK)
    {
        PANIC_CODE(kprintf("failed to scan ide disks. error: %s\n", string_error(result)));
    }

    result = driver_manager_init();
    if (result != EOK)
    {
        PANIC_CODE(kprintf("failed to initialize driver manager. error: %s\n", string_error(result)));
    }

    kprintf("\033[40m  \033[41m  \033[42m  \033[43m  \033[44m  \033[45m  \033[46m  \033[47m  \033[40;1m  \033[41;1m  \033[42;1m  \033[43;1m  \033[44;1m  \033[45;1m  \033[46;1m  \033[47;1m  \033[0m\n");

    run_kernel_shell();

    while (true)
    {
    }

    __asm__("int $3"); // Breakpoint exception
}
