package main

import "fmt"

func main() {
	var x int64 = 1
	var count int64 = 0
	for step := int64(0); step < 5000000; step++ {
		x = (x*1103515245 + 12345) % 2147483648
		ux := float64(x) / 2147483648.0 * 2.0 - 1.0
		x = (x*1103515245 + 12345) % 2147483648
		uy := float64(x) / 2147483648.0 * 2.0 - 1.0
		if ux*ux+uy*uy <= 1.0 {
			count = count + 1
		}
	}
	fmt.Println(count)
}
