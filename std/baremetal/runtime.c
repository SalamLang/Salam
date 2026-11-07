/*
 * Salam freestanding runtime (bare metal).
 *
 * Linked automatically by `salam build --target=*-none` and by the
 * salamos kernel triples.  It provides the small libc surface the code
 * generator uses, without libc, plus a serial console, a VGA text console
 * and a bump allocator.
 *
 * A kernel may replace any of the weak salam_rt_* hooks with its own code.
 */

#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>

#ifndef SALAM_RT_HEAP_SIZE
#  define SALAM_RT_HEAP_SIZE (16u * 1024u * 1024u)
#endif

#ifndef SALAM_RT_SERIAL_PORT
#  define SALAM_RT_SERIAL_PORT 0x3F8
#endif

#ifndef SALAM_RT_DEBUGCON_PORT
#  define SALAM_RT_DEBUGCON_PORT 0xE9
#endif

#define SALAM_WEAK __attribute__((weak))

#if defined(__i386__) || defined(__x86_64__)
#  define SALAM_RT_X86 1
#else
#  define SALAM_RT_X86 0
#endif

#if defined(__aarch64__)
#  define SALAM_RT_ARM64 1
#else
#  define SALAM_RT_ARM64 0
#endif

static inline void rt_outb(uint16_t port, uint8_t value)
{
#if SALAM_RT_X86
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
#else
    (void)port;
    (void)value;
#endif
}

static inline uint8_t rt_inb(uint16_t port)
{
#if SALAM_RT_X86
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
#else
    (void)port;
    return 0;
#endif
}

static void rt_halt_forever(void)
{
    for (;;) {
#if SALAM_RT_X86
        __asm__ volatile("cli; hlt");
#elif SALAM_RT_ARM64
        __asm__ volatile("wfi");
#endif
    }
}

void salam_rt_halt(void)
{
    rt_halt_forever();
}

SALAM_WEAK void salam_rt_exit(int code)
{
    (void)code;
    rt_halt_forever();
}

SALAM_WEAK void salam_rt_outb(uint16_t port, uint8_t value)
{
    rt_outb(port, value);
}

SALAM_WEAK uint8_t salam_rt_inb(uint16_t port)
{
    return rt_inb(port);
}

#if SALAM_RT_ARM64
#  define SALAM_RT_PL011_BASE 0x09000000u
static void arm64_serial_putc(char c)
{
    volatile uint32_t *dr = (volatile uint32_t *)(uintptr_t)(SALAM_RT_PL011_BASE + 0x00);
    volatile uint32_t *fr = (volatile uint32_t *)(uintptr_t)(SALAM_RT_PL011_BASE + 0x18);
    while ((*fr & (1u << 5)) != 0) {
    }
    *dr = (uint32_t)(unsigned char)c;
}
#endif

static int serial_ready = 0;

static void serial_init(void)
{
    if (serial_ready) {
        return;
    }
    serial_ready = 1;
#if SALAM_RT_X86
    rt_outb(SALAM_RT_SERIAL_PORT + 1, 0x00);
    rt_outb(SALAM_RT_SERIAL_PORT + 3, 0x80);
    rt_outb(SALAM_RT_SERIAL_PORT + 0, 0x03);
    rt_outb(SALAM_RT_SERIAL_PORT + 1, 0x00);
    rt_outb(SALAM_RT_SERIAL_PORT + 3, 0x03);
    rt_outb(SALAM_RT_SERIAL_PORT + 2, 0xC7);
    rt_outb(SALAM_RT_SERIAL_PORT + 4, 0x0B);
#endif
}

static void serial_putc(char c)
{
#if SALAM_RT_X86
    while ((rt_inb(SALAM_RT_SERIAL_PORT + 5) & 0x20) == 0) {
    }
    rt_outb(SALAM_RT_SERIAL_PORT, (uint8_t)c);
#elif SALAM_RT_ARM64
    arm64_serial_putc(c);
#else
    (void)c;
#endif
}

static void debugcon_putc(char c)
{
#if SALAM_RT_X86
    rt_outb(SALAM_RT_DEBUGCON_PORT, (uint8_t)c);
#else
    (void)c;
#endif
}

#define VGA_COLS 80
#define VGA_ROWS 25
#define VGA_MEM ((volatile uint16_t *)(uintptr_t)0xB8000u)

static int vga_row = 0;
static int vga_col = 0;
static uint8_t vga_color = 0x0F;
static int vga_ready = 0;

static void vga_init(void)
{
    if (vga_ready) {
        return;
    }
    vga_ready = 1;
    for (int i = 0; i < VGA_COLS * VGA_ROWS; i++) {
        VGA_MEM[i] = (uint16_t)(' ' | ((uint16_t)0x07 << 8));
    }
}

static void vga_scroll(void)
{
    for (int row = 1; row < VGA_ROWS; row++) {
        for (int col = 0; col < VGA_COLS; col++) {
            VGA_MEM[(row - 1) * VGA_COLS + col] = VGA_MEM[row * VGA_COLS + col];
        }
    }
    for (int col = 0; col < VGA_COLS; col++) {
        VGA_MEM[(VGA_ROWS - 1) * VGA_COLS + col] =
            (uint16_t)(' ' | ((uint16_t)0x07 << 8));
    }
    vga_row = VGA_ROWS - 1;
}

static void vga_putc(char c)
{
    vga_init();
    if (c == '\n') {
        vga_col = 0;
        vga_row++;
    } else if (c == '\r') {
        vga_col = 0;
    } else if (c == '\t') {
        int next = (vga_col + 8) & ~7;
        while (vga_col < next && vga_col < VGA_COLS) {
            VGA_MEM[vga_row * VGA_COLS + vga_col] =
                (uint16_t)(' ' | ((uint16_t)vga_color << 8));
            vga_col++;
        }
    } else if ((unsigned char)c >= 32) {
        VGA_MEM[vga_row * VGA_COLS + vga_col] =
            (uint16_t)((unsigned char)c | ((uint16_t)vga_color << 8));
        vga_col++;
        if (vga_col >= VGA_COLS) {
            vga_col = 0;
            vga_row++;
        }
    }
    if (vga_row >= VGA_ROWS) {
        vga_scroll();
    }
}

SALAM_WEAK void salam_rt_write(int32_t fd, void *buf, uint64_t n)
{
    const char *p = (const char *)buf;
    if (!p) {
        return;
    }
    serial_init();
    vga_color = (fd == 2) ? 0x0C : 0x0F;
    for (uint64_t i = 0; i < n; i++) {
        char c = p[i];
        serial_putc(c);
        debugcon_putc(c);
        vga_putc(c);
    }
}

SALAM_WEAK void salam_rt_write_at(int32_t row, int32_t col, const char *text,
                                  uint8_t color)
{
    vga_init();
    if (row < 0 || row >= VGA_ROWS || col < 0 || col >= VGA_COLS || !text) {
        return;
    }
    int i = 0;
    while (text[i] && col + i < VGA_COLS) {
        VGA_MEM[row * VGA_COLS + col + i] =
            (uint16_t)((unsigned char)text[i] | ((uint16_t)color << 8));
        i++;
    }
}

SALAM_WEAK void salam_rt_clear(void)
{
    vga_ready = 0;
    vga_row = 0;
    vga_col = 0;
    vga_color = 0x0F;
    vga_init();
}

SALAM_WEAK void salam_rt_serial_write(const char *text)
{
    if (!text) {
        return;
    }
    serial_init();
    for (const char *p = text; *p; p++) {
        serial_putc(*p);
    }
}

SALAM_WEAK void salam_rt_exit_qemu(int code)
{
    rt_outb(0xF4, (uint8_t)code);
    rt_halt_forever();
}

long write(int fd, const void *buf, unsigned long n)
{
    salam_rt_write(fd, (void *)buf, (uint64_t)n);
    return (long)n;
}

int isatty(int fd)
{
    (void)fd;
    return 0;
}

int fflush(void *stream)
{
    (void)stream;
    return 0;
}

int atexit(void (*fn)(void))
{
    (void)fn;
    return 0;
}

void (*signal(int sig, void (*fn)(int)))(int)
{
    (void)sig;
    (void)fn;
    return 0;
}

int raise(int sig)
{
    (void)sig;
    return 0;
}

int sched_yield(void)
{
    return 0;
}

void abort(void)
{
    static const char msg[] = "\n[kernel] abort\n";
    salam_rt_write(2, (void *)msg, sizeof(msg) - 1);
    rt_halt_forever();
}

void exit(int code)
{
    salam_rt_exit(code);
    rt_halt_forever();
}

void __stack_chk_fail(void)
{
    abort();
}

void *memcpy(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    for (size_t i = 0; i < n; i++) {
        d[i] = s[i];
    }
    return dst;
}

void *memmove(void *dst, const void *src, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    const unsigned char *s = (const unsigned char *)src;
    if (d == s || n == 0) {
        return dst;
    }
    if (d < s) {
        for (size_t i = 0; i < n; i++) {
            d[i] = s[i];
        }
    } else {
        for (size_t i = n; i > 0; i--) {
            d[i - 1] = s[i - 1];
        }
    }
    return dst;
}

void *memset(void *dst, int value, size_t n)
{
    unsigned char *d = (unsigned char *)dst;
    for (size_t i = 0; i < n; i++) {
        d[i] = (unsigned char)value;
    }
    return dst;
}

int memcmp(const void *a, const void *b, size_t n)
{
    const unsigned char *pa = (const unsigned char *)a;
    const unsigned char *pb = (const unsigned char *)b;
    for (size_t i = 0; i < n; i++) {
        if (pa[i] != pb[i]) {
            return (int)pa[i] - (int)pb[i];
        }
    }
    return 0;
}

size_t strlen(const char *s)
{
    size_t n = 0;
    if (!s) {
        return 0;
    }
    while (s[n]) {
        n++;
    }
    return n;
}

int strcmp(const char *a, const char *b)
{
    if (!a) {
        a = "";
    }
    if (!b) {
        b = "";
    }
    while (*a && *a == *b) {
        a++;
        b++;
    }
    return (int)(unsigned char)*a - (int)(unsigned char)*b;
}

int strncmp(const char *a, const char *b, size_t n)
{
    if (!a) {
        a = "";
    }
    if (!b) {
        b = "";
    }
    for (size_t i = 0; i < n; i++) {
        unsigned char ca = (unsigned char)a[i];
        unsigned char cb = (unsigned char)b[i];
        if (ca != cb || ca == 0) {
            return (int)ca - (int)cb;
        }
    }
    return 0;
}

char *strstr(const char *haystack, const char *needle)
{
    if (!haystack || !needle) {
        return 0;
    }
    if (!*needle) {
        return (char *)haystack;
    }
    for (const char *h = haystack; *h; h++) {
        const char *a = h;
        const char *b = needle;
        while (*a && *b && *a == *b) {
            a++;
            b++;
        }
        if (!*b) {
            return (char *)h;
        }
    }
    return 0;
}

static int digit_value(char c)
{
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'z') {
        return c - 'a' + 10;
    }
    if (c >= 'A' && c <= 'Z') {
        return c - 'A' + 10;
    }
    return -1;
}

long strtol(const char *s, char **endptr, int base)
{
    const char *p = s;
    int neg = 0;
    long value = 0;
    if (!p) {
        if (endptr) {
            *endptr = (char *)s;
        }
        return 0;
    }
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }
    if (*p == '-' || *p == '+') {
        neg = (*p == '-');
        p++;
    }
    if (base == 0) {
        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
            base = 16;
            p += 2;
        } else if (p[0] == '0') {
            base = 8;
            p++;
        } else {
            base = 10;
        }
    } else if (base == 16 && p[0] == '0' && (p[1] == 'x' || p[1] == 'X')) {
        p += 2;
    }
    while (*p) {
        int d = digit_value(*p);
        if (d < 0 || d >= base) {
            break;
        }
        value = value * base + d;
        p++;
    }
    if (endptr) {
        *endptr = (char *)p;
    }
    return neg ? -value : value;
}

double strtod(const char *s, char **endptr)
{
    const char *p = s;
    double value = 0.0;
    double scale = 1.0;
    int neg = 0;
    int seen = 0;
    if (!p) {
        if (endptr) {
            *endptr = (char *)s;
        }
        return 0.0;
    }
    while (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\r') {
        p++;
    }
    if (*p == '-' || *p == '+') {
        neg = (*p == '-');
        p++;
    }
    while (*p >= '0' && *p <= '9') {
        value = value * 10.0 + (double)(*p - '0');
        p++;
        seen = 1;
    }
    if (*p == '.') {
        p++;
        while (*p >= '0' && *p <= '9') {
            scale *= 10.0;
            value = value * 10.0 + (double)(*p - '0');
            p++;
            seen = 1;
        }
    }
    if (seen && (*p == 'e' || *p == 'E')) {
        const char *e = p + 1;
        int eneg = 0;
        int exp = 0;
        if (*e == '-' || *e == '+') {
            eneg = (*e == '-');
            e++;
        }
        while (*e >= '0' && *e <= '9') {
            exp = exp * 10 + (*e - '0');
            e++;
        }
        while (exp-- > 0) {
            if (eneg) {
                value /= 10.0;
            } else {
                value *= 10.0;
            }
        }
        p = e;
    }
    if (endptr) {
        *endptr = (char *)p;
    }
    value /= scale;
    return neg ? -value : value;
}

static uint64_t rt_udiv64(uint64_t value, uint32_t base, uint32_t *rem)
{
    uint64_t quotient = 0;
    uint64_t rest = 0;
    for (int bit = 63; bit >= 0; bit--) {
        rest = (rest << 1) | ((value >> bit) & 1u);
        if (rest >= base) {
            rest -= base;
            quotient |= 1ull << bit;
        }
    }
    *rem = (uint32_t)rest;
    return quotient;
}

static char *fmt_uint(char *out, char *end, uint64_t value, unsigned base, int upper)
{
    char tmp[32];
    int n = 0;
    const char *digits = upper ? "0123456789ABCDEF" : "0123456789abcdef";
    if (value == 0) {
        tmp[n++] = '0';
    }
    while (value && n < (int)sizeof(tmp)) {
        uint32_t rem = 0;
        value = rt_udiv64(value, base, &rem);
        tmp[n++] = digits[rem];
    }
    while (n > 0 && out < end) {
        *out++ = tmp[--n];
    }
    return out;
}

static char *fmt_int(char *out, char *end, int64_t value)
{
    if (value < 0) {
        if (out < end) {
            *out++ = '-';
        }
        return fmt_uint(out, end, (uint64_t)(-(value + 1)) + 1u, 10, 0);
    }
    return fmt_uint(out, end, (uint64_t)value, 10, 0);
}

static char *fmt_double(char *out, char *end, double value, int precision)
{
    if (value < 0.0) {
        if (out < end) {
            *out++ = '-';
        }
        value = -value;
    }
    uint32_t whole = value > 4294967295.0 ? 0xFFFFFFFFu : (uint32_t)value;
    out = fmt_uint(out, end, whole, 10, 0);
    if (precision > 0 && out < end) {
        *out++ = '.';
        double frac = value - (double)whole;
        for (int i = 0; i < precision && out < end; i++) {
            frac *= 10.0;
            int digit = (int)frac;
            if (digit < 0) {
                digit = 0;
            }
            if (digit > 9) {
                digit = 9;
            }
            *out++ = (char)('0' + digit);
            frac -= (double)digit;
        }
    }
    return out;
}

int vsnprintf(char *buf, size_t size, const char *fmt, va_list args)
{
    char *out = buf;
    char *end = buf + (size > 0 ? size - 1 : 0);
    if (!fmt) {
        if (size > 0) {
            buf[0] = 0;
        }
        return 0;
    }
    while (*fmt) {
        if (*fmt != '%') {
            if (out < end) {
                *out++ = *fmt;
            }
            fmt++;
            continue;
        }
        fmt++;
        int long_count = 0;
        int precision = 6;
        while (*fmt == 'l') {
            long_count++;
            fmt++;
        }
        if (*fmt == 'z') {
            long_count = 1;
            fmt++;
        }
        if (*fmt == '.') {
            fmt++;
            precision = 0;
            while (*fmt >= '0' && *fmt <= '9') {
                precision = precision * 10 + (*fmt - '0');
                fmt++;
            }
        }
        while (*fmt == '0' || *fmt == '-' || *fmt == '+' || *fmt == ' ' || *fmt == '#') {
            fmt++;
        }
        while (*fmt >= '0' && *fmt <= '9') {
            fmt++;
        }
        switch (*fmt) {
        case 's': {
            const char *s = va_arg(args, const char *);
            if (!s) {
                s = "(null)";
            }
            while (*s && out < end) {
                *out++ = *s++;
            }
            break;
        }
        case 'c': {
            int c = va_arg(args, int);
            if (out < end) {
                *out++ = (char)c;
            }
            break;
        }
        case 'd':
        case 'i': {
            int64_t v;
            if (long_count >= 2) {
                v = va_arg(args, long long);
            } else if (long_count == 1) {
                v = va_arg(args, long);
            } else {
                v = va_arg(args, int);
            }
            out = fmt_int(out, end, v);
            break;
        }
        case 'u':
        case 'x':
        case 'X': {
            uint64_t v;
            if (long_count >= 2) {
                v = va_arg(args, unsigned long long);
            } else if (long_count == 1) {
                v = va_arg(args, unsigned long);
            } else {
                v = va_arg(args, unsigned int);
            }
            unsigned base = (*fmt == 'u') ? 10u : 16u;
            out = fmt_uint(out, end, v, base, *fmt == 'X');
            break;
        }
        case 'p': {
            uint64_t v = (uint64_t)(uintptr_t)va_arg(args, void *);
            if (out < end) {
                *out++ = '0';
            }
            if (out < end) {
                *out++ = 'x';
            }
            out = fmt_uint(out, end, v, 16, 0);
            break;
        }
        case 'f':
        case 'F':
        case 'g':
        case 'G': {
            double v = va_arg(args, double);
            out = fmt_double(out, end, v, precision);
            break;
        }
        case '%': {
            if (out < end) {
                *out++ = '%';
            }
            break;
        }
        case 0: {
            fmt--;
            break;
        }
        default:
            if (out < end) {
                *out++ = '%';
            }
            if (out < end) {
                *out++ = *fmt;
            }
            break;
        }
        if (*fmt) {
            fmt++;
        }
    }
    if (size > 0) {
        *out = 0;
    }
    return (int)(out - buf);
}

int snprintf(char *buf, size_t size, const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, size, fmt, args);
    va_end(args);
    return n;
}

int printf(const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    salam_rt_write(1, buf, (uint64_t)n);
    return n;
}

int dprintf(int fd, const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    int n = vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    salam_rt_write(fd, buf, (uint64_t)n);
    return n;
}

typedef struct {
    size_t size;
    size_t used;
    unsigned char *base;
} rt_heap_state;

static unsigned char rt_heap[SALAM_RT_HEAP_SIZE] __attribute__((aligned(16)));
static rt_heap_state rt_heap_info = {SALAM_RT_HEAP_SIZE, 0, rt_heap};

void *malloc(size_t size)
{
    size_t total = sizeof(size_t) + size;
    total = (total + 15u) & ~(size_t)15u;
    if (total > rt_heap_info.size - rt_heap_info.used) {
        abort();
    }
    unsigned char *p = rt_heap_info.base + rt_heap_info.used;
    rt_heap_info.used += total;
    *(size_t *)p = size;
    return p + sizeof(size_t);
}

void free(void *ptr)
{
    (void)ptr;
}

void *calloc(size_t count, size_t size)
{
    if (size != 0 && count > ((size_t)-1) / size) {
        abort();
    }
    size_t total = count * size;
    void *p = malloc(total);
    memset(p, 0, total);
    return p;
}

void *realloc(void *ptr, size_t size)
{
    if (!ptr) {
        return malloc(size);
    }
    size_t old = *(size_t *)((unsigned char *)ptr - sizeof(size_t));
    void *next = malloc(size);
    if (!next) {
        return 0;
    }
    memcpy(next, ptr, old < size ? old : size);
    return next;
}

uint64_t salam_rt_heap_used(void)
{
    return (uint64_t)rt_heap_info.used;
}

uint64_t salam_rt_heap_size(void)
{
    return (uint64_t)rt_heap_info.size;
}

SALAM_WEAK const char *salam_input(void)
{
    return "";
}

static char *rt_strdup(const char *s)
{
    size_t n = strlen(s);
    char *p = (char *)malloc(n + 1);
    if (!p) {
        return 0;
    }
    memcpy(p, s, n + 1);
    return p;
}

SALAM_WEAK const char *salam_strcat(const char *a, const char *b)
{
    if (!a) {
        a = "";
    }
    if (!b) {
        b = "";
    }
    size_t na = strlen(a);
    size_t nb = strlen(b);
    char *p = (char *)malloc(na + nb + 1);
    if (!p) {
        return "";
    }
    memcpy(p, a, na);
    memcpy(p + na, b, nb + 1);
    return p;
}

SALAM_WEAK uint64_t salam_str_hash(const char *s)
{
    uint64_t h = 1469598103934665603ull;
    if (!s) {
        return h;
    }
    while (*s) {
        h ^= (unsigned char)*s++;
        h *= 1099511628211ull;
    }
    return h;
}

SALAM_WEAK uint64_t salam_hash_int(uint64_t x)
{
    x ^= x >> 33;
    x *= 0xff51afd7ed558ccdull;
    x ^= x >> 33;
    x *= 0xc4ceb9fe1a85ec53ull;
    x ^= x >> 33;
    return x;
}

SALAM_WEAK const char *salam_char_from_code(int code)
{
    char *p = (char *)malloc(2);
    if (!p) {
        return "";
    }
    p[0] = (char)(unsigned char)code;
    p[1] = 0;
    return p;
}

static const char *rt_static_str(const char *s)
{
    return rt_strdup(s);
}

SALAM_WEAK const char *salam_tostr_i32(int32_t x)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", x);
    return rt_static_str(buf);
}

SALAM_WEAK const char *salam_tostr_i64(int64_t x)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%lld", (long long)x);
    return rt_static_str(buf);
}

SALAM_WEAK const char *salam_tostr_u32(uint32_t x)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%u", (unsigned)x);
    return rt_static_str(buf);
}

SALAM_WEAK const char *salam_tostr_u64(uint64_t x)
{
    char buf[32];
    snprintf(buf, sizeof(buf), "%llu", (unsigned long long)x);
    return rt_static_str(buf);
}

SALAM_WEAK const char *salam_tostr_bool(int b)
{
    return b ? "true" : "false";
}

SALAM_WEAK const char *salam_tostr_char(int c)
{
    return salam_char_from_code(c);
}

SALAM_WEAK const char *salam_tostr_f64(double x)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", x);
    return rt_static_str(buf);
}

SALAM_WEAK const char *salam_fmt_f64(double x)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "%g", x);
    return rt_static_str(buf);
}

SALAM_WEAK const char *salam_strcat_v(int n, const char **parts, unsigned char *own)
{
    size_t total = 1;
    for (int i = 0; i < n; i++) {
        total += strlen(parts[i]);
    }
    char *p = (char *)malloc(total);
    if (!p) {
        return "";
    }
    size_t at = 0;
    for (int i = 0; i < n; i++) {
        size_t len = strlen(parts[i]);
        memcpy(p + at, parts[i], len);
        at += len;
        if (own && own[i]) {
            free((void *)parts[i]);
        }
    }
    p[at] = 0;
    return p;
}
