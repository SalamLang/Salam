"""Collatz-sequence longest-chain benchmark."""

best_steps = 0
best_n = 1
for n in range(1, 30001):
    m = n
    c = 0
    while m != 1:
        if m % 2 == 0:
            m = m // 2
        else:
            m = 3 * m + 1
        c += 1
    if c > best_steps:
        best_steps = c
        best_n = n
print(best_steps)
print(best_n)
