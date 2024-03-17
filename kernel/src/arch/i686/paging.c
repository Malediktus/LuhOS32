#include <kernel/paging.h>
#include <kernel/tty.h>
#include <kernel/lib/cast.h>

extern void paging_load_directory(uint32_t *directory);

static uint32_t *current_directory = 0;

uint32_t *page_directory_create(uint8_t flags)
{
#warning this may cause an error when colliding with a mapped memory reagion (eg. kernel allocator)
    uint32_t *directory = page_alloc();

    uint32_t offset = 0;
    for (uint32_t i = 0; i < PAGING_TOTAL_ENTRIES_PER_TABLE; i++)
    {
        uint32_t *table = page_alloc();
        for (uint32_t j = 0; j < PAGING_TOTAL_ENTRIES_PER_TABLE; j++)
        {
            table[j] = (offset + (j * PAGE_SIZE)) | flags;
        }
        offset += (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGE_SIZE);
        directory[i] = (uint32_t)table | flags | PAGING_IS_WRITEABLE;
    }

    return directory;
}

void page_directory_free(uint32_t *page_directory)
{
    for (uint32_t i = 0; i < 1024; i++)
    {
        uint32_t entry = page_directory[i];
        uint32_t *table = (uint32_t *)(entry & 0xFFFFF000);

        page_free(table);
    }

    page_free(page_directory);
}

void paging_switch_directory(uint32_t *directory)
{
    paging_load_directory(directory);
    current_directory = directory;
}

static bool paging_is_aligned(void *addr)
{
    return !((uint32_t)addr % PAGE_SIZE);
}

static uint32_t paging_get_indices(void *virtual_address, uint32_t *directory_index, uint32_t *table_index)
{
    uint32_t res = EOK;
    if (!paging_is_aligned(virtual_address))
    {
        res = EINVARG;
        goto out;
    }

    *directory_index = ((uint32_t)virtual_address / (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGE_SIZE));
    *table_index = ((uint32_t)virtual_address % (PAGING_TOTAL_ENTRIES_PER_TABLE * PAGE_SIZE) / PAGE_SIZE);

out:
    return res;
}

uint32_t paging_set(uint32_t *directory, void *virt, uint32_t val)
{
    if (!paging_is_aligned(virt))
    {
        return -EINVARG;
    }

    uint32_t directory_index = 0;
    uint32_t table_index = 0;

    uint32_t res = paging_get_indices(virt, &directory_index, &table_index);
    if (res != EOK)
    {
        return res;
    }

    uint32_t entry = directory[directory_index];
    uint32_t *table = (uint32_t *)(entry & 0xFFFFF000);

    table[table_index] = val;

    return EOK;
}

uint32_t paging_map(uint32_t *directory, void *virt, void *phys, uint8_t flags)
{
    if (!paging_is_aligned(virt) || !paging_is_aligned(phys))
    {
        return -EINVARG;
    }

    paging_set(directory, virt, (uint32_t)phys | flags);

    return EOK;
}
