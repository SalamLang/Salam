#include <stdio.h>

int main(void)
{
    long long count = 0;
    long long total = 0;
    long long n;
    for (n = 2; n <= 30000; n++) {
        long long s = 1;
        long long j;
        for (j = 2; j * j <= n; j++) {
            if (n % j == 0) {
                s += j;
                long long q = n / j;
                if (q != j) s += q;
            }
        }
        if (s == n) {
            count++;
            total += n;
        }
    }
    printf("%lld\n%lld\n", count, total);
    return 0;
}
