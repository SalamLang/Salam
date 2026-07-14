#include <iostream>

int main() {
    long long s = 0;
    for (long long i = 1; i <= 500000; i++) {
        long long m = i;
        while (m > 0) {
            s += m % 10;
            m = m / 10;
        }
    }
    std::cout << s << "\n";
    return 0;
}
