package main

import "fmt"

func main() {
	var a int64 = 0
	var b int64 = 1
	for i := int64(0); i < 5000000; i++ {
		t := (a + b) % 1000000007
		a = b
		b = t
	}
	fmt.Println(a)
}
