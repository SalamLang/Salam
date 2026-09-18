package main

import "fmt"

func main() {
    reps := 200000
    var checksum int64 = 0
    for r := 0; r < reps; r++ {
        var a [4][4]float64
        for i := 0; i < 4; i++ {
            for j := 0; j < 4; j++ {
                a[i][j] = float64((r + i*3 + j*7 + 1) % 97)
            }
        }
        var c [4][4]float64
        for i := 0; i < 4; i++ {
            for j := 0; j < 4; j++ {
                s := 0.0
                for k := 0; k < 4; k++ {
                    s += a[i][k] * a[j][k]
                }
                c[i][j] = s
            }
        }
        tr := 0.0
        for i := 0; i < 4; i++ {
            tr += c[i][i]
        }
        checksum = (checksum + int64(tr)) % 1000000007
    }
    fmt.Println(checksum)
}
