// The Gin counterpart to main.salam, for the benchmark comparison. Same
// routes, same response bodies as the other servers, so the only thing that
// differs between the numbers is the server.
package main

import (
	"fmt"
	"net/http"
	"os"
	"path/filepath"
	"strconv"

	"github.com/gin-gonic/gin"
)

var cached []byte

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
	return "8106"
}

func intQuery(c *gin.Context, name string, fallback, lo, hi int) int {
	raw := c.Query(name)
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

func homePage() string {
	b := "<!doctype html>\n<html lang=\"en\">\n<head>\n"
	b += "<meta charset=\"utf-8\">\n"
	b += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n"
	b += "<title>Salam httpbench</title>\n"
	b += "<link rel=\"stylesheet\" href=\"/static/style.css\">\n"
	b += "</head>\n<body>\n<main>\n"
	b += "<h1>Salam httpbench</h1>\n"
	b += "<p>A server built to be measured. Each route isolates one cost.</p>\n"
	b += "<table>\n<tr><th>Route</th><th>What it costs</th></tr>\n"
	b += row("/plaintext", "the floor: accept, parse, route, write")
	b += row("/json", "small-object serialization")
	b += row("/", "this page, assembled per request")
	b += row("/file", "a disk read on every request")
	b += row("/cached", "the same bytes, read once at boot")
	b += row("/static/style.css", "the built-in static file path")
	b += row("/users/:id", "one router-extracted path parameter")
	b += row("/search?q=salam&n=5", "query-string parsing")
	b += row("/compute?n=1000", "tunable CPU work")
	b += row("/headers", "request headers walked and echoed")
	b += row("/echo (POST)", "request body read back out")
	b += row("/health", "a load balancer's poll")
	b += "</table>\n</main>\n</body>\n</html>\n"
	return b
}

func main() {
	assetsDir := assetsPath()
	dataPath := filepath.Join(assetsDir, "data.json")
	var err error
	cached, err = os.ReadFile(dataPath)
	if err != nil {
		fmt.Println("assets not found under", assetsDir)
		fmt.Println("run from the repository root, or set HTTPBENCH_ASSETS")
		os.Exit(1)
	}
	home := homePage()

	gin.SetMode(gin.ReleaseMode)
	r := gin.New()

	r.GET("/plaintext", func(c *gin.Context) {
		c.String(http.StatusOK, "Hello, World!")
	})

	r.GET("/health", func(c *gin.Context) {
		c.Data(http.StatusOK, "application/json", []byte(`{"status":"ok","service":"httpbench"}`))
	})

	r.GET("/json", func(c *gin.Context) {
		c.Data(http.StatusOK, "application/json", []byte(`{"message":"Hello, World!","server":"gin","routes":13,"ok":true}`))
	})

	r.GET("/file", func(c *gin.Context) {
		body, err := os.ReadFile(dataPath)
		if err != nil || len(body) == 0 {
			c.JSON(http.StatusInternalServerError, gin.H{"error": "asset missing"})
			return
		}
		c.Data(http.StatusOK, "application/json", body)
	})

	r.GET("/cached", func(c *gin.Context) {
		c.Data(http.StatusOK, "application/json", cached)
	})

	r.GET("/users/:id", func(c *gin.Context) {
		id := c.Param("id")
		if id == "" {
			c.JSON(http.StatusBadRequest, gin.H{"error": "missing id"})
			return
		}
		c.JSON(http.StatusOK, gin.H{"id": id, "name": "user-" + id, "active": true})
	})

	r.GET("/search", func(c *gin.Context) {
		q := c.Query("q")
		n := intQuery(c, "n", 5, 0, 100)
		results := make([]gin.H, 0, n)
		for i := 0; i < n; i++ {
			results = append(results, gin.H{"rank": i + 1, "title": fmt.Sprintf("%s result %d", q, i+1)})
		}
		c.JSON(http.StatusOK, gin.H{"query": q, "count": n, "results": results})
	})

	r.GET("/compute", func(c *gin.Context) {
		n := intQuery(c, "n", 1000, 0, 5000000)
		var acc int64
		for i := 0; i < n; i++ {
			acc += int64((i * i) % 7)
		}
		c.Data(http.StatusOK, "application/json", []byte(fmt.Sprintf(`{"n":%d,"sum":%d}`, n, acc)))
	})

	r.GET("/headers", func(c *gin.Context) {
		c.JSON(http.StatusOK, gin.H{
			"host":       c.Request.Host,
			"user_agent": c.Request.UserAgent(),
			"accept":     c.GetHeader("Accept"),
			"method":     c.Request.Method,
			"path":       c.Request.URL.Path,
		})
	})

	r.POST("/echo", func(c *gin.Context) {
		body, _ := c.GetRawData()
		c.JSON(http.StatusOK, gin.H{"bytes": len(body), "echo": string(body)})
	})

	r.GET("/", func(c *gin.Context) {
		c.Data(http.StatusOK, "text/html; charset=utf-8", []byte(home))
	})

	r.Static("/static", assetsDir)

	addr := "127.0.0.1:" + port()
	fmt.Println("httpbench-gin on http://" + addr)
	if err := r.Run(addr); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
