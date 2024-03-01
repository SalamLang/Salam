#!/usr/bin/env python3
def heapify(arr, n, i):
    largest = i
    left = 2 * i + 1
    right = 2 * i + 2
    if left < n and arr[left] > arr[largest]:
        largest = left
    if right < n and arr[right] > arr[largest]:
        largest = right
    if largest != i:
        arr[i], arr[largest] = arr[largest], arr[i]
        heapify(arr, n, largest)


def main():
    n = 300000
    x = 3
    arr = [0] * n
    for i in range(n):
        x = (x * 1103515245 + 12345) % 2147483648
        arr[i] = x
    for i in range(n // 2 - 1, -1, -1):
        heapify(arr, n, i)
    for end in range(n - 1, 0, -1):
        arr[0], arr[end] = arr[end], arr[0]
        heapify(arr, end, 0)
    checksum = 0
    for i in range(n):
        checksum = (
            checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)
        ) % 1000000007
    print(arr[0])
    print(arr[n - 1])
    print(checksum)


main()
