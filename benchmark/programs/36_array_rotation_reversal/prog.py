"""array rotation via triple reversal"""


def reverse_range(arr, lo, hi):
    while lo < hi:
        tmp = arr[lo]
        arr[lo] = arr[hi]
        arr[hi] = tmp
        lo = lo + 1
        hi = hi - 1


def main():
    n = 4000000
    k = 1234567 % n
    arr = [0] * n
    for i in range(n):
        arr[i] = i
    reverse_range(arr, 0, k - 1)
    reverse_range(arr, k, n - 1)
    reverse_range(arr, 0, n - 1)
    checksum = 0
    for i in range(n):
        checksum = (checksum + (arr[i] % 1000000007) * ((i + 1) % 1000000007)) % 1000000007
    print(arr[0])
    print(arr[n - 1])
    print(checksum)


main()
