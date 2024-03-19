#include <kernel/heap.h>
#include <kernel/page_allocator.h>
#include <kernel/paging.h>
#include <kernel/lib/string.h>

typedef struct memory_chunk
{
    struct memory_chunk *next;
    struct memory_chunk *prev;
    uint32_t size;
    bool allocated;
} memory_chunk_t;

struct
{
    void *virtual_address;
    uint32_t size;
} heap_allocator;

uint32_t heap_init(void *virtual_address, uint32_t num_pages, uint32_t *kernel_page_directory)
{
    if (num_pages < 1)
    {
        return -EINVARG;
    }

    heap_allocator.virtual_address = virtual_address;
    heap_allocator.size = num_pages * PAGE_SIZE;

    void *current_virtual_address = virtual_address;
    for (uint32_t i = 0; i < num_pages; i++)
    {
        void *ptr = page_alloc();
        paging_map(kernel_page_directory, current_virtual_address, ptr, PAGING_IS_PRESENT | PAGING_IS_WRITEABLE);
        current_virtual_address += PAGE_SIZE;
    }

    memory_chunk_t *first_chunk = (memory_chunk_t *)virtual_address;
    first_chunk->allocated = false;
    first_chunk->prev = NULL;
    first_chunk->next = NULL;
    first_chunk->size = heap_allocator.size - sizeof(memory_chunk_t);

    return EOK;
}

static void *allocate(uint32_t size)
{
    memory_chunk_t *result = NULL;

    for (memory_chunk_t *chunk = (memory_chunk_t *)heap_allocator.virtual_address; chunk != NULL && result == NULL; chunk = chunk->next)
    {
        if (chunk->size > size && chunk->allocated == false)
        {
            result = chunk;
        }
    }

    if (result == NULL)
    {
        PANIC_PRINT("out of kernel heap memory");
        return NULL;
    }

    if (result->size >= size + sizeof(memory_chunk_t) + 1)
    {
        memory_chunk_t *new_chunk = (memory_chunk_t *)((uint32_t)result + sizeof(memory_chunk_t) + size);
        new_chunk->allocated = false;
        new_chunk->size = result->size - size - sizeof(memory_chunk_t);
        new_chunk->prev = result;
        new_chunk->next = result->next;
        if (new_chunk->next != NULL)
        {
            new_chunk->next->prev = new_chunk;
        }

        result->size = size;
        result->next = new_chunk;
    }

    result->allocated = true;
    return (void *)(((uint32_t)result) + sizeof(memory_chunk_t));
}

void *kmalloc(uint32_t size)
{
    void *result = allocate(size);
    if (result == NULL)
    {
        return NULL;
    }

    memset(result, 0x00, size);
    return result;
}

void *kmalloc_aligned(uint32_t alignment, uint32_t size)
{
    void *result = allocate(size + alignment); // TODO: this is very memory inefficiant
    if (result == NULL)
    {
        return NULL;
    }

    result = (uintptr_t)result + (alignment - ((uintptr_t)result % alignment));

    memset(result, 0x00, size);
    return result;
}

void *krealloc(void *ptr, uint32_t size)
{
    void *result = allocate(size);
    if (result == NULL)
    {
        return NULL;
    }

    memcpy(result, ptr, size);
    kfree(ptr);
    return result;
}

void *kcalloc(uint32_t num, uint32_t element_size)
{
    uint32_t size = num * element_size;

    void *result = allocate(size);
    if (result == NULL)
    {
        return NULL;
    }

    memset(result, 0x00, size);
    return result;
}

void kfree(void *ptr)
{
    memory_chunk_t *chunk = (memory_chunk_t *)((uint32_t)ptr - sizeof(memory_chunk_t));
    chunk->allocated = false;

    if (chunk->prev != NULL && chunk->prev->allocated == false)
    {
        chunk->prev->next = chunk->next;
        chunk->prev->size += chunk->size + sizeof(memory_chunk_t);
        if (chunk->next != NULL)
        {
            chunk->next->prev = chunk->prev;
        }

        chunk = chunk->prev;
    }

    if (chunk->next != NULL && chunk->next->allocated == false)
    {
        chunk->size += chunk->next->size + sizeof(memory_chunk_t);
        chunk->next = chunk->next->next;
        if (chunk->next != NULL)
        {
            chunk->next->prev = chunk;
        }
    }
}
