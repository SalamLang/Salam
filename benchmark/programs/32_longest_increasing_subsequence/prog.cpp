#include <iostream>
#include <vector>

int main() {
    const int N = 6000;
    std::vector<long long> seq(N);
    std::vector<long long> dp(N);

    for (int i = 0; i < N; i++) {
        seq[i] = (i * 37 + 11) % 10007;
    }

    for (int i = 0; i < N; i++) {
        dp[i] = 1;
    }

    for (int i = 0; i < N; i++) {
        for (int j = 0; j < i; j++) {
            if (seq[j] < seq[i]) {
                if (dp[j] + 1 > dp[i]) {
                    dp[i] = dp[j] + 1;
                }
            }
        }
    }

    long long answer = dp[0];
    for (int i = 0; i < N; i++) {
        if (dp[i] > answer) {
            answer = dp[i];
        }
    }

    std::cout << answer << "\n";
    return 0;
}
