"""Prime factorization count (Omega function) benchmark via trial division."""

limit = 2000000
total = 0
for n in range(2, limit + 1):
    temp = n
    d = 2
    while d * d <= temp:
        while temp % d == 0:
            total += 1
            temp //= d
        d += 1
    if temp > 1:
        total += 1
print(total)
