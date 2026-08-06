#include <kernel.h>

extern "C" void terminal_app_banner(void) {
    kprintf("Yukka OS Terminal\n");
    kprintf("Type help to list commands.\n");
}
