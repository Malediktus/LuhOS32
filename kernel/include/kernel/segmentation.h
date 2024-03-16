#ifndef __KERNEL_SEGMENTATION_H
#define __KERNEL_SEGMENTATION_H

#include <kernel/types.h>
#include <kernel/tss.h>

#define KERNEL_CODE_SELECTOR 0x08
#define KERNEL_DATA_SELECTOR 0x10

uint32_t segmentation_init(struct tss *tss);

#endif