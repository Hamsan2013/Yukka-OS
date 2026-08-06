#include <kernel.h>

static int current = 0;

void scheduler_init(void) {
    current = 0;
}

void scheduler_yield(void) {
    current++;
}
