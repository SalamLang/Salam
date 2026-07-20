#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    int n = 2000000;
    long long *arr = (long long *)malloc((size_t)n * sizeof(long long));
    int i;
    for (i = 0; i < n; i++) {
        arr[i] = (long long)i * 2;
    }

    long long q = 5000000;
    long long x = 5;
    long long checksum = 0;
    long long qi;
    for (qi = 0; qi < q; qi++) {
        x = (x * 1103515245 + 12345) % 2147483648;
        long long target = x % (2 * n);
        int lo = 0;
        int hi = n;
        while (lo < hi) {
            int mid = (lo + hi) / 2;
            if (arr[mid] < target) {
                lo = mid + 1;
            } else {
                hi = mid;
            }
        }
        checksum = (checksum + (long long)lo) % 1000000007;
    }

    printf("%lld\n", checksum);
    free(arr);
    return 0;
}
