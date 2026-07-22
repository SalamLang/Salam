#!/usr/bin/env python3
import sys

sys.setrecursionlimit(10000)

N = 300000


def mergesort(arr, aux, lo, hi):
    if lo >= hi:
        return
    mid = (lo + hi) // 2
    mergesort(arr, aux, lo, mid)
    mergesort(arr, aux, mid + 1, hi)
    i = lo
    j = mid + 1
    k = lo
    while i <= mid and j <= hi:
        if arr[i] <= arr[j]:
            aux[k] = arr[i]
            i = i + 1
        else:
            aux[k] = arr[j]
            j = j + 1
        k = k + 1
    while i <= mid:
        aux[k] = arr[i]
        i = i + 1
        k = k + 1
    while j <= hi:
        aux[k] = arr[j]
        j = j + 1
        k = k + 1
    for m in range(lo, hi + 1):
        arr[m] = aux[m]


def main():
    x = 2
    arr = [0] * N
    aux = [0] * N

    for i in range(N):
        x = (x * 1103515245 + 12345) % 2147483648
        arr[i] = x

    mergesort(arr, aux, 0, N - 1)

    checksum = 0
    for i in range(N):
        checksum = (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007

    print(arr[0])
    print(arr[N - 1])
    print(checksum)


main()
