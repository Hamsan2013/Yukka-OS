.section .text
.code64

.macro ISR_NOERR num
.global isr_stub_\num
isr_stub_\num:
    pushq $0
    pushq $\num
    jmp isr_common_stub
.endm

.macro ISR_ERR num
.global isr_stub_\num
isr_stub_\num:
    pushq $\num
    jmp isr_common_stub
.endm

.global isr_stub_default
isr_stub_default:
    pushq $0
    pushq $255
    jmp isr_common_stub

ISR_NOERR 0
ISR_ERR 8
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 47

isr_common_stub:
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15

    # Print exception number to serial (skip IRQs >= 32)
    mov 120(%rsp), %rbx
    cmp $32, %rbx
    jge 9f

    mov $0x3F8, %dx
    mov $'X', %al
    outb %al, %dx
    mov $':', %al
    outb %al, %dx

    mov %rbx, %rax
    shr $4, %rax
    and $0xF, %rax
    cmp $10, %rax
    jb 1f
    sub $10, %rax
    add $'a', %rax
    jmp 2f
1:  add $'0', %rax
2:  outb %al, %dx

    mov %rbx, %rax
    and $0xF, %rax
    cmp $10, %rax
    jb 3f
    sub $10, %rax
    add $'a', %rax
    jmp 4f
3:  add $'0', %rax
4:  outb %al, %dx

    mov $0x0A, %al
    outb %al, %dx

9:
    mov %rbx, %rdi
    cmpq $32, %rdi
    je .Lpit
    cmpq $33, %rdi
    je .Lkeyboard
    cmpq $47, %rdi
    je .Lspurious
    cmpq $8, %rdi
    je .Lerror
    mov 128(%rsp), %rsi
    call isr_common_noerr
    jmp .Lrestore
.Lpit:
    call irq_pit_handler
    jmp .Lrestore
.Lkeyboard:
    call irq_keyboard_handler
    jmp .Lrestore
.Lspurious:
    call irq_spurious_handler
    jmp .Lrestore
.Lerror:
    mov 128(%rsp), %rsi
    call isr_common_error

.Lrestore:
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
    addq $16, %rsp
    iretq
