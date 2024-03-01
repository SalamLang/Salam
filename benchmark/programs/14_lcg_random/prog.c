#include <stdio.h>

int main(void)
{
    long long x = 1;
    long long sum = 0;
    long long i;
    for (i = 0; i < 5000000; i++) {
        x = (x * 1103515245 + 12345) % 2147483648LL;
        sum = (sum + x) % 1000000007;
    }
    printf("%lld\n", sum);
    return 0;
}
