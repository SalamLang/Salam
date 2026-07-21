//go:build windows

package main

import (
	"os"
	"os/exec"
	"strconv"
	"syscall"
)

func setProcessGroup(cmd *exec.Cmd) {
	cmd.SysProcAttr = &syscall.SysProcAttr{CreationFlags: syscall.CREATE_NEW_PROCESS_GROUP}
}

func killProcessTree(pid int) error {
	return exec.Command("taskkill", "/T", "/F", "/PID", strconv.Itoa(pid)).Run()
}

func platformEnv(jobDir string) []string {
	env := []string{
		"USERPROFILE=" + jobDir,
		"TEMP=" + jobDir,
		"TMP=" + jobDir,
	}
	for _, k := range []string{"SystemRoot", "PATH"} {
		if v := os.Getenv(k); v != "" {
			env = append(env, k+"="+v)
		}
	}
	return env
}
