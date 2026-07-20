"""Modular exponentiation benchmark."""


def powmod(base, e, m):
    """Return base**e mod m using square-and-multiply."""
    result = 1
    b = base % m
    ex = e
    while ex > 0:
        if ex % 2 == 1:
            result = (result * b) % m
        b = (b * b) % m
        ex = ex // 2
    return result


s = 0
for i in range(1, 30001):
    s = (s + powmod(i, 65537, 1000000007)) % 1000000007
print(s)
