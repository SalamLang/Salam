s = 0
for i in range(1, 500001):
    m = i
    while m > 0:
        s += m % 10
        m = m // 10
print(s)
