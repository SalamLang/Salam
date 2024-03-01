#include <iostream>

#define W 150
#define H 150
#define GENERATIONS 150

static int buf_a[W * H];
static int buf_b[W * H];

int main()
{
    int r, c, gen, dr, dc;
    long long live_count;
    int *grid = buf_a;
    int *next_grid = buf_b;

    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            grid[r * W + c] = (r * 31 + c * 17 + 7) % 5 == 0 ? 1 : 0;
        }
    }
    for (r = 0; r < H; r++) {
        for (c = 0; c < W; c++) {
            next_grid[r * W + c] = 0;
        }
    }

    for (gen = 0; gen < GENERATIONS; gen++) {
        int *tmp;
        for (r = 0; r < H; r++) {
            for (c = 0; c < W; c++) {
                int count = 0;
                for (dr = -1; dr <= 1; dr++) {
                    for (dc = -1; dc <= 1; dc++) {
                        int nr, nc;
                        if (dr == 0 && dc == 0) {
                            continue;
                        }
                        nr = r + dr;
                        nc = c + dc;
                        if (nr >= 0 && nr < H && nc >= 0 && nc < W) {
                            count = count + grid[nr * W + nc];
                        }
                    }
                }
                {
                    int alive = grid[r * W + c];
                    if (alive == 1) {
                        next_grid[r * W + c] = (count == 2 || count == 3) ? 1 : 0;
                    } else {
                        next_grid[r * W + c] = (count == 3) ? 1 : 0;
                    }
                }
            }
        }
        tmp = grid;
        grid = next_grid;
        next_grid = tmp;
    }

    live_count = 0;
    for (r = 0; r < W * H; r++) {
        live_count = live_count + grid[r];
    }
    std::cout << live_count << "\n";
    return 0;
}
