#include <iostream>

int main()
{
    std::ios::sync_with_stdio(false);
    for (long long i = 0; i < 1000000; i++) {
        std::cout << "hello\n";
    }
    return 0;
}
