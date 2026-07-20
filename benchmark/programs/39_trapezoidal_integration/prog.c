#include <stdio.h>

double f(double x)
{
    return x * x * x - 2.0 * x * x + 1.0;
}

int main(void)
{
    double a = 0.0;
    double b = 10.0;
    long long steps = 10000000;
    double h = (b - a) / (double)steps;
    double sum = 0.5 * (f(a) + f(b));
    long long i;
    for (i = 1; i < steps; i++) {
        double x = a + (double)i * h;
        sum = sum + f(x);
    }
    double result = sum * h;
    double scaled = result * 1000000.0;
    printf("%lld\n", (long long)scaled);
    return 0;
}
