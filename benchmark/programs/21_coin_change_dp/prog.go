package main

import "fmt"

func main() {
    const amount = 100000
    const mod = 1000000007
    coins := [6]int64{1, 5, 10, 25, 50, 100}
    ways := make([]int64, amount+1)
    ways[0] = 1

    for ci := 0; ci < 6; ci++ {
        c := coins[ci]
        for a := c; a <= amount; a++ {
            ways[a] = (ways[a] + ways[a-c]) % mod
        }
    }

    fmt.Println(ways[amount])
}
