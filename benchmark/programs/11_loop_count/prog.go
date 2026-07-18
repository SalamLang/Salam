package main

import "fmt"

func main() {
	var acc int64 = 0
	lens := []int64{1, 10, 100, 1000, 10000, 100000, 1000000}
	for _, L := range lens {
		for i := int64(1); i <= L; i++ {
			acc = (acc + i) % 1000000007
		}
	}
	fmt.Println(acc)
}
