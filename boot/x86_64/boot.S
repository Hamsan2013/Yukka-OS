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
    cld

    # Preserve Multiboot magic (EAX) and info (EBX)
    mov %eax, %ebp
    mov %ebx, 0x7F004

    # B = CPU reached _start
    mov $0x3F8, %dx
    mov $'B', %al
    outb %al, %dx

    mov %ebp, 0x7F000

    # 1 = registers saved
    mov $'1', %al
    outb %al, %dx

    # COM1 init 115200 8N1
    mov $0x3FB, %dx
    mov $0x80, %al
    outb %al, %dx
    mov $0x3F8, %dx
    mov $0x01, %al
    outb %al, %dx
    mov $0x3F9, %dx
    mov $0x00, %al
    outb %al, %dx
    mov $0x3FB, %dx
    mov $0x03, %al
    outb %al, %dx
    mov $0x3F9, %dx
    mov $0x00, %al
    outb %al, %dx
    mov $0x3FA, %dx
    mov $0xC7, %al
    outb %al, %dx
    mov $0x3FC, %dx
    mov $0x0B, %al
    outb %al, %dx

    # 2 = serial ready
    mov $0x3F8, %dx
    mov $'2', %al
    outb %al, %dx

    # Page tables (3 pages at 0x70000)
    mov $0x70000, %edi
    xor %eax, %eax
    mov $0xC00, %ecx
    rep stosl

    mov $0x71003, %eax
    mov %eax, 0x70000
    mov $0x72003, %eax
    mov %eax, 0x71000

    mov $0x72000, %edi
    xor %ebx, %ebx
    mov $512, %ecx
1:
    mov %ebx, %eax
    shl $21, %eax
    or $0x83, %eax
    mov %eax, (%edi)
    add $8, %edi
    inc %ebx
    loop 1b

    # 3 = page tables built
    mov $0x3F8, %dx
    mov $'3', %al
    outb %al, %dx

    # GDT at 0x73000
    mov $0x73000, %edi
    xor %eax, %eax
    mov $6, %ecx
    rep stosl
    movl $0x00209A00, 0x7300C
    movl $0x00009200, 0x73014
    movw $23, 0x73100
    movl $0x73000, 0x73102

    # 4 = GDT ready
    mov $0x3F8, %dx
    mov $'4', %al
    outb %al, %dx

    # Enable long mode + paging
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

    # 5 = paging on, about to lgdt + ljmp
    mov $0x3F8, %dx
    mov $'5', %al
    outb %al, %dx

    .byte 0x0f,0x01,0x16
    .long 0x73100

    ljmp $0x08, $long_mode_start

.code64
long_mode_start:
    # 6 = IN 64-BIT MODE!
    mov $0x3F8, %dx
    mov $'6', %al
    outb %al, %dx

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # 7 =
