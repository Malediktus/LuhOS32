#ifndef __KERNEL_FAT32_H
#define __KERNEL_FAT32_H

#include <kernel/types.h>
#include <kernel/fs/vfs.h>

fs_node_t *initialise_fat32(logical_block_device_t *lbdev);

#endif