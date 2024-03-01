package main

import (
	"bufio"
	"os"
)

func main() {
	w := bufio.NewWriter(os.Stdout)
	defer w.Flush()
	for i := 0; i < 1000000; i++ {
		w.WriteString("hello\n")
	}
}
