---
title: Where Salam could fit in industry: web services, cloud infrastructure, enterprise back offices and systems software
seo_title: Salam in Industry: Cloud, Enterprise and Systems Software Fit
description: An honest look at where Salam could fit in industry: web services, cloud infrastructure, enterprise software and systems tools, and the barriers to adoption.
keywords: industrial use, enterprise software, cloud infrastructure, Salam in production, programming language for backend services, systems software language, GPL compiler license, static binary deployment, Persian business software, language adoption risk
category: future
summary: Salam has the technical pieces for web services, cloud tools, back-office software and systems work. It does not yet have the stability promise, ecosystem or support model most companies need. This is a map of the fit and the gaps.
date: 2026-11-11
---

Salam could fit in industry wherever a team wants native performance, single-binary deployment and a large built-in standard library: HTTP and gRPC services, command-line and infrastructure tools, data-processing jobs, and back-office software, especially for Persian-speaking organizations. **Industrial use of a programming language means running it in software that a business depends on, with the stability, support, hiring and licensing that implies.** On that definition, Salam today is a candidate rather than an incumbent. The project publishes no case studies or list of production users, so this article is about technical fit and the conditions for adoption, not about who is already using it.

## What industry actually asks of a language {#criteria}

Engineering managers rarely choose a language for its syntax. The questions that decide adoption are more mundane:

1. **Does it do the job fast and predictably?** Throughput, latency, memory use, start-up time.
2. **Can we deploy and operate it?** Build artifacts, containers, cross-compilation, observability.
3. **Can we hire for it and train people?** Talent pool, learning curve, documentation.
4. **Will it still be there, and still compatible, in five years?** Stability policy, maintainers, release process.
5. **Can we use it legally in our product?** License terms for the compiler, the runtime and the standard library.
6. **Who helps when it breaks?** Support channels, security response, commercial options.

Salam answers the first two well, the third and fourth weakly, and the fifth with a condition that every company must check. The rest of this article goes through each domain with those questions in mind; the companion piece [What makes a language ready for production?](/blog/what-makes-a-language-production-ready/) runs a full checklist.

## Web services and APIs {#web-services}

This is the most natural industrial fit. The standard library includes an HTTP server with a router and middleware, HTTP/2, WebSocket, gRPC, GraphQL, TLS 1.3 and 1.2 with no OpenSSL dependency, sessions, validation and Swagger generation. A small inventory service looks like this:

```salam
import net.http
import net.router
import encoding.json
import env

mut items := Vector {} as Vector<Stock>

struct Stock:
    @json "sku"
    pub sku: str = ""
    @json "count"
    pub count: int = 0
end

func health(ctx: i64):
    http.Json(ctx, `{"status":"ok"}`)
end

func stock(ctx: i64):
    sku := http.Param(ctx, "sku")
    each s in items:
        if s.sku == sku:
            http.Json(ctx, json.Marshal(s))
            ret
        end
    end
    http.SetStatus(ctx, 404)
    http.Json(ctx, `{"error":"unknown sku"}`)
end

func main:
    items.push(Stock { sku = "RACK-42U", count = 7 })
    items.push(Stock { sku = "CABLE-CAT6", count = 310 })
    mut r := router.NewRouter()
    router.Get(r, "/healthz", health)
    router.Get(r, "/stock/:sku", stock)
    port := env.GetOr("PORT", "8080")
    mut s := http.NewServer(port.to_int())
    http.Use(s, r)
    http.Run(s)
end
```

The project measured its HTTP stack against eight other servers on eleven routes. On that run, Salam's `net.http` answered a geometric mean of 94,850 requests per second against 62,493 for Go's Fiber framework and 34,987 for Go's standard library. The caveats matter as much as the numbers: it was a shared 6-vCPU sandbox with unrelated load, the margin over Fiber was 16% on a quiet machine and 52% under that load, and on the `/` route the lead was only 4%. The [speed page](/speed/) publishes the full table and the harness. Read it as evidence that the HTTP stack is in the same class as fast compiled servers, not as a guarantee for your workload. The [web server article](/blog/fast-web-servers-with-salam/) goes deeper.

For operations, `salam serve --config` runs a multi-site web server from a config file with virtual hosts, redirects and FastCGI backends such as php-fpm, and the same `net.http` program can be built to `wasm32-wasi` and deployed to Cloudflare Workers.

## Cloud infrastructure and DevOps tooling {#cloud}

Infrastructure teams care about artifacts. A Salam program builds to one native executable with no virtual machine and no garbage collector, so the container image can be little more than the binary. On x86-64 Linux, the native backend's built-in linker produces a static executable against musl. Cross-compilation is a flag: `--target=x86_64-w64-windows-gnu` or an ARM triple builds for another platform from the same machine, as the [cross-compilation article](/blog/cross-compilation-salam/) shows. The compiler itself ships as a Docker image (`basemax/salam`) for x86-64, ARM64, ARMv7 and i386.

The kinds of tools that fit this profile are the ones Go became popular for: agents, CLIs, small proxies, exporters, deployment helpers. The standard library covers much of what they need: processes and shells, a pseudo-terminal package, SSH, TLS, DNS, NATS, cron schedules, and clients for Redis, ClickHouse, Elasticsearch and OpenSearch.

What is missing for cloud work is the surrounding ecosystem rather than the language. There are no official SDKs from cloud vendors (the standard library has an `aws` package for S3, SQS and STS and a `google` package for service account authentication, far from the breadth of the official SDKs for Go or Python), no OpenTelemetry library, and no Kubernetes client. Teams would write those integrations themselves or call vendor REST APIs over HTTP.

## Enterprise back-office software {#enterprise}

Line-of-business software is mostly data in, rules applied, data out. Salam brings drivers for SQLite, PostgreSQL, MySQL and ClickHouse, a `salam sql check` command that validates SQL strings without a database, and `salam sql generate`, which turns annotated queries into typed Salam functions (the [typed SQL article](/blog/typed-sql-database-apps/) covers both). For money there is an exact `decimal` package, and for dates a calendar package with Jalali, Hijri and Gregorian calendars and time zones.

That combination matters for one market in particular. Business software for Iran runs on the Jalali calendar and Persian text, and the standard library treats both as first-class. A small invoice with exact decimal arithmetic and a Jalali due date:

```salam
import decimal
import calendar

struct Line:
    pub item: str
    pub price: str
    pub qty: int
end

func money(text: str): decimal.Dec:
    mut ok := false
    ret decimal.Parse(text, ok)
end

func main:
    lines := [
        Line { item = "Server rack", price = "1250.00", qty = 2 },
        Line { item = "Support hours", price = "45.50", qty = 12 },
        Line { item = "Cable set", price = "19.99", qty = 3 },
    ]
    mut total := decimal.FromInt(0)
    each l in lines:
        cost := decimal.MulInt(money(l.price), l.qty as i64)
        println l.item, "x", l.qty, "=", decimal.ToStrFixed(cost, 2)
        total = decimal.Add(total, cost)
    end
    tax := decimal.RoundHalfUp(decimal.Percent(total, money("9"), 4), 2)
    println "subtotal", decimal.FormatGrouped(total, 2, ",", ".")
    println "tax 9%  ", decimal.FormatGrouped(tax, 2, ",", ".")
    println "total   ", decimal.FormatGrouped(decimal.Add(total, tax), 2, ",", ".")
    issued := calendar.GregorianToJalali(calendar.GregorianDate { year = 2026, month = 11, day = 11 })
    due := calendar.AddDaysJalali(issued, 30)
    println "issued", calendar.FormatJalali(issued), "due", calendar.FormatJalali(due), calendar.JalaliMonthNameEn(due.month)
end
```

```output
Server rack x 2 = 2500.00
Support hours x 12 = 546.00
Cable set x 3 = 59.97
subtotal 3,105.97
tax 9%   279.54
total    3,385.51
issued 1405-08-20 due 1405-09-20 Azar
```

The same program could be written in Persian, with Persian identifiers and Persian error messages, which is a real advantage for teams whose developers and domain experts read Persian more comfortably than English. There are also packages for Excel files, email, images and a Telegram bot client, which is how many Iranian businesses reach customers. The [standard library tour](/blog/salam-standard-library-tour/) lists the rest.

The counterweight is that enterprise software lives for decades and changes hands many times. A pre-1.0 language with a small talent pool is a harder sell here than anywhere else.

## Systems software and embedding {#systems}

At the low end, Salam reaches places most young languages do not. It can produce freestanding binaries with no operating system, and the repository contains a small test kernel that boots under QEMU (see the [bare-metal article](/blog/freestanding-salam-bare-metal/)). It calls C through `extern` blocks, so existing C libraries are usable without rewriting them.

For industry the more practical feature runs the other way. `salam build --lib` produces a shared or static library plus a C header for the functions in an `export:` block. That means a team can write one component in Salam, a parser, a pricing engine, a protocol handler, and link it into an existing C, C++, Go or Rust system through the C ABI. Introducing a new language as a library behind a C interface is far less risky than rewriting a service, and it is how most organizations should trial an unfamiliar language.

## The barriers, stated plainly {#barriers}

A fair assessment has to put these next to the strengths.

- **License.** The compiler, the standard library and the tools are GPL-3.0. The project's README describes Salam code that imports the standard library as GPL-derived once built, and the license file has no runtime-library exception of the kind GCC's runtime libraries carry. Companies that distribute binaries to customers therefore take on GPL-3.0 obligations. Software that is only run internally, or only served over a network, is treated differently by GPL-3.0, which unlike the AGPL does not count network use as distribution. This is the single most important question for a legal team, and it needs their answer, not a blog's.
- **Stability.** Salam is version 0.6 and still changes syntax between releases. Typed local declarations were removed in 2026, and in version 0.6.3 self-assignments such as `x = x + 1` became compile errors. There is no long-term-support release and no written compatibility promise yet.
- **Concentration.** Most commits come from the language's designer. That is normal for a young language and a real continuity risk for a business.
- **Talent.** There is effectively no hiring market for Salam developers. Teams would train their own, which the syntax makes reasonable but not free.
- **Support and security process.** There is no commercial support offering and no published security policy (`SECURITY.md`) describing how vulnerabilities are reported and fixed.
- **Memory safety.** Salam has bounds checks, ownership warnings and a memory checker, but no borrow checker. For security-critical systems code, Rust offers stronger guarantees.

## How a sensible pilot would look {#pilot}

For an organization that finds the fit interesting, the low-risk path is narrow and reversible:

1. **Choose a contained component**: an internal CLI, a batch job, a small internal API, or a library behind a C interface.
2. **Pin the compiler version.** Install one exact release in CI (the installer takes `--version X.Y.Z`), record the oldest supported compiler in `salam.toml` (`salam = ">=0.6.2"`), and upgrade deliberately.
3. **Get the license reviewed** for your distribution model before writing production code.
4. **Measure your own workload.** The published benchmarks are short programs and one HTTP harness; your service is neither.
5. **Plan the exit.** Keep interfaces language-neutral (HTTP, gRPC, a C ABI) so the component can be replaced if the experiment fails.

That is less exciting than a rewrite, and it is how languages such as Go and Rust actually entered most companies: one tool, one service, one team at a time. The [future of Salam](/blog/future-of-salam/) article looks at what the project itself would need to do to make that path easier.

## Frequently asked questions {#faq}

### Is Salam used in production?

The project does not publish case studies or a list of production users, so there is no public evidence to point to. Treat Salam as a language to pilot on contained components rather than one with a proven production track record.

### Can I use Salam for commercial software?

Salam is GPL-3.0, and the project considers programs that import the standard library to be GPL-derived. Commercial use is possible under the GPL's terms, but distributing proprietary binaries is a legal question you should take to counsel before starting.

### Is Salam a good choice for microservices?

Technically it fits well: native single binaries, fast HTTP and gRPC support, and drivers for common databases. The risks are the pre-1.0 stability, the small ecosystem (no cloud vendor SDKs or OpenTelemetry library) and the small talent pool.

### Can Salam code be called from an existing C or C++ system?

Yes. `salam build --lib` builds a shared or static library with a C header for the functions you export, so a Salam component can be linked into C, C++ or any language with a C foreign function interface.
