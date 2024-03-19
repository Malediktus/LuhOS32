#include <kernel/process.h>
#include <kernel/heap.h>
#include <kernel/fs/vfs.h>
#include <kernel/lib/string.h>

process_t *current_process = NULL;
static process_t *processes[KERNEL_MAX_PROCESSES] = {};

static void process_init(process_t *process)
{
    memset(process, 0, sizeof(process_t));
}

process_t *process_current()
{
    return current_process;
}

process_t *process_get(uint32_t index)
{
    if (index >= KERNEL_MAX_PROCESSES)
    {
        PANIC_PRINT("process_get: Invalid argument");
    }

    return processes[index];
}

static uint32_t process_load_binary(const char *path, process_t *process)
{
    uint32_t res = EOK;
    fs_node_t *file = open_fs(path);
    if (!file)
    {
        res = EIO;
        return res;
    }

    void *program_data_ptr = kmalloc_aligned(PAGE_SIZE, file->filesize);
    if (!program_data_ptr)
    {
        res = ENOMEM;
        goto out;
    }

    res = read_fs(file, 0, file->filesize, program_data_ptr);
    if (res != EOK)
    {
        goto out;
    }

    process->data = program_data_ptr;
    process->size = file->filesize;

out:
    close_fs(file);
    return res;
}

static uint32_t process_load_data(const char *path, process_t *process)
{
    uint32_t res = EOK;
    res = process_load_binary(path, process);
    return res;
}

extern uint32_t *kernel_page_directory;

uint32_t process_map_binary(process_t *process)
{
    uint32_t res = EOK;
    void *data_phys_addr = paging_get_phys_address(kernel_page_directory, process->data);
    res = paging_map_to(process->task->page_directory, (void *)KERNEL_TASK_VADDR, data_phys_addr, paging_align_address(data_phys_addr + process->size), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);
    return res;
}

uint32_t process_map_memory(process_t *process)
{
    uint32_t res = EOK;
    res = process_map_binary(process);
    if (res != EOK)
    {
        goto out;
    }

    // res = paging_map_to(process->task->page_directory, (void *)(KERNEL_TASK_STACK_VADDR - KERNEL_TASK_STACK_SIZE), process->stack, paging_align_address(process->stack + KERNEL_TASK_STACK_SIZE), PAGING_IS_PRESENT | PAGING_ACCESS_FROM_ALL | PAGING_IS_WRITEABLE);

out:
    return res;
}

uint32_t process_load_for_slot(const char *path, process_t **process, uint32_t process_slot)
{
    uint32_t res = EOK;
    task_t *task = NULL;
    process_t *_process = NULL;
    void *program_stack_ptr = NULL;

    if (process_get(process_slot) != NULL)
    {
        res = EINVARG;
        goto out;
    }

    _process = kmalloc(sizeof(process_t));
    if (!_process)
    {
        res = ENOMEM;
        goto out;
    }

    process_init(_process);
    res = process_load_data(path, _process);
    if (res != EOK)
    {
        kfree(_process);
        goto out;
    }

    program_stack_ptr = kmalloc_aligned(PAGE_SIZE, KERNEL_TASK_STACK_SIZE);
    if (!program_stack_ptr)
    {
        // TODO: memory leak... unload process
        res = -ENOMEM;
        goto out;
    }

    _process->stack = program_stack_ptr;
    strncpy(_process->path, path, sizeof(_process->path));

    _process->id = process_slot;

    // create task
    task = task_new(_process);
    if (!task)
    {
        // TODO: memory leak... unload process
        res = ENOMEM;
        goto out;
    }

    _process->task = task;

    res = process_map_memory(_process);
    if (res != EOK)
    {
        // TODO: memory leak... unload process
        task_free(task);
        goto out;
    }

    *process = _process;

    // add to array
    processes[process_slot] = _process;

out:
    return res;
}

int32_t process_get_free_slot()
{
    for (uint32_t i = 0; i < KERNEL_MAX_PROCESSES; i++)
    {
        if (processes[i] == NULL)
        {
            return (int32_t)i;
        }
    }

    return -1;
}

uint32_t process_load(const char *path, process_t **process)
{
    uint32_t res = EOK;
    int32_t process_slot = process_get_free_slot();
    if (process_slot < 0)
    {
        res = ENOMEM;
        goto out;
    }

    res = process_load_for_slot(path, process, (uint32_t)process_slot);

out:
    return res;
}
