"""Recursive Fibonacci benchmark."""


def fib(n):
    """Return the n-th Fibonacci number computed recursively."""
    if n < 2:
        return n
    return fib(n - 1) + fib(n - 2)


print(fib(30))
