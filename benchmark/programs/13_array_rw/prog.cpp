#include <iostream>
#include <vector>

int main()
{
    const long long N = 1000000;
    std::vector<long long> arr(N);
    for (long long i = 0; i < N; i++) {
        arr[i] = (i * i + 3 * i + 7) % 1000000007;
    }
    long long sum = 0;
    for (long long pass = 0; pass < 5; pass++) {
        for (long long i = 0; i < N; i++) {
            sum = (sum + arr[i] + pass) % 1000000007;
        }
    }
    std::cout << sum << "\n";
    return 0;
}
