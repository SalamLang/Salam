//go:build unix

package main

import (
	"os/exec"
	"syscall"
)

func setProcessGroup(cmd *exec.Cmd) {
	cmd.SysProcAttr = &syscall.SysProcAttr{Setpgid: true}
}

func killProcessTree(pid int) error {
	return syscall.Kill(-pid, syscall.SIGKILL)
}

func platformEnv(jobDir string) []string {
	return []string{"TMPDIR=" + jobDir}
}
