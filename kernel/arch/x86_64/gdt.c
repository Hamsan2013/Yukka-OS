#include <kernel.h>

typedef struct {
    uint16_t limit;
    uint64_t base;
} PACKED gdt_ptr_t;

typedef struct {
    uint64_t entries[3];
} PACKED gdt_t;

static gdt_t gdt;

void gdt_init(void) {
    gdt.entries[0] = 0;
    gdt.entries[1] = 0x00209A0000000000ULL;
    gdt.entries[2] = 0x0000920000000000ULL;

    gdt_ptr_t ptr;
    ptr.limit = sizeof(gdt_t) - 1;
    ptr.base = (uint64_t)(uintptr_t)&gdt;

    __asm__ volatile("lgdt %0" :: "m"(ptr));

    uint16_t data = 0x10;
    __asm__ volatile(
        "mov %0, %%ds\n"
        "mov %0, %%es\n"
        "mov %0, %%fs\n"
        "mov %0, %%gs\n"
        "mov %0, %%ss\n"
        :: "r"(data)
    );
}
