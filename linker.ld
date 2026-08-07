ENTRY(_start)

SECTIONS
{
    . = 1M;

    /* Multiboot2 header MUST be 8-byte aligned and in first 32KB */
    .multiboot : ALIGN(8) {
        KEEP(*(.multiboot))
    }

    .text : ALIGN(4K) {
        *(.text*)
    }

    .rodata : ALIGN(4K) {
        *(.rodata*)
    }

    .data : ALIGN(4K) {
        *(.data*)
    }

    .bss : ALIGN(4K) {
        *(.bss)
        *(COMMON)
    }

    . = ALIGN(4K);
    PROVIDE(_kernel_end = .);
}
