OUTPUT_FORMAT(binary)

ENTRY(_start)

SECTIONS
{
    . = 0x400000;
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

    /DISCARD/ : {
        *(.eh_frame)
        *(.eh_frame)
        *(.note .note.*)
        *(.note.gnu.build-id)
    }
}