"""Array read/write throughput benchmark."""

N = 1000000
MOD = 1000000007
arr = [0] * N
for i in range(N):
    arr[i] = (i * i + 3 * i + 7) % MOD
s = 0
for p in range(5):
    for i in range(N):
        s = (s + arr[i] + p) % MOD
print(s)
