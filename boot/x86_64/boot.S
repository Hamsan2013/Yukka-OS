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
    
    /* EARLY DEBUG: Print 'B' before saving registers */
    mov $0x3F8, %dx
    mov $'B', %al
    outb %al, %dx
    
    mov %eax, 0x74000
    mov %ebx, 0x74004

    cld
    mov $0x70000, %edi
    xor %eax, %eax
    mov $0x3000, %ecx
    rep stosl

    mov $0x71000, %eax
    or $3, %eax
    mov %eax, 0x70000

    mov $0x72000, %eax
    or $3, %eax
    mov %eax, 0x71000

    mov $0x72000, %edi
    xor %ebx, %ebx
    mov $512, %ecx
1:
    mov $0x83, %eax
    mov %ebx, %edx
    shl $21, %edx
    or %edx, %eax
    mov %eax, (%edi)
    add $8, %edi
    inc %ebx
    loop 1b

    mov $0x73000, %edi
    xor %eax, %eax
    mov $8, %ecx
    rep stosl

    movl $0x00000000, 0x73000
    movl $0x00000000, 0x73004
    movl $0x00000000, 0x73008
    movl $0x00209A00, 0x7300C
    movl $0x00000000, 0x73010
    movl $0x00009200, 0x73014

    movw $23, 0x73100
    movl $0x73000, 0x73102

    mov $0x70000, %eax
    mov %eax, %cr3

    mov %cr4, %eax
    or $0x20, %eax
    mov %eax, %cr4

    mov $0xC0000080, %ecx
    rdmsr
    or $0x100, %eax
    wrmsr

    mov %cr0, %eax
    or $0x80000001, %eax
    mov %eax, %cr0

    .byte 0x0f,0x01,0x16
    .long 0x73100

    ljmp $0x08, $long_mode_start

.code64
long_mode_start:
    /* DEBUG: 'L' = Long mode entered */
    mov $0x3F8, %dx
    mov $'L', %al
    outb %al, %dx

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    /* DEBUG: 'S' = Segments loaded */
    mov $'S', %al
    outb %al, %dx

    mov $0x90000, %rsp

    /* DEBUG: 'K' = Stack set */
    mov $'K', %al
    outb %al, %dx

    mov $0x74000, %rbx
    mov (%rbx), %edi

    mov $0x74004, %rbx
    mov (%rbx), %esi

    call kernel_main

    cli
1:
    hlt
    jmp 1b
