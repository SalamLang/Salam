#include <iostream>

long long ackermann(long long m, long long n)
{
    if (m == 0) return n + 1;
    if (n == 0) return ackermann(m - 1, 1);
    return ackermann(m - 1, ackermann(m, n - 1));
}

int main()
{
    std::cout << ackermann(3, 6) << "\n";
    return 0;
}
