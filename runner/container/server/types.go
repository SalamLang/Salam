package main

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
