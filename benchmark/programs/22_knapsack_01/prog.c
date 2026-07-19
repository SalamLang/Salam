#include <stdio.h>

#define NITEMS 500
#define CAPACITY 12500

int main(void) {
    long long weight[NITEMS];
    long long value[NITEMS];
    static long long dp[CAPACITY + 1];
    int i;
    long long w;

    for (i = 0; i < NITEMS; i++) {
        weight[i] = ((i * 7 + 3) % 50) + 1;
        value[i] = ((i * 13 + 5) % 100) + 1;
    }

    for (i = 0; i < NITEMS; i++) {
        for (w = CAPACITY; w >= weight[i]; w--) {
            long long cand = dp[w - weight[i]] + value[i];
            if (cand > dp[w]) {
                dp[w] = cand;
            }
        }
    }

    printf("%lld\n", dp[CAPACITY]);
    return 0;
}
