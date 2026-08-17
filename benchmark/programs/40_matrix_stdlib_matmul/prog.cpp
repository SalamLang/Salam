#include <iostream>

#define N 400

int main()
{
    static int a[N][N];
    static int b[N][N];
    int i, j, k;
    long long checksum = 0;
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            a[i][j] = (i * 3 + j * 7 + 1) % 97;
        }
    }
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            b[i][j] = (i * 5 + j * 2 + 3) % 89;
        }
    }
    for (i = 0; i < N; i++) {
        for (j = 0; j < N; j++) {
            long long c = 0;
            for (k = 0; k < N; k++) {
                c = c + (long long)a[i][k] * (long long)b[k][j];
            }
            checksum = (checksum + c) % 1000000007;
        }
    }
    std::cout << checksum << "\n";
    return 0;
}
