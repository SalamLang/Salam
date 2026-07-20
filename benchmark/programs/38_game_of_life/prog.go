package main

import "fmt"

const W = 150
const H = 150
const GENERATIONS = 150

func main() {
    grid := make([]int, W*H)
    nextGrid := make([]int, W*H)

    for r := 0; r < H; r++ {
        for c := 0; c < W; c++ {
            if (r*31+c*17+7)%5 == 0 {
                grid[r*W+c] = 1
            } else {
                grid[r*W+c] = 0
            }
        }
    }

    for gen := 0; gen < GENERATIONS; gen++ {
        for r := 0; r < H; r++ {
            for c := 0; c < W; c++ {
                count := 0
                for dr := -1; dr <= 1; dr++ {
                    for dc := -1; dc <= 1; dc++ {
                        if dr == 0 && dc == 0 {
                            continue
                        }
                        nr := r + dr
                        nc := c + dc
                        if nr >= 0 && nr < H && nc >= 0 && nc < W {
                            count = count + grid[nr*W+nc]
                        }
                    }
                }
                alive := grid[r*W+c]
                if alive == 1 {
                    if count == 2 || count == 3 {
                        nextGrid[r*W+c] = 1
                    } else {
                        nextGrid[r*W+c] = 0
                    }
                } else {
                    if count == 3 {
                        nextGrid[r*W+c] = 1
                    } else {
                        nextGrid[r*W+c] = 0
                    }
                }
            }
        }
        grid, nextGrid = nextGrid, grid
    }

    var liveCount int64 = 0
    for i := 0; i < W*H; i++ {
        liveCount = liveCount + int64(grid[i])
    }
    fmt.Println(liveCount)
}
