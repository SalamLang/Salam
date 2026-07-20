#include <iostream>

int main()
{
    long long n = 10000000;
    long long dot = 0;
    for (long long i = 0; i < n; i++) {
        long long a = (i * 3 + 1) % 1009;
        long long b = (i * 7 + 2) % 1013;
        dot = (dot + a * b) % 1000000007;
    }
    std::cout << dot << "\n";
    return 0;
}
