"""Modular sum-of-squares benchmark."""

s = 0
for i in range(1, 3000001):
    s = (s + i * i) % 1000000007
print(s)
