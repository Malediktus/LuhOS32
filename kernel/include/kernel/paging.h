#ifndef __KERNEL_PAGING_H
#define __KERNEL_PAGING_H

#include <kernel/types.h>
#include <kernel/page_allocator.h>

#define PAGING_CACHE_DISABLED 0b00010000
#define PAGING_WRITE_THROUGH 0b00001000
#define PAGING_ACCESS_FROM_ALL 0b00000100
#define PAGING_IS_WRITEABLE 0b00000010
#define PAGING_IS_PRESENT 0b00000001

#define PAGING_TOTAL_ENTRIES_PER_TABLE 1024

uint32_t *page_directory_create(uint8_t flags);
void page_directory_free(uint32_t *page_directory);
void paging_switch_directory(uint32_t *directory);
uint32_t paging_map_range(uint32_t *directory, void *virt, void *phys, uint32_t count, uint8_t flags);
uint32_t paging_map_to(uint32_t *directory, void *virt, void *phys, void *phys_end, uint8_t flags);
uint32_t paging_map(uint32_t *directory, void *virt, void *phys, uint8_t flags);
void *paging_get_phys_address(uint32_t *directory, void *virt);

void *paging_align_address(void *ptr);

extern void paging_enable();

#endif