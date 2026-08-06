#include <kernel.h>
#include "yakka_lang.hpp"

namespace yakka {

constexpr int MAX_VARS = 64;
constexpr int MAX_FUNCS = 16;
constexpr int MAX_FUNC_LINES = 32;

struct Variable {
    bool used;
    char name[64];
    Value value;
};

struct Function {
    bool used;
    char name[64];
    int lines;
    char body[MAX_FUNC_LINES][128];
};

static Variable vars[MAX_VARS];
static Function funcs[MAX_FUNCS];

static bool capturing = false;
static char capture_buffer[512];

void runtime_reset() {
    kmemset(vars, 0, sizeof(vars));
    kmemset(funcs, 0, sizeof(funcs));
    capturing = false;
    capture_buffer[0] = 0;
}

void runtime_start_capture() {
    capturing = true;
    capture_buffer[0] = 0;
}

void runtime_stop_capture() {
    capturing = false;
}

const char* runtime_captured() {
    return capture_buffer;
}

void runtime_print_value(const Value& value) {
    char buffer[128];
    value_to_string(value, buffer, sizeof(buffer));

    if (capturing) {
        kstrcat(capture_buffer, buffer);
        kstrcat(capture_buffer, "\n");
        return;
    }

    kprintf("%s\n", buffer);
}

bool runtime_get_variable(const char* name, Value& out) {
    for (int i = 0; i < MAX_VARS; i++) {
        if (vars[i].used && kstrcmp(vars[i].name, name) == 0) {
            out = vars[i].value;
            return true;
        }
    }

    return false;
}

void runtime_set_variable(const char* name, const Value& value) {
    for (int i = 0; i < MAX_VARS; i++) {
        if (vars[i].used && kstrcmp(vars[i].name, name) == 0) {
            vars[i].value = value;
            return;
        }
    }

    for (int i = 0; i < MAX_VARS; i++) {
        if (!vars[i].used) {
            vars[i].used = true;
            kstrncpy(vars[i].name, name, 63);
            vars[i].name[63] = 0;
            vars[i].value = value;
            return;
        }
    }
}

void runtime_define_function(const char* name, const char* lines[], int count) {
    Function* target = nullptr;

    for (int i = 0; i < MAX_FUNCS; i++) {
        if (funcs[i].used && kstrcmp(funcs[i].name, name) == 0) {
            target = &funcs[i];
            break;
        }
    }

    if (!target) {
        for (int i = 0; i < MAX_FUNCS; i++) {
            if (!funcs[i].used) {
                target = &funcs[i];
                break;
            }
        }
    }

    if (!target) return;

    target->used = true;
    kstrncpy(target->name, name, 63);
    target->name[63] = 0;

    target->lines = 0;

    for (int i = 0; i < count && i < MAX_FUNC_LINES; i++) {
        kstrncpy(target->body[i], lines[i], 127);
        target->body[i][127] = 0;
        target->lines++;
    }
}

bool runtime_call_function(const char* name) {
    for (int i = 0; i < MAX_FUNCS; i++) {
        if (funcs[i].used && kstrcmp(funcs[i].name, name) == 0) {
            const char* program[MAX_FUNC_LINES];

            for (int j = 0; j < funcs[i].lines; j++) {
                program[j] = funcs[i].body[j];
            }

            execute_program(program, funcs[i].lines);
            return true;
        }
    }

    return false;
}

}
