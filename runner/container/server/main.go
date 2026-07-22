package main

import (
	"flag"
	"log"
	"net/http"
	"os"
	"time"
)

func main() {
	healthcheck := flag.Bool("healthcheck", false, "check that a local server instance is responding, then exit")
	flag.Parse()

	if *healthcheck {
		os.Exit(runHealthcheck())
	}

	if err := os.MkdirAll(workRoot, 0o755); err != nil {
		log.Fatalf("could not create work root %q: %v", workRoot, err)
	}

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
