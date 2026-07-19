package main

import "fmt"

const N = 300000

func quicksort(a []int64, lo int64, hi int64) {
	if lo >= hi {
		return
	}
	pivot := a[(lo+hi)/2]
	i := lo
	j := hi
	for i <= j {
		for a[i] < pivot {
			i = i + 1
		}
		for a[j] > pivot {
			j = j - 1
		}
		if i <= j {
			a[i], a[j] = a[j], a[i]
			i = i + 1
			j = j - 1
		}
	}
	quicksort(a, lo, j)
	quicksort(a, i, hi)
}

func main() {
	arr := make([]int64, N)
	var x int64 = 1
	for i := int64(0); i < N; i++ {
		x = (x*1103515245 + 12345) % 2147483648
		arr[i] = x
	}
	quicksort(arr, 0, N-1)
	var checksum int64 = 0
	for i := int64(0); i < N; i++ {
		checksum = (checksum + (arr[i]%1000000007)*((i+1)%1000000007)) % 1000000007
	}
	fmt.Println(arr[0])
	fmt.Println(arr[N-1])
	fmt.Println(checksum)
}
