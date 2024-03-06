#ifndef __KERNEL_IDE_H
#define __KERNEL_IDE_H

#include <kernel/types.h>
#include <kernel/dev/block_device.h>

uint32_t ide_driver_init();
uint32_t ide_driver_scan_disks();
uint32_t ide_write_block(block_device_t *bdev, uint32_t lba, uint8_t *buf);
uint32_t ide_read_block(block_device_t *bdev, uint32_t lba, uint8_t *buf);

#endif