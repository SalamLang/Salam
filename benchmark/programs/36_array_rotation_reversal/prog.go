package main

import "fmt"

func reverseRange(arr []int64, lo int, hi int) {
    for lo < hi {
        tmp := arr[lo]
        arr[lo] = arr[hi]
        arr[hi] = tmp
        lo = lo + 1
        hi = hi - 1
    }
}

func main() {
    n := 4000000
    k := 1234567 % n
    arr := make([]int64, n)
    for i := 0; i < n; i++ {
        arr[i] = int64(i)
    }
    reverseRange(arr, 0, k-1)
    reverseRange(arr, k, n-1)
    reverseRange(arr, 0, n-1)
    var checksum int64 = 0
    for i := 0; i < n; i++ {
        checksum = (checksum + (arr[i]%1000000007)*(int64(i+1)%1000000007)) % 1000000007
    }
    fmt.Println(arr[0])
    fmt.Println(arr[n-1])
    fmt.Println(checksum)
}
