---
title: Building fast web servers with Salam: HTTP, routing, middleware, WebSocket and gRPC
seo_title: Fast Web Servers in Salam: HTTP Router, WebSocket, gRPC
description: Build an HTTP server, REST API, WebSocket hub and gRPC service with Salam's standard library, and read the router benchmark against Go, Fiber, nginx and Node.
keywords: web server, HTTP router, WebSocket, gRPC, REST API, Salam net/http, HTTP server performance, web server benchmark, middleware, epoll server
category: build
summary: Everything Salam's standard library offers for network services, from a ten-line HTTP server to WebSocket rooms and gRPC, how the server works inside, and what the router benchmark does and does not show.
date: 2026-11-11
---

A web server in Salam is an ordinary compiled program built on the standard library: `net.http` accepts connections and parses requests, `net.router` picks a handler by method and path, and companion packages add middleware, sessions, validation, WebSocket, HTTP/2 and gRPC. **Salam's HTTP router and server compile to a native executable with no garbage collector and no external dependencies, and in the project's published benchmark they answered more requests per second than Go's standard library, Fiber, Express and PHP on the same machine.** That benchmark has caveats, and this article covers them along with the code.

Everything below uses only packages that ship with the compiler. The runnable examples start their server in the background on a free port and act as their own client, so the output you see is real.

## A minimal HTTP server in Salam {#minimal}

This is a complete server. It answers `/` with text, `/hello/:name` with JSON built from a path parameter, and serves a folder of static files under `/assets`.

```salam
import net.http
import net.router

func hello(ctx: i64):
    http.Text(ctx, "Hello from Salam")
end

func greet(ctx: i64):
    http.Json(ctx, `{"hello":"` + http.Param(ctx, "name") + `"}`)
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/", hello)
    router.Get(r, "/hello/:name", greet)
    router.Static(r, "/assets", "public")
    mut s := http.NewServer(8080)
    http.EnableAccessLog(s)
    http.Use(s, r)
    http.Run(s)
end
```

`salam run server.salam` starts it; `salam build server.salam --output=server` produces a single executable you can copy to a Linux box and run. The handler shape is the same everywhere: a function that takes the request's context id. The request is read with `http.Param`, `http.Query`, `http.Header`, `http.Form`, `http.Body` and `http.CookieValue`, and the answer is written with `http.Text`, `http.Json`, `http.Html`, `http.SetStatus` and `http.SetHeader`. Nothing is sent until the handler returns, so status, headers and body go out together.

For production, the [net.http reference](/std/net/http/) shows the extra settings: `http.SetHost(s, "0.0.0.0")` inside a container, `http.SetMaxBodyBytes` and `http.SetMaxHeaderBytes` to reject oversized requests, `http.SetKeepAliveIdle` for idle connections, `http.HideTokens` to drop the server banner, and `http.RunTLS(s, cert, key)` to serve HTTPS directly instead of behind a proxy.

## HTTP routing: parameters, groups, 404 and 405 {#routing}

The router in [net.router](/std/net/router/) matches by method and path, with `:name` parameters and `*` wildcards. Its precedence rules are fixed and documented, which matters more than it sounds once an API has a few dozen routes:

- A literal segment beats a parameter, and a parameter beats `*`, so `/users/me` reaches its own handler even if `/users/:name` was registered first.
- Between two wildcard routes, the one registered first wins, so a catch-all `/*` goes last.
- A trailing slash still reaches a parameter route, and a `HEAD` request falls back to the `GET` route.
- `router.Group(r, "/api/v1", "GET", "/items", list)` registers a route under a prefix, which is how versioned APIs are usually laid out.

`router.Dispatch` lets you ask the router what it would do without a server, which is how the reference page builds a correct `405 Method Not Allowed` with an `Allow` header instead of a misleading 404. `router.SetFallback` installs a handler for everything else, and `router.Static` serves a folder next to the API, keeping files in memory after the first request.

One rule from the reference is worth repeating: register every route before `http.Use`, because the server keeps a copy of the router.

## A REST API with middleware and authentication {#api}

Salam has no middleware chain and no way to wrap a handler from outside. A middleware is an ordinary function you call on the first lines of a handler with the context id. Functions that only add headers return nothing; functions that may stop the request return a `bool`, write the error response themselves, and the handler returns at once. The order is always visible in the handler itself.

This API keeps items in memory, groups its routes under `/api/v1`, applies the security headers from [net.http.middleware](/std/net/http/middleware/) and protects the create route with a bearer token.

```salam
import net.http
import net.http.middleware
import net.router
import encoding.json
import conv
import str

mut items := Vector {} as Vector<Item>

struct Item:
    @json "id"
    pub id: int = 0
    @json "name"
    pub name: str = ""
end

func authorized(ctx: i64): bool:
    if http.Header(ctx, "Authorization") == "Bearer demo-token":
        ret true
    end
    http.SetStatus(ctx, 401)
    http.Json(ctx, `{"error":"missing or wrong token"}`)
    ret false
end

func list(ctx: i64):
    middleware.Secure(ctx)
    http.Json(ctx, json.Marshal(items))
end

func show(ctx: i64):
    middleware.Secure(ctx)
    id := conv.AtoiOr(http.Param(ctx, "id"), 0 - 1)
    each it in items:
        if it.id == id:
            http.Json(ctx, json.Marshal(it))
            ret
        end
    end
    http.SetStatus(ctx, 404)
    http.Json(ctx, `{"error":"not found"}`)
end

func create(ctx: i64):
    middleware.Secure(ctx)
    if not authorized(ctx): ret end
    name := json.Get(http.Body(ctx), "name")
    if name == "":
        http.SetStatus(ctx, 400)
        http.Json(ctx, `{"error":"name is required"}`)
        ret
    end
    item := Item { id = items.len() + 1, name = str.Clone(name) }
    items.push(item)
    http.SetStatus(ctx, 201)
    http.Json(ctx, json.Marshal(item))
end

func main:
    mut r := router.NewRouter()
    router.Group(r, "/api/v1", "GET", "/items", list)
    router.Group(r, "/api/v1", "GET", "/items/:id", show)
    router.Group(r, "/api/v1", "POST", "/items", create)
    mut s := http.NewServer(0)
    http.Use(s, r)
    port := http.RunBackgroundPort(s)
    base := "http://127.0.0.1:" + port + "/api/v1"

    a := http.Post(base + "/items", `{"name":"tea"}`)
    println http.Status(a), http.Body(a)

    mut auth := http.NewHeaders()
    auth.insert("Authorization", "Bearer demo-token")
    auth.insert("Content-Type", "application/json")
    b := http.PostWith(base + "/items", `{"name":"tea"}`, auth)
    println http.Status(b), http.Body(b)

    c := http.Get(base + "/items/1")
    println http.Status(c), http.Body(c), http.GetHeader(c, "x-frame-options")
    println http.Status(http.Get(base + "/items/9"))
    println http.Body(http.Get(base + "/items"))
end
```

```output
401 {"error":"missing or wrong token"}
201 {"id":1,"name":"tea"}
200 {"id":1,"name":"tea"} DENY
404
[{"id":1,"name":"tea"}]
```

`json.Marshal` turns the struct into JSON using the `@json` field names. For real authentication, the [jwt package](/std/jwt/) signs and verifies tokens and guards routes, [net.http.session](/std/net/http/session/) keeps logins in a cookie with CSRF tokens, [net.http.validate](/std/net/http/validate/) checks form input and answers with a 400, and [net.http.swagger](/std/net/http/swagger/) documents the API with OpenAPI and serves the documentation page. The same `http` package is also a client, as the example's `http.Post`, `http.PostWith` and `http.Get` calls show.

Notice `str.Clone(name)` in `create`. Values read from a request belong to that request and are freed when the handler returns; anything stored for later must be copied. Forgetting this is the most common mistake in a first Salam server, and it is the kind of rule a garbage-collected language never makes you think about.

## How the server works, and how fast it is {#performance}

On Linux, the server accepts connections on one thread and hands them to an epoll-based pool with one worker per CPU core, so a keep-alive connection does not hold a thread while it is idle. On other systems, each connection gets a thread. Request parsing, routing and response writing are Salam code in `std/net/http`, compiled with your program; there is no C web library underneath.

The project measured this server against eight others answering the same eleven routes on the same machine, on 28 October 2026. The geometric mean over all eleven routes:

| Server              | Requests per second |
| ------------------- | ------------------- |
| Salam (`net.http`)  | 94,850              |
| Go, Fiber           | 62,493              |
| Go, Echo            | 35,238              |
| Go, Gin             | 34,999              |
| Go, `net/http`      | 34,987              |
| Node, Express 5     | 22,536              |
| PHP built-in server | 11,992              |

On the seven routes nginx can answer from its own configuration, Salam measured 94,156 requests per second, Fiber 69,653 and nginx 60,490. The widest gap was query parsing on `/search`: Salam 99,143, Fiber 48,247, Go's `net/http` 33,193. The narrowest was `/`, where Salam was only 4% ahead of Fiber.

Read these numbers with the caveats the [speed page](/speed/) itself gives:

- The machine was a shared 6-vCPU AMD EPYC sandbox with a load average around 11 from unrelated work. Servers were pinned to 4 cores and the load generator to the other 2, with 64 keep-alive connections.
- Background load favors servers that keep few threads, Salam's included. On a quiet machine the lead over Fiber on the geometric mean was 16%; under load it was 52%.
- The static file row partly measures caching: Salam keeps static files in memory after the first request, while nginx was configured without `open_file_cache`.
- The repository's [BENCHMARK.md](https://github.com/SalamLang/Salam/blob/main/tests/en/apps/httpbench/bench/BENCHMARK.md), regenerated later on a 4-vCPU GitHub-hosted runner, shows a narrower picture: Salam and Fiber close on the overall mean, and Fiber and nginx slightly ahead on the routes nginx serves itself. Hardware and load change the ranking.

The honest summary: Salam's server is in the same class as the fastest Go frameworks and well ahead of Express and PHP's built-in server in these tests. Rerun the [harness](https://github.com/SalamLang/Salam/tree/main/tests/en/apps/httpbench) on your own hardware before you plan capacity with it. For the language-level benchmark, where Salam lands in the same range as C and Rust, see [Is Salam faster than C and Rust?](/blog/is-salam-faster-than-c-and-rust/)

## WebSocket servers and broadcast rooms {#websocket}

[net.websocket](/std/net/websocket/) is a two-way connection that stays open, with a `Hub` for rooms. The server-side is `Listen`, `Accept`, `Receive` and `SendText`; `Register` puts a connection in a room and `Broadcast` sends to everyone in it. `LoadWSSConfig` and `AcceptWSS` add TLS, and `Poll` and `Ping` handle heartbeats without blocking.

```salam
import net.websocket
import str

mut server := websocket.Server {}
mut hub := websocket.NewHub()

func accept two():
    repeat 2:
        conn := websocket.Accept(server)
        room := str.Replace(conn.path, "/rooms/", "")
        websocket.Register(hub, room, conn)
    end
end

func main:
    server = websocket.Listen(0)
    port := websocket.Port(server)
    t := spawn(accept two)
    mut ali := websocket.Dial("127.0.0.1", port, "/rooms/prices")
    mut sara := websocket.Dial("127.0.0.1", port, "/rooms/prices")
    join(t)
    sent := websocket.Broadcast(hub, "prices", `{"symbol":"GOLD","price":2411.5}`)
    println "delivered to", sent, "clients"
    println "ali got:", websocket.ReceiveText(ali)
    println "sara got:", websocket.ReceiveText(sara)
    websocket.Close(ali)
    websocket.Close(sara)
    websocket.CloseServer(server)
end
```

```output
delivered to 2 clients
ali got: {"symbol":"GOLD","price":2411.5}
sara got: {"symbol":"GOLD","price":2411.5}
```

The room name comes from the path the client connected to. A real chat server accepts in a loop and gives each connection its own thread or task; the reference page has a complete chat server and a token check during the handshake. The WebSocket server listens on its own port, separate from the `net.http` server. For Socket.IO there is a client in [net.socketio](/std/net/socketio/).

## gRPC and HTTP/2 {#grpc}

[grpc](/std/grpc/) implements gRPC over Salam's own [HTTP/2 package](/std/net/http2/), with unary and streaming calls, metadata and the standard status codes. Messages are protocol buffers, read and written with [encoding.protobuf](/std/encoding/protobuf/), which can also read `.proto` files and generate Salam code from them. For a quick service, the `Str` variants send plain strings:

```salam
import grpc
import sync

mut server := grpc.NewServer(0)
mut ready := sync.NewWaitGroup()

func greet(call: i64):
    mut ok := false
    name := grpc.RecvStr(call, ok)
    if not ok or name == "":
        grpc.Fail(call, grpc.GRPC_INVALID_ARGUMENT, "a name is required")
        ret
    end
    _ := grpc.SendStr(call, "salam, " + name)
    grpc.SetStatus(call, grpc.GRPC_OK, "")
end

func serve():
    _ := grpc.Listen(server)
    sync.Done(ready)
    _ := grpc.ServeCount(server, 1)
end

func main:
    grpc.RegisterMethod(grpc.MethodPath("hello.Greeter", "Greet"), greet)
    sync.Add(ready, 1)
    t := spawn(serve)
    sync.Wait(ready)

    mut ch := grpc.Dial("127.0.0.1", grpc.Port(server))
    grpc.SetTimeout(ch, 5000)
    mut st := grpc.Status {}
    reply := grpc.UnaryCallStr(ch, "/hello.Greeter/Greet", "Sara", st)
    println grpc.StatusString(st), reply
    empty := grpc.UnaryCallStr(ch, "/hello.Greeter/Greet", "", st)
    println grpc.StatusString(st), "[" + empty + "]"
    grpc.CloseChannel(ch)
    join(t)
    grpc.StopServer(server)
    sync.DestroyWaitGroup(ready)
end
```

```output
OK salam, Sara
INVALID_ARGUMENT: a name is required []
```

The server serves one connection here so the program ends by itself; a real service calls `grpc.Serve`. Errors travel as gRPC status codes, so a client written in Go, Python or any other language with gRPC support sees `INVALID_ARGUMENT` exactly as it would from any gRPC server. For JSON-RPC 2.0 and MessagePack-RPC there is [rpc](/std/rpc/), and for GraphQL a server in [graphql](/std/graphql/).

## salam serve: a config-driven web server {#serve}

Not every site needs a program. `salam serve` runs the web server from the [net.httpd](/std/net/httpd/) package: in its simple mode it serves one folder for development, and with `--config` it runs several sites from a file written in Salam's block syntax.

```bash
salam serve dist --port=8080                 # preview a folder
salam serve --config=web.salam --test        # check a config, open no port
salam serve --config=web.salam               # run every site in it
```

The config mode has virtual hosts chosen by the `Host` header, nginx-style location matching (exact, prefix, `^~` and regular expressions), redirects, cache headers, ETag and `304` responses, byte ranges, error pages, an access log and FastCGI backends such as php-fpm. It does not do TLS or reverse proxying in the current version, so the [salam serve lesson](/learn/serve/) puts Caddy or nginx in front for HTTPS. The same server can be embedded in a Salam program, since the command is a thin shell around the package.

## Limits to know before you build on it {#limits}

- **Young code.** The server, the router and the protocol packages are recent and change between releases. Pin a compiler version for production.
- **Manual memory rules.** Request values are freed when the handler returns; copy what you keep, and use `http.Own` to hand a string you built back to the request for freeing.
- **No async/await.** Concurrency is threads and task pools, covered in [Concurrent programming in Salam](/blog/concurrency-in-salam/). A handler that blocks on a slow database call keeps its worker busy.
- **Smaller ecosystem.** There is no equivalent yet of the large plugin ecosystems around Express or Gin, and few production deployments to learn from.
- **`salam serve` is not nginx.** No TLS and no proxy_pass today.

## Frequently asked questions {#faq}

### Is Salam's HTTP server production ready?

The project's own Salam AI backend is built on it and its benchmark harness is public, but the language is pre-1.0 and the server is young. Run it behind a TLS-terminating proxy or with `http.RunTLS`, set body and header limits, and load-test your own workload before relying on it.

### Is Salam faster than Go for web servers?

In the project's router benchmark of 28 October 2026, Salam's `net/http` answered about 2.7 times as many requests per second as Go's standard `net/http` and was 16% to 52% ahead of Fiber on the geometric mean. A later run on different hardware showed Fiber much closer. Treat it as "the same class as the fastest Go frameworks", and measure your own case.

### Does Salam support WebSocket and gRPC without extra libraries?

Yes. `net.websocket`, `net.http2` and `grpc` are part of the standard library, written in Salam, with no external C library to install.

### Can I deploy a Salam web server to the cloud?

A server builds to one executable for Linux, macOS or Windows, which fits a container or a plain VM. A `net/http` program can also be built to WebAssembly for Cloudflare Workers, with the limits described in [full-stack development with Salam](/blog/full-stack-development-with-salam/).
