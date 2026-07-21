package main

import "time"

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
