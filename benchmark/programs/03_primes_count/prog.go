package main

import "fmt"

func main() {
	var count int64 = 1
	for i := int64(3); i < 100000; i += 2 {
		var isP int64 = 1
		for j := int64(3); j*j <= i; j += 2 {
			if i%j == 0 {
				isP = 0
				break
			}
		}
		count += isP
	}
	fmt.Println(count)
}
