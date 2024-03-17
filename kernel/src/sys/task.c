#include <kernel/task.h>
#include <kernel/heap.h>
#include <kernel/segmentation.h>
#include <kernel/lib/string.h>

task_t *current_task = NULL;
task_t *task_tail = NULL;
task_t *task_head = NULL;

task_t *task_current()
{
    return current_task;
}

task_t *task_new(struct _process *process)
{
    task_t *task = kmalloc(sizeof(task_t));
    if (!task)
    {
        return NULL;
    }

    uint32_t res = task_init(task, process);
    if (res != EOK)
    {
        kfree(task);
        return NULL;
    }

    if (task_head == NULL)
    {
        task_head = task;
        task_tail = task;
    }
    else
    {
        task_tail->next = task;
        task->prev = task_tail;
        task_tail = task;
    }

    return task;
}

task_t *task_get_next()
{
    if (!current_task->next)
    {
        return task_head;
    }

    return current_task->next;
}

static void task_list_remove(task_t *task)
{
    if (task->prev)
    {
        task->prev->next = task->next;
    }

    if (task == task_head)
    {
        task_head = task->next;
    }

    if (task == task_tail)
    {
        task_tail = task->prev;
    }

    if (task == current_task)
    {
        current_task = task_get_next();
    }
}

void task_free(task_t *task)
{
    page_directory_free(task->page_directory);
    task_list_remove(task);
    kfree(task);
}

uint32_t task_switch(task_t *task)
{
    current_task = task;
    paging_switch_directory(task->page_directory);
    return EOK;
}

uint32_t task_page()
{
    user_registers();
    task_switch(current_task);
    return EOK;
}

void task_run_first_task()
{
    if (!current_task)
    {
        PANIC_PRINT("task_run_first_task: current_task is NULL");
    }

    task_switch(task_head);
    task_return(&task_head->registers);
}

uint32_t task_init(task_t *task, struct _process *process)
{
    memset(task, 0, sizeof(task_t));
    task->page_directory = page_directory_create(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL); // TODO: remove flags
    if (!task->page_directory)
    {
        return ENOMEM;
    }

    task->registers.ip = KERNEL_TASK_VADDR;
    task->registers.ss = USER_DATA_SELECTOR;
    task->registers.esp = KERNEL_TASK_STACK_VADDR;

    task->process = process;

    return EOK;
}
