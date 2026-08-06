// kernel/arch/x86_64/isr_stub.S
.section .text
.code64

.macro ISR_NOERR num
.global isr_stub_\num
isr_stub_\num:
    pushq $0          // Dummy error code
    pushq $\num       // Interrupt number
    jmp isr_common_stub
.endm

.macro ISR_ERR num
.global isr_stub_\num
isr_stub_\num:
    pushq $\num       // Interrupt number (error code already pushed by CPU)
    jmp isr_common_stub
.endm

// Default stub for unused vectors
.global isr_stub_default
isr_stub_default:
    pushq $0
    pushq $255
    jmp isr_common_stub

// Specific stubs
ISR_NOERR 0
ISR_ERR   8
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 47

// Common stub: save all registers, call C handler, restore
isr_common_stub:
    // Save all general-purpose registers
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

    // Pass interrupt number as first argument (System V AMD64 ABI)
    movq 120(%rsp), %rdi   // int_num is at rsp+120 after 15 pushes + 2 original pushes

    // Call appropriate C handler based on vector
    cmpq $32, %rdi
    je .Lpit
    cmpq $33, %rdi
    je .Lkeyboard
    cmpq $47, %rdi
    je .Lspurious
    cmpq $8, %rdi
    je .Lerror
    // Default: no-error exception
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
    movq 128(%rsp), %rsi   // error code at rsp+128
    call isr_common_error

.Lrestore:
    // Restore all registers
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

    // Remove int_num and error code
    addq $16, %rsp

    iretq
