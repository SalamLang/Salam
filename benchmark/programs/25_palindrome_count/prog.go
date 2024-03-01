package main

import "fmt"

func main() {
	var count int64 = 0
	for n := 1; n <= 2000000; n++ {
		temp := n
		var rev int64 = 0
		for temp > 0 {
			rev = rev*10 + int64(temp%10)
			temp = temp / 10
		}
		if rev == int64(n) {
			count = count + 1
		}
	}
	fmt.Println(count)
}
