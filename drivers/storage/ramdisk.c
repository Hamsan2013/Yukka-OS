#include <kernel.h>

#define MAX_NODES 64
#define MAX_NAME 32
#define MAX_DATA 2048

typedef struct {
    bool used;
    bool dir;
    char name[MAX_NAME];
    int parent;
    uint8_t data[MAX_DATA];
    uint32_t size;
} node_t;

static node_t nodes[MAX_NODES];
static int cwd = 0;

void ramdisk_init(void) {
    kmemset(nodes, 0, sizeof(nodes));
    nodes[0].used = true;
    nodes[0].dir = true;
    nodes[0].parent = -1;
    kstrcpy(nodes[0].name, "/");
    cwd = 0;
}

static int child_of(int parent, const char* name) {
    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && nodes[i].parent == parent && kstrcmp(nodes[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

static int resolve(const char* path) {
    if (!path || !*path) return cwd;

    int cur = cwd;
    if (path[0] == '/') cur = 0;
    if (path[0] == '/' && path[1] == 0) return 0;

    char token[MAX_NAME];
    int i = 0;

    while (path[i]) {
        while (path[i] == '/') i++;
        if (!path[i]) break;

        int start = i;
        while (path[i] && path[i] != '/') i++;

        int len = i - start;
        if (len >= MAX_NAME) return -1;

        kmemcpy(token, path + start, (size_t)len);
        token[len] = 0;

        if (kstrcmp(token, ".") == 0) continue;

        if (kstrcmp(token, "..") == 0) {
            if (nodes[cur].parent >= 0) cur = nodes[cur].parent;
            continue;
        }

        int child = child_of(cur, token);
        if (child < 0) return -1;
        cur = child;
    }

    return cur;
}

static void split_path(const char* path, char* parent, char* base) {
    int len = (int)kstrlen(path);
    int last = -1;

    for (int i = len - 1; i >= 0; i--) {
        if (path[i] == '/') {
            last = i;
            break;
        }
    }

    if (last < 0) {
        parent[0] = 0;
        kstrcpy(base, path);
        return;
    }

    if (last == 0) {
        kstrcpy(parent, path[0] == '/' ? "/" : "");
    } else {
        kmemcpy(parent, path, (size_t)last);
        parent[last] = 0;
    }

    kstrcpy(base, path + last + 1);
}

static int create_node(const char* path, bool dir) {
    if (!path || !*path) return -1;

    int existing = resolve(path);
    if (existing >= 0) {
        if (nodes[existing].dir == dir) return existing;
        return -1;
    }

    char parent_path[128];
    char base[MAX_NAME];

    split_path(path, parent_path, base);
    if (!base[0]) return -1;

    int parent = resolve(parent_path);
    if (parent < 0 || !nodes[parent].dir) return -1;

    for (int i = 0; i < MAX_NODES; i++) {
        if (!nodes[i].used) {
            nodes[i].used = true;
            nodes[i].dir = dir;
            nodes[i].parent = parent;
            nodes[i].size = 0;
            kstrncpy(nodes[i].name, base, MAX_NAME - 1);
            nodes[i].name[MAX_NAME - 1] = 0;
            return i;
        }
    }

    return -1;
}

int fs_make_file(const char* path) {
    return create_node(path, false) >= 0 ? 0 : -1;
}

int fs_make_dir(const char* path) {
    return create_node(path, true) >= 0 ? 0 : -1;
}

int fs_write_file(const char* path, const char* data) {
    int idx = resolve(path);

    if (idx < 0) {
        idx = create_node(path, false);
        if (idx < 0) return -1;
    }

    if (nodes[idx].dir) return -1;

    size_t len = kstrlen(data);
    if (len >= MAX_DATA) return -1;

    kmemcpy(nodes[idx].data, data, len);
    nodes[idx].size = (uint32_t)len;
    nodes[idx].data[len] = 0;

    return (int)len;
}

int fs_read_file(const char* path, char* buf, size_t buflen) {
    int idx = resolve(path);
    if (idx < 0 || nodes[idx].dir) return -1;

    size_t n = nodes[idx].size;
    if (n + 1 > buflen) n = buflen - 1;

    kmemcpy(buf, nodes[idx].data, n);
    buf[n] = 0;

    return (int)nodes[idx].size;
}

static void delete_node(int idx) {
    if (idx == 0) return;

    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && nodes[i].parent == idx) {
            delete_node(i);
        }
    }

    if (idx == cwd) {
        cwd = nodes[idx].parent >= 0 ? nodes[idx].parent : 0;
    }

    nodes[idx].used = false;
}

int fs_delete(const char* path) {
    int idx = resolve(path);
    if (idx <= 0) return -1;

    delete_node(idx);
    return 0;
}

int fs_list(fs_stat_t* out, int max) {
    int count = 0;

    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && nodes[i].parent == cwd) {
            if (out && count < max) {
                kstrncpy(out[count].name, nodes[i].name, 63);
                out[count].name[63] = 0;
                out[count].is_dir = nodes[i].dir;
                out[count].size = nodes[i].size;
            }
            count++;
        }
    }

    return count;
}

int fs_exists(const char* path) {
    return resolve(path) >= 0;
}

int fs_is_dir(const char* path) {
    int idx = resolve(path);
    return idx >= 0 && nodes[idx].dir;
}

int fs_copy(const char* src, const char* dst) {
    int s = resolve(src);
    if (s < 0 || nodes[s].dir) return -1;

    int d = resolve(dst);

    if (d >= 0 && nodes[d].dir) {
        char target[128];
        kstrcpy(target, dst);
        if (target[kstrlen(target) - 1] != '/') kstrcat(target, "/");
        kstrcat(target, nodes[s].name);
        d = resolve(target);
        if (d < 0) {
            d = create_node(target, false);
            if (d < 0) return -1;
        }
    } else {
        if (d < 0) {
            d = create_node(dst, false);
            if (d < 0) return -1;
        }
    }

    if (nodes[d].dir) return -1;

    kmemcpy(nodes[d].data, nodes[s].data, nodes[s].size);
    nodes[d].size = nodes[s].size;
    nodes[d].data[nodes[d].size] = 0;

    return 0;
}

int fs_move(const char* src, const char* dst) {
    int s = resolve(src);
    if (s <= 0) return -1;

    int d = resolve(dst);

    if (d >= 0 && nodes[d].dir) {
        nodes[s].parent = d;
        return 0;
    }

    char parent[128];
    char base[MAX_NAME];
    split_path(dst, parent, base);
    if (!base[0]) return -1;

    int parent_idx = resolve(parent);
    if (parent_idx < 0 || !nodes[parent_idx].dir) return -1;

    if (d >= 0 && !nodes[d].dir) {
        delete_node(d);
    }

    nodes[s].parent = parent_idx;
    kstrncpy(nodes[s].name, base, MAX_NAME - 1);
    nodes[s].name[MAX_NAME - 1] = 0;

    return 0;
}

int fs_rename(const char* src, const char* dst) {
    return fs_move(src, dst);
}

static void build_path(int idx, char* buf, size_t len) {
    if (idx == 0) return;

    build_path(nodes[idx].parent, buf, len);
    kstrcat(buf, "/");
    kstrcat(buf, nodes[idx].name);
    (void)len;
}

int fs_cwd(char* buf, size_t len) {
    buf[0] = 0;

    if (cwd == 0) {
        kstrcpy(buf, "/");
        return 0;
    }

    build_path(cwd, buf, len);
    return 0;
}

int fs_chdir(const char* path) {
    int idx = resolve(path);
    if (idx < 0 || !nodes[idx].dir) return -1;

    cwd = idx;
    return 0;
}

static void append_tree(int dir, int depth, char* buf, size_t len) {
    for (int i = 0; i < MAX_NODES; i++) {
        if (!nodes[i].used || nodes[i].parent != dir) continue;

        if (kstrlen(buf) > len - 64) return;

        for (int d = 0; d < depth; d++) kstrcat(buf, "  ");
        kstrcat(buf, nodes[i].name);
        if (nodes[i].dir) kstrcat(buf, "/");
        kstrcat(buf, "\n");

        if (nodes[i].dir) {
            append_tree(i, depth + 1, buf, len);
        }
    }
}

int fs_tree(char* buf, size_t len) {
    buf[0] = 0;
    kstrcat(buf, "/\n");
    append_tree(0, 1, buf, len);
    return 0;
}

uint32_t fs_used_bytes(void) {
    uint32_t total = 0;

    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && !nodes[i].dir) {
            total += nodes[i].size;
        }
    }

    return total;
}

int fs_count_files(void) {
    int count = 0;

    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && !nodes[i].dir) count++;
    }

    return count;
}

int fs_count_dirs(void) {
    int count = 0;

    for (int i = 0; i < MAX_NODES; i++) {
        if (nodes[i].used && nodes[i].dir && i != 0) count++;
    }

    return count;
}
