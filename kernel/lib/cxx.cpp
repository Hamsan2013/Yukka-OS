#include <kernel.h>

extern "C" {

void* __dso_handle = nullptr;

int __cxa_atexit(void (*func)(void*), void* arg, void* dso) {
    (void)func;
    (void)arg;
    (void)dso;
    return 0;
}

void __cxa_pure_virtual() {
    panic("pure virtual called");
}

void abort() {
    panic("abort");
}

void _exit(int status) {
    (void)status;
    power_shutdown();
}

}

void* operator new(size_t size) {
    return kmalloc(size);
}

void* operator new[](size_t size) {
    return kmalloc(size);
}

void operator delete(void* ptr) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr) noexcept {
    kfree(ptr);
}

void operator delete(void* ptr, size_t) noexcept {
    kfree(ptr);
}

void operator delete[](void* ptr, size_t) noexcept {
    kfree(ptr);
}
