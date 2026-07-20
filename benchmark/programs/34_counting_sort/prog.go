package main

import "fmt"

func main() {
    var n int64 = 5000000
    var rangeN int64 = 100000
    var x int64 = 6
    counts := make([]int64, rangeN)
    var i int64
    for i = 0; i < n; i = i + 1 {
        x = (x*1103515245 + 12345) % 2147483648
        v := x % rangeN
        counts[v] = counts[v] + 1
    }

    var checksum int64 = 0
    var pos int64 = 0
    var v int64
    for v = 0; v < rangeN; v = v + 1 {
        c := counts[v]
        var t int64
        for t = 0; t < c; t = t + 1 {
            pos = pos + 1
            checksum = (checksum + (v%1000000007)*(pos%1000000007)) % 1000000007
        }
    }

    fmt.Println(checksum)
}
