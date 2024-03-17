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

task_t *task_new()
{
    uint32_t res = EOK;
    task_t *task = kmalloc(sizeof(task_t));
    if (!task)
    {
        res = ENOMEM;
        goto out;
    }

    res = task_init(task);
    if (res != EOK)
    {
        kfree(task);
        goto out;
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

out:
    return res;
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

uint32_t task_init(task_t *task)
{
    memset(task, 0, sizeof(task_t));
    task->page_directory = page_directory_create(PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL);
    if (!task->page_directory)
    {
        return ENOMEM;
    }

    task->registers.ip = KERNEL_TASK_VADDR;
    task->registers.ss = USER_DATA_SELECTOR;
    task->registers.esp = KERNEL_TASK_STACK_VADDR;

    return EOK;
}
