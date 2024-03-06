#ifndef __KERNEL_MBR_H
#define __KERNEL_MBR_H

#include <kernel/types.h>
#include <kernel/dev/block_device.h>

uint32_t scan_logical_block_devices_mbr();

#endif