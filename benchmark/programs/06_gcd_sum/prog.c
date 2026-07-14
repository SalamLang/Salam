#include <stdio.h>

long long gcd(long long a, long long b) {
    long long x = a;
    long long y = b;
    while (y != 0) {
        long long t = x % y;
        x = y;
        y = t;
    }
    return x;
}

int main(void) {
    long long s = 0;
    long long i;
    for (i = 1; i <= 300000; i++) {
        s += gcd(i, 987654321);
    }
    printf("%lld\n", s);
    return 0;
}
