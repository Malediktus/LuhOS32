#ifndef __KERNEL_BLOCK_DEVICE_H
#define __KERNEL_BLOCK_DEVICE_H

#include <kernel/types.h>
#include <kernel/dev/driver_manager.h>

#define MAX_BLOCK_DEVICES 10
#define MAX_LOGICAL_BLOCK_DEVICES 50

struct block_device;

typedef struct
{
    uint32_t (*write_block)(struct block_device *, uint32_t, uint8_t *);
    uint32_t (*read_block)(struct block_device *, uint32_t, uint8_t *);
    void *private_data;
} block_device_impl_t;

typedef struct block_device
{
    uint32_t device_id;
    char device_name[32];
    uint32_t block_size;
    uint32_t total_blocks;
    block_device_impl_t implementation;
} block_device_t;

typedef struct
{
    struct block_device *bdev;
    uint32_t lba;
    uint32_t num_blocks;
    uint8_t *buffer;
    bool is_write;
} block_request_t;

void register_block_device(block_device_t *bdev);
void submit_read_request(block_device_t *bdev, block_request_t *request);
void submit_write_request(block_device_t *bdev, block_request_t *request);

block_device_t **get_block_devices();
uint32_t get_num_block_devices();

#define LOGICAL_BLOCK_DEVICE_FLAGS_BOOTABLE 1 << 0

typedef struct
{
    uint32_t device_id;
    char device_name[32];
    block_device_t *parent;

    uint8_t flags;
    uint32_t lba_offset;
    uint32_t num_blocks;
    uint8_t type;
    uint8_t local_id;
} logical_block_device_t;

void register_logical_block_device(logical_block_device_t *lbdev);

logical_block_device_t **get_logical_block_devices();
uint32_t get_num_logical_block_devices();

#endif