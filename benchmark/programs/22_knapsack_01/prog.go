package main

import "fmt"

func main() {
    const nitems int64 = 500
    const capacity int64 = 12500
    weight := make([]int64, nitems)
    value := make([]int64, nitems)
    dp := make([]int64, capacity+1)

    for i := int64(0); i < nitems; i++ {
        weight[i] = ((i*7 + 3) % 50) + 1
        value[i] = ((i*13 + 5) % 100) + 1
    }

    for i := int64(0); i < nitems; i++ {
        wi := weight[i]
        vi := value[i]
        for w := capacity; w >= wi; w-- {
            cand := dp[w-wi] + vi
            if cand > dp[w] {
                dp[w] = cand
            }
        }
    }

    fmt.Println(dp[capacity])
}
