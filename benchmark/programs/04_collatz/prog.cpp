#include <iostream>

int main()
{
    long long best_steps = 0;
    long long best_n = 1;
    for (long long n = 1; n <= 30000; n++) {
        long long m = n;
        long long c = 0;
        while (m != 1) {
            if (m % 2 == 0)
                m = m / 2;
            else
                m = 3 * m + 1;
            c++;
        }
        if (c > best_steps) {
            best_steps = c;
            best_n = n;
        }
    }
    std::cout << best_steps << "\n" << best_n << "\n";
    return 0;
}
