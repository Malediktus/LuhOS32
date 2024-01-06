#ifndef __KERNEL_FAT32_H
#define __KERNEL_FAT32_H

#include <kernel/types.h>
#include <kernel/fs/vfs.h>

uint32_t fat32_read(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data);
uint32_t fat32_write(fs_node_t *node, uint32_t offset, uint32_t size, uint8_t *buffer, logical_block_device_t *lbdev, void *private_data);
void fat32_open(fs_node_t *node, uint8_t mode, logical_block_device_t *lbdev, void *private_data);
void fat32_close(fs_node_t *node, logical_block_device_t *lbdev, void *private_data);
struct dirent *fat32_readdir(fs_node_t *node, uint32_t index, logical_block_device_t *lbdev, void *private_data);
fs_node_t *fat32_finddir(fs_node_t *node, char *name, logical_block_device_t *lbdev, void *private_data);

#endif