#include <stdio.h>

int main(void)
{
    static int a[200][200];
    static int b[200][200];
    int i, j, k;
    long long checksum = 0;
    for (i = 0; i < 200; i++) {
        for (j = 0; j < 200; j++) {
            a[i][j] = (i * 3 + j * 7 + 1) % 97;
        }
    }
    for (i = 0; i < 200; i++) {
        for (j = 0; j < 200; j++) {
            b[i][j] = (i * 5 + j * 2 + 3) % 89;
        }
    }
    for (i = 0; i < 200; i++) {
        for (j = 0; j < 200; j++) {
            long long c = 0;
            for (k = 0; k < 200; k++) {
                c = c + (long long)a[i][k] * (long long)b[k][j];
            }
            checksum = (checksum + c) % 1000000007;
        }
    }
    printf("%lld\n", checksum);
    return 0;
}
