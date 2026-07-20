#include <stdio.h>

int main(void)
{
    long long n = 5000000;
    long long total_runs = 0;
    long long sumsq = 0;
    long long curval = -1;
    long long curlen = 0;
    long long i;
    for (i = 0; i < n; i++) {
        long long v = (i / 50) % 7;
        if (v == curval) {
            curlen = curlen + 1;
        } else {
            if (curlen > 0) {
                total_runs = total_runs + 1;
                sumsq = (sumsq + curlen * curlen) % 1000000007;
            }
            curval = v;
            curlen = 1;
        }
    }
    total_runs = total_runs + 1;
    sumsq = (sumsq + curlen * curlen) % 1000000007;
    printf("%lld\n", total_runs);
    printf("%lld\n", sumsq);
    return 0;
}
