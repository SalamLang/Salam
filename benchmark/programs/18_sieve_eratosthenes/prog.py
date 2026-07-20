"""Sieve of Eratosthenes benchmark using array marking."""

LIMIT = 3000000
is_composite = [0] * (LIMIT + 1)
is_composite[0] = 1
is_composite[1] = 1
i = 2
while i * i <= LIMIT:
    if is_composite[i] == 0:
        j = i * i
        while j <= LIMIT:
            is_composite[j] = 1
            j = j + i
    i = i + 1
count = 0
k = 2
while k <= LIMIT:
    if is_composite[k] == 0:
        count = count + 1
    k = k + 1
print(count)
