"""counting_sort benchmark"""
N = 5000000
RANGE = 100000
x = 6
counts = [0] * RANGE
for i in range(N):
    x = (x * 1103515245 + 12345) % 2147483648
    v = x % RANGE
    counts[v] = counts[v] + 1

checksum = 0
pos = 0
for v in range(RANGE):
    c = counts[v]
    for t in range(c):
        pos = pos + 1
        checksum = (checksum + (v % 1000000007) * (pos % 1000000007)) % 1000000007

print(checksum)
