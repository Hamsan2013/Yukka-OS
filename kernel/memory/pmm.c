#include <kernel.h>

extern char _kernel_end[];

static uint8_t* bitmap = 0;
static uint64_t total_pages = 0;
static uint64_t used_pages = 0;

static void set_page(uint64_t page) {
    bitmap[page / 8] |= (uint8_t)(1u << (page % 8));
}

static void clear_page(uint64_t page) {
    bitmap[page / 8] &= (uint8_t)~(1u << (page % 8));
}

static int page_used(uint64_t page) {
    return bitmap[page / 8] & (uint8_t)(1u << (page % 8));
}

void pmm_init(uint32_t magic, uint32_t mbi) {
    uint64_t memory = multiboot_memory(magic, mbi);

    if (memory < 64ull * 1024 * 1024) {
        memory = 64ull * 1024 * 1024;
    }

    if (memory > 1024ull * 1024 * 1024) {
        memory = 1024ull * 1024 * 1024;
    }

    total_pages = memory / 4096;
    uint64_t bitmap_size = (total_pages + 7) / 8;

    uint64_t start = ((uint64_t)(uintptr_t)_kernel_end + 0xFFFull) & ~0xFFFull;
    bitmap = (uint8_t*)(uintptr_t)start;

    kmemset(bitmap, 0, bitmap_size);

    uint64_t reserved_end = start + bitmap_size;
    uint64_t reserved_pages = (reserved_end / 4096) + 1;

    for (uint64_t i = 0; i < reserved_pages; i++) {
        set_page(i);
    }

    used_pages = reserved_pages;
}

uint64_t pmm_alloc_page(void) {
    for (uint64_t i = 0; i < total_pages; i++) {
        if (!page_used(i)) {
            set_page(i);
            used_pages++;
            return i * 4096;
        }
    }

    panic("Out of physical memory");
    return 0;
}

void pmm_free_page(uint64_t addr) {
    uint64_t page = addr / 4096;
    if (page < total_pages && page_used(page)) {
        clear_page(page);
        used_pages--;
    }
}

uint64_t pmm_total_bytes(void) {
    return total_pages * 4096;
}

uint64_t pmm_used_bytes(void) {
    return used_pages * 4096;
}

void memory_init(uint32_t magic, uint32_t mbi) {
    pmm_init(magic, mbi);
    vmm_init();
    paging_init();
    heap_init();
}
