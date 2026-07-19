#include <stdio.h>
#include <stdlib.h>

int main(void) {
    long long N = 5000000;
    long long RANGE = 100000;
    long long x = 6;
    int *counts = (int *)calloc((size_t)RANGE, sizeof(int));
    long long i;
    for (i = 0; i < N; i = i + 1) {
        x = (x * 1103515245 + 12345) % 2147483648;
        int v = (int)(x % RANGE);
        counts[v] = counts[v] + 1;
    }

    long long checksum = 0;
    long long pos = 0;
    long long v;
    for (v = 0; v < RANGE; v = v + 1) {
        int c = counts[v];
        int t;
        for (t = 0; t < c; t = t + 1) {
            pos = pos + 1;
            checksum = (checksum + (v % 1000000007) * (pos % 1000000007)) % 1000000007;
        }
    }

    printf("%lld\n", checksum);
    free(counts);
    return 0;
}
