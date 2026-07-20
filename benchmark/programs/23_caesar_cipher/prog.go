package main

import "fmt"

func main() {
    var length int64 = 3000000
    var shift int64 = 13
    var x int64 = 1
    var checksum int64 = 0
    var step int64
    for step = 0; step < length; step++ {
        x = (x*1103515245 + 12345) % 2147483648
        letter := x % 26
        shifted := (letter + shift) % 26
        checksum = (checksum + shifted) % 1000000007
    }
    fmt.Println(checksum)
}
