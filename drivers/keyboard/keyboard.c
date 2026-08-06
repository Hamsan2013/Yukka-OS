#include <kernel.h>

static volatile int buffer[256];
static volatile uint16_t head = 0;
static volatile uint16_t tail = 0;

void keyboard_init(void) {
    head = 0;
    tail = 0;
}

void keyboard_handle_scancode(uint8_t scancode) {
    int c = scancode_decode(scancode);

    if (c > 0) {
        buffer[head] = c;
        head = (head + 1) & 255;
    }
}

int keyboard_poll(void) {
    interrupts_disable();

    int c = -1;
    if (tail != head) {
        c = buffer[tail];
        tail = (tail + 1) & 255;
    }

    interrupts_enable();
    return c;
}
