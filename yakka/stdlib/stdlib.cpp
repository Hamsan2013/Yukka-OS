#include <kernel.h>
#include "yakka_lang.hpp"

namespace yakka {

static uint64_t random_state = 0x123456789ABCDEFULL;

static long random_long(long mod) {
    random_state = random_state * 6364136223846793005ULL + 1442695040888963407ULL;
    random_state ^= pit_ticks();

    if (mod <= 0) mod = 100;
    return (long)(random_state % (uint64_t)mod);
}

static void string_transform(Value& value, bool upper) {
    char buffer[128];
    value_to_string(value, buffer, sizeof(buffer));

    for (int i = 0; buffer[i]; i++) {
        if (upper && buffer[i] >= 'a' && buffer[i] <= 'z') {
            buffer[i] = (char)(buffer[i] - 'a' + 'A');
        }

        if (!upper && buffer[i] >= 'A' && buffer[i] <= 'Z') {
            buffer[i] = (char)(buffer[i] - 'A' + 'a');
        }
    }

    value_set_string(value, buffer);
}

Value call_builtin(const char* name, Value* args, int argc) {
    Value result;
    value_set_number(result, 0);

    if (kstrcmp(name, "print") == 0) {
        if (argc > 0) {
            runtime_print_value(args[0]);
        } else {
            Value empty;
            runtime_print_value(empty);
        }
        return result;
    }

    if (kstrcmp(name, "input") == 0) {
        char buffer[128];
        kprintf("? ");
        terminal_getline(buffer, sizeof(buffer));
        value_set_string(result, buffer);
        return result;
    }

    if (kstrcmp(name, "read") == 0) {
        char buffer[2049];

        if (argc > 0) {
            char path[128];
            value_to_string(args[0], path, sizeof(path));

            if (fs_read_file(path, buffer, sizeof(buffer)) >= 0) {
                value_set_string(result, buffer);
            } else {
                value_set_string(result, "");
            }
        } else {
            value_set_string(result, "");
        }

        return result;
    }

    if (kstrcmp(name, "write") == 0) {
        if (argc >= 2) {
            char path[128];
            char data[2048];

            value_to_string(args[0], path, sizeof(path));
            value_to_string(args[1], data, sizeof(data));

            value_set_number(result, fs_write_file(path, data) >= 0 ? 1 : 0);
        }

        return result;
    }

    if (kstrcmp(name, "delete") == 0) {
        if (argc > 0) {
            char path[128];
            value_to_string(args[0], path, sizeof(path));
            value_set_number(result, fs_delete(path) == 0 ? 1 : 0);
        }

        return result;
    }

    if (kstrcmp(name, "random") == 0) {
        long mod = argc > 0 ? args[0].num : 100;
        value_set_number(result, random_long(mod));
        return result;
    }

    if (kstrcmp(name, "time") == 0) {
        value_set_number(result, (long)time_unix());
        return result;
    }

    if (kstrcmp(name, "length") == 0) {
        if (argc > 0) {
            char buffer[128];
            value_to_string(args[0], buffer, sizeof(buffer));
            value_set_number(result, (long)kstrlen(buffer));
        } else {
            value_set_number(result, 0);
        }

        return result;
    }

    if (kstrcmp(name, "upper") == 0) {
        if (argc > 0) {
            result = args[0];
            string_transform(result, true);
        }
        return result;
    }

    if (kstrcmp(name, "lower") == 0) {
        if (argc > 0) {
            result = args[0];
            string_transform(result, false);
        }
        return result;
    }

    if (!runtime_call_function(name)) {
        kprintf("Unknown function %s\n", name);
    }

    return result;
}

}
