package main

import "fmt"

func main() {
	var limit int64 = 2000000
	var total int64 = 0
	for n := int64(2); n <= limit; n++ {
		temp := n
		d := int64(2)
		for d*d <= temp {
			for temp%d == 0 {
				total += 1
				temp /= d
			}
			d += 1
		}
		if temp > 1 {
			total += 1
		}
	}
	fmt.Println(total)
}
