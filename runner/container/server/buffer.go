package main

import "bytes"

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
