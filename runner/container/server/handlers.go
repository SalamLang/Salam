package main

import (
	"log"
	"net/http"
)

func handleHealthz(w http.ResponseWriter, _ *http.Request) {
	w.WriteHeader(http.StatusOK)
	if _, err := w.Write([]byte("ok")); err != nil {
		log.Printf("healthz write error=%q", err)
	}
}

func handleVersion(w http.ResponseWriter, _ *http.Request) {
	writeJSON(w, http.StatusOK, map[string]string{"salamVersion": salamVersion})
}

func handleRun(w http.ResponseWriter, r *http.Request) {
	reqID := requestID(r)

	if r.Method != http.MethodPost {
		writeError(w, http.StatusMethodNotAllowed, reqID, "method_not_allowed", "")
		return
	}

	req, err := decodeRunRequest(r)
	if err != nil {
		writeError(w, http.StatusBadRequest, reqID, "invalid_request", err.Error())
		return
	}
	normalizeRunRequest(&req)

	if status, errCode, message := validateRunRequest(req); status != 0 {
		writeError(w, status, reqID, errCode, message)
		return
	}

	if !acquireJobSlot() {
		writeError(w, http.StatusTooManyRequests, reqID, "instance_busy", "this sandbox instance is at capacity")
		return
	}
	defer releaseJobSlot()

	// reqID is control-char-stripped (sanitizeRequestID); req.Type/Engine/Language
	// are enum-validated against fixed allowlists by validateRunRequest above, so
	// none of these can forge or split log lines.
	log.Printf("run start id=%s type=%s engine=%s lang=%s codeBytes=%d", reqID, req.Type, req.Engine, req.Language, len(req.Code)) // #nosec G706

	resp, status := runSalam(r.Context(), req, resolveTimeout(req.TimeoutMs), reqID)
	resp.RequestID = reqID

	// reqID is sanitized above; the remaining fields are numeric/bool, not injectable.
	log.Printf("run done id=%s status=%d exit=%d durationMs=%d timedOut=%v truncated=%v stdoutBytes=%d stderrBytes=%d",
		reqID, status, resp.ExitCode, resp.DurationMs, resp.TimedOut, resp.Truncated, len(resp.Stdout), len(resp.Stderr)) // #nosec G706

	writeJSON(w, status, resp)
}
