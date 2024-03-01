#include <stdio.h>

int main(void)
{
    long long a = 0;
    long long b = 1;
    long long i;
    for (i = 0; i < 5000000; i++) {
        long long t = (a + b) % 1000000007;
        a = b;
        b = t;
    }
    printf("%lld\n", a);
    return 0;
}
