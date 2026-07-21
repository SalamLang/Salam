package main

import (
	"os"
	"time"
)

const (
	maxSourceBytes = 64 * 1024
	maxOutputBytes = 200 * 1024
	defaultTimeout = 10 * time.Second
	maxTimeout     = 20 * time.Second
	maxConcurrent  = 4
)

var (
	workRoot    = envOrDefault("SALAM_WORK_ROOT", defaultWorkRoot)
	salamBin    = envOrDefault("SALAM_BIN", defaultSalamBin)
	salamStdDir = envOrDefault("SALAM_STD", defaultSalamStd)
)

func envOrDefault(key, def string) string {
	if v := os.Getenv(key); v != "" {
		return v
	}
	return def
}

var salamVersion = "unknown"

var jobSlots chan struct{}

func initJobSlots() {
	jobSlots = make(chan struct{}, maxConcurrent)
	for i := 0; i < maxConcurrent; i++ {
		jobSlots <- struct{}{}
	}
}

func acquireJobSlot() bool {
	select {
	case <-jobSlots:
		return true
	default:
		return false
	}
}

func releaseJobSlot() {
	jobSlots <- struct{}{}
}
