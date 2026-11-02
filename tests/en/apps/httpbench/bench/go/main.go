// The net/http + 1.22 ServeMux counterpart to main.salam, for the benchmark
// comparison. Same routes, same response bodies as the other four servers,
// so the only thing that differs between the numbers is the server.
package main

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"net/http"
	"os"
	"path/filepath"
	"strconv"
	"strings"
	"time"
)

// writeAll writes a response body and logs a failed write. A client that hangs
// up mid-benchmark must not take the server down with it.
func writeAll(w http.ResponseWriter, body []byte) {
	//nolint:gosec // every caller passes a response body it built itself
	if _, err := w.Write(body); err != nil {
		log.Printf("write: %v", err)
	}
}

var (
	assetsDir string
	cached    []byte
)

func assetsPath() string {
	if d := os.Getenv("HTTPBENCH_ASSETS"); d != "" {
		return d
	}
	return "tests/en/apps/httpbench/public"
}

func port() string {
	if p := os.Getenv("PORT"); p != "" {
		return p
	}
	return "8104"
}

func writeJSON(w http.ResponseWriter, v any) {
	var buf bytes.Buffer
	enc := json.NewEncoder(&buf)
	enc.SetEscapeHTML(false)
	if err := enc.Encode(v); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	body := bytes.TrimRight(buf.Bytes(), "\n")
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	writeAll(w, body)
}

func writeJSONRaw(w http.ResponseWriter, body []byte) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(http.StatusOK)
	writeAll(w, body)
}

func writeText(w http.ResponseWriter, body string) {
	w.Header().Set("Content-Type", "text/plain; charset=utf-8")
	w.WriteHeader(http.StatusOK)
	writeAll(w, []byte(body))
}

func writeError(w http.ResponseWriter, code int, msg string) {
	writeJSONPlusStatus(w, code, map[string]string{"error": msg})
}

func writeJSONPlusStatus(w http.ResponseWriter, code int, v any) {
	var buf bytes.Buffer
	enc := json.NewEncoder(&buf)
	enc.SetEscapeHTML(false)
	if err := enc.Encode(v); err != nil {
		http.Error(w, err.Error(), http.StatusInternalServerError)
		return
	}
	body := bytes.TrimRight(buf.Bytes(), "\n")
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(code)
	writeAll(w, body)
}

func intQuery(r *http.Request, name string, fallback, lo, hi int) int {
	raw := r.URL.Query().Get(name)
	if raw == "" {
		return fallback
	}
	v, err := strconv.Atoi(raw)
	if err != nil {
		return fallback
	}
	if v < lo {
		return lo
	}
	if v > hi {
		return hi
	}
	return v
}

func row(p, note string) string {
	return fmt.Sprintf("<tr><td><a href=\"%s\">%s</a></td><td>%s</td></tr>\n", p, p, note)
}

func homeHandler(w http.ResponseWriter, _ *http.Request) {
	var b strings.Builder
	b.WriteString("<!doctype html>\n<html lang=\"en\">\n<head>\n")
	b.WriteString("<meta charset=\"utf-8\">\n")
	b.WriteString("<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n")
	b.WriteString("<title>Salam httpbench</title>\n")
	b.WriteString("<link rel=\"stylesheet\" href=\"/static/style.css\">\n")
	b.WriteString("</head>\n<body>\n<main>\n")
	b.WriteString("<h1>Salam httpbench</h1>\n")
	b.WriteString("<p>A server built to be measured. Each route isolates one cost.</p>\n")
	b.WriteString("<table>\n<tr><th>Route</th><th>What it costs</th></tr>\n")
	b.WriteString(row("/plaintext", "the floor: accept, parse, route, write"))
	b.WriteString(row("/json", "small-object serialization"))
	b.WriteString(row("/", "this page, assembled per request"))
	b.WriteString(row("/file", "a disk read on every request"))
	b.WriteString(row("/cached", "the same bytes, read once at boot"))
	b.WriteString(row("/static/style.css", "the built-in static file path"))
	b.WriteString(row("/users/:id", "one router-extracted path parameter"))
	b.WriteString(row("/search?q=salam&n=5", "query-string parsing"))
	b.WriteString(row("/compute?n=1000", "tunable CPU work"))
	b.WriteString(row("/headers", "request headers walked and echoed"))
	b.WriteString(row("/echo (POST)", "request body read back out"))
	b.WriteString(row("/health", "a load balancer's poll"))
	b.WriteString("</table>\n</main>\n</body>\n</html>\n")
	w.Header().Set("Content-Type", "text/html; charset=utf-8")
	w.WriteHeader(http.StatusOK)
	writeAll(w, []byte(b.String()))
}

func main() {
	assetsDir = assetsPath()
	dataPath := filepath.Join(assetsDir, "data.json")
	var err error
	cached, err = os.ReadFile(dataPath)
	if err != nil {
		fmt.Println("assets not found under", assetsDir)
		fmt.Println("run from the repository root, or set HTTPBENCH_ASSETS")
		os.Exit(1)
	}

	mux := http.NewServeMux()

	mux.HandleFunc("GET /plaintext", func(w http.ResponseWriter, _ *http.Request) {
		writeText(w, "Hello, World!")
	})

	mux.HandleFunc("GET /health", func(w http.ResponseWriter, _ *http.Request) {
		writeJSONRaw(w, []byte(`{"status":"ok","service":"httpbench"}`))
	})

	mux.HandleFunc("GET /json", func(w http.ResponseWriter, _ *http.Request) {
		writeJSONRaw(w, []byte(`{"message":"Hello, World!","server":"go","routes":13,"ok":true}`))
	})

	mux.HandleFunc("GET /file", func(w http.ResponseWriter, _ *http.Request) {
		body, err := os.ReadFile(dataPath)
		if err != nil || len(body) == 0 {
			writeError(w, http.StatusInternalServerError, "asset missing")
			return
		}
		writeJSONRaw(w, body)
	})

	mux.HandleFunc("GET /cached", func(w http.ResponseWriter, _ *http.Request) {
		writeJSONRaw(w, cached)
	})

	mux.HandleFunc("GET /users/{id}", func(w http.ResponseWriter, r *http.Request) {
		id := r.PathValue("id")
		if id == "" {
			writeError(w, http.StatusBadRequest, "missing id")
			return
		}
		writeJSON(w, map[string]any{
			"id":     id,
			"name":   "user-" + id,
			"active": true,
		})
	})

	mux.HandleFunc("GET /search", func(w http.ResponseWriter, r *http.Request) {
		q := r.URL.Query().Get("q")
		n := intQuery(r, "n", 5, 0, 100)
		results := make([]map[string]any, 0, n)
		for i := 0; i < n; i++ {
			results = append(results, map[string]any{
				"rank":  i + 1,
				"title": fmt.Sprintf("%s result %d", q, i+1),
			})
		}
		writeJSON(w, map[string]any{"query": q, "count": n, "results": results})
	})

	mux.HandleFunc("GET /compute", func(w http.ResponseWriter, r *http.Request) {
		n := intQuery(r, "n", 1000, 0, 5000000)
		var acc int64
		for i := 0; i < n; i++ {
			acc += int64((i * i) % 7)
		}
		writeJSONRaw(w, []byte(fmt.Sprintf(`{"n":%d,"sum":%d}`, n, acc)))
	})

	mux.HandleFunc("GET /headers", func(w http.ResponseWriter, r *http.Request) {
		writeJSON(w, map[string]any{
			"host":       r.Host,
			"user_agent": r.UserAgent(),
			"accept":     r.Header.Get("Accept"),
			"method":     r.Method,
			"path":       r.URL.Path,
		})
	})

	mux.HandleFunc("POST /echo", func(w http.ResponseWriter, r *http.Request) {
		body, err := io.ReadAll(io.LimitReader(r.Body, 10<<20))
		if err != nil {
			http.Error(w, err.Error(), http.StatusBadRequest)
			return
		}
		writeJSON(w, map[string]any{"bytes": len(body), "echo": string(body)})
	})

	mux.HandleFunc("GET /{$}", homeHandler)

	mux.Handle("GET /static/", http.StripPrefix("/static/", http.FileServer(http.Dir(assetsDir))))

	addr := "127.0.0.1:" + port()
	fmt.Println("httpbench-go on http://" + addr)
	srv := &http.Server{
		Addr:              addr,
		Handler:           mux,
		ReadHeaderTimeout: 10 * time.Second,
	}
	if err := srv.ListenAndServe(); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
