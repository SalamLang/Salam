#include <iostream>

int main()
{
    long long acc = 0;
    long long lens[7] = {1, 10, 100, 1000, 10000, 100000, 1000000};
    for (int li = 0; li < 7; li++) {
        long long L = lens[li];
        for (long long i = 1; i <= L; i++) {
            acc = (acc + i) % 1000000007;
        }
    }
    std::cout << acc << "\n";
    return 0;
}
