OUTPUT_FORMAT(elf32-i386)

ENTRY(_start)

SECTIONS
{
    . = 0x0100000;
    _kernel_start = .;

    .multiboot BLOCK(4K) : ALIGN(4K) {
        *(.multiboot)
    }

    .text BLOCK(4K) : ALIGN(4K) {
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K) {
        *(.data)
    }

    .bss BLOCK(4K) : ALIGN(4K) {
        *(.bss)
        *(COMMON)
    }

    _kernel_end = .;

    /DISCARD/ : {
        *(.eh_frame)
        *(.eh_frame)
        *(.note .note.*)
        *(.note.gnu.build-id)
    }
}