#include <kernel.h>

static uint8_t cmos_read(uint8_t reg) {
    outb(0x70, (uint8_t)(0x80 | reg));
    return inb(0x71);
}

static uint8_t bcd_to_bin(uint8_t value) {
    return (uint8_t)((value >> 4) * 10 + (value & 0x0F));
}

static void append_two(char* buf, int value) {
    char tmp[3];
    tmp[0] = (char)('0' + value / 10);
    tmp[1] = (char)('0' + value % 10);
    tmp[2] = 0;
    kstrcat(buf, tmp);
}

void time_init(void) {
}

void time_string(char* buf, size_t len) {
    (void)len;
    buf[0] = 0;

    append_two(buf, bcd_to_bin(cmos_read(0x04)));
    kstrcat(buf, ":");
    append_two(buf, bcd_to_bin(cmos_read(0x02)));
    kstrcat(buf, ":");
    append_two(buf, bcd_to_bin(cmos_read(0x00)));
}

void date_string(char* buf, size_t len) {
    (void)len;
    buf[0] = 0;

    int century = bcd_to_bin(cmos_read(0x32));
    int year = bcd_to_bin(cmos_read(0x09));
    int full_year = century ? century * 100 + year : 2000 + year;

    char tmp[8];
    kitoa(full_year, tmp, 10);
    kstrcat(buf, tmp);
    kstrcat(buf, "-");
    append_two(buf, bcd_to_bin(cmos_read(0x08)));
    kstrcat(buf, "-");
    append_two(buf, bcd_to_bin(cmos_read(0x07)));
}

uint64_t time_unix(void) {
    uint64_t s = bcd_to_bin(cmos_read(0x00));
    uint64_t m = bcd_to_bin(cmos_read(0x02));
    uint64_t h = bcd_to_bin(cmos_read(0x04));
    uint64_t d = bcd_to_bin(cmos_read(0x07));

    return s + m * 60 + h * 3600 + d * 86400;
}
