#!/usr/bin/env python3
N = 12
cols = [0] * N
diag1 = [0] * (2 * N - 1)
diag2 = [0] * (2 * N - 1)
solutions = 0


def solve(row):
    global solutions
    if row == N:
        solutions = solutions + 1
        return
    for col in range(N):
        d1 = row + col
        d2 = row - col + N - 1
        if cols[col] == 0 and diag1[d1] == 0 and diag2[d2] == 0:
            cols[col] = 1
            diag1[d1] = 1
            diag2[d2] = 1
            solve(row + 1)
            cols[col] = 0
            diag1[d1] = 0
            diag2[d2] = 0


solve(0)
print(solutions)
