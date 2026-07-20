#include <iostream>
#include <vector>

int main()
{
    long long N = 5000000;
    long long RANGE = 100000;
    long long x = 6;
    std::vector<int> counts(RANGE, 0);
    for (long long i = 0; i < N; i = i + 1) {
        x = (x * 1103515245 + 12345) % 2147483648;
        int v = (int)(x % RANGE);
        counts[v] = counts[v] + 1;
    }

    long long checksum = 0;
    long long pos = 0;
    for (long long v = 0; v < RANGE; v = v + 1) {
        int c = counts[v];
        for (int t = 0; t < c; t = t + 1) {
            pos = pos + 1;
            checksum = (checksum + (v % 1000000007) * (pos % 1000000007)) % 1000000007;
        }
    }

    std::cout << checksum << "\n";
    return 0;
}
