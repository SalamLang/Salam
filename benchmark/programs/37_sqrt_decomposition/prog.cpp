#include <iostream>
#include <vector>

static const int N = 2000000;
static const int BLOCK = 1414;

void update(std::vector<long long> &arr, std::vector<long long> &blocksum, int i, long long delta) {
    arr[i] = arr[i] + delta;
    int b = i / BLOCK;
    blocksum[b] = blocksum[b] + delta;
}

long long prefix_sum(std::vector<long long> &arr, std::vector<long long> &blocksum, int i) {
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

int main() {
    int nblocks = (N + BLOCK - 1) / BLOCK;
    std::vector<long long> arr(N, 0);
    std::vector<long long> blocksum(nblocks, 0);

    int Q = 20000;
    long long x = 8;
    long long checksum = 0;

    for (int t = 0; t < Q; t = t + 1) {
        x = (x * 1103515245 + 12345) % 2147483648;
        int idx = (int)(x % N);
        x = (x * 1103515245 + 12345) % 2147483648;
        long long delta = (x % 1000) + 1;
        update(arr, blocksum, idx, delta);
        x = (x * 1103515245 + 12345) % 2147483648;
        int qi = (int)(x % N);
        long long s = prefix_sum(arr, blocksum, qi);
        checksum = (checksum + s) % 1000000007;
    }

    std::cout << checksum << "\n";
    return 0;
}
