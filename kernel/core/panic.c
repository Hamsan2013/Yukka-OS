#include <kernel.h>

void panic(const char* fmt, ...) {
    va_list args;
    kprintf("\nPANIC: ");
    va_start(args, fmt);
    vkprintf(fmt, args);
    va_end(args);
    kprintf("\n");

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
