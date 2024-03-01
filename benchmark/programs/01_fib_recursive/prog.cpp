#include <iostream>

long long fib(long long n)
{
    if (n < 2) return n;
    return fib(n - 1) + fib(n - 2);
}

int main()
{
    std::cout << fib(30) << "\n";
    return 0;
}
