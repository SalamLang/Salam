package main

import "fmt"

func main() {
	var s int64 = 0
	for i := int64(1); i <= 3000000; i++ {
		s = (s + i*i) % 1000000007
	}
	fmt.Println(s)
}
