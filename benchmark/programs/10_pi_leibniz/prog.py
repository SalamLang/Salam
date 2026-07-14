sum_ = 0.0
sign = 1.0
den = 1.0
for k in range(3000000):
    sum_ = sum_ + sign / den
    sign = 0.0 - sign
    den = den + 2.0
pi = 4.0 * sum_
v = pi * 1000000.0
print(int(v))
