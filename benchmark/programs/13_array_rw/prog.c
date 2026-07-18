#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    long long N = 1000000;
    long long *arr = malloc((size_t)N * sizeof(long long));
    long long i;
    for (i = 0; i < N; i++) {
        arr[i] = (i * i + 3 * i + 7) % 1000000007;
    }
    long long sum = 0;
    long long pass;
    for (pass = 0; pass < 5; pass++) {
        for (i = 0; i < N; i++) {
            sum = (sum + arr[i] + pass) % 1000000007;
        }
    }
    printf("%lld\n", sum);
    free(arr);
    return 0;
}
