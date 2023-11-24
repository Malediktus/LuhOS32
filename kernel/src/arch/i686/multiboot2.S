#define ASM_FILE 1
#include <kernel/multiboot2.h>

#define GRUB_MULTIBOOT_ARCHITECTURE_I386 0
#define STACK_SIZE                      0x4000
        
    .text

    .extern kernel_main
    .globl _start
_start:
    jmp multiboot_entry

    .section .multiboot
    .align 8
multiboot_header:
    .long MULTIBOOT2_HEADER_MAGIC
    .long GRUB_MULTIBOOT_ARCHITECTURE_I386
    .long multiboot_header_end - multiboot_header
    .long -(MULTIBOOT2_HEADER_MAGIC + GRUB_MULTIBOOT_ARCHITECTURE_I386 + (multiboot_header_end - multiboot_header))

/*
    .align 8
framebuffer_tag_start:
    .short MULTIBOOT_HEADER_TAG_FRAMEBUFFER
    .short MULTIBOOT_HEADER_TAG_OPTIONAL
    .long framebuffer_tag_end - framebuffer_tag_start
    .long 1024
    .long 768
    .long 32
*/
framebuffer_tag_end:

    .align 8
    .short MULTIBOOT_HEADER_TAG_END
    .short 0
    .long 8
multiboot_header_end:

    .text
multiboot_entry:
    movl $(stack + STACK_SIZE), %esp

    pushl $0
    popf

    pushl %ebx
    pushl %eax
    call kernel_main

loop:
    cli
    hlt
    jmp loop

.bss
    .comm stack, STACK_SIZE
