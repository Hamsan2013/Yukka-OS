#include <kernel.h>

static const char* command_names[] = {
    "help", "clear", "echo", "version", "about", "time", "date",
    "whoami", "memory", "cpu", "disk", "files", "folder", "new",
    "make", "open", "read", "write", "delete", "copy", "move",
    "rename", "run", "list", "tree", "mkdir", "pwd", "cd", "goto",
    "back", "home", "exit"
};

extern "C" int command_count(void) {
    return (int)(sizeof(command_names) / sizeof(command_names[0]));
}

extern "C" const char* command_name(int index) {
    if (index < 0 || index >= command_count()) return "";
    return command_names[index];
}

static int split_args(const char* line, char args[][64], int max) {
    int n = 0;
    const char* p = line;

    while (*p) {
        while (*p == ' ' || *p == '\t') p++;
        if (!*p) break;

        char delim = 0;
        if (*p == '"') {
            delim = '"';
            p++;
        }

        int len = 0;
        while (*p && len < 63) {
            if (delim && *p == delim) {
                p++;
                break;
            }

            if (!delim && (*p == ' ' || *p == '\t')) {
                break;
            }

            args[n][len++] = *p++;
        }

        args[n][len] = 0;
        n++;

        if (n == max) break;
    }

    return n;
}

static void join_args(char args[][64], int n, int start, char* out, size_t out_len) {
    out[0] = 0;

    for (int i = start; i < n; i++) {
        if (i > start) kstrcat(out, " ");
        kstrcat(out, args[i]);

        if (kstrlen(out) >= out_len - 1) break;
    }
}

static void print_list(bool dirs_only, bool files_only) {
    fs_stat_t entries[32];
    int count = fs_list(entries, 32);

    for (int i = 0; i < count; i++) {
        if (dirs_only && !entries[i].is_dir) continue;
        if (files_only && entries[i].is_dir) continue;

        if (entries[i].is_dir) {
            kprintf("%s/\n", entries[i].name);
        } else {
            kprintf("%s %u\n", entries[i].name, entries[i].size);
        }
    }
}

static void read_file(const char* path) {
    char buf[2049];

    if (fs_read_file(path, buf, sizeof(buf)) >= 0) {
        kprintf("%s\n", buf);
    } else {
        kprintf("Cannot read %s\n", path);
    }
}

extern "C" int command_execute(const char* line) {
    char args[8][64];
    int n = split_args(line, args, 8);

    if (n == 0) return 1;

    const char* cmd = args[0];

    if (kstrcmp(cmd, "help") == 0) {
        kprintf("Yukka OS commands:\n");
        for (int i = 0; i < command_count(); i++) {
            kprintf("%s\n", command_name(i));
        }
        kprintf("Yakka statements: let, print, if, repeat, function, end\n");
        return 1;
    }

    if (kstrcmp(cmd, "clear") == 0) {
        terminal_clear();
        return 1;
    }

    if (kstrcmp(cmd, "echo") == 0) {
        char out[256];
        join_args(args, n, 1, out, sizeof(out));
        kprintf("%s\n", out);
        return 1;
    }

    if (kstrcmp(cmd, "version") == 0) {
        kprintf("Yukka OS 1.0.0\n");
        return 1;
    }

    if (kstrcmp(cmd, "about") == 0) {
        kprintf("Yukka OS is a terminal-only operating system.\n");
        kprintf("It includes the Yakka terminal language.\n");
        return 1;
    }

    if (kstrcmp(cmd, "time") == 0) {
        char buf[16];
        time_string(buf, sizeof(buf));
        kprintf("%s\n", buf);
        return 1;
    }

    if (kstrcmp(cmd, "date") == 0) {
        char buf[24];
        date_string(buf, sizeof(buf));
        kprintf("%s\n", buf);
        return 1;
    }

    if (kstrcmp(cmd, "whoami") == 0) {
        kprintf("root\n");
        return 1;
    }

    if (kstrcmp(cmd, "memory") == 0) {
        kprintf("Memory: %u KiB used, %u KiB total\n",
                (unsigned int)(pmm_used_bytes() / 1024),
                (unsigned int)(pmm_total_bytes() / 1024));
        return 1;
    }

    if (kstrcmp(cmd, "cpu") == 0) {
        kprintf("Yukka x86_64 terminal CPU\n");
        return 1;
    }

    if (kstrcmp(cmd, "disk") == 0) {
        kprintf("Disk: %u bytes, %u files, %u folders\n",
                fs_used_bytes(), fs_count_files(), fs_count_dirs());
        return 1;
    }

    if (kstrcmp(cmd, "files") == 0) {
        print_list(false, true);
        return 1;
    }

    if (kstrcmp(cmd, "folder") == 0) {
        if (n > 1) {
            if (fs_make_dir(args[1]) == 0) {
                kprintf("Folder created\n");
            } else {
                kprintf("Cannot create folder\n");
            }
        } else {
            print_list(true, false);
        }
        return 1;
    }

    if (kstrcmp(cmd, "new") == 0 || kstrcmp(cmd, "make") == 0) {
        if (n > 1) {
            if (fs_make_file(args[1]) == 0) {
                kprintf("File created\n");
            } else {
                kprintf("Cannot create file\n");
            }
        } else {
            kprintf("Missing file name\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "open") == 0 || kstrcmp(cmd, "read") == 0) {
        if (n > 1) {
            read_file(args[1]);
        } else {
            kprintf("Missing file name\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "write") == 0) {
        if (n >= 3) {
            char content[2048];
            join_args(args, n, 2, content, sizeof(content));
            if (fs_write_file(args[1], content) >= 0) {
                kprintf("Written\n");
            } else {
                kprintf("Cannot write\n");
            }
        } else if (n == 2) {
            fs_write_file(args[1], "");
            kprintf("Written\n");
        } else {
            kprintf("Usage: write file \"text\"\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "delete") == 0) {
        if (n > 1) {
            if (fs_delete(args[1]) == 0) {
                kprintf("Deleted\n");
            } else {
                kprintf("Cannot delete\n");
            }
        } else {
            kprintf("Missing name\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "copy") == 0) {
        if (n > 2) {
            if (fs_copy(args[1], args[2]) == 0) {
                kprintf("Copied\n");
            } else {
                kprintf("Cannot copy\n");
            }
        } else {
            kprintf("Usage: copy source target\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "move") == 0) {
        if (n > 2) {
            if (fs_move(args[1], args[2]) == 0) {
                kprintf("Moved\n");
            } else {
                kprintf("Cannot move\n");
            }
        } else {
            kprintf("Usage: move source target\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "rename") == 0) {
        if (n > 2) {
            if (fs_rename(args[1], args[2]) == 0) {
                kprintf("Renamed\n");
            } else {
                kprintf("Cannot rename\n");
            }
        } else {
            kprintf("Usage: rename old new\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "run") == 0) {
        if (n > 1) {
            char buf[2049];
            if (fs_read_file(args[1], buf, sizeof(buf)) >= 0) {
                yakka_run_text(buf);
            } else {
                kprintf("Cannot run %s\n", args[1]);
            }
        } else {
            kprintf("Missing file name\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "list") == 0) {
        print_list(false, false);
        return 1;
    }

    if (kstrcmp(cmd, "tree") == 0) {
        char buf[1024];
        fs_tree(buf, sizeof(buf));
        kprintf("%s", buf);
        return 1;
    }

    if (kstrcmp(cmd, "mkdir") == 0) {
        if (n > 1) {
            if (fs_make_dir(args[1]) == 0) {
                kprintf("Folder created\n");
            } else {
                kprintf("Cannot create folder\n");
            }
        } else {
            kprintf("Missing folder name\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "pwd") == 0) {
        char buf[128];
        fs_cwd(buf, sizeof(buf));
        kprintf("%s\n", buf);
        return 1;
    }

    if (kstrcmp(cmd, "cd") == 0 || kstrcmp(cmd, "goto") == 0) {
        if (n > 1) {
            if (fs_chdir(args[1]) != 0) {
                kprintf("Cannot change folder\n");
            }
        } else {
            kprintf("Missing folder\n");
        }
        return 1;
    }

    if (kstrcmp(cmd, "back") == 0) {
        fs_chdir("..");
        return 1;
    }

    if (kstrcmp(cmd, "home") == 0) {
        fs_chdir("/");
        return 1;
    }

    if (kstrcmp(cmd, "exit") == 0) {
        return 2;
    }

    return 0;
}
