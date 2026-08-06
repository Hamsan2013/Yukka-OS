#include <kernel.h>
#include "yakka_lang.hpp"

namespace yakka {

static void trim_line(const char* src, char* dst, size_t len) {
    while (*src == ' ' || *src == '\t' || *src == '\r') src++;

    size_t n = kstrlen(src);
    while (n > 0 && (src[n - 1] == ' ' || src[n - 1] == '\t' || src[n - 1] == '\r')) {
        n--;
    }

    if (n >= len) n = len - 1;

    kmemcpy(dst, src, n);
    dst[n] = 0;
}

static bool starts_word(const char* line, const char* word) {
    size_t len = kstrlen(word);

    if (kstrncmp(line, word, len) != 0) {
        return false;
    }

    char c = line[len];
    return c == 0 || c == ' ' || c == '\t' || c == '(';
}

static const char* after_word(const char* line, const char* word) {
    line += kstrlen(word);

    while (*line == ' ' || *line == '\t') {
        line++;
    }

    return line;
}

static bool is_block_start(const char* line) {
    return starts_word(line, "if") ||
           starts_word(line, "repeat") ||
           starts_word(line, "function");
}

static int find_end(const char* lines[], int count, int start) {
    int depth = 1;

    for (int i = start; i < count; i++) {
        char line[128];
        trim_line(lines[i], line, sizeof(line));

        if (is_block_start(line)) {
            depth++;
        } else if (starts_word(line, "end")) {
            depth--;
            if (depth == 0) {
                return i;
            }
        }
    }

    return count;
}

static void strip_then(char* line) {
    size_t len = kstrlen(line);

    if (len >= 4 && kstrcmp(line + len - 4, "then") == 0) {
        line[len - 4] = 0;
    }
}

int execute_program(const char* lines[], int count) {
    for (int i = 0; i < count; i++) {
        char line[128];
        trim_line(lines[i], line, sizeof(line));

        if (!line[0] || line[0] == '#') {
            continue;
        }

        if (starts_word(line, "let")) {
            const char* rest = after_word(line, "let");

            char name[64];
            int n = 0;

            while (*rest && *rest != '=' && *rest != ' ' && n < 63) {
                name[n++] = *rest++;
            }

            name[n] = 0;

            while (*rest == ' ' || *rest == '=') {
                rest++;
            }

            Parser parser(rest);
            Value value = parser.parse_expression();
            runtime_set_variable(name, value);
            continue;
        }

        if (starts_word(line, "print")) {
            const char* rest = after_word(line, "print");

            if (!*rest) {
                Value empty;
                runtime_print_value(empty);
                continue;
            }

            Parser parser(rest);
            Value value = parser.parse_expression();
            runtime_print_value(value);
            continue;
        }

        if (starts_word(line, "if")) {
            char condition[128];
            trim_line(after_word(line, "if"), condition, sizeof(condition));
            strip_then(condition);

            int end = find_end(lines, count, i + 1);

            Parser parser(condition);
            Value value = parser.parse_expression();

            if (value_truthy(value)) {
                execute_program(lines + i + 1, end - i - 1);
            }

            i = end;
            continue;
        }

        if (starts_word(line, "repeat")) {
            const char* rest = after_word(line, "repeat");

            Parser parser(rest);
            Value value = parser.parse_expression();

            long times = value.num;
            if (times > 10000) times = 10000;
            if (times < 0) times = 0;

            int end = find_end(lines, count, i + 1);

            for (long r = 0; r < times; r++) {
                execute_program(lines + i + 1, end - i - 1);
            }

            i = end;
            continue;
        }

        if (starts_word(line, "function")) {
            const char* rest = after_word(line, "function");

            char name[64];
            int n = 0;

            while (*rest && *rest != '(' && *rest != ' ' && n < 63) {
                name[n++] = *rest++;
            }

            name[n] = 0;

            int end = find_end(lines, count, i + 1);
            runtime_define_function(name, lines + i + 1, end - i - 1);
            i = end;
            continue;
        }

        if (starts_word(line, "end")) {
            continue;
        }

        Parser parser(line);
        parser.parse_expression();
    }

    return 0;
}

struct Capture {
    bool active;
    int kind;
    bool condition;
    long repeat;
    char name[64];
    char lines[64][128];
    int count;
    int depth;
};

static Capture capture;

static void finish_capture() {
    capture.active = false;

    const char* program[64];
    for (int i = 0; i < capture.count; i++) {
        program[i] = capture.lines[i];
    }

    if (capture.kind == 1) {
        if (capture.condition) {
            execute_program(program, capture.count);
        }
    } else if (capture.kind == 2) {
        for (long i = 0; i < capture.repeat; i++) {
            execute_program(program, capture.count);
        }
    } else if (capture.kind == 3) {
        runtime_define_function(capture.name, program, capture.count);
    }
}

static int feed_one_line(const char* raw_line) {
    char line[128];
    trim_line(raw_line, line, sizeof(line));

    if (!line[0]) {
        return 0;
    }

    if (capture.active) {
        if (starts_word(line, "end")) {
            if (capture.depth == 0) {
                finish_capture();
            } else {
                capture.depth--;
            }
            return 0;
        }

        if (is_block_start(line)) {
            capture.depth++;
        }

        if (capture.count < 64) {
            kstrncpy(capture.lines[capture.count++], line, 127);
            capture.lines[capture.count - 1][127] = 0;
        }

        return 0;
    }

    if (starts_word(line, "if")) {
        char condition[128];
        trim_line(after_word(line, "if"), condition, sizeof(condition));
        strip_then(condition);

        Parser parser(condition);
        Value value = parser.parse_expression();

        capture.active = true;
        capture.kind = 1;
        capture.condition = value_truthy(value);
        capture.count = 0;
        capture.depth = 0;
        capture.name[0] = 0;
        return 0;
    }

    if (starts_word(line, "repeat")) {
        Parser parser(after_word(line, "repeat"));
        Value value = parser.parse_expression();

        long times = value.num;
        if (times > 10000) times = 10000;
        if (times < 0) times = 0;

        capture.active = true;
        capture.kind = 2;
        capture.condition = false;
        capture.repeat = times;
        capture.count = 0;
        capture.depth = 0;
        capture.name[0] = 0;
        return 0;
    }

    if (starts_word(line, "function")) {
        const char* rest = after_word(line, "function");

        capture.active = true;
        capture.kind = 3;
        capture.condition = false;
        capture.repeat = 0;
        capture.count = 0;
        capture.depth = 0;

        int n = 0;
        while (*rest && *rest != '(' && *rest != ' ' && n < 63) {
            capture.name[n++] = *rest++;
        }
        capture.name[n] = 0;

        return 0;
    }

    const char* one[1] = {line};
    return execute_program(one, 1);
}

static int run_text_internal(const char* text) {
    static char buffer[8192];
    static const char* lines[256];

    kstrncpy(buffer, text, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = 0;

    int count = 0;
    char* p = buffer;

    while (*p && count < 256) {
        lines[count++] = p;

        while (*p && *p != '\n') {
            p++;
        }

        if (*p) {
            *p = 0;
            p++;
        }
    }

    return execute_program(lines, count);
}

}

extern "C" void yakka_init(void) {
    yakka::runtime_reset();
    yakka::capture.active = false;
    yakka::capture.count = 0;
    yakka::capture.depth = 0;
}

extern "C" int yakka_feed_line(const char* line) {
    return yakka::feed_one_line(line);
}

extern "C" int yakka_run_text(const char* text) {
    return yakka::run_text_internal(text);
}

extern "C" int yakka_self_test(void) {
    const char* program =
        "let x = 2 + 3\n"
        "print x\n"
        "if x > 4\n"
        "print ok\n"
        "end\n"
        "function hello()\n"
        "print fn\n"
        "end\n"
        "hello()\n";

    yakka::runtime_start_capture();
    yakka::run_text_internal(program);
    yakka::runtime_stop_capture();

    const char* output = yakka::runtime_captured();

    if (!kstrstr(output, "5")) return 0;
    if (!kstrstr(output, "ok")) return 0;
    if (!kstrstr(output, "fn")) return 0;

    return 1;
}
