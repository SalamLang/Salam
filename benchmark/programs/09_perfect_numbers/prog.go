package main

import "fmt"

func main() {
	var count int64 = 0
	var total int64 = 0
	for n := int64(2); n <= 30000; n++ {
		var s int64 = 1
		for j := int64(2); j*j <= n; j++ {
			if n%j == 0 {
				s += j
				q := n / j
				if q != j {
					s += q
				}
			}
		}
		if s == n {
			count++
			total += n
		}
	}
	fmt.Println(count)
	fmt.Println(total)
}
