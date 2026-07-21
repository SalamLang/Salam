package main

import (
	"encoding/json"
	"log"
	"net/http"
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
