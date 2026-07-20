package main

import "fmt"

func main() {
    n := 2000000
    arr := make([]int64, n)
    for i := 0; i < n; i++ {
        arr[i] = int64(i) * 2
    }

    var q int64 = 5000000
    var x int64 = 5
    var checksum int64 = 0
    var qi int64
    for qi = 0; qi < q; qi++ {
        x = (x*1103515245 + 12345) % 2147483648
        target := x % (2 * int64(n))
        lo := 0
        hi := n
        for lo < hi {
            mid := (lo + hi) / 2
            if arr[mid] < target {
                lo = mid + 1
            } else {
                hi = mid
            }
        }
        checksum = (checksum + int64(lo)) % 1000000007
    }

    fmt.Println(checksum)
}
