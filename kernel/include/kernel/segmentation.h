#ifndef __KERNEL_SEGMENTATION_H
#define __KERNEL_SEGMENTATION_H

#include <kernel/types.h>

#define KERNEL_CODE_SELECTOR 0x08

uint32_t segmentation_init(void);

#endif