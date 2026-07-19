#include <iostream>

const int N = 300000;

static long long arr[N];

void quicksort(long long *a, long long lo, long long hi)
{
    if (lo >= hi) {
        return;
    }
    long long pivot = a[(lo + hi) / 2];
    long long i = lo;
    long long j = hi;
    while (i <= j) {
        while (a[i] < pivot) {
            i = i + 1;
        }
        while (a[j] > pivot) {
            j = j - 1;
        }
        if (i <= j) {
            long long tmp = a[i];
            a[i] = a[j];
            a[j] = tmp;
            i = i + 1;
            j = j - 1;
        }
    }
    quicksort(a, lo, j);
    quicksort(a, i, hi);
}

int main()
{
    long long x = 1;
    for (long long i = 0; i < N; i++) {
        x = (x * 1103515245 + 12345) % 2147483648LL;
        arr[i] = x;
    }
    quicksort(arr, 0, N - 1);
    long long checksum = 0;
    for (long long i = 0; i < N; i++) {
        checksum = (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007;
    }
    std::cout << arr[0] << "\n";
    std::cout << arr[N - 1] << "\n";
    std::cout << checksum << "\n";
    return 0;
}
