#include <cstdio>

const int N = 300000;

long long arr[N];
long long aux[N];

void mergesort(long long lo, long long hi)
{
    if (lo >= hi) return;
    long long mid = (lo + hi) / 2;
    mergesort(lo, mid);
    mergesort(mid + 1, hi);
    long long i = lo;
    long long j = mid + 1;
    long long k = lo;
    while (i <= mid && j <= hi) {
        if (arr[i] <= arr[j]) {
            aux[k] = arr[i];
            i = i + 1;
        } else {
            aux[k] = arr[j];
            j = j + 1;
        }
        k = k + 1;
    }
    while (i <= mid) {
        aux[k] = arr[i];
        i = i + 1;
        k = k + 1;
    }
    while (j <= hi) {
        aux[k] = arr[j];
        j = j + 1;
        k = k + 1;
    }
    for (long long m = lo; m <= hi; m++) {
        arr[m] = aux[m];
    }
}

int main()
{
    long long x = 2;
    long long checksum = 0;

    for (long long i = 0; i < N; i++) {
        x = (x * 1103515245 + 12345) % 2147483648LL;
        arr[i] = x;
    }

    mergesort(0, N - 1);

    for (long long i = 0; i < N; i++) {
        checksum =
            (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007;
    }

    printf("%lld\n", arr[0]);
    printf("%lld\n", arr[N - 1]);
    printf("%lld\n", checksum);
    return 0;
}
