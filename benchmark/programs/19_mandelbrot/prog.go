package main

import "fmt"

func main() {
	width := 400
	height := 400
	var maxIter int64 = 100
	var sum int64 = 0
	for py := 0; py < height; py++ {
		for px := 0; px < width; px++ {
			x0 := -2.0 + 3.0*float64(px)/float64(width)
			y0 := -1.5 + 3.0*float64(py)/float64(height)
			zx := 0.0
			zy := 0.0
			var iter int64 = 0
			for zx*zx+zy*zy <= 4.0 && iter < maxIter {
				xt := zx*zx - zy*zy + x0
				zy = 2.0*zx*zy + y0
				zx = xt
				iter = iter + 1
			}
			sum = sum + iter
		}
	}
	fmt.Println(sum)
}
