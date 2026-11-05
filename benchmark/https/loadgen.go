// HTTP and HTTPS load generator for benchmark/https/run.sh.
package main

import (
	"bufio"
	"crypto/tls"
	"flag"
	"fmt"
	"io"
	"net"
	"sort"
	"strconv"
	"strings"
	"sync"
	"sync/atomic"
	"time"
)

func readResponse(r *bufio.Reader) (int, error) {
	length := -1
	for {
		line, err := r.ReadString('\n')
		if err != nil {
			return 0, err
		}
		if line == "\r\n" {
			break
		}
		if len(line) > 16 && strings.EqualFold(line[:15], "content-length:") {
			length, _ = strconv.Atoi(strings.TrimSpace(line[15:]))
		}
	}
	if length < 0 {
		return 0, fmt.Errorf("no content-length")
	}
	n, err := io.CopyN(io.Discard, r, int64(length))
	return int(n), err
}

func main() {
	addr := flag.String("addr", "127.0.0.1:8443", "host:port")
	path := flag.String("path", "/json", "request path")
	secure := flag.Bool("tls", true, "use TLS")
	maxVer := flag.Int("tlsver", 13, "13 or 12")
	keep := flag.Bool("keep", true, "reuse connections")
	conns := flag.Int("c", 16, "concurrent clients")
	secs := flag.Float64("d", 3, "seconds")
	flag.Parse()

	cfg := &tls.Config{InsecureSkipVerify: true, MinVersion: tls.VersionTLS12, MaxVersion: tls.VersionTLS13}
	if *maxVer == 12 {
		cfg.MaxVersion = tls.VersionTLS12
	}
	connHdr := "keep-alive"
	if !*keep {
		connHdr = "close"
	}
	req := []byte("GET " + *path + " HTTP/1.1\r\nHost: bench\r\nConnection: " + connHdr + "\r\n\r\n")

	dial := func() (net.Conn, error) {
		c, err := net.DialTimeout("tcp", *addr, 10*time.Second)
		if err != nil {
			return nil, err
		}
		if *secure {
			t := tls.Client(c, cfg)
			if err := t.Handshake(); err != nil {
				c.Close()
				return nil, err
			}
			return t, nil
		}
		return c, nil
	}

	var total, errs, bytes atomic.Int64
	var mu sync.Mutex
	var lat []time.Duration
	deadline := time.Now().Add(time.Duration(*secs * float64(time.Second)))
	var wg sync.WaitGroup
	for i := 0; i < *conns; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			local := make([]time.Duration, 0, 4096)
			var c net.Conn
			var r *bufio.Reader
			for time.Now().Before(deadline) {
				t0 := time.Now()
				if c == nil {
					var err error
					if c, err = dial(); err != nil {
						errs.Add(1)
						time.Sleep(time.Millisecond)
						continue
					}
					r = bufio.NewReaderSize(c, 65536)
				}
				_, err := c.Write(req)
				var n int
				if err == nil {
					n, err = readResponse(r)
				}
				if err != nil {
					errs.Add(1)
					c.Close()
					c = nil
					continue
				}
				total.Add(1)
				bytes.Add(int64(n))
				local = append(local, time.Since(t0))
				if !*keep {
					c.Close()
					c = nil
				}
			}
			if c != nil {
				c.Close()
			}
			mu.Lock()
			lat = append(lat, local...)
			mu.Unlock()
		}()
	}
	wg.Wait()
	sort.Slice(lat, func(a, b int) bool { return lat[a] < lat[b] })
	pct := func(p float64) float64 {
		if len(lat) == 0 {
			return 0
		}
		return float64(lat[int(float64(len(lat)-1)*p)].Microseconds()) / 1000
	}
	fmt.Printf("%.0f %.1f %.2f %.2f %d\n", float64(total.Load())/(*secs), float64(bytes.Load())/(*secs)/1e6, pct(0.5), pct(0.99), errs.Load())
}
