"""Perfect-numbers benchmark."""

count = 0
total = 0
for n in range(2, 30001):
    s = 1
    j = 2
    while j * j <= n:
        if n % j == 0:
            s += j
            q = n // j
            if q != j:
                s += q
        j += 1
    if s == n:
        count += 1
        total += n
print(count)
print(total)
