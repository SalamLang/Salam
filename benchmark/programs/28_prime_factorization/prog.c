#include <stdio.h>

int main(void)
{
    long long limit = 2000000;
    long long total = 0;
    long long n;
    for (n = 2; n <= limit; n++) {
        long long temp = n;
        long long d = 2;
        while (d * d <= temp) {
            while (temp % d == 0) {
                total += 1;
                temp /= d;
            }
            d += 1;
        }
        if (temp > 1) {
            total += 1;
        }
    }
    printf("%lld\n", total);
    return 0;
}
