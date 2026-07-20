#include <stdio.h>

#define N 12

int cols[N];
int diag1[2 * N - 1];
int diag2[2 * N - 1];
long long solutions = 0;

void solve(int row)
{
    if (row == N) {
        solutions = solutions + 1;
        return;
    }
    int col;
    for (col = 0; col < N; col++) {
        int d1 = row + col;
        int d2 = row - col + N - 1;
        if (cols[col] == 0 && diag1[d1] == 0 && diag2[d2] == 0) {
            cols[col] = 1;
            diag1[d1] = 1;
            diag2[d2] = 1;
            solve(row + 1);
            cols[col] = 0;
            diag1[d1] = 0;
            diag2[d2] = 0;
        }
    }
}

int main(void)
{
    solve(0);
    printf("%lld\n", solutions);
    return 0;
}
