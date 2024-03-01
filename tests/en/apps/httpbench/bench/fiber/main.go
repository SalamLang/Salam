// The Fiber (fasthttp) counterpart to main.salam, for the benchmark
// comparison. Same routes, same response bodies as the other servers, so
// the only thing that differs between the numbers is the server.
package main

import (
	"fmt"
	"os"
	"path/filepath"
	"strconv"

	"github.com/gofiber/fiber/v2"
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
	return "8105"
}

func intQuery(c *fiber.Ctx, name string, fallback, lo, hi int) int {
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

	app := fiber.New(fiber.Config{
		DisableStartupMessage: true,
	})

	app.Get("/plaintext", func(c *fiber.Ctx) error {
		c.Set("Content-Type", "text/plain; charset=utf-8")
		return c.SendString("Hello, World!")
	})

	app.Get("/health", func(c *fiber.Ctx) error {
		c.Set("Content-Type", "application/json")
		return c.SendString(`{"status":"ok","service":"httpbench"}`)
	})

	app.Get("/json", func(c *fiber.Ctx) error {
		c.Set("Content-Type", "application/json")
		return c.SendString(`{"message":"Hello, World!","server":"fiber","routes":13,"ok":true}`)
	})

	app.Get("/file", func(c *fiber.Ctx) error {
		body, err := os.ReadFile(dataPath)
		if err != nil || len(body) == 0 {
			c.Status(fiber.StatusInternalServerError)
			return c.JSON(fiber.Map{"error": "asset missing"})
		}
		c.Set("Content-Type", "application/json")
		return c.Send(body)
	})

	app.Get("/cached", func(c *fiber.Ctx) error {
		c.Set("Content-Type", "application/json")
		return c.Send(cached)
	})

	app.Get("/users/:id", func(c *fiber.Ctx) error {
		id := c.Params("id")
		if id == "" {
			c.Status(fiber.StatusBadRequest)
			return c.JSON(fiber.Map{"error": "missing id"})
		}
		return c.JSON(fiber.Map{"id": id, "name": "user-" + id, "active": true})
	})

	app.Get("/search", func(c *fiber.Ctx) error {
		q := c.Query("q")
		n := intQuery(c, "n", 5, 0, 100)
		results := make([]fiber.Map, 0, n)
		for i := 0; i < n; i++ {
			results = append(results, fiber.Map{"rank": i + 1, "title": fmt.Sprintf("%s result %d", q, i+1)})
		}
		return c.JSON(fiber.Map{"query": q, "count": n, "results": results})
	})

	app.Get("/compute", func(c *fiber.Ctx) error {
		n := intQuery(c, "n", 1000, 0, 5000000)
		var acc int64
		for i := 0; i < n; i++ {
			acc += int64((i * i) % 7)
		}
		c.Set("Content-Type", "application/json")
		return c.SendString(fmt.Sprintf(`{"n":%d,"sum":%d}`, n, acc))
	})

	app.Get("/headers", func(c *fiber.Ctx) error {
		return c.JSON(fiber.Map{
			"host":       c.Hostname(),
			"user_agent": c.Get("User-Agent"),
			"accept":     c.Get("Accept"),
			"method":     c.Method(),
			"path":       c.Path(),
		})
	})

	app.Post("/echo", func(c *fiber.Ctx) error {
		body := c.Body()
		return c.JSON(fiber.Map{"bytes": len(body), "echo": string(body)})
	})

	app.Get("/", func(c *fiber.Ctx) error {
		c.Set("Content-Type", "text/html; charset=utf-8")
		return c.SendString(home)
	})

	app.Static("/static", assetsDir)

	addr := "127.0.0.1:" + port()
	fmt.Println("httpbench-fiber on http://" + addr)
	if err := app.Listen(addr); err != nil {
		fmt.Println(err)
		os.Exit(1)
	}
}
