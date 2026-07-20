package main

import "fmt"

func main() {
    const NITEMS = 1000
    const TARGET = 30000
    weight := make([]int, NITEMS)
    reachable := make([]int, TARGET+1)

    for i := 0; i < NITEMS; i++ {
        weight[i] = ((i*11 + 5) % 40) + 1
    }

    reachable[0] = 1

    for i := 0; i < NITEMS; i++ {
        w := weight[i]
        for s := TARGET; s >= w; s-- {
            if reachable[s-w] == 1 {
                reachable[s] = 1
            }
        }
    }

    var count int64 = 0
    for s := 0; s <= TARGET; s++ {
        if reachable[s] == 1 {
            count = count + 1
        }
    }

    fmt.Println(count)
}
