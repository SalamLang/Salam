#include <stdio.h>
#include <stdlib.h>

int main(void)
{
    long long LIMIT = 3000000;
    char *is_composite = calloc((size_t)LIMIT + 1, sizeof(char));
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
    long long k;
    for (k = 2; k <= LIMIT; k++) {
        if (is_composite[k] == 0) {
            count = count + 1;
        }
    }
    printf("%lld\n", count);
    free(is_composite);
    return 0;
}
