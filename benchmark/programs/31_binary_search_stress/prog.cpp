#include <iostream>
#include <vector>

int main() {
    int n = 2000000;
    std::vector<long long> arr(n);
    for (int i = 0; i < n; i++) {
        arr[i] = static_cast<long long>(i) * 2;
    }

    long long q = 5000000;
    long long x = 5;
    long long checksum = 0;
    for (long long qi = 0; qi < q; qi++) {
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
        checksum = (checksum + static_cast<long long>(lo)) % 1000000007;
    }

    std::cout << checksum << "\n";
    return 0;
}
