#include <stdio.h>

#define NITEMS 1000
#define TARGET 30000

int main(void) {
    int weight[NITEMS];
    static int reachable[TARGET + 1];
    int i;
    int s;
    long long count;

    for (i = 0; i < NITEMS; i++) {
        weight[i] = ((i * 11 + 5) % 40) + 1;
    }

    reachable[0] = 1;

    for (i = 0; i < NITEMS; i++) {
        int w = weight[i];
        for (s = TARGET; s >= w; s--) {
            if (reachable[s - w]) {
                reachable[s] = 1;
            }
        }
    }

    count = 0;
    for (s = 0; s <= TARGET; s++) {
        if (reachable[s]) {
            count = count + 1;
        }
    }

    printf("%lld\n", count);
    return 0;
}
