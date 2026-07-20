"""Conway's Game of Life benchmark."""

W = 150
H = 150
GENERATIONS = 150

grid = [0] * (W * H)
next_grid = [0] * (W * H)

for r in range(H):
    for c in range(W):
        grid[r * W + c] = 1 if (r * 31 + c * 17 + 7) % 5 == 0 else 0

for gen in range(GENERATIONS):
    for r in range(H):
        for c in range(W):
            count = 0
            for dr in (-1, 0, 1):
                for dc in (-1, 0, 1):
                    if dr == 0 and dc == 0:
                        continue
                    nr = r + dr
                    nc = c + dc
                    if 0 <= nr < H and 0 <= nc < W:
                        count = count + grid[nr * W + nc]
            alive = grid[r * W + c]
            if alive == 1:
                next_grid[r * W + c] = 1 if count in (2, 3) else 0
            else:
                next_grid[r * W + c] = 1 if (count == 3) else 0
    grid, next_grid = next_grid, grid

live_count = 0
for i in range(W * H):
    live_count = live_count + grid[i]
print(live_count)
