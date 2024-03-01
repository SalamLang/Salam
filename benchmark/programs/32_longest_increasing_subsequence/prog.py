N = 6000
seq = [0] * N
dp = [0] * N

for i in range(N):
    seq[i] = (i * 37 + 11) % 10007

for i in range(N):
    dp[i] = 1

for i in range(N):
    for j in range(i):
        if seq[j] < seq[i]:
            dp[i] = max(dp[i], dp[j] + 1)

answer = dp[0]
for i in range(N):
    answer = max(answer, dp[i])

print(answer)
