package main

import "fmt"

func main() {
    var a [200][200]int
    var b [200][200]int
    var checksum int64 = 0
    for i := 0; i < 200; i++ {
        for j := 0; j < 200; j++ {
            a[i][j] = (i*3 + j*7 + 1) % 97
        }
    }
    for i := 0; i < 200; i++ {
        for j := 0; j < 200; j++ {
            b[i][j] = (i*5 + j*2 + 3) % 89
        }
    }
    for i := 0; i < 200; i++ {
        for j := 0; j < 200; j++ {
            var c int64 = 0
            for k := 0; k < 200; k++ {
                c = c + int64(a[i][k])*int64(b[k][j])
            }
            checksum = (checksum + c) % 1000000007
        }
    }
    fmt.Println(checksum)
}
