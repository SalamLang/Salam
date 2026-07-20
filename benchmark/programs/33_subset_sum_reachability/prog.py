"""0/1 subset-sum reachability via boolean DP array."""

NITEMS = 1000
TARGET = 30000
weight = [0] * NITEMS

for i in range(NITEMS):
    weight[i] = ((i * 11 + 5) % 40) + 1

reachable = [0] * (TARGET + 1)
reachable[0] = 1

for i in range(NITEMS):
    w = weight[i]
    for s in range(TARGET, w - 1, -1):
        if reachable[s - w]:
            reachable[s] = 1

count = 0
for s in range(TARGET + 1):
    if reachable[s]:
        count = count + 1

print(count)
