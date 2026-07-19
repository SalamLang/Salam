"""binary_search_stress benchmark"""
n = 2000000
arr = [i * 2 for i in range(n)]

q = 5000000
x = 5
checksum = 0
for qi in range(q):
    x = (x * 1103515245 + 12345) % 2147483648
    target = x % (2 * n)
    lo = 0
    hi = n
    while lo < hi:
        mid = (lo + hi) // 2
        if arr[mid] < target:
            lo = mid + 1
        else:
            hi = mid
    checksum = (checksum + lo) % 1000000007

print(checksum)
