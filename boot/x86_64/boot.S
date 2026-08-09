.section .multiboot, "a"
.align 8
multiboot_header_start:
    .long 0xe85250d6
    .long 0
    .long multiboot_header_end - multiboot_header_start
    .long -(0xe85250d6 + 0 + (multiboot_header_end - multiboot_header_start))
    .align 8
    .short 0
    .short 0
    .long 8
multiboot_header_end:

.section .text
.code32
.global _start
_start:
    cli
    mov $0x3F8, %dx
    mov $'B', %al
    outb %al, %dx
    
    # Skip all page table/GDT setup - just call kernel directly
    # This will crash but we'll see how far we get
    mov $0x90000, %esp
    call kernel_main_simple

1:  hlt
    jmp 1b

.code64
kernel_main_simple:
    mov $0x3F8, %dx
    mov $'K', %al
    outb %al, %dx
    ret
