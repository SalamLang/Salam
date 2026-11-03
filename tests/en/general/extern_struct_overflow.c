#include <stdarg.h>

int fill_ptrs(const char *src, int n, ...) {
    va_list ap;
    int k;
    (void)src;
    va_start(ap, n);
    for (k = 0; k < n; k++) {
        *va_arg(ap, long long *) = k + 1;
    }
    va_end(ap);
    return n;
}
