---
title: Salam vs Go: which is better for fast web servers and concurrent software?
seo_title: Salam vs Go: Concurrency, Web Servers and Performance
description: Salam vs Go for web servers and concurrency: goroutines against threads and channels, garbage collection against manual memory, HTTP benchmarks with caveats.
keywords: Salam vs Go, Go alternative, web server performance, goroutines vs threads, Go garbage collector, Go channels, HTTP router benchmark, Fiber vs net/http, concurrent programming, backend language comparison
category: compare
summary: Go's goroutines and garbage collector against Salam's threads, task pool, channels and manual memory, with the same programs in both languages and an honest reading of the HTTP benchmark.
date: 2026-11-11
---

Salam and Go are both compiled languages with simple syntax, fast builds, a strong standard library for networking and single-file deployment. They differ in two fundamental choices. **Go has goroutines, lightweight tasks scheduled by its runtime, and a garbage collector. Salam has neither: concurrency uses operating-system threads, a bounded task pool and channels, and heap memory is freed by hand.** That makes Go easier to write highly concurrent code in, and gives Salam lower and more predictable memory overhead with no collector running beside the program.

On the project's own HTTP router benchmark Salam's server answered more requests per second than the Go servers tested, with caveats this article spells out. For most teams, though, the choice turns on the concurrency model and the ecosystem more than on throughput.

## Salam vs Go at a glance {#at-a-glance}

|                 | Go                                                                         | Salam                                                                                   |
| --------------- | -------------------------------------------------------------------------- | --------------------------------------------------------------------------------------- |
| Concurrency     | Goroutines (M:N scheduling by the runtime), channels, `select`             | OS threads (`spawn`), a bounded task pool (`go f(x)`), channels, `chan.Select`, futures |
| Blocking I/O    | Parks the goroutine; the runtime multiplexes sockets                       | Blocks the thread; the HTTP server uses an event loop with a worker pool on Linux       |
| Memory          | Garbage collected                                                          | Manual: `.Free()` and `defer`, with compile-time ownership checks                       |
| Errors          | `error` values, `if err != nil`                                            | `throws`, `try`, `catch`, or result enums                                               |
| Generics        | Since Go 1.18                                                              | Yes, with interface bounds                                                              |
| Sum types       | No (interfaces and type switches)                                          | Enums with data and exhaustive `match`                                                  |
| HTTP            | `net/http` in the standard library, frameworks such as Gin, Echo and Fiber | `net/http` and `net/router` in the standard library                                     |
| Cross-compiling | `GOOS` and `GOARCH`                                                        | `--target` with a built-in linker and bundled sysroots                                  |
| Maturity        | 1.0 in 2012, compatibility promise, huge ecosystem                         | Pre-1.0, small ecosystem                                                                |

## Concurrency: goroutines against threads, pools and channels {#concurrency}

Here is a classic worker pool in Go: three goroutines read numbers from a channel, square them and send the results to another channel.

```go
package main

import (
    "fmt"
    "sync"
)

func main() {
    jobs := make(chan int64, 4)
    results := make(chan int64, 16)
    var wg sync.WaitGroup
    for range 3 {
        wg.Add(1)
        go func() {
            defer wg.Done()
            for n := range jobs {
                results <- n * n
            }
        }()
    }
    for i := int64(1); i <= 10; i++ {
        jobs <- i
    }
    close(jobs)
    wg.Wait()
    close(results)
    var sum int64
    for v := range results {
        sum += v
    }
    fmt.Println("sum of squares:", sum)
}
```

The Salam version has the same shape. The workers are threads started with `spawn`, the channels come from the `chan` package, and `recv` sets `ok` to `false` once a channel is closed and empty:

```salam
import chan

mut jobs := chan.Chan {} as chan.Chan<i64>
mut results := chan.Chan {} as chan.Chan<i64>

func worker():
    mut ok := true
    until true:
        n := jobs.recv(ok)
        if not ok:
            break
        end
        results.send(n * n)
    end
end

func main:
    jobs.init(4)
    results.init(16)
    mut workers := Vector {} as Vector<i64>
    defer workers.Free()
    repeat 3:
        workers.Push(spawn(worker))
    end
    repeat 1 to 10 in i:
        jobs.send(i as i64)
    end
    jobs.Close()
    each t in workers:
        join(t)
    end
    results.Close()
    mut sum := 0 as i64
    mut ok := true
    until true:
        v := results.recv(ok)
        if not ok:
            break
        end
        sum += v
    end
    println "sum of squares:", sum
    jobs.Free()
    results.Free()
end
```

```output
sum of squares: 385
```

The programs read alike, but the machinery underneath does not. A goroutine starts with a small stack that grows as needed, and the Go scheduler can run hundreds of thousands of them on a handful of OS threads, parking any goroutine that waits on a channel or a socket. A Salam `spawn` is a real operating-system thread with its own fixed stack: fine for a few workers or a few hundred, not for one per connection at large scale.

For many small jobs, Salam has a `go` keyword of its own. `go f(x)` runs the call as a task on a shared pool with one worker per CPU core and a bounded queue; when the queue is full, the caller waits, so a fast loop cannot pile up unlimited work. Here it counts primes in eight ranges in parallel:

```salam
import atomic
import pool

mut found := atomic.NewI64(0 as i64)

func is prime(n: i64): bool:
    if n < 2: ret false end
    mut d := 2 as i64
    until d * d <= n:
        if n % d == 0: ret false end
        d++
    end
    ret true
end

func count range(lo: i64, hi: i64):
    mut n := 0 as i64
    mut x := lo
    until x < hi:
        if is prime(x): n++ end
        x++
    end
    _ := atomic.Add(found, n)
end

func main:
    repeat 8 in i:
        lo := (i as i64) * 25000
        go count range(lo, lo + 25000)
    end
    pool.GoWait()
    println "primes below 200000:", atomic.Load(found)
    atomic.DestroyI64(found)
end
```

```output
primes below 200000: 17984
```

The Go equivalent uses `sync.WaitGroup` with its `Go` method (added in Go 1.25) and `atomic.Int64`:

```go
package main

import (
    "fmt"
    "sync"
    "sync/atomic"
)

func isPrime(n int64) bool {
    if n < 2 {
        return false
    }
    for d := int64(2); d*d <= n; d++ {
        if n%d == 0 {
            return false
        }
    }
    return true
}

func main() {
    var found atomic.Int64
    var wg sync.WaitGroup
    for i := range int64(8) {
        lo := i * 25000
        wg.Go(func() {
            var n int64
            for x := lo; x < lo+25000; x++ {
                if isPrime(x) {
                    n++
                }
            }
            found.Add(n)
        })
    }
    wg.Wait()
    fmt.Println("primes below 200000:", found.Load())
}
```

The difference to remember: Salam's `go` is a thread pool, not a scheduler. A task that blocks on a socket or a lock holds one of the pool's workers until it returns, where a blocked goroutine costs Go almost nothing. Salam fills the rest of the toolbox with `pool.Async` and `pool.Await` for futures, `pool.NewGroup` for a group of tasks with a concurrency limit and first-error reporting, `chan.Select` for waiting on several channels with a timeout, `ctx` for cancellation and deadlines, and `sync` and `atomic` for locks and counters. The [concurrency article](/blog/concurrency-in-salam/) and the [threads lesson](/learn/threads/) cover them. There is no `async`/`await` in Salam.

## Garbage collection against manual memory {#memory}

Go's garbage collector is concurrent and tuned for low pause times, and it is a large part of why Go code is quick to write: you allocate and forget. The costs are memory headroom (a collected heap needs room to grow between cycles) and CPU time spent marking, which tuning with `GOGC` and `GOMEMLIMIT` can trade against each other.

Salam has no collector. Values live on the stack, heap containers are freed with `.Free()`, usually written as `defer x.Free()` right after creation, and a function that returns a new string or container hands ownership to its caller. The compiler's ownership checker reports use after free, double free and similar mistakes, and `salam memcheck` runs a program under AddressSanitizer. Memory use is what the program allocates, with no collector overhead, but forgetting a `free` is a leak that only tests or tools will find, and in a server that runs for weeks that matters. The [memory management article](/blog/memory-management-in-salam/) explains the rules.

## Writing a web server in Go and in Salam {#web-servers}

Both languages put an HTTP server in the standard library. In Go, with the path patterns added in Go 1.22:

```go
package main

import (
    "fmt"
    "log"
    "net/http"
)

func main() {
    mux := http.NewServeMux()
    mux.HandleFunc("GET /hello/{name}", func(w http.ResponseWriter, r *http.Request) {
        fmt.Fprintf(w, "Hello, %s!\n", r.PathValue("name"))
    })
    log.Fatal(http.ListenAndServe(":8080", mux))
}
```

In Salam, with [net/http](/std/net/http/) and [net/router](/std/net/router/):

```salam
import net.http
import net.router

func hello(ctx: i64):
    http.Text(ctx, "Hello, " + http.Param(ctx, "name") + "!\n")
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/hello/:name", hello)
    mut s := http.NewServer(8080)
    http.Use(s, r)
    http.Run(s)
end
```

Both answer `curl localhost:8080/hello/Sara` with `Hello, Sara!` and return 404 for unknown paths. Salam's handler receives a context id and reads parameters through functions; Go's receives a writer and a request object. Salam's standard library adds middleware, sessions, validation, OpenAPI generation, WebSocket, HTTP/2 and gRPC, and `salam serve --config` runs a multi-site server with virtual hosts and FastCGI backends. One rule differs from Go and is easy to miss: strings read from a request are freed when the handler returns, so anything you keep must be copied with `str.Clone`. The article on [fast web servers with Salam](/blog/fast-web-servers-with-salam/) goes deeper.

## What the HTTP router benchmark shows {#http-benchmark}

The project published a benchmark of nine servers answering the same eleven routes (plain text, JSON, a cached response, a file, a path parameter, query parsing, a computation, headers, a static file, the root page and a POST echo). In the run of 28 October 2026, the geometric mean over all eleven routes was:

| Server              | Requests per second |
| ------------------- | ------------------- |
| Salam `net/http`    | 94,850              |
| Go, Fiber           | 62,493              |
| Go, Echo            | 35,238              |
| Go, Gin             | 34,999              |
| Go, `net/http`      | 34,987              |
| Node, Express       | 22,536              |
| PHP built-in server | 11,992              |

Salam answered more requests per second than Fiber on all 11 routes, and was 2.7 times Go's own `net/http`. The widest gap was query parsing with list building (`/search`, 99,143 against Fiber's 48,247); the thinnest was `/`, where Salam was 4% ahead.

Now the caveats, which matter as much as the table:

- The run used a shared 6-vCPU AMD EPYC sandbox with a load average around 11 from unrelated work, servers pinned to 4 cores and the load generator to 2, 64 keep-alive connections, and 10 seconds per route after a 3-second warm-up, with routes interleaved across servers.
- Background load favors servers that keep few threads, Salam's included. That is why the speed page gives the margin over Fiber as 16% on a quiet machine and 52% in the full run under load.
- Fiber ran in its default configuration (no prefork), on Go 1.26.0. Go servers can be tuned, and so can Salam's.
- The harness's `BENCHMARK.md` in `tests/en/apps/httpbench` now holds a later run on a 4-vCPU GitHub-hosted runner in which the margin over Fiber is much narrower and Fiber is ahead on some routes, most clearly static files.

The fair reading is that Salam's server is in the same league as the fastest Go frameworks on small, CPU-light routes, and comfortably ahead of Go's standard library server and Node in these runs. It is not evidence that Salam will serve your application faster: real services spend their time in databases, serialization and business logic, and the benchmark does not test long-lived connections at scale, where goroutines shine. Rerun it on your hardware before planning capacity; the [speed page](/speed/) links the harness.

## CPU-bound performance {#cpu-performance}

On the separate benchmark of 39 small programs with the same algorithm in every language, Salam averaged 10.6 ms and Go 14.2 ms, which makes Go about 1.3 times slower on that suite. Go was not the fastest compiled language on any of those programs. Each run is a whole short-lived process, so the Go runtime's start-up is part of every number, and the benchmark does not separate that from the quality of the generated code. The [careful reading of the benchmark](/blog/is-salam-faster-than-c-and-rust/) explains why such averages are a range, not a ranking.

## Tooling, deployment and ecosystem {#ecosystem}

Both languages produce single executables that are easy to deploy, and both cross-compile from one machine: Go with `GOOS` and `GOARCH`, Salam with `--target` and its built-in linker (see [cross-compilation in Salam](/blog/cross-compilation-salam/)). Both ship a formatter, a test runner and documentation tools in the main command.

The ecosystems are not comparable. Go has a decade and a half of production use, a compatibility promise for 1.x, mature observability and cloud libraries, and a large hiring pool. Salam has a broad standard library, a package manager that installs packages from GitHub, and a small community. If your service needs a specific cloud SDK, tracing system or database driver today, check that Salam has it before you commit.

## When to choose Go, when to choose Salam {#when}

Choose **Go** when:

- you need very high concurrency with simple code: tens of thousands of connections, each in its own goroutine;
- the team values a garbage collector over manual memory management;
- you depend on the Go ecosystem: cloud SDKs, Kubernetes tooling, observability libraries;
- long-term stability and hiring matter more than raw efficiency.

Choose **Salam** when:

- you want compiled performance without a garbage collector and with predictable memory use;
- your server's routes are CPU-light and you want the throughput the benchmark shows, after verifying it on your hardware;
- you want sum types, exhaustive matching and an expressive standard library in a language that is still simple;
- you want to write or teach in Persian.

## Frequently asked questions {#faq}

### Does Salam have goroutines?

No. Salam's `go f(x)` runs a call as a task on a bounded thread pool, and `spawn` starts an operating-system thread. There is no M:N scheduler, so a task that blocks holds its worker thread until it returns.

### Is Salam faster than Go for web servers?

In the project's router benchmark Salam's `net/http` answered more requests per second than Fiber, Gin, Echo and Go's `net/http`, but the margin depended heavily on the machine and its load, and a later run showed a much narrower gap. Benchmark your own workload before deciding.

### Does Salam have a garbage collector?

No. Memory is managed manually with `.Free()` and `defer`, with compile-time checks for common mistakes. That avoids collector overhead but makes leaks possible.

### Can I use channels in Salam like in Go?

Yes. The `chan` package provides buffered and unbuffered channels with `send`, `recv`, `close`, timeouts and `chan.Select` over several channels. Channels carry values between threads or pool tasks rather than goroutines.
