#ifndef KERNEL_H
#define KERNEL_H

#include <stdarg.h>
#ifndef __cplusplus
#include <stdbool.h>
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;

typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef signed long int64_t;

typedef __SIZE_TYPE__ size_t;
typedef long ssize_t;
typedef unsigned long uintptr_t;

#ifndef NULL
#define NULL ((void*)0)
#endif

#define PACKED __attribute__((packed))

#define KEY_ENTER 10
#define KEY_BACKSPACE 8
#define KEY_TAB 9
#define KEY_UP 0x100
#define KEY_DOWN 0x101

#ifdef __cplusplus
extern "C" {
#endif

static inline void outb(uint16_t port, uint8_t value) {
    __asm__ volatile("outb %0, %1" :: "a"(value), "Nd"(port));
}

static inline uint8_t inb(uint16_t port) {
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outw(uint16_t port, uint16_t value) {
    __asm__ volatile("outw %0, %1" :: "a"(value), "Nd"(port));
}

static inline uint16_t inw(uint16_t port) {
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void io_wait(void) {
    outb(0x80, 0);
}

void serial_init(void);
void serial_write(char c);
void serial_puts(const char* s);

void terminal_init(void);
void terminal_clear(void);
void terminal_putchar(char c);
void terminal_write(const char* s);
void terminal_set_color(uint8_t fg, uint8_t bg);
void terminal_cursor_update(void);
int terminal_getline(char* buf, size_t size);

void kprintf(const char* fmt, ...);
void vkprintf(const char* fmt, va_list args);
void panic(const char* fmt, ...);

size_t kstrlen(const char* s);
int kstrcmp(const char* a, const char* b);
int kstrncmp(const char* a, const char* b, size_t n);
char* kstrcpy(char* dst, const char* src);
char* kstrncpy(char* dst, const char* src, size_t n);
char* kstrcat(char* dst, const char* src);
char* kstrstr(const char* haystack, const char* needle);
void* kmemcpy(void* dst, const void* src, size_t n);
void* kmemset(void* dst, int value, size_t n);
int kmemcmp(const void* a, const void* b, size_t n);
void* kmemmove(void* dst, const void* src, size_t n);
int katoi(const char* s);
void kitoa(long value, char* buf, int base);

void memory_init(uint32_t magic, uint32_t mbi);
void pmm_init(uint32_t magic, uint32_t mbi);
uint64_t pmm_alloc_page(void);
void pmm_free_page(uint64_t addr);
uint64_t pmm_total_bytes(void);
uint64_t pmm_used_bytes(void);

void vmm_init(void);
void paging_init(void);
void heap_init(void);
void* kmalloc(size_t size);
void kfree(void* ptr);

uint64_t multiboot_memory(uint32_t magic, uint32_t mbi);

void gdt_init(void);
void idt_init(void);
void pic_init(void);
void pic_enable(void);
void pit_init(void);
void pit_tick(void);
uint64_t pit_ticks(void);
void interrupts_enable(void);
void interrupts_disable(void);

void keyboard_init(void);
void keyboard_handle_scancode(uint8_t scancode);
int keyboard_poll(void);
int scancode_decode(uint8_t code);

typedef struct {
    char name[64];
    bool is_dir;
    uint32_t size;
} fs_stat_t;

void ramdisk_init(void);
int fs_make_file(const char* path);
int fs_make_dir(const char* path);
int fs_write_file(const char* path, const char* data);
int fs_read_file(const char* path, char* buf, size_t buflen);
int fs_delete(const char* path);
int fs_list(fs_stat_t* out, int max);
int fs_exists(const char* path);
int fs_is_dir(const char* path);
int fs_copy(const char* src, const char* dst);
int fs_move(const char* src, const char* dst);
int fs_rename(const char* src, const char* dst);
int fs_cwd(char* buf, size_t len);
int fs_chdir(const char* path);
int fs_tree(char* buf, size_t len);
uint32_t fs_used_bytes(void);
int fs_count_files(void);
int fs_count_dirs(void);

void time_init(void);
void time_string(char* buf, size_t len);
void date_string(char* buf, size_t len);
uint64_t time_unix(void);

void power_shutdown(void);
void power_reboot(void);
void power_test_exit(void);

void shell_run(void);
void terminal_app_banner(void);
int command_execute(const char* line);
int command_count(void);
const char* command_name(int index);

void yakka_init(void);
int yakka_feed_line(const char* line);
int yakka_run_text(const char* text);
int yakka_self_test(void);

void kernel_self_tests(void);

int process_create(const char* name);
void scheduler_init(void);
void scheduler_yield(void);

void driver_core_init(void);
void vfs_init(void);

#ifdef __cplusplus
}
#endif

#endif
