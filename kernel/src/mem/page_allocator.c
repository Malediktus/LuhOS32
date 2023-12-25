#include <kernel/page_allocator.h>
#include <kernel/tty.h>
#include <kernel/lib/string.h>
#include <kernel/lib/cast.h>

struct page_allocator_info
{
  uint32_t *bitmap;
  uint32_t num_pages;
  uint32_t last_index;
} page_allocator;

static void bit_set(uint32_t *bitmap, uint32_t index)
{
  uint32_t array_index = index / 32;
  uint32_t bit_offset = index % 32;
  bitmap[array_index] |= (1UL << bit_offset);
}

static void bit_clear(uint32_t *bitmap, uint32_t index)
{
  uint32_t array_index = index / 32;
  uint32_t bit_offset = index % 32;
  bitmap[array_index] &= ~(1UL << bit_offset);
}

static bool bit_get(const uint32_t *bitmap, uint32_t index)
{
  uint32_t array_index = index / 32;
  uint32_t bit_offset = index % 32;
  return (bitmap[array_index] & (1UL << bit_offset)) != 0;
}

uint32_t page_alloc_init(struct multiboot_tag_mmap *mmap, uint32_t mmap_size)
{
  uint32_t res = EOK;

  memset(&page_allocator, 0, sizeof(struct page_allocator_info));
  uint32_t max_addr = 0;

  kprintf("memory map received from bootloader:\n");
  for (struct multiboot_mmap_entry *entry = mmap->entries;
       (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap_size;
       entry = (multiboot_memory_map_t *)((unsigned long)entry + mmap->entry_size))
  {
    kprintf("\tentry 0x%x, size 0x%x, ", (uint32_t)(entry->addr), (uint32_t)(entry->len));

    switch (entry->type)
    {
    case MULTIBOOT_MEMORY_AVAILABLE:
      kprintf("AVAILABLE");
      break;
    case MULTIBOOT_MEMORY_RESERVED:
      kprintf("RESERVED");
      break;
    case MULTIBOOT_MEMORY_ACPI_RECLAIMABLE:
      kprintf("ACPI_RECLAIMABLE");
      break;
    case MULTIBOOT_MEMORY_NVS:
      kprintf("NVS");
      break;
    case MULTIBOOT_MEMORY_BADRAM:
      kprintf("BADRAM");
      break;
    default:
      PANIC_PRINT("invalid type value in mmap entry");
    }
    kprintf("\n");

    uint32_t end_addr = entry->addr + entry->len;
    if (end_addr > max_addr && entry->type == MULTIBOOT_MEMORY_AVAILABLE)
      max_addr = end_addr;
  }

  page_allocator.num_pages = max_addr / PAGE_SIZE;
  uint32_t bitmap_size = (page_allocator.num_pages + 8 - 1) / 8;
  kprintf("page allocator bitmap size 0x%x\n", bitmap_size);

  page_allocator.bitmap = (uint32_t *)max_addr;

  for (struct multiboot_mmap_entry *entry = mmap->entries;
       (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap_size;
       entry = (multiboot_memory_map_t *)((unsigned long)entry + mmap->entry_size))
  {
    if (entry->type != MULTIBOOT_MEMORY_AVAILABLE)
      continue;

    if (entry->len < bitmap_size)
      continue;

    page_allocator.bitmap = (uint32_t *)entry->addr;
    entry->addr += bitmap_size;
    entry->len -= bitmap_size;
    break;
  }

  if ((uint32_t)(page_allocator.bitmap) == max_addr)
  {
    res = ENOMEM;
    goto out;
  }

  kprintf("page allocator bitmap located at 0x%x\n", (uint32_t)(page_allocator.bitmap));

  memset(page_allocator.bitmap, 0xFF, bitmap_size);

  for (struct multiboot_mmap_entry *entry = mmap->entries;
       (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap_size;
       entry = (multiboot_memory_map_t *)((unsigned long)entry + mmap->entry_size))
  {
    if (entry->type != MULTIBOOT_MEMORY_AVAILABLE)
      continue;

    uint32_t start_page = (entry->addr + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint32_t j = 0; j < entry->len / PAGE_SIZE; j++)
    {
      uint32_t page_index = start_page + j;
      bit_clear(page_allocator.bitmap, page_index);
    }
  }

out:
  return res;
}

void *page_alloc(void)
{
  for (uint32_t i = page_allocator.last_index; i < page_allocator.num_pages; i++)
  {
    if (bit_get(page_allocator.bitmap, i))
      continue;

    bit_set(page_allocator.bitmap, i);
    page_allocator.last_index = i;

    return (void *)(i * PAGE_SIZE);
  }

  PANIC_PRINT("out of free pages");
  return NULL;
}

void page_free(void *ptr)
{
  uint32_t index = (uint32_t)ptr / PAGE_SIZE;
  bit_clear(page_allocator.bitmap, index);
  page_allocator.last_index = index;
}

uint32_t get_num_pages()
{
  return page_allocator.num_pages;
}

void page_reserve(void *ptr)
{
  uint32_t index = (uint32_t)ptr / PAGE_SIZE;
  bit_set(page_allocator.bitmap, index);
  page_allocator.last_index = index;
}

void *get_largest_memory_hole(struct multiboot_tag_mmap *mmap, uint32_t mmap_size, uint32_t *size)
{
  void *largest_reserved_segment = NULL;
  *size = 0;

  for (struct multiboot_mmap_entry *entry = mmap->entries;
       (multiboot_uint8_t *)entry < (multiboot_uint8_t *)mmap + mmap_size;
       entry = (multiboot_memory_map_t *)((unsigned long)entry + mmap->entry_size))
  {
    if (entry->type == MULTIBOOT_MEMORY_RESERVED && entry->len > *size)
    {
      largest_reserved_segment = (void *)entry->addr;
      *size = entry->len;
    }
  }

  return largest_reserved_segment;
}
