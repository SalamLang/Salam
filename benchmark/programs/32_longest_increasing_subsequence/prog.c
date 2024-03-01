#include <stdio.h>

#define N 6000

int main(void)
{
    static long long seq[N];
    static long long dp[N];
    int i;
    int j;
    long long answer;

    for (i = 0; i < N; i++) {
        seq[i] = (i * 37 + 11) % 10007;
    }

    for (i = 0; i < N; i++) {
        dp[i] = 1;
    }

    for (i = 0; i < N; i++) {
        for (j = 0; j < i; j++) {
            if (seq[j] < seq[i]) {
                if (dp[j] + 1 > dp[i]) {
                    dp[i] = dp[j] + 1;
                }
            }
        }
    }

    answer = dp[0];
    for (i = 0; i < N; i++) {
        if (dp[i] > answer) {
            answer = dp[i];
        }
    }

    printf("%lld\n", answer);
    return 0;
}
