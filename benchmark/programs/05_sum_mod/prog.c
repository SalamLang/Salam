#include <stdio.h>

int main(void)
{
    long long s = 0;
    long long i;
    for (i = 1; i <= 3000000; i++) {
        s = (s + i * i) % 1000000007;
    }
    printf("%lld\n", s);
    return 0;
}
