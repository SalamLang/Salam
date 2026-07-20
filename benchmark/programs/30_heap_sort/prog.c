#include <stdio.h>

long long arr[300000];

void heapify(long long *arr, long long n, long long i)
{
    long long largest = i;
    long long l = 2 * i + 1;
    long long r = 2 * i + 2;
    if (l < n && arr[l] > arr[largest]) {
        largest = l;
    }
    if (r < n && arr[r] > arr[largest]) {
        largest = r;
    }
    if (largest != i) {
        long long tmp = arr[i];
        arr[i] = arr[largest];
        arr[largest] = tmp;
        heapify(arr, n, largest);
    }
}

int main(void)
{
    long long n = 300000;
    long long x = 3;
    long long i;
    for (i = 0; i < n; i++) {
        x = (x * 1103515245 + 12345) % 2147483648;
        arr[i] = x;
    }
    for (i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i);
    }
    for (i = n - 1; i >= 1; i--) {
        long long tmp = arr[0];
        arr[0] = arr[i];
        arr[i] = tmp;
        heapify(arr, i, 0);
    }
    long long checksum = 0;
    for (i = 0; i < n; i++) {
        checksum =
            (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007;
    }
    printf("%lld\n", arr[0]);
    printf("%lld\n", arr[n - 1]);
    printf("%lld\n", checksum);
    return 0;
}
