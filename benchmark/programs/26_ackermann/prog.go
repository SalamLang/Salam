package main

import "fmt"

func ackermann(m int64, n int64) int64 {
	if m == 0 {
		return n + 1
	}
	if n == 0 {
		return ackermann(m-1, 1)
	}
	return ackermann(m-1, ackermann(m, n-1))
}

func main() {
	fmt.Println(ackermann(3, 6))
}
