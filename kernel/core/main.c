#include <kernel.h>

void kernel_main(uint32_t magic, uint32_t mbi) {
    outb(0x3F8, 'M');

    serial_init();
    kprintf("STEP1 serial ok\n");

    idt_init();
    kprintf("STEP2 idt ok\n");

    gdt_init();
    kprintf("STEP3 gdt ok\n");

    terminal_init();
    kprintf("STEP4 terminal ok\n");

    pic_init();
    kprintf("STEP5 pic ok\n");

    pit_init();
    kprintf("STEP6 pit ok\n");

    keyboard_init();
    kprintf("STEP7 keyboard ok\n");

    memory_init(magic, mbi);
    kprintf("STEP8 memory ok\n");

    ramdisk_init();
    kprintf("STEP9 ramdisk ok\n");

    kprintf("Magic: 0x%x MBI: 0x%x\n", magic, mbi);
    kprintf("ALL STEPS PASSED - kernel alive\n");

    for (;;) {
        __asm__ volatile("cli; hlt");
    }
}
