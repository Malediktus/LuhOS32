#ifndef __KERNEL_TTY_H
#define __KERNEL_TTY_H

#include <kernel/types.h>
#include <kernel/dev/driver_manager.h>

void clear_tty(void);
void kprintf(const char *fmt, ...);

#endif