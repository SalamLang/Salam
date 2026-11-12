---
title: From native to WebAssembly and JavaScript: everywhere a Salam program can run
seo_title: Salam to WebAssembly and JavaScript: Every Place It Runs
description: Compile Salam to WebAssembly with WASI, to browser JavaScript and to Cloudflare Workers: the commands, what each target supports and the limits to know first.
keywords: Salam WebAssembly, compile to WebAssembly, compile to JavaScript, wasm32-wasi, WASI, Cloudflare Workers WebAssembly, salam js, browser programming language, WebAssembly runtime, online compiler in the browser
category: compiler
summary: One source file, several destinations: native executables, WASI modules, browser JavaScript, web pages, edge workers, an in-browser editor, phone apps and bare metal, with the trade-offs of each.
date: 2026-11-11
---

Salam programs can run as native executables, as WebAssembly modules, as JavaScript in a web browser, as request handlers on Cloudflare Workers, inside an online editor and on bare metal. To compile Salam to WebAssembly you build with `--target=wasm32-wasi`; to compile Salam to JavaScript you use `salam js`. The language and the standard library stay the same, but each destination has its own rules about threads, files, sockets and numbers, and those rules decide which target fits a job.

This article walks through every target that exists today, shows the commands, and lists what each one cannot do yet.

## Where Salam programs run {#targets}

| Target             | Command                                      | Runs on                                     | Good for                                    |
| ------------------ | -------------------------------------------- | ------------------------------------------- | ------------------------------------------- |
| Native executable  | `salam build app.salam`                      | Linux, macOS, Windows (x86-64, ARM, 32-bit) | Tools, servers, desktop apps                |
| WebAssembly (WASI) | `salam build app.salam --target=wasm32-wasi` | Any WASI preview 1 runtime                  | Sandboxed modules, plug-ins, edge functions |
| Cloudflare Workers | WASI build plus a small `worker.js`          | Cloudflare's edge                           | HTTP handlers close to users                |
| JavaScript         | `salam js app.salam`                         | Browsers and Node                           | Interactive pages, browser logic            |
| Web project        | `salam web`                                  | Browsers                                    | A page plus its Salam scripts in one build  |
| Online editor      | `editor.salamlang.ir`                        | Any modern browser                          | Learning, sharing, quick experiments        |
| Phone IDE          | Android and iOS apps                         | Phones and tablets                          | Writing and running code offline            |
| Freestanding       | `--target=x86_64-unknown-none`               | No operating system                         | Kernels and boot payloads                   |

The native targets have their own article on [cross-compilation](/blog/cross-compilation-salam/). The rest of this one is about the others.

The program below runs on three of them unchanged. It uses compile-time flags to say where it is running: `SALAM_JS` is defined only when the JavaScript backend compiles the file, and `SALAM_ARCH_WASM` is true for WebAssembly builds.

```salam
func is prime(n: int): bool:
    if n < 2: ret false end
    mut d := 2
    until d * d <= n:
        if n % d == 0: ret false end
        d++
    end
    ret true
end

func main:
    mut found := ""
    mut count := 0
    repeat 2 to 50 in n:
        if is prime(n):
            found += " " + n
            count++
        end
    end
    println "primes below 50:" + found
    println "count:", count
    if defined(SALAM_JS):
        println "running as JavaScript"
    else SALAM_ARCH_WASM:
        println "running as WebAssembly"
    else:
        println "running as native code"
    end
end
```

```output
primes below 50: 2 3 5 7 11 13 17 19 23 29 31 37 41 43 47
count: 15
running as native code
```

Compiled with `salam js` and run with Node, the last line reads `running as JavaScript`; built with `--target=wasm32-wasi` and run under Node's WASI support, it reads `running as WebAssembly`. The first two lines are identical everywhere.

## Compiling Salam to WebAssembly with WASI {#wasi}

WebAssembly is a portable binary format that runs inside a sandbox. On its own it has no way to print, read a file or look at the clock; WASI (the WebAssembly System Interface) adds those as a small set of imported functions. Salam targets WASI preview 1 through its LLVM backend:

```bash
salam build primes.salam --target=wasm32-wasi --release --output=primes.wasm
```

The release builds of the compiler carry a WASI sysroot and run the LLD linker in-process, so for a program like the one above nothing else needs to be installed. The release workflow checks this on every release by building a small program with `WASI_SDK_PATH` unset and a minimal `PATH`, then running it with Node's built-in `node:wasi` module. A runner script for Node is a few lines:

```javascript
const { WASI } = require('node:wasi');
const fs = require('node:fs');

const wasi = new WASI({
  version: 'preview1',
  args: ['primes'],
  returnOnExit: true,
});
WebAssembly.instantiate(
  fs.readFileSync('primes.wasm'),
  wasi.getImportObject(),
).then(({ instance }) => process.exit(wasi.start(instance)));
```

The module for the prime program is about 30 KB. Programs that pull in more of the runtime need more of the C library: a program that uses the HTTP server stack also needs threading support, and the WASI sysroot embedded in the 0.6.2 release could not link one (the linker reported a missing `libpthread`). For those, install wasi-sdk; the compiler finds it through `WASI_SDK_PATH`, `~/.salam/wasi-sdk` or `/opt/wasi-sdk`.

What a WASI module can do is set by its host. Standard output and error, arguments, environment variables, clocks and random numbers are part of preview 1. Files appear only when the host grants a directory, and the module cannot open network connections of its own.

## Salam on Cloudflare Workers {#workers}

Cloudflare Workers can run WebAssembly modules, and Salam uses that to put a whole [net/http](/std/net/http/) program at the edge. The program is an ordinary Salam web server built with [net/router](/std/net/router/):

```salam
import net.http
import net.router

func hello(ctx: i64):
    where := http.IsEdge() ? "at the edge" : "on a server"
    http.Text(ctx, "Hello, " + http.Param(ctx, "name") + ", from Salam " + where + "\n")
end

func main:
    mut r := router.NewRouter()
    router.Get(r, "/hello/:name", hello)
    mut s := http.NewServer(8080)
    http.Use(s, r)
    http.Run(s)
end
```

Built natively, this listens on port 8080. Built with `--target=wasm32-wasi`, `http.Run` changes behavior: it serves exactly one request per process. The host script `std/net/http/cloudflare/worker.js` writes the raw HTTP/1.1 request to the module's standard input and reads the response from file descriptor 3; `println` output goes to the Worker's logs, and `os.Env` reads wrangler variables and secrets. The Worker entry point is three lines:

```javascript
import { createWorker } from './worker-host.js';
import app from './build/app.wasm';
export default createWorker(app);
```

The limits follow from the model. Each request gets a fresh instance, so global variables do not survive between requests. There is no file system, so `router.Static` has nothing to serve (use Workers static assets). There are no outbound sockets, so `http.Get` cannot call another server from inside the module; a few lines of `fetch` in `worker.js` do that instead. Workers storage such as KV, D1 and R2 is asynchronous and has to be exchanged through the host script.

The repository holds four complete Workers apps under `tests/en/apps/`: `cloudflare-worker-hello`, `-router`, `-telegram-bot`, and `-pure`, a CGI-style program that reads the request from environment variables and needs no `net/http` at all. The CGI-style app links with the embedded WASI sysroot; the `net/http` ones need wasi-sdk, as described above.

## Compiling Salam to JavaScript with salam js {#javascript}

For code that has to run in a browser tab and touch the page, Salam has a JavaScript backend. It turns a program into one JavaScript file, minified by default, or into a self-contained HTML page when the output name ends in `.html`:

```bash
salam js counter.salam --output=counter.js
salam js counter.salam --output=counter.html
```

The [dom](/std/dom/) package gives Salam code access to the page: creating elements, setting text and attributes, styles, classes, events, timers and animation frames.

```salam
import dom

mut clicks := 0
mut label := 0 as i64

func main:
    label = dom.Create("p")
    dom.SetText(label, "No clicks yet")
    btn := dom.Create("button")
    dom.SetText(btn, "Click me")
    dom.OnEvent(btn, "click", ():
        clicks++
        dom.SetText(label, "Clicked " + clicks + " times")
    end)
    dom.Append(dom.Body(), btn)
    dom.Append(dom.Body(), label)
end
```

Compiled to `counter.html`, the whole page with its script is under 1 KB. The generated code is plain JavaScript: no WebAssembly, no runtime library to download, and calls such as `dom.SetText` become direct DOM operations. The `tests/en/js` directory holds about a hundred programs that are compiled with `salam js`, run, and compared with their expected output.

The JavaScript backend has limits that come from JavaScript itself:

- **64-bit integers.** JavaScript numbers hold integers exactly only up to `2^53`. On this backend an `i64` beyond that loses precision: `9007199254740993 + 2` prints `9007199254740995` natively and `9007199254740994` in JavaScript. `i128` and `u128` are exact, because they become BigInts.
- **Threads.** `spawn` is not supported on this backend, and the atomic intrinsics become plain reads and writes, since the generated code runs on the page's single thread.
- **System access.** Files, sockets and processes belong to native and WASI targets. In the browser the `dom` package gives you the page, events, timers, localStorage, the canvas, and generic property and method access to any JavaScript object.

## Web projects: salam web and the page language {#web}

Salam also has a page language: a `page:` block describes HTML elements, CSS styles and events in Salam syntax, and `salam page build` turns it into HTML, CSS and JavaScript. `salam web` builds a whole web project, a page plus the Salam programs it loads with `script: src="app.salam"`, into one `.html` file, or into separate `.html`, `.css` and `.js` files with `--split`. The article [web development with Salam](/blog/web-development-with-salam/) and the course lessons from [page text](/learn/page-text/) onward cover the page language.

## The online editor: the compiler in a browser tab {#editor}

The [online editor](https://editor.salamlang.ir) runs Salam with no server and no installation, and it is a good example of these targets working together:

- The Salam compiler (lexer, parser, semantic analysis, the interpreter and the layout engine) is emitted as C with `--backend=c --emit-c --target=wasm32-unknown-emscripten` and compiled to WebAssembly with Emscripten, together with a minified copy of the standard library.
- The editor's own interface is written in Salam: `page.salam` holds the structure and styles and is built with `salam web`, and `app.salam`, which handles state, syntax highlighting, keyboard shortcuts and the WebAssembly boot, is compiled with `salam js`.
- Programs typed into the editor run in the interpreter inside the WebAssembly module, because a browser cannot run a C compiler or load a freshly built native binary.

Because programs run in the interpreter, the editor is ideal for learning and for checking logic, while code that needs threads, FFI or full native speed should be built with the compiler on your machine. The article [salam run vs salam build](/blog/salam-run-vs-salam-build/) explains the interpreter.

## Android, iOS and bare metal {#mobile-and-bare-metal}

The `android/` and `ios/` projects in the repository are native IDE apps for phones. Both embed the whole compiler, emitted as C from a library entry point, so highlighting, checking and running work offline. Programs run in the interpreter: neither Android nor iOS lets an app run a binary it just built, so `salam build` stays a desktop command. The Android app supports Android 5.0 and later, the iOS app iOS 15 and later. An Android APK is attached to a release only when that release published one.

At the other extreme, a `*-unknown-none` target builds a program with no operating system, no libc and no system calls, for kernels and boot payloads. The compiler links a small runtime with an allocator and serial, VGA and QEMU debug-console output, and `tests/kernel/` boots under QEMU. That target has its own article, [freestanding Salam](/blog/freestanding-salam-bare-metal/).

For desktop applications with a web interface, Salam uses a system WebView rather than WebAssembly; see [desktop GUI apps with Salam](/blog/desktop-gui-apps-with-salam/).

## Choosing a target {#choosing}

| You need                                                      | Choose                                                   |
| ------------------------------------------------------------- | -------------------------------------------------------- |
| Maximum speed, threads, files, sockets, C libraries           | Native build                                             |
| A sandboxed module a host can embed and limit                 | `wasm32-wasi`                                            |
| An HTTP handler at the edge with no server to run             | `wasm32-wasi` on Cloudflare Workers                      |
| Logic that runs in the visitor's browser and changes the page | `salam js`                                               |
| A static or interactive site written in Salam                 | The page language with `salam page build` or `salam web` |
| Learning or sharing a snippet                                 | The online editor                                        |
| An operating system or bare-metal program                     | A freestanding `*-none` target                           |

A sensible pattern for a web product is to keep shared logic (validation, formatting, pure calculations) in plain Salam packages with no system calls, compile it natively for the server and with `salam js` for the browser, and keep platform code at the edges behind `if defined(SALAM_JS):` or `if SALAM_ARCH_WASM:`.

## Frequently asked questions {#faq}

### Can Salam compile to WebAssembly?

Yes. `salam build app.salam --target=wasm32-wasi` produces a WASI preview 1 module. Release builds of the compiler contain the sysroot and linker needed for simple programs; programs that use the HTTP server stack need wasi-sdk installed.

### Does Salam compile to JavaScript?

Yes. `salam js app.salam` writes a minified JavaScript file, or a complete HTML page when the output ends in `.html`. The `dom` package gives access to elements, events and timers. Integers in `i64` lose precision above `2^53` on this backend.

### Can I run Salam in the browser without installing anything?

Yes. The online editor at editor.salamlang.ir runs the compiler and interpreter as WebAssembly in the page, with English, Persian and Arabic interfaces. Programs there run in the interpreter, so very heavy computations are better built natively.

### Can Salam run on Cloudflare Workers?

Yes. A `net/http` and `net/router` program built for `wasm32-wasi` serves one request per instance through a host script that ships with the standard library. Globals do not persist between requests, and outbound calls go through `fetch` in the host script.
