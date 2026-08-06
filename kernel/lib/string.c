#include <kernel.h>

size_t kstrlen(const char* s) {
    size_t n = 0;
    while (s[n]) n++;
    return n;
}

int kstrcmp(const char* a, const char* b) {
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (unsigned char)*a - (unsigned char)*b;
}

int kstrncmp(const char* a, const char* b, size_t n) {
    while (n && *a && *a == *b) {
        a++;
        b++;
        n--;
    }

    if (n == 0) return 0;
    return (unsigned char)*a - (unsigned char)*b;
}

char* kstrcpy(char* dst, const char* src) {
    char* r = dst;
    while ((*dst++ = *src++)) {}
    return r;
}

char* kstrncpy(char* dst, const char* src, size_t n) {
    size_t i = 0;
    while (i < n && src[i]) {
        dst[i] = src[i];
        i++;
    }
    while (i < n) {
        dst[i++] = 0;
    }
    return dst;
}

char* kstrcat(char* dst, const char* src) {
    char* r = dst + kstrlen(dst);
    while ((*r++ = *src++)) {}
    return dst;
}

char* kstrstr(const char* haystack, const char* needle) {
    if (!*needle) return (char*)haystack;

    for (; *haystack; haystack++) {
        const char* a = haystack;
        const char* b = needle;

        while (*a && *b && *a == *b) {
            a++;
            b++;
        }

        if (!*b) return (char*)haystack;
    }

    return 0;
}

void* kmemcpy(void* dst, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;

    while (n--) {
        *d++ = *s++;
    }

    return dst;
}

void* kmemset(void* dst, int value, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    while (n--) {
        *d++ = (uint8_t)value;
    }
    return dst;
}

int kmemcmp(const void* a, const void* b, size_t n) {
    const uint8_t* x = (const uint8_t*)a;
    const uint8_t* y = (const uint8_t*)b;

    while (n--) {
        if (*x != *y) return *x - *y;
        x++;
        y++;
    }

    return 0;
}

void* kmemmove(void* dst, const void* src, size_t n) {
    uint8_t* d = (uint8_t*)dst;
    const uint8_t* s = (const uint8_t*)src;

    if (d < s) {
        while (n--) *d++ = *s++;
    } else {
        while (n--) d[n] = s[n];
    }

    return dst;
}

int katoi(const char* s) {
    int sign = 1;
    int value = 0;

    while (*s == ' ') s++;
    if (*s == '-') {
        sign = -1;
        s++;
    }

    while (*s >= '0' && *s <= '9') {
        value = value * 10 + (*s - '0');
        s++;
    }

    return sign * value;
}

void kitoa(long value, char* buf, int base) {
    char tmp[32];
    const char* digits = "0123456789abcdef";
    int i = 0;

    if (base == 10 && value < 0) {
        *buf++ = '-';
        value = -value;
    }

    unsigned long v = (unsigned long)value;

    do {
        tmp[i++] = digits[v % (unsigned long)base];
        v /= (unsigned long)base;
    } while (v);

    while (i > 0) {
        *buf++ = tmp[--i];
    }

    *buf = 0;
}

void* memcpy(void* dst, const void* src, size_t n) {
    return kmemcpy(dst, src, n);
}

void* memset(void* dst, int value, size_t n) {
    return kmemset(dst, value, n);
}

int memcmp(const void* a, const void* b, size_t n) {
    return kmemcmp(a, b, n);
}

void* memmove(void* dst, const void* src, size_t n) {
    return kmemmove(dst, src, n);
}

size_t strlen(const char* s) {
    return kstrlen(s);
}

int strcmp(const char* a, const char* b) {
    return kstrcmp(a, b);
}
