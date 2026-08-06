#include <kernel.h>

static void out_char(char c) {
    terminal_putchar(c);
    serial_write(c);
}

static void print_ulong(unsigned long value, int base) {
    char buf[32];
    int i = 0;
    const char* digits = "0123456789abcdef";

    if (value == 0) {
        out_char('0');
        return;
    }

    while (value) {
        buf[i++] = digits[value % (unsigned long)base];
        value /= (unsigned long)base;
    }

    while (i > 0) {
        out_char(buf[--i]);
    }
}

void vkprintf(const char* fmt, va_list args) {
    while (*fmt) {
        if (*fmt != '%') {
            out_char(*fmt++);
            continue;
        }

        fmt++;
        int is_long = 0;

        if (*fmt == 'l') {
            is_long = 1;
            fmt++;
            if (*fmt == 'l') {
                fmt++;
            }
        }

        switch (*fmt) {
            case 'd':
            case 'i': {
                long value = is_long ? va_arg(args, long) : (long)va_arg(args, int);
                if (value < 0) {
                    out_char('-');
                    value = -value;
                }
                print_ulong((unsigned long)value, 10);
                break;
            }
            case 'u': {
                unsigned long value = is_long ? va_arg(args, unsigned long)
                                              : (unsigned long)va_arg(args, unsigned int);
                print_ulong(value, 10);
                break;
            }
            case 'x':
            case 'p': {
                unsigned long value = is_long ? va_arg(args, unsigned long)
                                              : (unsigned long)va_arg(args, unsigned int);
                if (*fmt == 'p') {
                    out_char('0');
                    out_char('x');
                }
                print_ulong(value, 16);
                break;
            }
            case 's': {
                const char* s = va_arg(args, const char*);
                if (!s) s = "(null)";
                while (*s) out_char(*s++);
                break;
            }
            case 'c': {
                char c = (char)va_arg(args, int);
                out_char(c);
                break;
            }
            case '%':
                out_char('%');
                break;
            default:
                out_char('%');
                out_char(*fmt);
                break;
        }

        fmt++;
    }
}

void kprintf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vkprintf(fmt, args);
    va_end(args);
}
