#include <stdio.h>

int main(void) {
    long long best_steps = 0;
    long long best_n = 1;
    long long n;
    for (n = 1; n <= 30000; n++) {
        long long m = n;
        long long c = 0;
        while (m != 1) {
            if (m % 2 == 0)
                m = m / 2;
            else
                m = 3 * m + 1;
            c++;
        }
        if (c > best_steps) {
            best_steps = c;
            best_n = n;
        }
    }
    printf("%lld\n%lld\n", best_steps, best_n);
    return 0;
}
