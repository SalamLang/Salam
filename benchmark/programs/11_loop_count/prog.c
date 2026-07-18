#include <stdio.h>

int main(void)
{
    long long acc = 0;
    long long lens[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
    int li;
    for (li = 0; li < 7; li++) {
        long long L = lens[li];
        long long i;
        for (i = 1; i <= L; i++) {
            acc = (acc + i) % 1000000007;
        }
    }
    printf("%lld\n", acc);
    return 0;
}
