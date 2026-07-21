package main

import (
	"context"
	"log"
	"net/http"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"syscall"
	"time"
)

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

	runCtx, cancel := context.WithTimeout(ctx, timeout)
	defer cancel()

	cmd, stdoutBuf, stderrBuf := buildSalamCmd(runCtx, req, srcPath, jobDir)

	start := time.Now()
	if err := cmd.Start(); err != nil {
		log.Printf("run error id=%s stage=cmd_start error=%q", reqID, err)
		return runResponse{OK: false, Error: "internal_error", Message: "could not start salam"}, http.StatusInternalServerError
	}

	waitErr := cmd.Wait()
	duration := time.Since(start)

	timedOut := runCtx.Err() == context.DeadlineExceeded
	if timedOut && cmd.Process != nil {
		// Belt-and-suspenders: kill the whole process group in case salam
		// forked children directly. With the shell removed from the image,
		// system()/popen() already fail outright rather than spawning
		// anything, so this mainly guards future FFI additions.
		_ = syscall.Kill(-cmd.Process.Pid, syscall.SIGKILL)
	}

	resp := buildRunResponse(waitErr, stdoutBuf, stderrBuf, duration, timedOut)

	if req.Type == "layout" {
		resp.Artifacts = collectLayoutArtifacts(jobDir)
	}

	return resp, http.StatusOK
}

// buildSalamCmd wires up the salam invocation: isolated env, its own
// process group (so a timeout can kill any children too), and byte-capped
// stdout/stderr capture.
func buildSalamCmd(runCtx context.Context, req runRequest, srcPath, jobDir string) (*exec.Cmd, *capBuffer, *capBuffer) {
	cmd := exec.CommandContext(runCtx, salamBin, buildArgs(req, srcPath)...)
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
	if req.Type == "layout" {
		// --inline folds generated CSS/JS into a single self-contained HTML
		// document, so the sandbox only has to read one artifact back.
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
