#include <stdio.h>

#define N 4000000

static long long arr[N];

void reverse(long long *a, int lo, int hi)
{
    while (lo < hi) {
        long long tmp = a[lo];
        a[lo] = a[hi];
        a[hi] = tmp;
        lo = lo + 1;
        hi = hi - 1;
    }
}

int main(void)
{
    int i;
    int k = 1234567 % N;
    for (i = 0; i < N; i++) {
        arr[i] = (long long)i;
    }
    reverse(arr, 0, k - 1);
    reverse(arr, k, N - 1);
    reverse(arr, 0, N - 1);
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        checksum = (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007;
    }
    printf("%lld\n", arr[0]);
    printf("%lld\n", arr[N - 1]);
    printf("%lld\n", checksum);
    return 0;
}
