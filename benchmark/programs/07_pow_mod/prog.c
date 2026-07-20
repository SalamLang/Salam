#include <stdio.h>

long long powmod(long long base, long long e, long long m)
{
    long long result = 1;
    long long b = base % m;
    long long ex = e;
    while (ex > 0) {
        if (ex % 2 == 1) {
            result = (result * b) % m;
        }
        b = (b * b) % m;
        ex = ex / 2;
    }
    return result;
}

int main(void)
{
    long long s = 0;
    long long i;
    for (i = 1; i <= 30000; i++) {
        s = (s + powmod(i, 65537, 1000000007)) % 1000000007;
    }
    printf("%lld\n", s);
    return 0;
}
