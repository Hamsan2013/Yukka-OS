#include <kernel.h>

#define MB1_MAGIC 0x2BADB002u
#define MB2_MAGIC 0x36d76289u

typedef struct {
    uint32_t flags;
    uint32_t mem_lower;
    uint32_t mem_upper;
    uint32_t boot_device;
    uint32_t cmdline;
    uint32_t mods_count;
    uint32_t mods_addr;
    uint32_t syms[4];
    uint32_t mmap_length;
    uint32_t mmap_addr;
} PACKED mb1_info_t;

typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} PACKED mb1_mmap_t;

typedef struct {
    uint32_t type;
    uint32_t size;
} PACKED mb2_tag_t;

typedef struct {
    uint32_t type;
    uint32_t size;
    uint32_t entry_size;
    uint32_t entry_version;
} PACKED mb2_mmap_t;

typedef struct {
    uint64_t addr;
    uint64_t len;
    uint32_t type;
    uint32_t zero;
} PACKED mb2_mmap_entry_t;

uint64_t multiboot_memory(uint32_t magic, uint32_t mbi) {
    uint64_t max = 0;

    if (!mbi) {
        return 0;
    }

    if (magic == MB1_MAGIC) {
        mb1_info_t* info = (mb1_info_t*)(uintptr_t)mbi;

        if (info->flags & (1u << 6)) {
            uint32_t addr = info->mmap_addr;
            uint32_t end = addr + info->mmap_length;

            while (addr < end) {
                mb1_mmap_t* entry = (mb1_mmap_t*)(uintptr_t)addr;

                if (entry->type == 1) {
                    uint64_t top = entry->addr + entry->len;
                    if (top > max) max = top;
                }

                addr += entry->size + sizeof(entry->size);
            }
        }
    } else if (magic == MB2_MAGIC) {
        uint32_t total = *(uint32_t*)(uintptr_t)mbi;
        uint32_t addr = mbi + 8;
        uint32_t end = mbi + total;

        while (addr < end) {
            mb2_tag_t* tag = (mb2_tag_t*)(uintptr_t)addr;

            if (tag->type == 0) {
                break;
            }

            if (tag->type == 6) {
                mb2_mmap_t* mmap = (mb2_mmap_t*)(uintptr_t)addr;
                uint32_t entry_addr = addr + 16;
                uint32_t tag_end = addr + tag->size;

                while (entry_addr < tag_end) {
                    mb2_mmap_entry_t* entry = (mb2_mmap_entry_t*)(uintptr_t)entry_addr;

                    if (entry->type == 1) {
                        uint64_t top = entry->addr + entry->len;
                        if (top > max) max = top;
                    }

                    entry_addr += mmap->entry_size;
                }
            }

            addr = (addr + tag->size + 7u) & ~7u;
        }
    }

    return max;
}
