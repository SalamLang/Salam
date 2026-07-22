package main

import (
	"context"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"time"
)

const errInternalError = "internal_error"

func runSalam(ctx context.Context, req runRequest, timeout time.Duration, reqID string) (runResponse, int) {
	jobDir, err := os.MkdirTemp(workRoot, "job-*")
	if err != nil {
		log.Printf("run error id=%s stage=mkdir_temp error=%q", reqID, err) // #nosec G706 -- reqID is control-char-stripped (sanitizeRequestID) and %q escapes err into a single quoted token, so neither can forge log lines
		return runResponse{OK: false, Error: errInternalError, Message: "could not allocate work dir"}, http.StatusInternalServerError
	}
	defer os.RemoveAll(jobDir)

	srcPath := filepath.Join(jobDir, "main.salam")
	if err := os.WriteFile(srcPath, []byte(req.Code), 0o600); err != nil {
		log.Printf("run error id=%s stage=write_source error=%q", reqID, err) // #nosec G706 -- see mkdir_temp above
		return runResponse{OK: false, Error: errInternalError, Message: "could not write source"}, http.StatusInternalServerError
	}

	runCtx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()

	cmd, stdoutBuf, stderrBuf := buildSalamCmd(runCtx, req, srcPath, jobDir)

	start := time.Now()
	if err := cmd.Start(); err != nil {
		log.Printf("run error id=%s stage=cmd_start error=%q", reqID, err) // #nosec G706 -- see mkdir_temp above
		return runResponse{OK: false, Error: errInternalError, Message: "could not start salam"}, http.StatusInternalServerError
	}

	waitErr := cmd.Wait()
	duration := time.Since(start)

	timedOut := runCtx.Err() == context.DeadlineExceeded
	if timedOut && cmd.Process != nil {
		if err := killProcessTree(cmd.Process.Pid); err != nil {
			log.Printf("run warning id=%s stage=kill_process_group error=%q", reqID, err)
		}
	}

	resp := buildRunResponse(waitErr, stdoutBuf, stderrBuf, duration, timedOut)

	if req.Type == reqTypeLayout {
		resp.Artifacts = collectLayoutArtifacts(jobDir)
	}

	return resp, http.StatusOK
}

func buildSalamCmd(runCtx context.Context, req runRequest, srcPath, jobDir string) (*exec.Cmd, *capBuffer, *capBuffer) {
	// req.Type, req.Engine, and req.Language are checked against fixed
	// allowlists in validateRunRequest before runSalam is ever reached, and
	// srcPath is a server-generated temp path, so buildArgs never forwards
	// unvalidated client input to the child process.
	cmd := exec.CommandContext(runCtx, salamBin, buildArgs(req, srcPath)...) // #nosec G204 -- args are allowlist-validated, see above
	cmd.Dir = jobDir
	cmd.Env = childEnv(jobDir)
	setProcessGroup(cmd)

	stdoutBuf := &capBuffer{limit: maxOutputBytes}
	stderrBuf := &capBuffer{limit: maxOutputBytes}
	cmd.Stdout = stdoutBuf
	cmd.Stderr = stderrBuf

	return cmd, stdoutBuf, stderrBuf
}

func buildRunResponse(waitErr error, stdoutBuf, stderrBuf *capBuffer, duration time.Duration, timedOut bool) runResponse {
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
	return resp
}

func buildArgs(req runRequest, srcPath string) []string {
	lang := "--lang=" + req.Language
	if req.Type == reqTypeLayout {
		return []string{reqTypeLayout, "build", srcPath, "--inline", lang}
	}
	if req.Engine == engineLLVM {
		return []string{engineLLVM, srcPath, "--jit", lang, "--log-level=warn"}
	}
	return []string{"exec", srcPath, lang, "--log-level=warn"}
}

func childEnv(jobDir string) []string {
	env := []string{"HOME=" + jobDir}
	if salamStdDir != "" {
		env = append(env, "SALAM_STD="+salamStdDir)
	}
	return append(env, platformEnv(jobDir)...)
}

func collectLayoutArtifacts(jobDir string) map[string]string {
	data, err := os.ReadFile(filepath.Join(jobDir, "main.html"))
	if err != nil {
		return nil
	}
	return map[string]string{"html": string(data)}
}
