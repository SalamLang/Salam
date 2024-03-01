#include <stdio.h>

int main(void)
{
    long long n = 10000000;
    long long dot = 0;
    long long i;
    for (i = 0; i < n; i++) {
        long long a = (i * 3 + 1) % 1009;
        long long b = (i * 7 + 2) % 1013;
        dot = (dot + a * b) % 1000000007;
    }
    printf("%lld\n", dot);
    return 0;
}
