"""Quicksort benchmark."""

import sys

sys.setrecursionlimit(10000)

N = 300000


def quicksort(a, lo, hi):
    if lo >= hi:
        return
    pivot = a[(lo + hi) // 2]
    i = lo
    j = hi
    while i <= j:
        while a[i] < pivot:
            i = i + 1
        while a[j] > pivot:
            j = j - 1
        if i <= j:
            a[i], a[j] = a[j], a[i]
            i = i + 1
            j = j - 1
    quicksort(a, lo, j)
    quicksort(a, i, hi)


arr = [0] * N
x = 1
for i in range(N):
    x = (x * 1103515245 + 12345) % 2147483648
    arr[i] = x

quicksort(arr, 0, N - 1)

checksum = 0
for i in range(N):
    checksum = (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007

print(arr[0])
print(arr[N - 1])
print(checksum)
