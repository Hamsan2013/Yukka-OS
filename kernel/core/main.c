#include <kernel.h>

void kernel_main(uint32_t magic, uint32_t mbi) {
    /* CRITICAL: Serial MUST initialize before anything else */
    outb(0x3F8, 'M');  // Debug marker
    
    serial_init();
    kprintf("SAFE MODE: Serial initialized\n");
    
    terminal_init();
    kprintf("SAFE MODE: Terminal initialized\n");
    
    /* Temporarily skip all other init to isolate crash */
    kprintf("Yukka OS Safe Mode - Kernel alive!\n");
    kprintf("Magic: 0x%x, MBI: 0x%x\n", magic, mbi);
    
    /* Halt safely - no interrupts, no shell */
    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
