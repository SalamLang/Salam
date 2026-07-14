package main

import "fmt"

func main() {
	var bestSteps int64 = 0
	var bestN int64 = 1
	for n := int64(1); n <= 30000; n++ {
		m := n
		var c int64 = 0
		for m != 1 {
			if m%2 == 0 {
				m = m / 2
			} else {
				m = 3*m + 1
			}
			c++
		}
		if c > bestSteps {
			bestSteps = c
			bestN = n
		}
	}
	fmt.Println(bestSteps)
	fmt.Println(bestN)
}
