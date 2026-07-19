"""sqrt decomposition point-update / prefix-sum benchmark"""
N = 2000000
BLOCK = 1414


def update(arr, blocksum, i, delta):
    arr[i] = arr[i] + delta
    b = i // BLOCK
    blocksum[b] = blocksum[b] + delta


def prefix_sum(arr, blocksum, i):
    b = i // BLOCK
    total = 0
    k = 0
    while k < b:
        total = total + blocksum[k]
        k = k + 1
    start = b * BLOCK
    while start <= i:
        total = total + arr[start]
        start = start + 1
    return total


nblocks = (N + BLOCK - 1) // BLOCK
arr = [0] * N
blocksum = [0] * nblocks

Q = 20000
x = 8
checksum = 0

for t in range(Q):
    x = (x * 1103515245 + 12345) % 2147483648
    idx = x % N
    x = (x * 1103515245 + 12345) % 2147483648
    delta = (x % 1000) + 1
    update(arr, blocksum, idx, delta)
    x = (x * 1103515245 + 12345) % 2147483648
    qi = x % N
    s = prefix_sum(arr, blocksum, qi)
    checksum = (checksum + s) % 1000000007

print(checksum)
