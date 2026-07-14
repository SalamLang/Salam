package main

import "fmt"

func gcd(a int64, b int64) int64 {
	x := a
	y := b
	for y != 0 {
		t := x % y
		x = y
		y = t
	}
	return x
}

func main() {
	var s int64 = 0
	for i := int64(1); i <= 300000; i++ {
		s += gcd(i, 987654321)
	}
	fmt.Println(s)
}
