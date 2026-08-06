#include <kernel.h>

#define VGA_MEMORY ((uint16_t*)0xB8000)
#define VGA_WIDTH 80
#define VGA_HEIGHT 25

static uint8_t row = 0;
static uint8_t col = 0;
static uint8_t attr = 0x0F;

static void put_entry(char c, uint8_t color, int x, int y) {
    VGA_MEMORY[y * VGA_WIDTH + x] = (uint16_t)(((uint16_t)color << 8) | (uint8_t)c);
}

void terminal_set_color(uint8_t fg, uint8_t bg) {
    attr = (uint8_t)((bg << 4) | (fg & 0x0F));
}

void terminal_cursor_update(void) {
    uint16_t pos = (uint16_t)(row * VGA_WIDTH + col);
    outb(0x3D4, 0x0F);
    outb(0x3D5, (uint8_t)(pos & 0xFF));
    outb(0x3D4, 0x0E);
    outb(0x3D5, (uint8_t)((pos >> 8) & 0xFF));
}

static void scroll(void) {
    kmemcpy(VGA_MEMORY, VGA_MEMORY + VGA_WIDTH, VGA_WIDTH * (VGA_HEIGHT - 1) * 2);

    for (int x = 0; x < VGA_WIDTH; x++) {
        put_entry(' ', attr, x, VGA_HEIGHT - 1);
    }

    row = VGA_HEIGHT - 1;
}

void terminal_clear(void) {
    for (int y = 0; y < VGA_HEIGHT; y++) {
        for (int x = 0; x < VGA_WIDTH; x++) {
            put_entry(' ', attr, x, y);
        }
    }

    row = 0;
    col = 0;
    terminal_cursor_update();
}

void terminal_init(void) {
    attr = 0x0F;
    terminal_clear();
}

void terminal_putchar(char c) {
    if (c == '\n') {
        col = 0;
        row++;
        if (row == VGA_HEIGHT) scroll();
    } else if (c == '\r') {
        col = 0;
    } else if (c == '\t') {
        col = (col + 8) & ~7;
        if (col >= VGA_WIDTH) {
            col = 0;
            row++;
            if (row == VGA_HEIGHT) scroll();
        }
    } else if (c == 8) {
        if (col > 0) {
            col--;
        } else if (row > 0) {
            row--;
            col = VGA_WIDTH - 1;
        }
        put_entry(' ', attr, col, row);
    } else {
        put_entry(c, attr, col, row);
        col++;
        if (col == VGA_WIDTH) {
            col = 0;
            row++;
            if (row == VGA_HEIGHT) scroll();
        }
    }

    terminal_cursor_update();
}

void terminal_write(const char* s) {
    while (*s) {
        terminal_putchar(*s++);
    }
}
