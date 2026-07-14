package main

import "fmt"

func powmod(base int64, e int64, m int64) int64 {
	var result int64 = 1
	b := base % m
	ex := e
	for ex > 0 {
		if ex%2 == 1 {
			result = (result * b) % m
		}
		b = (b * b) % m
		ex = ex / 2
	}
	return result
}

func main() {
	var s int64 = 0
	for i := int64(1); i <= 30000; i++ {
		s = (s + powmod(i, 65537, 1000000007)) % 1000000007
	}
	fmt.Println(s)
}
