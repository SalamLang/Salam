//go:build windows

package main

import (
	"os"
	"path/filepath"
)

var (
	defaultWorkRoot = filepath.Join(os.TempDir(), "salam-work")
	defaultSalamStd = ""
)

const defaultSalamBin = "salam.exe"
