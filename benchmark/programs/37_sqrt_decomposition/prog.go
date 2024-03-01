package main

import "fmt"

const N = 2000000
const BLOCK = 1414

func update(arr []int64, blocksum []int64, i int, delta int64) {
    arr[i] = arr[i] + delta
    b := i / BLOCK
    blocksum[b] = blocksum[b] + delta
}

func prefixSum(arr []int64, blocksum []int64, i int) int64 {
    b := i / BLOCK
    var total int64 = 0
    k := 0
    for k < b {
        total = total + blocksum[k]
        k = k + 1
    }
    start := b * BLOCK
    for start <= i {
        total = total + arr[start]
        start = start + 1
    }
    return total
}

func main() {
    nblocks := (N + BLOCK - 1) / BLOCK
    arr := make([]int64, N)
    blocksum := make([]int64, nblocks)

    Q := 20000
    var x int64 = 8
    var checksum int64 = 0

    for t := 0; t < Q; t = t + 1 {
        x = (x*1103515245 + 12345) % 2147483648
        idx := int(x % N)
        x = (x*1103515245 + 12345) % 2147483648
        delta := (x % 1000) + 1
        update(arr, blocksum, idx, delta)
        x = (x*1103515245 + 12345) % 2147483648
        qi := int(x % N)
        s := prefixSum(arr, blocksum, qi)
        checksum = (checksum + s) % 1000000007
    }

    fmt.Println(checksum)
}
