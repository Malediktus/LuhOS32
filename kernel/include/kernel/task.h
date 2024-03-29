#ifndef __KERNEL_TASK_H
#define __KERNEL_TASK_H

#include <kernel/types.h>
#include <kernel/paging.h>

#define KERNEL_TASK_VADDR 0x400000
#define KERNEL_TASK_STACK_VADDR 0x3FF000
#define KERNEL_TASK_STACK_SIZE (1024 * 16)

typedef struct
{
    uint32_t edi;
    uint32_t esi;
    uint32_t ebp;
    uint32_t ebx;
    uint32_t edx;
    uint32_t ecx;
    uint32_t eax;

    uint32_t ip;
    uint32_t cs;
    uint32_t flags;
    uint32_t esp;
    uint32_t ss;
} __attribute__((packed)) task_registers_t;

struct _process;
typedef struct _task
{
    uint32_t *page_directory;
    task_registers_t registers;

    struct _process *process;

    // double linked list
    struct _task *next;
    struct _task *prev;
} task_t;

task_t *task_current();
task_t *task_get_next();

task_t *task_new(struct _process *process);
void task_free(task_t *task);
uint32_t task_init(task_t *task, struct _process *process);

uint32_t task_switch(task_t *task);
uint32_t task_page();
void task_run_first_task();

void task_return(task_registers_t *regs);
void restore_gp_registers(task_registers_t *regs);
void user_registers();

#endif