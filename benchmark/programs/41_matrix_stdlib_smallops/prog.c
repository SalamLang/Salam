#include <stdio.h>

int main(void)
{
    int reps = 200000;
    long long checksum = 0;
    for (int r = 0; r < reps; r++) {
        double a[4][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                a[i][j] = (double)((r + i * 3 + j * 7 + 1) % 97);
            }
        }
        double c[4][4];
        for (int i = 0; i < 4; i++) {
            for (int j = 0; j < 4; j++) {
                double s = 0.0;
                for (int k = 0; k < 4; k++) {
                    s += a[i][k] * a[j][k];
                }
                c[i][j] = s;
            }
        }
        double tr = 0.0;
        for (int i = 0; i < 4; i++) {
            tr += c[i][i];
        }
        checksum = (checksum + (long long)tr) % 1000000007;
    }
    printf("%lld\n", checksum);
    return 0;
}
