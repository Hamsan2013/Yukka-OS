#include <kernel.h>

static char history[16][128];
static int history_count = 0;

static void history_add(const char* line) {
    if (!line[0]) return;

    if (history_count > 0 && kstrcmp(history[history_count - 1], line) == 0) {
        return;
    }

    if (history_count < 16) {
        kstrcpy(history[history_count++], line);
        return;
    }

    for (int i = 1; i < 16; i++) {
        kstrcpy(history[i - 1], history[i]);
    }

    kstrcpy(history[15], line);
}

static void redraw_line(const char* buf, size_t old_len) {
    terminal_putchar('\r');

    for (size_t i = 0; i < old_len; i++) {
        terminal_putchar(' ');
    }

    terminal_putchar('\r');
    terminal_write(buf);
}

static void autocomplete(char* buf, size_t& len, size_t size) {
    if (len == 0 || len + 2 >= size) return;

    size_t start = len;
    for (size_t i = len; i > 0; i--) {
        if (buf[i - 1] == ' ') {
            start = i;
            break;
        }
    }

    size_t token_len = len - start;
    size_t old_len = len;

    if (start == 0) {
        int count = command_count();

        for (int i = 0; i < count; i++) {
            const char* name = command_name(i);

            if (kstrncmp(name, buf, token_len) == 0) {
                kstrcpy(buf, name);
                len = kstrlen(name);
                buf[len++] = ' ';
                buf[len] = 0;
                redraw_line(buf, old_len);
                return;
            }
        }
    } else {
        fs_stat_t entries[32];
        int count = fs_list(entries, 32);

        for (int i = 0; i < count; i++) {
            if (kstrncmp(entries[i].name, buf + start, token_len) == 0) {
                kstrcpy(buf + start, entries[i].name);
                len = start + kstrlen(entries[i].name);
                buf[len++] = ' ';
                buf[len] = 0;
                redraw_line(buf, old_len);
                return;
            }
        }
    }
}

extern "C" int terminal_getline(char* buf, size_t size) {
    size_t len = 0;
    int hist_index = history_count;

    while (true) {
        int c = keyboard_poll();

        if (c < 0) {
            __asm__ volatile("hlt");
            continue;
        }

        if (c == KEY_ENTER) {
            terminal_putchar('\n');
            buf[len] = 0;
            return (int)len;
        }

        if (c == KEY_BACKSPACE) {
            if (len > 0) {
                len--;
                buf[len] = 0;
                terminal_putchar(KEY_BACKSPACE);
                terminal_putchar(' ');
                terminal_putchar(KEY_BACKSPACE);
            }
            continue;
        }

        if (c == KEY_TAB) {
            autocomplete(buf, len, size);
            continue;
        }

        if (c == KEY_UP) {
            if (history_count > 0 && hist_index > 0) {
                hist_index--;
                size_t old_len = len;
                kstrcpy(buf, history[hist_index]);
                len = kstrlen(buf);
                redraw_line(buf, old_len);
            }
            continue;
        }

        if (c == KEY_DOWN) {
            size_t old_len = len;

            if (hist_index < history_count - 1) {
                hist_index++;
                kstrcpy(buf, history[hist_index]);
            } else {
                hist_index = history_count;
                buf[0] = 0;
            }

            len = kstrlen(buf);
            redraw_line(buf, old_len);
            continue;
        }

        if (c >= 32 && c < 127 && len + 1 < size) {
            buf[len++] = (char)c;
            buf[len] = 0;
            terminal_putchar((char)c);
        }
    }
}

extern "C" void shell_run(void) {
    interrupts_enable();

    char line[128];

    while (true) {
        kprintf("> ");
        terminal_getline(line, sizeof(line));

        if (!line[0]) {
            continue;
        }

        history_add(line);

        int result = command_execute(line);

        if (result == 2) {
            kprintf("Goodbye\n");
            power_shutdown();
        }

        if (result == 0) {
            yakka_feed_line(line);
        }
    }
}
