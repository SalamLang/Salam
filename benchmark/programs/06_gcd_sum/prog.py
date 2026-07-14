def gcd(a, b):
    x = a
    y = b
    while y != 0:
        x, y = y, x % y
    return x


s = 0
for i in range(1, 300001):
    s += gcd(i, 987654321)
print(s)
