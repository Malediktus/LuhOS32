#ifndef __KERNEL_INITRD_H
#define __KERNEL_INITRD_H

#include <kernel/types.h>
#include <kernel/fs/vfs.h>

fs_node_t *initialise_initrd(uint32_t location);

#endif