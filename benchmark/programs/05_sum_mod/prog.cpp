#include <iostream>

int main() {
    long long s = 0;
    for (long long i = 1; i <= 3000000; i++) {
        s = (s + i * i) % 1000000007;
    }
    std::cout << s << "\n";
    return 0;
}
