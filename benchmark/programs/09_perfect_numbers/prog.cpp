#include <iostream>

int main() {
    long long count = 0;
    long long total = 0;
    for (long long n = 2; n <= 30000; n++) {
        long long s = 1;
        for (long long j = 2; j * j <= n; j++) {
            if (n % j == 0) {
                s += j;
                long long q = n / j;
                if (q != j)
                    s += q;
            }
        }
        if (s == n) {
            count++;
            total += n;
        }
    }
    std::cout << count << "\n" << total << "\n";
    return 0;
}
