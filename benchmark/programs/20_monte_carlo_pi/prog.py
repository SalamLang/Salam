"""Monte Carlo pi estimation benchmark."""

x = 1
count = 0
for step in range(5000000):
    x = (x * 1103515245 + 12345) % 2147483648
    ux = float(x) / 2147483648.0 * 2.0 - 1.0
    x = (x * 1103515245 + 12345) % 2147483648
    uy = float(x) / 2147483648.0 * 2.0 - 1.0
    if ux * ux + uy * uy <= 1.0:
        count = count + 1
print(count)
