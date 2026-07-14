#include <iostream>

int main() {
    long long count = 1;
    for (long long i = 3; i < 100000; i += 2) {
        long long is_p = 1;
        for (long long j = 3; j * j <= i; j += 2) {
            if (i % j == 0) {
                is_p = 0;
                break;
            }
        }
        count += is_p;
    }
    std::cout << count << "\n";
    return 0;
}
