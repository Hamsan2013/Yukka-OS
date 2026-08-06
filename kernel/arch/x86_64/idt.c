// kernel/arch/x86_64/idt.c
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

// Plain C handlers - NO __attribute__((interrupt))
void isr_common_noerr(void) {
    panic("CPU exception (no error code)");
}

void isr_common_error(uint64_t error_code) {
    (void)error_code;
    panic("CPU exception (with error code)");
}

void irq_pit_handler(void) {
    pit_tick();
    outb(0x20, 0x20);
}

void irq_keyboard_handler(void) {
    uint8_t scancode = inb(0x60);
    keyboard_handle_scancode(scancode);
    outb(0x20, 0x20);
}

void irq_spurious_handler(void) {
    outb(0x20, 0x20);
}

// Assembly stubs declared externally
extern void isr_stub_0(void);
extern void isr_stub_8(void);
extern void isr_stub_32(void);
extern void isr_stub_33(void);
extern void isr_stub_47(void);
extern void isr_stub_default(void);

static void idt_set_gate(int index, void (*handler)(void), uint8_t type_attr) {
    uint64_t addr = (uint64_t)(uintptr_t)handler;
    idt[index].offset_low = addr & 0xFFFF;
    idt[index].selector = 0x08;
    idt[index].ist = 0;
    idt[index].type_attr = type_attr;
    idt[index].offset_mid = (addr >> 16) & 0xFFFF;
    idt[index].offset_high = (addr >> 32) & 0xFFFFFFFF;
    idt[index].zero = 0;
}

void idt_init(void) {
    // Set all entries to default handler first
    for (int i = 0; i < 256; i++) {
        idt_set_gate(i, isr_stub_default, 0x8E);
    }

    // Override specific vectors
    idt_set_gate(0, isr_stub_0, 0x8E);   // Divide by zero (no err)
    idt_set_gate(8, isr_stub_8, 0x8E);   // Double fault (err)
    idt_set_gate(32, isr_stub_32, 0x8E); // PIT
    idt_set_gate(33, isr_stub_33, 0x8E); // Keyboard
    idt_set_gate(47, isr_stub_47, 0x8E); // Spurious

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
