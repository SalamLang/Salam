package main

import "fmt"

func main() {
	var s int64 = 0
	for i := int64(1); i <= 500000; i++ {
		m := i
		for m > 0 {
			s += m % 10
			m = m / 10
		}
	}
	fmt.Println(s)
}
