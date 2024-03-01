package main

import "fmt"

func main() {
    var n int64 = 10000000
    var dot int64 = 0
    var i int64
    for i = 0; i < n; i++ {
        a := (i*3 + 1) % 1009
        b := (i*7 + 2) % 1013
        dot = (dot + a*b) % 1000000007
    }
    fmt.Println(dot)
}
