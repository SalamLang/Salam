package main

import "fmt"

func main() {
	sum := 0.0
	sign := 1.0
	den := 1.0
	for k := int64(0); k < 3000000; k++ {
		sum = sum + sign/den
		sign = 0.0 - sign
		den = den + 2.0
	}
	pi := 4.0 * sum
	v := pi * 1000000.0
	fmt.Println(int64(v))
}
