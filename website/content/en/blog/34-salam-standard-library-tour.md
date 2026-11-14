---
title: A tour of Salam's standard library: from networking and cryptography to databases, images and AI
seo_title: Salam Standard Library Tour: 170+ Packages | Salam Magazine
description: A guided tour of the Salam standard library: text and data formats, files, HTTP and networking, cryptography, databases, math, AI, audio and GUI, with code.
keywords: standard library, Salam std, Salam standard library, batteries included language, Salam packages, std library tour, JSON library, cryptography library, HTTP library, systems programming library
category: build
summary: Salam ships more than 170 packages with the compiler. This tour walks through them by area, runs a few of them, and says where the library is deep, where it is thin, and what it costs to have so much in std.
date: 2026-11-11
---

The Salam standard library is the set of packages that ships with the compiler, so a program can import them with no download, no package manager and no C library to install for most of them. **Salam's standard library has more than 170 documented packages, covering text and data formats, files and processes, HTTP and networking, cryptography and TLS, databases, mathematics, machine learning, audio, images and graphical user interfaces.** Each one has a page in the [library reference](/std/) with an example for every public function, and every package and function also has a Persian name.

This article is a map. It goes through the library area by area, runs a few packages so you can see what typical code looks like, and links to the reference page of each package it mentions. It ends with an honest look at the trade-offs of a "batteries included" library in a young language.

## How the standard library is organized {#organization}

Packages are imported by path, `import encoding.json` or `import net.http`, and used with the last part of the name as a prefix: `json.Marshal`, `http.Get`. The reference sorts them into groups: basics, text, numbers and collections, encoding and data, system and tools, time and concurrency, network, security and cryptography, database, web and services, graphics and games, audio and AI, plus a few smaller ones.

Three properties hold across the library and are worth knowing before you use it:

- **Most of it is written in Salam.** HTTP, TLS 1.3, the PostgreSQL and MySQL protocols, JSON, compression, ONNX inference and the native UI's layout engine are Salam code compiled with your program. A few packages wrap C libraries that already exist and that nobody should rewrite: [db.sqlite](/std/db/sqlite/), [gui.raylib](/std/gui/raylib/), [opencv](/std/opencv/) and the system web view in [gui.webview](/std/gui/webview/).
- **Heap values are yours to free.** Packages that return a `Vector`, a buffer or an owned string say so, and you free them with `.Free()` or the package's `Free` function, usually through `defer`. The [memory lesson](/learn/memory/) explains the rules.
- **Errors use the language's own mechanisms.** Functions that can fail either return a status the caller checks or are marked `throws`, in which case the compiler makes you handle them with `try`.

## Text, data formats and encodings {#text}

The text packages are where most programs start: [str](/std/str/) for searching, slicing, replacing and building strings, [regex](/std/regex/) for patterns, [unicode](/std/unicode/) for character properties, and [template](/std/template/) for text and HTML templates with automatic escaping. Because Salam is bilingual, the library has unusual depth for right-to-left text: [bidi](/std/bidi/) implements the Unicode bidirectional algorithm, [arabic](/std/arabic/) normalizes Arabic-script text (yeh and kaf variants, diacritics, tatweel, the zero width non-joiner), [conv](/std/conv/) reads Persian digits, [calendar](/std/calendar/) converts between the Jalali, Hijri and Gregorian calendars, and [i18n](/std/i18n/) handles translated messages and plurals.

Data formats are just as broad: [encoding.json](/std/encoding/json/) from plain strings through struct marshalling to streaming, [YAML](/std/encoding/yaml/), [TOML](/std/encoding/toml/), [CSV](/std/encoding/csv/), [XML](/std/encoding/xml/) with XPath, [MessagePack](/std/encoding/msgpack/), [Protocol Buffers](/std/encoding/protobuf/) with `.proto` parsing and code generation, [serde](/std/encoding/serde/) for converting between them, [excel](/std/excel/) for xlsx and ods spreadsheets, and [npy](/std/npy/) for NumPy arrays.

```salam
import encoding.json
import regex
import calendar

struct Invoice:
    @json "number"
    pub number: str = ""
    @json "total"
    pub total: int = 0
    @json "paid"
    pub paid: bool = false
end

func main throws:
    inv := Invoice { number = "INV-1403-0042", total = 125000 }
    text := json.Marshal(inv)
    println text

    mut back := Invoice {}
    try json.UnmarshalLenient(`{"number":"INV-7","total":99,"paid":true,"extra":1}`, back)
    println back.number, back.total, back.paid

    digits := try regex.Compile("[0-9]+")
    println regex.FindAll(digits, inv.number)

    day := calendar.JalaliDate { year = 1403, month = 12, day = 29 }
    println calendar.FormatJalali(calendar.AddDaysJalali(day, 1)), calendar.JalaliMonthNameEn(12)
end
```

```output
{"number":"INV-1403-0042","total":125000,"paid":false}
INV-7 99 true
["1403", "0042"]
1403-12-30 Esfand
```

`json.Marshal` uses the `@json` names on the fields, `UnmarshalLenient` ignores the unknown `extra` key, and the regular expression and the calendar work without any setup. The last line shows a detail that only a calendar library that takes Persian dates seriously gets right: 1403 is a leap year in the Jalali calendar, so Esfand has a 30th day.

## Files, processes and the operating system {#system}

For system programming the library has [os](/std/os/) and [fs](/std/fs/) for files, directories and environment variables, [fs.path](/std/fs/path/) for paths, [os.process](/std/os/process/) for running programs with pipes and time limits, [os.pty](/std/os/pty/) for driving interactive programs on a pseudo terminal, [os.dl](/std/os/dl/) for loading shared libraries at runtime, and [os.mmap](/std/os/mmap/) for memory pages with permissions.

Command-line tools get [flag](/std/flag/) for options, [cli](/std/cli/) for programs with subcommands such as `git commit`, [term](/std/term/) for colors, cursor movement, progress bars and key input, [log](/std/log/) for leveled and structured logs, [config](/std/config/) and [env](/std/env/) for settings, and [testing](/std/testing/) for assertions. Time is covered by [time](/std/time/) and [cron](/std/cron/), which schedules jobs from cron expressions.

Concurrency lives here too: [thread](/std/thread/), [sync](/std/sync/), [atomic](/std/atomic/), [chan](/std/chan/), [pool](/std/pool/) and [ctx](/std/ctx/), described in [Concurrent programming in Salam](/blog/concurrency-in-salam/). At the lowest level, [mem](/std/mem/) gives direct allocation, arenas, pools and leak checking.

## Networking, web services and databases {#network}

Networking is the largest group. The web stack is [net.http](/std/net/http/) (server and client), [net.router](/std/net/router/), [middleware](/std/net/http/middleware/), [sessions](/std/net/http/session/), [validation](/std/net/http/validate/), [OpenAPI documentation](/std/net/http/swagger/), [HTTP/2](/std/net/http2/), [WebSocket](/std/net/websocket/), [gRPC](/std/grpc/), [JSON-RPC](/std/rpc/) and [GraphQL](/std/graphql/). [net.httpd](/std/net/httpd/) is a config-driven web server with virtual hosts and FastCGI, which `salam serve` runs. The article [Building fast web servers with Salam](/blog/fast-web-servers-with-salam/) covers this stack with code and benchmark numbers.

Lower down are [net.tcp](/std/net/tcp/), [net.udp](/std/net/udp/), [net.dns](/std/net/dns/) and encrypted DNS in [net.doh](/std/net/doh/). Protocol clients include [net.email](/std/net/email/) (SMTP, IMAP, POP3), [ssh](/std/ssh/) with SFTP, [ftp](/std/ftp/), [net.nats](/std/net/nats/) with JetStream and [net.socketio](/std/net/socketio/). Service clients include [aws](/std/aws/) (S3, SQS and STS with Signature Version 4), [google](/std/google/) service accounts, [telegram.bot](/std/telegram/bot/) and [webpush](/std/webpush/) for browser notifications. A family of proxy packages, from [net.socks](/std/net/socks/) to [net.vpn](/std/net/vpn/), implements SOCKS, HTTP proxies and VPN-style tunnel protocols.

Databases have drivers for [SQLite](/std/db/sqlite/), [MySQL and MariaDB](/std/db/mysql/), [PostgreSQL](/std/db/postgres/), [Redis](/std/db/redis/) and [ClickHouse](/std/db/clickhouse/), search clients for [Elasticsearch](/std/db/elasticsearch/), [OpenSearch](/std/db/opensearch/), [Meilisearch](/std/db/meilisearch/) and [Typesense](/std/db/typesense/), and [db.sql](/std/db/sql/), which checks SQL against your schema without a database. The details are in [Database applications in Salam](/blog/typed-sql-database-apps/).

## Security, cryptography and compression {#security}

[crypto](/std/crypto/) has SHA-1, SHA-2 and MD5 digests, HMAC, secure random numbers and password hashing. [jwt](/std/jwt/) signs and verifies tokens and guards API routes. [tls](/std/tls/) implements TLS 1.3 and 1.2 with no OpenSSL dependency, and [ssl](/std/ssl/) puts a familiar OpenSSL-style interface on top of it. On the data side, [compress](/std/compress/) has DEFLATE, gzip, bzip2, zstd and checksums, and [archive](/std/archive/) creates and extracts ZIP, tar and compressed tar files and reads RAR archives, including password-protected and multi-volume ones.

```salam
import crypto
import jwt
import compress

func main throws:
    println crypto.Sha256Hex("salam")
    println crypto.HmacSha256Hex("server-key", "order=42&amount=1250")

    hash := crypto.HashPassword("correct horse battery staple")
    println crypto.VerifyPassword("correct horse battery staple", hash), crypto.VerifyPassword("wrong", hash)

    mut c := jwt.NewClaims()
    c.sub = "42"
    c.exp = 1900000000
    token := try jwt.SignHS256(c, "a-long-random-secret")
    mut o := jwt.DefaultOptions()
    o.now = 1800000000
    t := try jwt.Verify(token, "a-long-random-secret", o)
    println t.valid, t.claims.sub

    text := "salam salam salam salam salam salam salam salam"
    packed := compress.GzipCompressStr(text)
    mut ok := false
    unpacked := compress.GzipDecompressBuf(packed, ok)
    println text.len(), "bytes ->", packed.len, "bytes gzip, round trip:", ok and compress.BufToStrUnsafe(unpacked) == text
end
```

```output
0582bd2c13fff71d7f40ef5586e3f4da05a3a61fe5ba9f0b4d06e99905ab83ea
c69338adcc451a761f397467d009e15999529518e33201ad7b91085463534298
true false
true 42
47 bytes -> 28 bytes gzip, round trip: true
```

The digest and the HMAC match what `sha256sum` and `openssl dgst -hmac` print for the same input, which is the first thing to check about any cryptography code. Cryptography written in a young language deserves caution: the implementations are tested, but they have not had the years of independent review that OpenSSL, BoringSSL or Go's `crypto` packages have had.

## Numbers, math and data structures {#numbers}

[math](/std/math/) has the usual functions, [rand](/std/rand/) random numbers and shuffling, and [sort](/std/sort/) sorting and binary search. [collections](/std/collections/) adds stacks, queues, deques, sets, linked lists, priority queues and counters on top of the built-in `Vector` and `HashMap`. For exact arithmetic there are [decimal](/std/decimal/) for money and [bigint](/std/bigint/) for integers of any size. For analysis there are [stats](/std/stats/), [matrix](/std/matrix/) for linear algebra and [graph](/std/graph/) for shortest paths, flows and centrality.

```salam
import decimal
import bigint
import stats

func main:
    price := decimal.FromScaled(1999, 2)
    vat := decimal.Percent(price, decimal.FromInt(9), 2)
    println "0.1 + 0.2 as f64:", 0.1 + 0.2
    println "price", decimal.ToStr(price), "VAT", decimal.ToStr(vat), "total", decimal.ToStr(decimal.Add(price, vat))

    println "30! =", bigint.ToStr(bigint.Factorial(30))

    mut ms := Vector {} as Vector<f64>
    defer ms.Free()
    each v in [12.0, 15.0, 11.0, 48.0, 13.0]:
        ms.Push(v)
    end
    println "mean", stats.Mean(ms), "median", stats.Median(ms)
end
```

```output
0.1 + 0.2 as f64: 0.30000000000000004
price 19.99 VAT 1.80 total 21.79
30! = 265252859812191058636308480000000
mean 19.8 median 13
```

The first line is the reason [decimal](/std/decimal/) exists: binary floating point cannot represent 0.1 exactly, and money should never be stored in it. The last line is a small reminder of why medians matter when one value is an outlier.

## AI, images and audio {#ai}

The AI group is unusually large for a systems language. [ai.tensor](/std/ai/tensor/) provides multi-dimensional arrays, [ai.autograd](/std/ai/autograd/) reverse-mode differentiation, [ai.nn](/std/ai/nn/) layers from linear to transformer, [ai.optim](/std/ai/optim/) SGD and Adam, and [ai.ml](/std/ai/ml/) classic machine learning such as regression, trees and clustering. For trained models, [ai.onnx](/std/ai/onnx/) runs ONNX models with no outside dependency, [ai.gguf](/std/ai/gguf/) reads llama.cpp's GGUF files, and [ai.llm](/std/ai/llm/) runs Llama and Qwen language models from them. [image](/std/image/) reads, edits and writes images, and [opencv](/std/opencv/) wraps OpenCV for computer vision. The article [Machine learning with Salam](/blog/machine-learning-with-salam/) goes deeper, and it is candid about scale: this is a CPU-oriented library, not a replacement for PyTorch on a GPU cluster.

Audio is its own family: [audio](/std/audio/) for clips of samples, readers and writers for [WAV](/std/audio/wav/), [FLAC](/std/audio/flac/), [AIFF](/std/audio/aiff/) and more, [audio.dsp](/std/audio/dsp/) for Fourier transforms and filters, [audio.synth](/std/audio/synth/) for oscillators and envelopes, [audio.midi](/std/audio/midi/) for MIDI files, and [audio.music](/std/audio/music/) for notes, scales and tunings, including the Persian dastgah system.

## Graphics, GUI and the browser {#gui}

For desktop programs there are [gui.webview](/std/gui/webview/), a native window around HTML with a whole application layer on top, and the native UI runtime in [gui.ui](/std/gui/ui/), which draws Salam pages without a browser using its own [CSS engine](/std/gui/css/), [box layout](/std/gui/cssbox/), [text shaping](/std/gui/typeset/) and [painter](/std/gui/paint/). Games use [gui.raylib](/std/gui/raylib/). In the browser, [dom](/std/dom/) works with page elements and events, [web.canvas](/std/web/canvas/) draws on a canvas, and [web.keys](/std/web/keys/) tracks keyboard state for games. The articles on [desktop and GUI applications](/blog/desktop-gui-apps-with-salam/) and [interactive UI components](/blog/interactive-ui-components-state-events/) cover them.

## The trade-offs of a large standard library {#limits}

A big standard library is a deliberate choice, and it has costs as well as benefits.

The benefits are concrete. A new project can parse JSON, serve HTTPS, talk to PostgreSQL and hash passwords without choosing among competing third-party packages or auditing their dependency trees. Every package is documented in one place with runnable examples, and every package follows the same naming and memory conventions. The Persian names mean a Persian-speaking programmer can use the whole library in their own language, which no third-party ecosystem would provide.

The costs are real too:

- **Depth varies.** Some packages, such as `net.http`, `encoding.json` and `db.sql`, are central to the project and well exercised. Others are newer and thinner. Check the reference page and the tests before relying on a niche package.
- **Young code.** Much of the library is recent. Protocol implementations such as TLS, PostgreSQL and SSH have far less field experience than their established C, Go or Rust counterparts.
- **One release cadence.** A fix in a package ships with the next compiler release. Third-party packages, added with `salam add` from GitHub as described on the [packages page](/packages/), are where faster-moving code belongs.
- **A small ecosystem around it.** The standard library is large, but the wider community and the number of third-party packages are small compared with Python, Go or Rust.

## Frequently asked questions {#faq}

### How many packages are in the Salam standard library?

More than 170 documented packages, each with its own page in the [library reference](/std/). Some areas, such as networking, are split into many small packages, so the count reflects organization as much as size.

### Does the Salam standard library depend on C libraries?

Mostly not. HTTP, TLS, the MySQL and PostgreSQL drivers, compression and ONNX inference are written in Salam. A few packages wrap established C libraries on purpose, including SQLite, raylib, OpenCV and the system web view.

### Can I use the standard library from Persian code?

Yes. Every package and every public function has a Persian name, so a Persian program can call the whole library in Persian. The article [One language, two scripts](/blog/one-language-two-scripts/) explains how.

### Where do I find examples for a package?

Every page in the [library reference](/std/) has guide sections with complete programs and an example for each public function, and the [free course](/learn/) uses many of the packages along the way.
