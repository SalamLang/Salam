#include <iostream>

int main()
{
    long long count = 0;
    for (int n = 1; n <= 2000000; n++) {
        int temp = n;
        long long rev = 0;
        while (temp > 0) {
            rev = rev * 10 + temp % 10;
            temp = temp / 10;
        }
        if (rev == n) {
            count = count + 1;
        }
    }
    std::cout << count << "\n";
    return 0;
}
