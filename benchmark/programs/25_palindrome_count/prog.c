#include <stdio.h>

int main(void)
{
    long long count = 0;
    int n;
    for (n = 1; n <= 2000000; n++) {
        int temp = n;
        long long rev = 0;
        while (temp > 0) {
            rev = rev * 10 + temp % 10;
            temp = temp / 10;
        }
        if (rev == n) {
            count = count + 1;
        }
    }
    printf("%lld\n", count);
    return 0;
}
