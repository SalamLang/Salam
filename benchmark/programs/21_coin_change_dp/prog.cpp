#include <iostream>
#include <vector>

int main() {
    const long long AMOUNT = 100000;
    const long long MOD = 1000000007LL;
    long long coins[6] = {1, 5, 10, 25, 50, 100};
    std::vector<long long> ways(AMOUNT + 1, 0);
    ways[0] = 1;

    for (int ci = 0; ci < 6; ci++) {
        long long c = coins[ci];
        for (long long a = c; a <= AMOUNT; a++) {
            ways[a] = (ways[a] + ways[a - c]) % MOD;
        }
    }

    std::cout << ways[AMOUNT] << "\n";
    return 0;
}
