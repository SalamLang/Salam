package main

import "fmt"

const N = 12

var cols [N]int
var diag1 [2*N - 1]int
var diag2 [2*N - 1]int
var solutions int64 = 0

func solve(row int) {
    if row == N {
        solutions = solutions + 1
        return
    }
    for col := 0; col < N; col++ {
        d1 := row + col
        d2 := row - col + N - 1
        if cols[col] == 0 && diag1[d1] == 0 && diag2[d2] == 0 {
            cols[col] = 1
            diag1[d1] = 1
            diag2[d2] = 1
            solve(row + 1)
            cols[col] = 0
            diag1[d1] = 0
            diag2[d2] = 0
        }
    }
}

func main() {
    solve(0)
    fmt.Println(solutions)
}
