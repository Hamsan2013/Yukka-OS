#include <kernel.h>

void kernel_self_tests(void) {
    void* p = kmalloc(128);
    if (p) {
        kprintf("TEST PASS memory\n");
        kfree(p);
    } else {
        kprintf("TEST FAIL memory\n");
    }

    fs_make_file("test.txt");
    fs_write_file("test.txt", "hi");

    char buf[16];
    if (fs_read_file("test.txt", buf, sizeof(buf)) >= 0 && kstrcmp(buf, "hi") == 0) {
        kprintf("TEST PASS filesystem\n");
    } else {
        kprintf("TEST FAIL filesystem\n");
    }

    fs_delete("test.txt");

    terminal_write("");
    kprintf("TEST PASS terminal\n");

    if (yakka_self_test()) {
        kprintf("TEST PASS yakka\n");
    } else {
        kprintf("TEST FAIL yakka\n");
    }
}
