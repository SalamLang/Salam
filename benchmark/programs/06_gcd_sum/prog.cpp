#include <iostream>

long long gcd(long long a, long long b) {
    long long x = a;
    long long y = b;
    while (y != 0) {
        long long t = x % y;
        x = y;
        y = t;
    }
    return x;
}

int main() {
    long long s = 0;
    for (long long i = 1; i <= 300000; i++) {
        s += gcd(i, 987654321);
    }
    std::cout << s << "\n";
    return 0;
}
