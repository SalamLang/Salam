"""Trapezoidal integration benchmark."""


def f(x):
    return x * x * x - 2.0 * x * x + 1.0


A = 0.0
B = 10.0
STEPS = 10000000
H = (B - A) / float(STEPS)
sum_ = 0.5 * (f(A) + f(B))
for i in range(1, STEPS):
    x = A + float(i) * H
    sum_ = sum_ + f(x)
RESULT = sum_ * H
SCALED = RESULT * 1000000.0
print(int(SCALED))
