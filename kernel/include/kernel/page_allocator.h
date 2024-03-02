#ifndef __KERNEL_PAGE_ALLOCATOR_H
#define __KERNEL_PAGE_ALLOCATOR_H

#include <kernel/types.h>
#include <kernel/multiboot2.h>

uint32_t page_alloc_init(struct multiboot_tag_mmap *mmap, uint32_t mmap_size);
void *page_alloc(void);
void page_free(void *ptr);
void page_reserve(void *ptr);
uint32_t get_num_pages(void);

#endif