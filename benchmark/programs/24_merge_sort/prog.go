package main

import "fmt"

const N = 300000

var arr [N]int64
var aux [N]int64

func mergesort(lo int64, hi int64) {
    if lo >= hi {
        return
    }
    mid := (lo + hi) / 2
    mergesort(lo, mid)
    mergesort(mid+1, hi)
    i := lo
    j := mid + 1
    k := lo
    for i <= mid && j <= hi {
        if arr[i] <= arr[j] {
            aux[k] = arr[i]
            i = i + 1
        } else {
            aux[k] = arr[j]
            j = j + 1
        }
        k = k + 1
    }
    for i <= mid {
        aux[k] = arr[i]
        i = i + 1
        k = k + 1
    }
    for j <= hi {
        aux[k] = arr[j]
        j = j + 1
        k = k + 1
    }
    for m := lo; m <= hi; m++ {
        arr[m] = aux[m]
    }
}

func main() {
    var x int64 = 2
    var checksum int64 = 0

    for i := int64(0); i < N; i++ {
        x = (x*1103515245 + 12345) % 2147483648
        arr[i] = x
    }

    mergesort(0, N-1)

    for i := int64(0); i < N; i++ {
        checksum = (checksum + (arr[i]%1000000007)*((i+1)%1000000007)) % 1000000007
    }

    fmt.Println(arr[0])
    fmt.Println(arr[N-1])
    fmt.Println(checksum)
}
