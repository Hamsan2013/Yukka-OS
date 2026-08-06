#include <kernel.h>

#define SERIAL_PORT 0x3F8

void serial_init(void) {
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x80);
    outb(SERIAL_PORT + 0, 0x03);
    outb(SERIAL_PORT + 1, 0x00);
    outb(SERIAL_PORT + 3, 0x03);
    outb(SERIAL_PORT + 2, 0xC7);
    outb(SERIAL_PORT + 4, 0x0B);
}

static int serial_ready(void) {
    return inb(SERIAL_PORT + 5) & 0x20;
}

void serial_write(char c) {
    while (!serial_ready()) {}
    outb(SERIAL_PORT, (uint8_t)c);
}

void serial_puts(const char* s) {
    while (*s) {
        serial_write(*s++);
    }
}
