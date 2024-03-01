#include <iostream>

int main()
{
    long long x = 1;
    long long count = 0;
    double ux, uy;
    for (long long step = 0; step < 5000000; step++) {
        x = (x * 1103515245 + 12345) % 2147483648LL;
        ux = (double)x / 2147483648.0 * 2.0 - 1.0;
        x = (x * 1103515245 + 12345) % 2147483648LL;
        uy = (double)x / 2147483648.0 * 2.0 - 1.0;
        if (ux * ux + uy * uy <= 1.0) {
            count = count + 1;
        }
    }
    std::cout << count << "\n";
    return 0;
}
