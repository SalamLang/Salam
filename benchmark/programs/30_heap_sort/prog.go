package main

import "fmt"

func heapify(arr []int64, n int64, i int64) {
    largest := i
    l := 2*i + 1
    r := 2*i + 2
    if l < n && arr[l] > arr[largest] {
        largest = l
    }
    if r < n && arr[r] > arr[largest] {
        largest = r
    }
    if largest != i {
        arr[i], arr[largest] = arr[largest], arr[i]
        heapify(arr, n, largest)
    }
}

func main() {
    var n int64 = 300000
    var x int64 = 3
    arr := make([]int64, n)
    var i int64
    for i = 0; i < n; i++ {
        x = (x*1103515245 + 12345) % 2147483648
        arr[i] = x
    }
    for i = n/2 - 1; i >= 0; i-- {
        heapify(arr, n, i)
    }
    for end := n - 1; end >= 1; end-- {
        arr[0], arr[end] = arr[end], arr[0]
        heapify(arr, end, 0)
    }
    var checksum int64
    for i = 0; i < n; i++ {
        checksum = (checksum + (arr[i]%1000000007)*((i+1)%1000000007)) % 1000000007
    }
    fmt.Println(arr[0])
    fmt.Println(arr[n-1])
    fmt.Println(checksum)
}
