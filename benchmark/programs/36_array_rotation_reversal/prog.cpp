#include <cstdio>
#include <vector>

static const int N = 4000000;

void reverse_range(std::vector<long long> &a, int lo, int hi)
{
    while (lo < hi) {
        long long tmp = a[lo];
        a[lo] = a[hi];
        a[hi] = tmp;
        lo = lo + 1;
        hi = hi - 1;
    }
}

int main()
{
    std::vector<long long> arr(N);
    int i;
    int k = 1234567 % N;
    for (i = 0; i < N; i++) {
        arr[i] = (long long)i;
    }
    reverse_range(arr, 0, k - 1);
    reverse_range(arr, k, N - 1);
    reverse_range(arr, 0, N - 1);
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        checksum =
            (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007;
    }
    std::printf("%lld\n", arr[0]);
    std::printf("%lld\n", arr[N - 1]);
    std::printf("%lld\n", checksum);
    return 0;
}
