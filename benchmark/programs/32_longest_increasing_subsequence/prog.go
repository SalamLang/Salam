package main

import "fmt"

func main() {
    const n int64 = 6000
    seq := make([]int64, n)
    dp := make([]int64, n)

    for i := int64(0); i < n; i++ {
        seq[i] = (i*37 + 11) % 10007
    }

    for i := int64(0); i < n; i++ {
        dp[i] = 1
    }

    for i := int64(0); i < n; i++ {
        for j := int64(0); j < i; j++ {
            if seq[j] < seq[i] {
                if dp[j]+1 > dp[i] {
                    dp[i] = dp[j] + 1
                }
            }
        }
    }

    answer := dp[0]
    for i := int64(0); i < n; i++ {
        if dp[i] > answer {
            answer = dp[i]
        }
    }

    fmt.Println(answer)
}
