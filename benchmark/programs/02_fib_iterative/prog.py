a = 0
b = 1
for i in range(5000000):
    a, b = b, (a + b) % 1000000007
print(a)
