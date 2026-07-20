"""Nested loop-count benchmark."""

acc = 0
for L in (1, 10, 100, 1000, 10000, 100000, 1000000):
    for i in range(1, L + 1):
        acc = (acc + i) % 1000000007
print(acc)
