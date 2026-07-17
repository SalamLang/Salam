package main

import "fmt"

func main() {
	var x int64 = 1
	var sum int64 = 0
	for i := int64(0); i < 5000000; i++ {
		x = (x*1103515245 + 12345) % 2147483648
		sum = (sum + x) % 1000000007
	}
	fmt.Println(sum)
}
