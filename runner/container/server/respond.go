package main

import (
	"encoding/json"
	"log"
	"net/http"
	"time"
)

func requestID(r *http.Request) string {
	if id := r.Header.Get("X-Request-Id"); id != "" {
		return id
	}
	return "unknown"
}

func writeJSON(w http.ResponseWriter, status int, v interface{}) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	if err := json.NewEncoder(w).Encode(v); err != nil {
		log.Printf("write json error=%q", err)
	}
}

func writeError(w http.ResponseWriter, status int, reqID, errCode, message string) {
	log.Printf("reject id=%s status=%d error=%s message=%q", reqID, status, errCode, message)
	writeJSON(w, status, runResponse{OK: false, Error: errCode, Message: message, RequestID: reqID})
}

func runHealthcheck() int {
	client := http.Client{Timeout: 3 * time.Second}
	resp, err := client.Get("http://127.0.0.1:8080/healthz")
	if err != nil {
		return 1
	}
	defer resp.Body.Close()
	if resp.StatusCode != http.StatusOK {
		return 1
	}
	return 0
}
