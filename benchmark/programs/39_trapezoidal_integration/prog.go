package main

import "fmt"

func f(x float64) float64 {
	return x*x*x - 2.0*x*x + 1.0
}

func main() {
	a := 0.0
	b := 10.0
	steps := int64(10000000)
	h := (b - a) / float64(steps)
	sum := 0.5 * (f(a) + f(b))
	for i := int64(1); i < steps; i++ {
		x := a + float64(i)*h
		sum = sum + f(x)
	}
	result := sum * h
	scaled := result * 1000000.0
	fmt.Println(int64(scaled))
}
