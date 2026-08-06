#include <kernel.h>

void kernel_main(uint32_t magic, uint32_t mbi) {
    serial_init();
    terminal_init();
    gdt_init();
    idt_init();
    pic_init();
    pic_enable();
    pit_init();
    keyboard_init();
    memory_init(magic, mbi);
    time_init();
    ramdisk_init();
    yakka_init();

    terminal_app_banner();
    kprintf("Yukka OS\n");
    kprintf("Version 1.0.0\n");

#ifdef ENABLE_TESTS
    kprintf("TEST PASS boot\n");
    kernel_self_tests();
    power_test_exit();
#endif

    shell_run();
}
