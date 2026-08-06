ENTRY(_start)

SECTIONS
{
    . = 1M;

    .multiboot : { KEEP(*(.multiboot)) }

    .text : { *(.text*) }

    .rodata : { *(.rodata*) }

    .data : { *(.data*) }

    .bss : {
        *(.bss)
        *(COMMON)
    }

    . = ALIGN(4096);
    PROVIDE(_kernel_end = .);
}
