#include <iostream>

int main()
{
    long long limit = 2000000;
    long long total = 0;
    for (long long n = 2; n <= limit; n++) {
        long long temp = n;
        long long d = 2;
        while (d * d <= temp) {
            while (temp % d == 0) {
                total += 1;
                temp /= d;
            }
            d += 1;
        }
        if (temp > 1) {
            total += 1;
        }
    }
    std::cout << total << "\n";
    return 0;
}
