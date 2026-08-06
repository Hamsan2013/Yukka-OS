#include <kernel.h>

typedef struct {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} interrupt_frame_t;

typedef struct {
    uint16_t offset_low;
    uint16_t selector;
    uint8_t ist;
    uint8_t type_attr;
    uint16_t offset_mid;
    uint32_t offset_high;
    uint32_t zero;
} PACKED idt_entry_t;

typedef struct {
    uint16_t limit;
    uint64_t base;
} PACKED idtr_t;

static idt_entry_t idt[256];
static idtr_t idtr;

__attribute__((interrupt))
void isr_common_noerr(interrupt_frame_t* frame) {
    (void)frame;
    panic("CPU exception");
}

__attribute__((interrupt))
void isr_common_error(interrupt_frame_t* frame, uint64_t error) {
    (void)frame;
    (void)error;
    panic("CPU exception with error");
}

__attribute__((interrupt))
void irq_pit(interrupt_frame_t* frame) {
    (void)frame;
    pit_tick();
    outb(0x20, 0x20);
}

__attribute__((interrupt))
void irq_keyboard(interrupt_frame_t* frame) {
    (void)frame;
    uint8_t scancode = inb(0x60);
    keyboard_handle_scancode(scancode);
    outb(0x20, 0x20);
}

__attribute__((interrupt))
void irq_spurious(interrupt_frame_t* frame) {
    (void)frame;
    outb(0x20, 0x20);
}

static void idt_set_gate(int index, uint64_t handler, uint8_t type_attr) {
    idt[index].offset_low = handler & 0xFFFF;
    idt[index].selector = 0x08;
    idt[index].ist = 0;
    idt[index].type_attr = type_attr;
    idt[index].offset_mid = (handler >> 16) & 0xFFFF;
    idt[index].offset_high = (handler >> 32) & 0xFFFFFFFF;
    idt[index].zero = 0;
}

void idt_init(void) {
    uint64_t noerr = (uint64_t)(uintptr_t)(void*)isr_common_noerr;
    uint64_t err = (uint64_t)(uintptr_t)(void*)isr_common_error;
    uint64_t pit = (uint64_t)(uintptr_t)(void*)irq_pit;
    uint64_t keyboard = (uint64_t)(uintptr_t)(void*)irq_keyboard;
    uint64_t spurious = (uint64_t)(uintptr_t)(void*)irq_spurious;

    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, noerr, 0x8E);
    }

    int error_vectors[] = {8, 10, 11, 12, 13, 14, 17, 21, 29, 30};
    for (int i = 0; i < 10; i++) {
        idt_set_gate(error_vectors[i], err, 0x8E);
    }

    idt_set_gate(32, pit, 0x8E);
    idt_set_gate(33, keyboard, 0x8E);
    idt_set_gate(47, spurious, 0x8E);

    idtr.limit = sizeof(idt) - 1;
    idtr.base = (uint64_t)(uintptr_t)&idt;

    __asm__ volatile("lidt %0" :: "m"(idtr));
}

void interrupts_enable(void) {
    __asm__ volatile("sti");
}

void interrupts_disable(void) {
    __asm__ volatile("cli");
}
