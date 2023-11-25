#ifndef __KERNEL_IDE_H
#define __KERNEL_IDE_H

#include <kernel/types.h>
#include <kernel/dev/driver_manager.h>

uint32_t ide_driver_init(driver_t *driver, disk_t **disks, uint8_t *num_ide_disks);
uint32_t ide_driver_write_sector(disk_t *disk, uint32_t lba, uint8_t *buf);
uint32_t ide_driver_read_sector(disk_t *disk, uint32_t lba, uint8_t *buf);

#endif