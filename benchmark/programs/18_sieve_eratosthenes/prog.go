package main

import "fmt"

func main() {
	var limit int64 = 3000000
	isComposite := make([]int8, limit+1)
	isComposite[0] = 1
	isComposite[1] = 1
	var i int64 = 2
	for i*i <= limit {
		if isComposite[i] == 0 {
			var j int64 = i * i
			for j <= limit {
				isComposite[j] = 1
				j = j + i
			}
		}
		i = i + 1
	}
	var count int64 = 0
	var k int64 = 2
	for k <= limit {
		if isComposite[k] == 0 {
			count = count + 1
		}
		k = k + 1
	}
	fmt.Println(count)
}
