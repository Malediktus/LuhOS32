#ifndef _KERNEL_HEAP_H
#define _KERNEL_HEAP_H

#include <kernel/types.h>

#define MINIMUM_ALLOCATION_SIZE 64

uint32_t heap_init(void *virtual_address, uint32_t num_pages, uint32_t *kernel_page_directory);

void *kmalloc(uint32_t size);
void *kmalloc_aligned(uint32_t alignment, uint32_t size); // TODO: Not yet implemented
void *krealloc(void *ptr, uint32_t size);
void *kcalloc(uint32_t num, uint32_t element_size);

void kfree(void *ptr);

#endif