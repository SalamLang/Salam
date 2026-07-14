#include <iostream>

int main() {
    long long a = 0;
    long long b = 1;
    for (long long i = 0; i < 5000000; i++) {
        long long t = (a + b) % 1000000007;
        a = b;
        b = t;
    }
    std::cout << a << "\n";
    return 0;
}
