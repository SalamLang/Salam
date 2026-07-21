package main

import (
	"encoding/json"
	"errors"
	"io"
	"net/http"
	"time"
)

func decodeRunRequest(r *http.Request) (runRequest, error) {
	body, err := io.ReadAll(io.LimitReader(r.Body, maxSourceBytes+4096))
	if err != nil {
		return runRequest{}, errors.New("could not read body")
	}
	var req runRequest
	if err := json.Unmarshal(body, &req); err != nil {
		return runRequest{}, errors.New("malformed JSON")
	}
	return req, nil
}

func normalizeRunRequest(req *runRequest) {
	if req.Type == "" {
		req.Type = "program"
	}
	if req.Engine == "" {
		req.Engine = "interp"
	}
	if req.Language == "" {
		req.Language = "en"
	}
}

func resolveTimeout(requestedMs int) time.Duration {
	if requestedMs <= 0 {
		return defaultTimeout
	}
	t := time.Duration(requestedMs) * time.Millisecond
	if t > maxTimeout {
		return maxTimeout
	}
	return t
}

var (
	allowedTypes     = map[string]bool{"program": true, "layout": true}
	allowedEngines   = map[string]bool{"interp": true, "llvm": true}
	allowedLanguages = map[string]bool{"en": true, "fa": true, "ar": true}
)

type enumField struct {
	label   string
	value   string
	allowed map[string]bool
	options string
}

func firstEnumError(fields ...enumField) string {
	for _, f := range fields {
		if !f.allowed[f.value] {
			return f.label + " must be one of: " + f.options
		}
	}
	return ""
}

func validateRunRequest(req runRequest) (status int, errCode, message string) {
	if req.Code == "" {
		return http.StatusBadRequest, "invalid_request", "'code' is required"
	}
	if len(req.Code) > maxSourceBytes {
		return http.StatusRequestEntityTooLarge, "payload_too_large", ""
	}
	if msg := firstEnumError(
		enumField{"type", req.Type, allowedTypes, "program, layout"},
		enumField{"engine", req.Engine, allowedEngines, "interp, llvm"},
		enumField{"language", req.Language, allowedLanguages, "en, fa, ar"},
	); msg != "" {
		return http.StatusBadRequest, "invalid_request", msg
	}
	return 0, "", ""
}
