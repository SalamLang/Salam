#include <iostream>
#include <vector>

int main()
{
    const int NITEMS = 1000;
    const int TARGET = 30000;
    std::vector<int> weight(NITEMS);
    std::vector<int> reachable(TARGET + 1, 0);

    for (int i = 0; i < NITEMS; i++) {
        weight[i] = ((i * 11 + 5) % 40) + 1;
    }

    reachable[0] = 1;

    for (int i = 0; i < NITEMS; i++) {
        int w = weight[i];
        for (int s = TARGET; s >= w; s--) {
            if (reachable[s - w]) {
                reachable[s] = 1;
            }
        }
    }

    long long count = 0;
    for (int s = 0; s <= TARGET; s++) {
        if (reachable[s]) {
            count = count + 1;
        }
    }

    std::cout << count << "\n";
    return 0;
}
