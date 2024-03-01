// The Echo counterpart to main.salam, for the benchmark comparison. Same
// routes, same response bodies as the other servers, so the only thing that
// differs between the numbers is the server.
package main

import (
	"fmt"
	"io"
	"net/http"
	"os"
	"path/filepath"
	"strconv"

	"github.com/labstack/echo/v4"
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
	return "8107"
}

func intQuery(c echo.Context, name string, fallback, lo, hi int) int {
	raw := c.QueryParam(name)
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

	e := echo.New()
	e.HideBanner = true
	e.HidePort = true

	e.GET("/plaintext", func(c echo.Context) error {
		return c.String(http.StatusOK, "Hello, World!")
	})

	e.GET("/health", func(c echo.Context) error {
		return c.JSONBlob(http.StatusOK, []byte(`{"status":"ok","service":"httpbench"}`))
	})

	e.GET("/json", func(c echo.Context) error {
		return c.JSONBlob(http.StatusOK, []byte(`{"message":"Hello, World!","server":"echo","routes":13,"ok":true}`))
	})

	e.GET("/file", func(c echo.Context) error {
		body, err := os.ReadFile(dataPath)
		if err != nil || len(body) == 0 {
			return c.JSON(http.StatusInternalServerError, echo.Map{"error": "asset missing"})
		}
		return c.JSONBlob(http.StatusOK, body)
	})

	e.GET("/cached", func(c echo.Context) error {
		return c.JSONBlob(http.StatusOK, cached)
	})

	e.GET("/users/:id", func(c echo.Context) error {
		id := c.Param("id")
		if id == "" {
			return c.JSON(http.StatusBadRequest, echo.Map{"error": "missing id"})
		}
		return c.JSON(http.StatusOK, echo.Map{"id": id, "name": "user-" + id, "active": true})
	})

	e.GET("/search", func(c echo.Context) error {
		q := c.QueryParam("q")
		n := intQuery(c, "n", 5, 0, 100)
		results := make([]echo.Map, 0, n)
		for i := 0; i < n; i++ {
			results = append(results, echo.Map{"rank": i + 1, "title": fmt.Sprintf("%s result %d", q, i+1)})
		}
		return c.JSON(http.StatusOK, echo.Map{"query": q, "count": n, "results": results})
	})

	e.GET("/compute", func(c echo.Context) error {
		n := intQuery(c, "n", 1000, 0, 5000000)
		var acc int64
		for i := 0; i < n; i++ {
			acc += int64((i * i) % 7)
		}
		return c.JSONBlob(http.StatusOK, []byte(fmt.Sprintf(`{"n":%d,"sum":%d}`, n, acc)))
	})

	e.GET("/headers", func(c echo.Context) error {
		r := c.Request()
		return c.JSON(http.StatusOK, echo.Map{
			"host":       r.Host,
			"user_agent": r.UserAgent(),
			"accept":     r.Header.Get("Accept"),
			"method":     r.Method,
			"path":       r.URL.Path,
		})
	})

	e.POST("/echo", func(c echo.Context) error {
		body, _ := io.ReadAll(io.LimitReader(c.Request().Body, 10<<20))
		return c.JSON(http.StatusOK, echo.Map{"bytes": len(body), "echo": string(body)})
	})

	e.GET("/", func(c echo.Context) error {
		return c.HTML(http.StatusOK, home)
	})

	e.Static("/static", assetsDir)

	addr := "127.0.0.1:" + port()
	fmt.Println("httpbench-echo on http://" + addr)
	if err := e.Start(addr); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
