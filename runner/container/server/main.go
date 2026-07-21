package main

import (
	"log"
	"net/http"
	"time"
)

func main() {
	initJobSlots()

	mux := http.NewServeMux()
	mux.HandleFunc("/healthz", handleHealthz)
	mux.HandleFunc("/version", handleVersion)
	mux.HandleFunc("/run", handleRun)

	srv := &http.Server{
		Addr:              ":8080",
		Handler:           mux,
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       15 * time.Second,
		WriteTimeout:      maxTimeout + 15*time.Second,
		IdleTimeout:       30 * time.Second,
	}

	log.Printf("salam sandbox server listening on %s (salam %s)", srv.Addr, salamVersion)
	if err := srv.ListenAndServe(); err != nil {
		log.Fatal(err)
	}
}
