#include <kernel.h>

typedef struct {
    int pid;
    char name[32];
    bool running;
} process_t;

static process_t processes[16];
static int next_pid = 1;

int process_create(const char* name) {
    for (int i = 0; i < 16; i++) {
        if (!processes[i].running) {
            processes[i].pid = next_pid++;
            kstrncpy(processes[i].name, name, 31);
            processes[i].name[31] = 0;
            processes[i].running = true;
            return processes[i].pid;
        }
    }

    return -1;
}
