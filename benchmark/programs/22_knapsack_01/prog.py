"""0/1 knapsack via 1D rolling DP array."""
NITEMS = 500
CAPACITY = 12500
weight = [0] * NITEMS
value = [0] * NITEMS

for i in range(NITEMS):
    weight[i] = ((i * 7 + 3) % 50) + 1
    value[i] = ((i * 13 + 5) % 100) + 1

dp = [0] * (CAPACITY + 1)

for i in range(NITEMS):
    wi = weight[i]
    vi = value[i]
    for w in range(CAPACITY, wi - 1, -1):
        cand = dp[w - wi] + vi
        if cand > dp[w]:
            dp[w] = cand

print(dp[CAPACITY])
