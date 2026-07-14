#include <stdio.h>

int main(void) {
    double sum = 0.0;
    double sign = 1.0;
    double den = 1.0;
    long long k;
    for (k = 0; k < 3000000; k++) {
        sum = sum + sign / den;
        sign = 0.0 - sign;
        den = den + 2.0;
    }
    double pi = 4.0 * sum;
    double v = pi * 1000000.0;
    printf("%lld\n", (long long)v);
    return 0;
}
