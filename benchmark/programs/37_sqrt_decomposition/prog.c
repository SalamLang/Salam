#include <stdio.h>

#define N 2000000
#define BLOCK 1414

static long long arr[N];
static long long blocksum[(N + BLOCK - 1) / BLOCK];

void update(int i, long long delta)
{
    arr[i] = arr[i] + delta;
    int b = i / BLOCK;
    blocksum[b] = blocksum[b] + delta;
}

long long prefix_sum(int i)
{
    int b = i / BLOCK;
    long long total = 0;
    int k = 0;
    while (k < b) {
        total = total + blocksum[k];
        k = k + 1;
    }
    int start = b * BLOCK;
    while (start <= i) {
        total = total + arr[start];
        start = start + 1;
    }
    return total;
}

int main(void)
{
    int nblocks = (N + BLOCK - 1) / BLOCK;
    (void)nblocks;

    int Q = 20000;
    long long x = 8;
    long long checksum = 0;

    for (int t = 0; t < Q; t = t + 1) {
        x = (x * 1103515245 + 12345) % 2147483648;
        int idx = (int)(x % N);
        x = (x * 1103515245 + 12345) % 2147483648;
        long long delta = (x % 1000) + 1;
        update(idx, delta);
        x = (x * 1103515245 + 12345) % 2147483648;
        int qi = (int)(x % N);
        long long s = prefix_sum(qi);
        checksum = (checksum + s) % 1000000007;
    }

    printf("%lld\n", checksum);
    return 0;
}
