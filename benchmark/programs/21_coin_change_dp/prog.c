#include <stdio.h>

#define AMOUNT 100000
#define MOD 1000000007LL

int main(void) {
    long long coins[6] = {1, 5, 10, 25, 50, 100};
    static long long ways[AMOUNT + 1];
    int i;
    int ci;
    long long a;

    ways[0] = 1;
    for (i = 1; i <= AMOUNT; i++) {
        ways[i] = 0;
    }

    for (ci = 0; ci < 6; ci++) {
        long long c = coins[ci];
        for (a = c; a <= AMOUNT; a++) {
            ways[a] = (ways[a] + ways[a - c]) % MOD;
        }
    }

    printf("%lld\n", ways[AMOUNT]);
    return 0;
}
