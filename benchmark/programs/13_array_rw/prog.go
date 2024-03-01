package main

import "fmt"

func main() {
	const N = 1000000
	arr := make([]int64, N)
	for i := int64(0); i < N; i++ {
		arr[i] = (i*i + 3*i + 7) % 1000000007
	}
	var sum int64 = 0
	for pass := int64(0); pass < 5; pass++ {
		for i := 0; i < N; i++ {
			sum = (sum + arr[i] + pass) % 1000000007
		}
	}
	fmt.Println(sum)
}
