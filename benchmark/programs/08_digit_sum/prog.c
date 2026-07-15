#include <stdio.h>

int main(void)
{
    long long s = 0;
    long long i;
    for (i = 1; i <= 500000; i++) {
        long long m = i;
        while (m > 0) {
            s += m % 10;
            m = m / 10;
        }
    }
    printf("%lld\n", s);
    return 0;
}
