#include <kernel/dev/block_device.h>

static block_device_t *block_devices[MAX_BLOCK_DEVICES];
static uint32_t num_block_devices = 0;

static logical_block_device_t *logical_block_devices[MAX_LOGICAL_BLOCK_DEVICES];
static uint32_t num_logical_block_devices = 0;

void register_block_device(block_device_t *bdev)
{
    bdev->device_id = num_block_devices; // TODO: use uuids from parition/filsystem metadata
    bdev->device_name[0] = 's';
    bdev->device_name[1] = 'd';
    bdev->device_name[2] = 'a' + num_block_devices;
    bdev->device_name[3] = '\0';
    block_devices[num_block_devices++] = bdev;
}

void submit_read_request(block_device_t *bdev, block_request_t *request)
{
    // TODO: corrrecting io errors (retrying and marking bad sectors)
    // TODO: request queue and io scheduling

    uint32_t result = EOK;
    for (uint32_t i = 0; i < request->num_blocks; i++)
    {
        result = bdev->implementation.read_block(bdev, request->lba + i, request->buffer + (i * bdev->block_size));
        if (result != EOK)
        {
            PANIC_PRINT("block device read failure");
        }
    }
}

void submit_write_request(block_device_t *bdev, block_request_t *request)
{
    uint32_t result = EOK;
    for (uint32_t i = 0; i < request->num_blocks; i++)
    {
        result = bdev->implementation.write_block(bdev, request->lba + i, request->buffer + (i * bdev->block_size));
        if (result != EOK)
        {
            PANIC_PRINT("block device write failure");
        }
    }
}

block_device_t **get_block_devices()
{
    return block_devices;
}

uint32_t get_num_block_devices()
{
    return num_block_devices;
}

void register_logical_block_device(logical_block_device_t *lbdev)
{
    lbdev->device_id = num_block_devices; // TODO: use uuids from parition/filsystem metadata
    lbdev->device_name[0] = 's';
    lbdev->device_name[1] = 'd';
    lbdev->device_name[2] = lbdev->parent->device_name[2];
    lbdev->device_name[3] = '1' + lbdev->local_id;
    lbdev->device_name[4] = '\0';
    logical_block_devices[num_logical_block_devices++] = lbdev;
}

logical_block_device_t **get_logical_block_devices()
{
    return logical_block_devices;
}

uint32_t get_num_logical_block_devices()
{
    return num_logical_block_devices;
}
