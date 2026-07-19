"""Matrix multiplication checksum benchmark."""
a = [[0] * 200 for _ in range(200)]
b = [[0] * 200 for _ in range(200)]
for i in range(200):
    for j in range(200):
        a[i][j] = (i * 3 + j * 7 + 1) % 97
for i in range(200):
    for j in range(200):
        b[i][j] = (i * 5 + j * 2 + 3) % 89
checksum = 0
for i in range(200):
    for j in range(200):
        c = 0
        for k in range(200):
            c = c + a[i][k] * b[k][j]
        checksum = (checksum + c) % 1000000007
print(checksum)
