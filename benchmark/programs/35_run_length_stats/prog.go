package main

import "fmt"

func main() {
    var n int64 = 5000000
    var totalRuns int64 = 0
    var sumsq int64 = 0
    var curval int64 = -1
    var curlen int64 = 0
    var i int64
    for i = 0; i < n; i++ {
        v := (i / 50) % 7
        if v == curval {
            curlen = curlen + 1
        } else {
            if curlen > 0 {
                totalRuns = totalRuns + 1
                sumsq = (sumsq + curlen*curlen) % 1000000007
            }
            curval = v
            curlen = 1
        }
    }
    totalRuns = totalRuns + 1
    sumsq = (sumsq + curlen*curlen) % 1000000007
    fmt.Println(totalRuns)
    fmt.Println(sumsq)
}
