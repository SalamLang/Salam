// Reference HTTP and HTTPS server on Go's net/http for benchmark/https/run.sh.
package main

import (
	"net/http"
	"os"
	"strings"
)

func main() {
	body := []byte(strings.Repeat("x", 102400))
	mux := http.NewServeMux()
	mux.HandleFunc("/json", func(w http.ResponseWriter, r *http.Request) {
		w.Header().Set("Content-Type", "application/json")
		w.Write([]byte(`{"message":"Hello, World!"}`))
	})
	mux.HandleFunc("/100k", func(w http.ResponseWriter, r *http.Request) { w.Write(body) })
	mux.Handle("/s/", http.StripPrefix("/s/", http.FileServer(http.Dir("."))))
	if os.Args[1] == "https" {
		http.ListenAndServeTLS(":"+os.Args[2], os.Args[3], os.Args[4], mux)
	} else {
		http.ListenAndServe(":"+os.Args[2], mux)
	}
}
