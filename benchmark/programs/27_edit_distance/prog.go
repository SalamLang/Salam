package main

import "fmt"

func main() {
    var LEN1 int64 = 2000
    var LEN2 int64 = 2000

    seq1 := make([]int64, LEN1)
    seq2 := make([]int64, LEN2)
    prev := make([]int64, LEN2+1)
    cur := make([]int64, LEN2+1)

    var i int64
    for i = 0; i < LEN1; i = i + 1 {
        seq1[i] = (i*31 + 7) % 26
    }

    var j int64
    for j = 0; j < LEN2; j = j + 1 {
        seq2[j] = (j*17 + 11) % 26
    }

    for j = 0; j <= LEN2; j = j + 1 {
        prev[j] = j
    }

    for i = 1; i <= LEN1; i = i + 1 {
        cur[0] = i
        for j = 1; j <= LEN2; j = j + 1 {
            var cost int64
            if seq1[i-1] == seq2[j-1] {
                cost = 0
            } else {
                cost = 1
            }
            delCost := prev[j] + 1
            insCost := cur[j-1] + 1
            subCost := prev[j-1] + cost
            m := delCost
            if insCost < m {
                m = insCost
            }
            if subCost < m {
                m = subCost
            }
            cur[j] = m
        }
        for j = 0; j <= LEN2; j = j + 1 {
            prev[j] = cur[j]
        }
    }

    fmt.Println(prev[LEN2])
}
