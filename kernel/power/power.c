#include <kernel.h>

void power_shutdown(void) {
    kprintf("Shutdown\n");
    outw(0x604, 0x2000);
    outw(0xB004, 0x2000);
    outw(0x4004, 0x3400);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

void power_reboot(void) {
    uint8_t good = 0x02;
    while (good & 0x02) {
        good = inb(0x64);
    }
    outb(0x64, 0xFE);

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}

void power_test_exit(void) {
    outw(0xF4, 0x00);
    power_shutdown();
}
