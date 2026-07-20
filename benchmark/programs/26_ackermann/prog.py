"""Recursive Ackermann benchmark."""

import sys

sys.setrecursionlimit(10000)


def ackermann(m, n):
    """Return the Ackermann-Peter function value computed recursively."""
    if m == 0:
        return n + 1
    if n == 0:
        return ackermann(m - 1, 1)
    return ackermann(m - 1, ackermann(m, n - 1))


print(ackermann(3, 6))
