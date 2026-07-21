package main

import (
	"bytes"
	"context"
	"encoding/json"
	"io"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"syscall"
	"time"
)

const (
	maxSourceBytes = 64 * 1024
	maxOutputBytes = 200 * 1024
	defaultTimeout = 10 * time.Second
	maxTimeout     = 20 * time.Second
	workRoot       = "/work"
	maxConcurrent  = 4
	salamBin       = "/opt/salam/salam"
)

var salamVersion = "unknown"

var jobSlots = make(chan struct{}, maxConcurrent)

type runRequest struct {
	Code      string `json:"code"`
	Type      string `json:"type"`     // "program" | "layout"
	Engine    string `json:"engine"`   // "interp" | "llvm"
	Language  string `json:"language"` // "en" | "fa" | "ar"
	TimeoutMs int    `json:"timeoutMs"`
}

type runResponse struct {
	OK         bool              `json:"ok"`
	Stdout     string            `json:"stdout,omitempty"`
	Stderr     string            `json:"stderr,omitempty"`
	ExitCode   int               `json:"exitCode"`
	DurationMs int64             `json:"durationMs"`
	Truncated  bool              `json:"truncated"`
	TimedOut   bool              `json:"timedOut"`
	Artifacts  map[string]string `json:"artifacts,omitempty"`
	Error      string            `json:"error,omitempty"`
	Message    string            `json:"message,omitempty"`
	RequestID  string            `json:"requestId,omitempty"`
}

func requestID(r *http.Request) string {
	if id := r.Header.Get("X-Request-Id"); id != "" {
		return id
	}
	return "unknown"
}

func writeError(w http.ResponseWriter, status int, reqID, errCode, message string) {
	log.Printf("reject id=%s status=%d error=%s message=%q", reqID, status, errCode, message)
	writeJSON(w, status, runResponse{OK: false, Error: errCode, Message: message, RequestID: reqID})
}

func main() {
	for i := 0; i < maxConcurrent; i++ {
		jobSlots <- struct{}{}
	}

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

func handleHealthz(w http.ResponseWriter, _ *http.Request) {
	w.WriteHeader(http.StatusOK)
	_, _ = w.Write([]byte("ok"))
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

	body, err := io.ReadAll(io.LimitReader(r.Body, maxSourceBytes+4096))
	if err != nil {
		writeError(w, http.StatusBadRequest, reqID, "invalid_request", "could not read body")
		return
	}

	var req runRequest
	if err := json.Unmarshal(body, &req); err != nil {
		writeError(w, http.StatusBadRequest, reqID, "invalid_request", "malformed JSON")
		return
	}

	if req.Type == "" {
		req.Type = "program"
	}
	if req.Engine == "" {
		req.Engine = "interp"
	}
	if req.Language == "" {
		req.Language = "en"
	}

	if req.Code == "" {
		writeError(w, http.StatusBadRequest, reqID, "invalid_request", "'code' is required")
		return
	}
	if len(req.Code) > maxSourceBytes {
		writeError(w, http.StatusRequestEntityTooLarge, reqID, "payload_too_large", "")
		return
	}
	if req.Type != "program" && req.Type != "layout" {
		writeError(w, http.StatusBadRequest, reqID, "invalid_request", "type must be 'program' or 'layout'")
		return
	}
	if req.Engine != "interp" && req.Engine != "llvm" {
		writeError(w, http.StatusBadRequest, reqID, "invalid_request", "engine must be 'interp' or 'llvm'")
		return
	}
	if req.Language != "en" && req.Language != "fa" && req.Language != "ar" {
		writeError(w, http.StatusBadRequest, reqID, "invalid_request", "language must be 'en', 'fa', or 'ar'")
		return
	}

	timeout := defaultTimeout
	if req.TimeoutMs > 0 {
		timeout = time.Duration(req.TimeoutMs) * time.Millisecond
		if timeout > maxTimeout {
			timeout = maxTimeout
		}
	}

	select {
	case <-jobSlots:
	default:
		writeError(w, http.StatusTooManyRequests, reqID, "instance_busy", "this sandbox instance is at capacity")
		return
	}
	defer func() { jobSlots <- struct{}{} }()

	log.Printf("run start id=%s type=%s engine=%s lang=%s codeBytes=%d", reqID, req.Type, req.Engine, req.Language, len(req.Code))

	resp, status := runSalam(r.Context(), req, timeout, reqID)
	resp.RequestID = reqID

	log.Printf("run done id=%s status=%d exit=%d durationMs=%d timedOut=%v truncated=%v stdoutBytes=%d stderrBytes=%d",
		reqID, status, resp.ExitCode, resp.DurationMs, resp.TimedOut, resp.Truncated, len(resp.Stdout), len(resp.Stderr))

	writeJSON(w, status, resp)
}

func runSalam(ctx context.Context, req runRequest, timeout time.Duration, reqID string) (runResponse, int) {
	jobDir, err := os.MkdirTemp(workRoot, "job-*")
	if err != nil {
		log.Printf("run error id=%s stage=mkdir_temp error=%q", reqID, err)
		return runResponse{OK: false, Error: "internal_error", Message: "could not allocate work dir"}, http.StatusInternalServerError
	}
	defer os.RemoveAll(jobDir)

	srcPath := filepath.Join(jobDir, "main.salam")
	if err := os.WriteFile(srcPath, []byte(req.Code), 0o600); err != nil {
		log.Printf("run error id=%s stage=write_source error=%q", reqID, err)
		return runResponse{OK: false, Error: "internal_error", Message: "could not write source"}, http.StatusInternalServerError
	}

	args := buildArgs(req, srcPath)

	runCtx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()

	cmd := exec.CommandContext(runCtx, salamBin, args...)
	cmd.Dir = jobDir
	cmd.Env = []string{
		"SALAM_STD=/opt/salam/std",
		"HOME=" + jobDir,
		"TMPDIR=" + jobDir,
	}
	cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}

	stdoutBuf := &capBuffer{limit: maxOutputBytes}
	stderrBuf := &capBuffer{limit: maxOutputBytes}
	cmd.Stdout = stdoutBuf
	cmd.Stderr = stderrBuf

	start := time.Now()
	startErr := cmd.Start()
	if startErr != nil {
		log.Printf("run error id=%s stage=cmd_start error=%q", reqID, startErr)
		return runResponse{OK: false, Error: "internal_error", Message: "could not start salam"}, http.StatusInternalServerError
	}

	waitErr := cmd.Wait()
	duration := time.Since(start)

	timedOut := runCtx.Err() == context.DeadlineExceeded
	if timedOut && cmd.Process != nil {
		_ = syscall.Kill(-cmd.Process.Pid, syscall.SIGKILL)
	}

	exitCode := 0
	if waitErr != nil {
		if exitErr, ok := waitErr.(*exec.ExitError); ok {
			exitCode = exitErr.ExitCode()
		} else {
			exitCode = -1
		}
	}

	resp := runResponse{
		OK:         true,
		Stdout:     stdoutBuf.String(),
		Stderr:     stderrBuf.String(),
		ExitCode:   exitCode,
		DurationMs: duration.Milliseconds(),
		Truncated:  stdoutBuf.truncated || stderrBuf.truncated,
		TimedOut:   timedOut,
	}
	if timedOut {
		resp.Stderr = strings.TrimSpace(resp.Stderr + "\n[salam-runner] execution timed out and was terminated")
	}

	if req.Type == "layout" {
		resp.Artifacts = collectLayoutArtifacts(jobDir)
	}

	return resp, http.StatusOK
}

func buildArgs(req runRequest, srcPath string) []string {
	lang := "--lang=" + req.Language
	if req.Type == "layout" {
		return []string{"layout", "build", srcPath, "--inline", lang}
	}
	if req.Engine == "llvm" {
		return []string{"llvm", srcPath, "--jit", lang, "--log-level=warn"}
	}
	return []string{"exec", srcPath, lang, "--log-level=warn"}
}

func collectLayoutArtifacts(jobDir string) map[string]string {
	data, err := os.ReadFile(filepath.Join(jobDir, "main.html"))
	if err != nil {
		return nil
	}
	return map[string]string{"html": string(data)}
}

type capBuffer struct {
	buf       bytes.Buffer
	limit     int
	truncated bool
}

func (c *capBuffer) Write(p []byte) (int, error) {
	if c.truncated {
		return len(p), nil
	}
	remaining := c.limit - c.buf.Len()
	if remaining <= 0 {
		c.truncated = true
		return len(p), nil
	}
	if len(p) > remaining {
		c.buf.Write(p[:remaining])
		c.truncated = true
		return len(p), nil
	}
	c.buf.Write(p)
	return len(p), nil
}

func (c *capBuffer) String() string { return c.buf.String() }

func writeJSON(w http.ResponseWriter, status int, v interface{}) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(v)
}
