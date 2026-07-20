#include <iostream>
#include <vector>

int main()
{
    long long LIMIT = 3000000;
    std::vector<char> is_composite(LIMIT + 1, 0);
    long long i, j;
    is_composite[0] = 1;
    is_composite[1] = 1;
    i = 2;
    while (i * i <= LIMIT) {
        if (is_composite[i] == 0) {
            j = i * i;
            while (j <= LIMIT) {
                is_composite[j] = 1;
                j = j + i;
            }
        }
        i = i + 1;
    }
    long long count = 0;
    for (long long k = 2; k <= LIMIT; k++) {
        if (is_composite[k] == 0) {
            count = count + 1;
        }
    }
    std::cout << count << "\n";
    return 0;
}
