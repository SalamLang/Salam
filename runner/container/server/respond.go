package main

import (
	"encoding/json"
	"log"
	"net/http"
	"time"
)

const maxRequestIDLen = 128

// requestID reads the client-supplied X-Request-Id header and restricts it to a
// safe charset so it can't be used to inject fake entries into the server log.
func requestID(r *http.Request) string {
	id := r.Header.Get("X-Request-Id")
	if id == "" || !isSafeRequestID(id) {
		return "unknown"
	}
	return id
}

func isSafeRequestID(id string) bool {
	if len(id) > maxRequestIDLen {
		return false
	}
	for _, c := range id {
		safe := (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') ||
			c == '-' || c == '_' || c == '.'
		if !safe {
			return false
		}
	}
	return true
}

func writeJSON(w http.ResponseWriter, status int, v interface{}) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	if err := json.NewEncoder(w).Encode(v); err != nil {
		log.Printf("write json error=%q", err)
	}
}

func writeError(w http.ResponseWriter, status int, reqID, errCode, message string) {
	// #nosec G706 -- reqID is restricted to a safe charset by requestID(), errCode is
	// always a hardcoded literal from call sites, and message is logged with %q, which
	// escapes newlines/control characters, preventing log injection.
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
