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
    PANIC_CODE(kprint_color("failed to initialize driver manager. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
  }

  clear_tty();
  result = segmentation_init();
  if (result != EOK)
  {
    PANIC_CODE(kprint_color("failed to initialize segmentation. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
  }

  kprint("kernel booted from ");
  kprint(bootloader_name);
  kprint("\n");

  result = interrupts_init();
  if (result != EOK)
  {
    PANIC_CODE(kprint_color("failed to initialize interrupts. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
  }

  enable_interrupts();

  result = page_alloc_init(mmap, mmap_size);
  if (result != EOK)
  {
    PANIC_CODE(kprint_color("failed to initialize page allocator. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
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

  result = driver_manager_init();
  if (result != EOK)
  {
    PANIC_CODE(kprint_color("failed to initialize driver manager. error: ", 0xf4); kprint_color(string_error(result), 0xf4); kprint_color("\n", 0xf4));
  }

  uint32_t num_disks = get_num_disks();
  if (num_disks < 1)
  {
    PANIC_PRINT("no disks identified");
  }

  disk_t **disks = get_disks();

  kprint("reached end of kernel init routine\n");
  kprint("\033[40m A \033[41m B \033[42m C \033[43m D \033[0m");
  kprint("\033[44m A \033[45m B \033[46m C \033[47m D \033[0m");
  kprint("\033[40;1m A \033[41;1m B \033[42;1m C \033[43;1m D \033[0m");
  kprint("\033[44;1m A \033[45;1m B \033[46;1m C \033[47;1m D \033[0m");

  while (true)
  {
    __asm__("hlt");
  }

  __asm__("int $3"); // Breakpoint exception
}
