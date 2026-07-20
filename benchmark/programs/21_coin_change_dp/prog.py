"""coin_change_dp benchmark"""

AMOUNT = 100000
MOD = 1000000007
coins = [1, 5, 10, 25, 50, 100]
ways = [0] * (AMOUNT + 1)
ways[0] = 1

for c in coins:
    for a in range(c, AMOUNT + 1):
        ways[a] = (ways[a] + ways[a - c]) % MOD

print(ways[AMOUNT])
