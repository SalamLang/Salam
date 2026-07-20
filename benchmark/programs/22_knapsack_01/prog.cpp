#include <iostream>
#include <vector>

int main()
{
    const long long NITEMS = 500;
    const long long CAPACITY = 12500;
    std::vector<long long> weight(NITEMS);
    std::vector<long long> value(NITEMS);
    std::vector<long long> dp(CAPACITY + 1, 0);

    for (long long i = 0; i < NITEMS; i++) {
        weight[i] = ((i * 7 + 3) % 50) + 1;
        value[i] = ((i * 13 + 5) % 100) + 1;
    }

    for (long long i = 0; i < NITEMS; i++) {
        for (long long w = CAPACITY; w >= weight[i]; w--) {
            long long cand = dp[w - weight[i]] + value[i];
            if (cand > dp[w]) {
                dp[w] = cand;
            }
        }
    }

    std::cout << dp[CAPACITY] << "\n";
    return 0;
}
