#ifndef __KERNEL_PROCESS_H
#define __KERNEL_PROCESS_H

#define KERNEL_MAX_PROCESSES 128
#define KERNEL_MAX_PROCESS_PAGE_ALLOCATIONS 1024

#include <kernel/types.h>
#include <kernel/task.h>

typedef struct _process
{
    uint32_t id;
    char path[128];

    task_t *task;
    void *page_allocations[KERNEL_MAX_PROCESS_PAGE_ALLOCATIONS]; // physical addresses
    void *data;
    void *stack;
    uint32_t size; // size of "data"
} __attribute__((packed)) process_t;

process_t *process_current();
uint32_t process_load(const char *path, process_t **process);

#endif