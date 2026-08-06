#include <kernel.h>

#define HEAP_SIZE (256 * 1024)

typedef struct {
    size_t size;
    int free;
} header_t;

static uint8_t heap[HEAP_SIZE] __attribute__((aligned(16)));

static header_t* next_block(header_t* h) {
    return (header_t*)((uint8_t*)h + sizeof(header_t) + h->size);
}

void heap_init(void) {
    header_t* h = (header_t*)heap;
    h->size = HEAP_SIZE - sizeof(header_t);
    h->free = 1;
}

void* kmalloc(size_t size) {
    if (size == 0) return 0;

    size = (size + 15) & ~(size_t)15;

    for (header_t* h = (header_t*)heap;
         (uint8_t*)h < heap + HEAP_SIZE;
         h = next_block(h)) {
        if (h->free && h->size >= size) {
            if (h->size >= size + sizeof(header_t) + 16) {
                header_t* n = (header_t*)((uint8_t*)h + sizeof(header_t) + size);
                n->size = h->size - size - sizeof(header_t);
                n->free = 1;
                h->size = size;
            }

            h->free = 0;
            return (uint8_t*)h + sizeof(header_t);
        }
    }

    return 0;
}

void kfree(void* ptr) {
    if (!ptr) return;

    header_t* h = (header_t*)((uint8_t*)ptr - sizeof(header_t));
    h->free = 1;

    header_t* n = next_block(h);
    while ((uint8_t*)n < heap + HEAP_SIZE && n->free) {
        h->size += sizeof(header_t) + n->size;
        n = next_block(h);
    }
}
