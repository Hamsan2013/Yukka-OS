#include <kernel.h>

static volatile uint64_t ticks = 0;

void pit_tick(void) {
    ticks++;
}

uint64_t pit_ticks(void) {
    return ticks;
}

void pit_init(void) {
    uint32_t divisor = 1193180 / 100;
    outb(0x43, 0x36);
    outb(0x40, (uint8_t)(divisor & 0xFF));
    outb(0x40, (uint8_t)((divisor >> 8) & 0xFF));
}
