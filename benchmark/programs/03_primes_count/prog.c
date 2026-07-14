#include <stdio.h>

int main(void) {
    long long count = 1;
    long long i;
    for (i = 3; i < 100000; i += 2) {
        long long is_p = 1;
        long long j;
        for (j = 3; j * j <= i; j += 2) {
            if (i % j == 0) {
                is_p = 0;
                break;
            }
        }
        count += is_p;
    }
    printf("%lld\n", count);
    return 0;
}
