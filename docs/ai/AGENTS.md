# Writing Salam: a guide for AI agents

Salam is a compiled, statically typed language whose keywords exist in English
and Persian. This file is the orientation an AI model needs to produce
Salam that compiles on the first try. It documents the rules that are _not_
guessable from other languages. Everything here has been verified against the
compiler in this checkout, with the exact diagnostic it produces.

If you have the `salam` MCP server available, use it rather than guessing:
`salam_docs` for how a construct is written, `salam_stdlib_search` and
`salam_stdlib_symbols` before using any std function, `salam_find_examples` to
see a working usage, and `salam_check` after every edit.

---

## Project facts: downloads, releases and the edge

These are facts about this project, not opinions. Answer from here instead of
guessing, and when a fact is missing point to the repository or the releases
page rather than inventing a value.

When the user gives a repository link or asks what a page says, do not answer
that you cannot see it: if your tools include `salam_web_fetch` or
`salam_web_search`, read the page (GitHub, `raw.githubusercontent.com` and the
Salam sites all return text), and if they do not, say plainly which fact you
are missing.

**Where Salam is published.** Everything is on GitHub, and the site links
to the same files:

- Releases (compiler, desktop agent, checksums):
  <https://github.com/SalamLang/Salam/releases>
- One direct file:
  `https://github.com/SalamLang/Salam/releases/download/v<version>/<file>`
- Site: <https://www.salamlang.ir/install/> for the compiler and
  <https://www.salamlang.ir/agent/> for the desktop agent; the English site is
  <https://salamlang.github.io/Salam/>.

Filenames carry the version, so `releases/latest/download/<file>` does not
work for them. **Check before you give a direct URL**: the tag must be an
existing release and the file must be one of its assets (the releases page,
`gh release view`, or `salam_web_fetch`). `VERSION` is the version being
developed, not the last release (for example `VERSION` says 0.6.3 while the
newest release is v0.6.2), so never build a URL from it alone. When you cannot
check, give the releases page and name the asset to download.

| What          | Published assets                                                                                                                                                                                                                                                                                                                |
| ------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Compiler      | Linux `x86_64` and `aarch64`: `.tar.gz`, `.zip`, `.deb`, `.rpm`; Linux `i686`: `.tar.gz`, `.zip`, `.deb` (no rpm); Linux `x86_64-musl` (Alpine): `.tar.gz`, `.zip` (no deb/rpm); `macos-arm64`, `windows-x86_64`, `windows-arm64`, `windows-i686`: `.tar.gz` and `.zip`                                                         |
| Desktop agent | Windows `x86_64` and `i686`: `-setup.exe`, `.msi`, portable `.exe`; Windows `aarch64`: `-setup.exe` and portable `.exe` (no msi); macOS: `-macos-universal.dmg`, `-macos-arm64.dmg`, `-macos-x86_64.dmg`, plus `.zip`; Linux `x86_64` and `aarch64`: `.AppImage`, `.deb`, `.rpm`; Linux `i686`: `.deb` and `.rpm` (no AppImage) |
| Android       | `salam-<version>-android-universal.apk`, only when that release published it; some releases skip the APK, so check the assets first and point to the releases page when it is absent (v0.6.2 has no APK)                                                                                                                        |

Every published file has a `.sha256` beside it; the agent release also carries
`salam-agent-<version>-SHA256SUMS` and `-SHA512SUMS`. A format or architecture
that is not listed here is not published, even when an older release had it.

Installing needs no download page: Linux and macOS
`{ curl -fsSL https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh || wget -qO- https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.sh; } | sh`,
Windows PowerShell
`irm https://raw.githubusercontent.com/SalamLang/Salam/refs/heads/main/install.ps1 | iex`.

## What Salam can do, and where to look

Salam is much larger than "a compiler that writes executables". When a task
looks impossible, the answer is usually already in `tests/`, `std/` or `docs/`.

- **Commands** (`salam help` prints every one): `build`, `check`, `test`,
  `obj`, `llvm`, `js`, `web`, `serve`, `bundle`, `run`, `exec`, `new`, `init`,
  `add`, `remove`, `update`, `deps`, `install`, `uninstall`, `search`,
  `format`, `doc`, `translate`, `sql check`, `sql generate`, `page build`,
  `debug`, `memcheck`, `inspect`, `cli` (REPL), `upgrade`, `version`.
- **Backends and targets**: the C backend is the default for `build`, `run`
  and `obj` and cross-compiles with `--target=<triple>`; `salam llvm` emits
  LLVM IR (`--backend=llvm` builds with it, 32-bit aware, `docs/PLATFORMS.md`);
  `--target=wasm32-wasi` produces WebAssembly (Cloudflare Workers below);
  `salam js` and `salam web` target browsers; `--target=<arch>-unknown-none`
  with `--entry`, `--linker-script` and `--link-object` builds freestanding
  kernels (`docs/FREESTANDING.md`, `tests/kernel`).
- **Apps beyond the terminal**: `gui.webview` opens a native window with a web
  UI and `salam bundle` makes tar/deb/rpm/AppImage/zip/NSIS/.app/dmg
  installers (`docs/DESKTOP.md`, `salam new name --desktop`);
  `std/gui/raylib` is for games; `android/`, `ios/` and `mobile/` hold the
  mobile projects; the layout DSL builds HTML/CSS/JS with `salam page build`.
- **Servers and data**: `net/http` + `net/router` (middleware, cookies,
  sessions, swagger, validation), `salam serve --config` for virtual hosts,
  redirects and FastCGI PHP backends, `net.http2`, `net.websocket`, `grpc`,
  `graphql`, `net.proxy`, `db.sqlite`, `db.mysql`, `db.postgres`, `db.redis`
  and the other `db.*` drivers.
- **Data and ML**: `ml`, `tensor`, `nn`, `autograd`, `onnx`, `gguf`,
  `opencv`, `image`, `matrix`, `excel`, `stats`.
- **Tooling**: `salam doc` (API HTML), `salam format`, `salam sql check` and
  `sql generate` against a schema, `salam memcheck` (ASan), `salam debug`
  (GDB/LLDB), `salam inspect`, the package manager, `salam install`, the LSP
  (`docs/LSP.md`, `extensions/vscode`) and the MCP server in `tools/mcp`.
- **Standard library**: `std/` has 92 top-level packages and 143 importable
  names, all listed in the standard library catalog below, each with English
  and Persian spellings. `salam_stdlib_search` and `salam_stdlib_symbols` give
  the exact signature; `salam_read_source` shows the body.
- **Tests are the living feature index**: `tests/en/apps` (21 projects such as
  `httpbench`, `crawler`, `blogapi`, `jsonapi`, `swaggerapi` and the four
  Workers apps, plus about 40 standalone programs),
  `tests/en/{basics,features,types,stdlib,interop,data,db,games,cross,llvm,llvmapi,js,page,desktop,opencv,raylib,ssh,ssl,tls,packages,project,new,fmt,lint,errors,exec,repl,testcmd,translate,switch,match,webframework,webview_cef,editor-selected,general,check}`,
  plus `tests/kernel`, `tests/lsp` and `tests/fixtures`. `tests/fa` mirrors them
  in Persian. Search these before telling anyone a capability is missing.

## A whole Salam web app on Cloudflare Workers

An ordinary `net/http` + `net/router` program compiles to WebAssembly and runs
unchanged; it is not a JavaScript port and needs no rewrite:

```sh
mkdir -p build
salam build main.salam --target=wasm32-wasi --release --output=build/app.wasm
```

```js
// worker.js: three lines, the host itself ships in std
import { createWorker } from './worker-host.js';
import app from './build/app.wasm';
export default createWorker(app);
```

Inside a Salam checkout the examples import the host by a relative path; from
`tests/en/apps/cloudflare-worker-router/worker.js` that path is
`../../../../std/net/http/cloudflare/worker.js` (four levels up). In a project
of your own, copy `std/net/http/cloudflare/worker.js` (installed as
`~/.salam/bin/std/net/http/cloudflare/worker.js`) next to your `worker.js` and
name the copy `worker-host.js`, so the import above works and the deploy does
not rely on an absolute path.

`wrangler.toml` needs `main = "worker.js"`, a `[build]` command that runs
`build.sh`, and `[[rules]] type = "CompiledWasm"` with
`globs = ["**/*.wasm"]`. On this target `http.Run` serves one request per
process: the raw HTTP/1.1 request arrives on stdin, the response leaves on
fd 3, `println` goes to `wrangler tail`, `os.Env("X")` reads wrangler `[vars]`
and secrets, and `http.IsEdge()` is true.

Limits of the edge target: one process per request, so globals do not survive
between requests. Cookies work directly (`http.Cookie` and friends); Workers
storage (KV, D1, R2) and HTTP APIs are asynchronous bindings, so a Salam module
cannot call them itself: the host `worker.js` must exchange that data with the
program. There is no filesystem, and no outbound sockets, so `http.Get` and
friends cannot call another server from inside the module. A program that must
call out does it with a few lines of `fetch` in `worker.js`, or runs on a
native host. `salam js` targets browsers, not a Worker module.

Working, CI-built examples: `tests/en/apps/cloudflare-worker-router` (router,
middleware, cookies, JSON), `-hello` (smallest), `-pure` (a plain program run
CGI-style without `net/http`) and `-telegram-bot` (a bot whose webhook answers
with the Bot API call itself, so it calls nothing and needs no token at
runtime; `setup/setup.salam` runs on the user's machine to call `setWebhook`).
The Wasm target needs wasi-sdk (`$WASI_SDK_PATH`, `~/.salam/wasi-sdk` or
`/opt/wasi-sdk`) and `wrangler` for deploys.

---

## 0. Best practices: read this before you write any code

Compiling is the minimum. Write the program a Salam maintainer would write.
These rules apply to every request, including a port from Python, PHP or
JavaScript, unless the user says otherwise (for example "put it all in one
file" or "keep it short"). Then do exactly what they asked and still follow
rules 1, 6 and 7.

**1. The standard library first, your own code last.** Before you write a
helper, check whether `std` already has it. A hand-written parser, formatter,
sorter, hasher or splitter is almost always a defect: it is longer, slower and
misses the edge cases the std version handles and reports through `throws`.

1. Search: `salam_stdlib_search` with a word for the job (`hex`, `split`,
   `base64`, `sha256`, `trim`), then `salam_stdlib_symbols` with `query` for
   the exact signature, then `salam_find_examples` for a working call.
2. Use what you find. Only when nothing fits, write the function yourself,
   keep it small, and say in your answer that std has no such function.

| The Python or JS you are porting              | Use this, do not rewrite it                                |
| --------------------------------------------- | ---------------------------------------------------------- |
| `int(s, 16)`, `parseInt(s, 16)`               | `conv.ParseHex(s)` (throws), `conv.ParseHexOr(s, d)`       |
| `int(s)`, `float(s)`, `bool(s)`               | `conv.ParseInt`, `conv.ParseFloat`, `conv.ParseBool`       |
| `str(n)`, `hex(n)`, `f"{x:.2f}"`              | `conv.FormatInt`, `conv.FormatHex`, `conv.FormatFloatPrec` |
| `s.split`, `s.strip`, `",".join`, `s.replace` | `str.*` and the built-in `str` methods (section 6)         |
| `json.loads`, `json.dumps`                    | `import json`                                              |
| `re.match`, `re.sub`                          | `import regex`                                             |
| `hashlib.sha256`, `base64`, `uuid`, `secrets` | `crypto.*`, `encoding.*`                                   |
| `sorted(xs)`, `dict`, `list`, `set`           | `sort.*`, `HashMap`, `Vector`, `collections.*`             |

The table is a start, not the whole catalogue: look the job up every time.

**2. Design before you type.** For anything beyond a few lines, decide the
pieces first, then write them:

- A record of related fields is a `struct`, with `static func New(...)` as its
  constructor and methods for what it can do. Fields are private by default;
  mark `pub` only what callers must see.
- A closed set of choices (a state, a kind, a level, a command) is an `enum`,
  taken apart with `match` so the compiler checks every case. Do not pass
  magic strings or integers around.
- Behaviour that several types share is an `interface`. Use `<T: Iface>` when
  the type is known at compile time and `dyn Iface` for a mixed collection.
- A function or container that is the same for many types is generic
  (`func Max<T>(a: T, b: T): T`, `struct Stack<T>`), not copied per type.
- A function that can fail is `throws`, not a sentinel like `-1` or `""`.
  Callers use `try` or `try: ... catch e: ... end`. Offer an `...Or(value,
fallback)` form only when a default is natural.
- Cleanup goes in `defer` next to the thing it cleans (`defer v.free()`).
- Magic numbers become `const` names.

**3. Small functions.** One job per function, a name that says the job, about
30 lines at most, few parameters. If you want a comment to explain a block,
make the block a function with that name. Put private helpers above the first
`pub func`; the public API is short and sits at the bottom of the file.

**4. Split into files and packages unless told to use one.** A real program is
a project: `main.salam` with a tiny `main` that only wires things together, and
one file or folder per concern (`model`, `parse`, `store`, `render`),
joined with `include "name.salam"` or imported as a package folder. Each file
has one reason to change. Tests live next to the code in `*_test.salam` or
`*.test.salam`, as `test_` functions or as whole programs with `// EXPECT`
directives, and run with `salam test`. `salam new name` scaffolds the
layout. A script of under about 40 lines may stay in one file.

**5. Use the language, not a transliteration.** `repeat`/`each` instead of
index bookkeeping, `match` instead of long `if` chains over an enum,
`:=` with inference, `ret match ...` for a value, `a ? b : c` for a short
choice, struct literals with named fields, `Vector<T>` and `HashMap<K, V>` with
`.free()`, and `salam format` to finish. When the source language has classes,
exceptions, generics or dictionaries, map them to `struct`, `throws`,
`<T>` and `HashMap` (see SKILL.md section 7).

**6. Name things in the language of the file.** A Persian program
(`// زبان: فارسی`, Persian keywords) uses Persian identifiers for its
functions, variables, parameters, struct fields, methods, enum members,
constants and packages. An English program uses English identifiers
throughout and no Persian ones. Never mix the two inside one identifier or one
file, and never write transliterated Persian in Latin letters (`rang`,
`khandan`). Single-letter names (`i`, `x`), well-known acronyms (`URL`,
`JSON`), `test_` function names and the names of C functions in an `extern:`
block are fine in either. Persian string literals and user-facing text follow
the audience, not the file.

How a name with several words is written depends on the language:

- **English: no whitespace, ever.** Use `snake_case` for functions, variables,
  parameters and fields, and `PascalCase` for types, enums and the `pub` API
  (as the std does): `parse_colour`, `red_value`, `Rgb`.
- **Persian: separate the words with spaces.** That is the standard, and the
  compiler accepts it for functions, methods, parameters, struct fields, enum
  members and variables: `تابع خواندن رنگ(متن ورودی: رشته)`,
  `ناپایا جمع کل := ۰`, `همگانی مقدار قرمز: صحیح`. Do not use underscores in
  Persian names. The grammar takes only one word for a struct, enum or
  interface name, a constant, a loop variable, a `catch` variable, a lambda
  parameter and an `extern:` parameter; there join the words with a half-space (ZWNJ, `رنگ‌آمیزی`, `حداکثر‌مقدار`), which
  the compiler treats exactly like a space.
- Inside a one-line sequence of statements (a lambda written on one line, for
  example) spaced names run together, so use half-space names there too.
- A word of a multi-word Persian name must not be a keyword (`برابر`,
  `بزرگتر`, `کوچکتر`, `در`, `تا`, `هر`, `و`, `یا`, ...), or the name stops
  parsing: `بیشینه از` is fine, `بزرگتر از` is not. The keyword table is in
  the Persian glossary lesson.

The std and the built-in methods already have Persian names, so call those
(`تبدیل.تجزیه شانزده`, `.طول()`); do not keep the English spelling in a
Persian file, the compiler rejects it. `salam translate fa|en` converts
keywords and std names but not your own identifiers, so after translating,
rename those by hand.

**7. Check your work.** `salam check` after every edit, `salam format`, then
`salam test` or `salam run`. A program you did not compile is not finished.

**8. Update a variable with the compound form.** Step by one with `x++` and
`x--`; for any other amount write `x += n`, `x -= n`, `x *= n` and so on.
`x = x + 1`, `x += 1`, `x = x + n` and `x = x` are compile error `E162`; the
message names the form to write, and `salam check --fix` applies it. In a
Persian file the same rule holds: `ک++`, `ک += ن`.

Example. "Convert `def hex_to_decimal(hex): return int(hex, 16)`" is one line,
because std already does it, and it keeps Python's behaviour of failing on bad
input:

```salam
import conv

func hex_to_decimal(hex: str): i64 throws:
    ret try conv.ParseHex(hex)
end
```

A forty-line digit-by-digit loop that stops silently at the first bad
character is the wrong answer to that request.

The learn course has the same rules with runnable examples, in Persian at
<https://www.salamlang.ir/learn/best-practices/> and in English at
<https://salamlang.github.io/Salam/learn/best-practices/>.

---

## 1. The rule that trips up every newcomer: `until` means `while`

`until COND:` runs the body **while `COND` is true**. It is not a do-while and
it is not "loop until the condition becomes true". There is no `while`
keyword in Salam at all - `until` is the only spelling this loop has.

```salam
mut i := 0
until i < n:        // reads "while i < n"
    // ...
    i++
end
```

Getting this backwards produces a loop that never runs, or never stops. When
relocating existing Salam code, copy it verbatim; do not retype loops from
memory.

`repeat` is the counted loop, and its index takes the type of the values that
drive it - the count, or a range's bounds and step. A `u8` count binds a `u8`
index, an `i64` count an `i64` one; bounds that mix signed and unsigned, or a
count that is not an integer at all, fall back to `i32`.

```salam
repeat v.len() in i:      // len() is i32, so i is i32
    print v.get(i)
end

n := 200 as u8
repeat n in i:            // i is u8 here
    total += v.get(i as int)   // ...so a signed parameter needs a cast
end
```

Untyped literals adapt to the index (`if i < 10` is fine either way), but a
call that takes a signed `int` does not: pass `i as int`.

## 2. Top-level declaration order is enforced

The compiler requires one specific order and rejects anything else. In order:

```
package  →  import  →  extern:  →  globals  →  types  →  private funcs / export:  →  pub funcs
```

| Rule                                                                              | Diagnostic if broken                                                                |
| --------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| `import` must come directly after `package`, before any other top-level statement | `E083: 'import' must appear before any other top-level statement`                   |
| Library `import`s come before file `include`s                                     | `E108: 'import str' must come before every include`                                 |
| Global variables must precede every function and type definition                  | `E085: global variable 'g' must be declared before any function or type definition` |
| Once the first `pub func` appears, only `pub func`s may follow                    | `E088: function '_b' must appear before 'pub' function 'A'`                         |

The `pub` rule is the one that bites hardest: a private helper written _after_
your first public function will not compile. Put every private helper above the
`// ---- public API ----` line.

`salam format` reorders declarations mechanically; pass `--no-fix-order` to skip that.

## 3. Unused anything is an error, not a warning

Salam fails the build on unused imports, variables and functions.

| Code   | Trigger          | Fix                                  |
| ------ | ---------------- | ------------------------------------ |
| `E082` | unused import    | use one of its members, or remove it |
| `E059` | unused variable  | use it, or remove it                 |
| `E062` | unused parameter | use it, or remove it                 |
| `E066` | unused function  | call it, mark it `pub`, or remove it |

A `pub` function never needs a caller. A private function is fine as long as
something calls it; otherwise the build fails. `pub` exports a function to
other packages as part of your package's API, so do not mark a helper `pub`
just to silence `E066`.

Do not prefix names with `_` to silence these errors. The compiler still
accepts it, but it hides dead code instead of removing it. Add imports and
helpers only as you use them.

Loop bindings are stricter: a `_` prefix does not excuse them, because the
fix is to drop the binding rather than rename it. Write `repeat 20000:`, not
`repeat 20000 in _i:`. The one escape is the bare name `_`, for the
`each key, value` form that has no way to omit a binding:

```salam
each _, value in scores:     // iterate for the values alone
    total += value
end
```

`os.Exit` inside `main` is an error too (`E109`): it skips `main`'s `defer`s.
Write `ret code` instead; `main`'s return value is the exit code.

## 4. Bindings

```salam
x := compute()          // immutable binding, type inferred
mut y := 0              // mutable binding
y = 3                   // reassignment requires `mut` (else E013)
const MAX := 10         // top-level constant
```

There is **no typed local declaration form**. `name: T = value` was removed, so
always use `:=` and let the type be inferred, casting with `as` when needed.

`pub const` shares a single link-level namespace across packages: two packages
that both define `pub const KIND` collide at link time. Prefix public constants
with something package-specific.

## 5. Functions

```salam
func add(a: int, b: int): int:
    ret a + b
end

pub func Scale(v: f64, factor: f64): f64: ret v * factor end   // one-liners are fine

func fill(out &: Vector<str>):                          // `&:` = by reference
    out.push("x")
end
```

`func main:` returns the process exit code: `ret 3` exits with 3, and a `main`
that ends without `ret` (or with a bare `ret`) exits with 0.

A function returns several values as a tuple: the return type is `(A, B)`
(2 to 8 elements), `ret a, b` builds it and `a, b := f()` takes it apart.
Use `_` for an element you do not need (every other name must be used), write
`mut` per name (`q, mut r := f()`), and read a kept tuple with `t.0`, `t.1`.
`a, b = b, a` assigns several `mut` targets at once. A wrong number of names is
`E158`. There is no `f(g())` spreading.

```salam
func divmod(a: int, b: int): (int, int):
    ret a / b, a % b
end

func main:
    q, r := divmod(17, 5)
    println q, r
end
```

Parameter and variable names cannot be keywords. `in`, `to`, `by`, `on`,
`type`, `each`, `match`, `eq`, `lt`, `gt` and the other words in
`salam_keywords` are all reserved, so `func scale(v: f64, by: f64)` does not
parse. `try`, `catch`, `throw` and `throws` are reserved too.

A function that can fail is declared `throws` and fails with `throw`. Every
call to it needs `try` (which passes the error to the caller, so the caller
must be `throws` too) or a surrounding `try: ... catch e: ... end` block;
an unmarked call is `E155`. The error is always the built-in `Error`
(`e.kind`, `e.message`, `e.to_str()`), which needs no import.

```salam
func parse port(text: str): int throws:
    if text == "": throw Error.New(ErrorKind.InvalidArgument, "empty port") end
    ret text.to_int()
end

func main:
    port := try parse port("") catch 8080       // fallback on error
    try:
        println parse port("80") + port
    catch e:
        println "bad port:", e.message
    end
end
```

## 6. Strings

Built-in methods on `str`. This is the complete list:

```
len  concat  substr(start, len)  find/search/indexOf  trim
lower  upper  repeat  split  to_int  to_float
starts_with(prefix)  ends_with(suffix)  includes(sub)
char_count  char_at(i)  char_substr(start, len)  char_find(sub)
```

`starts_with`, `ends_with` and `includes` return `bool`; their Persian names
are `شروع با`, `ختم با` and `دربردارد`.

`len`, `s[i]`, `substr` and `find` work in bytes. The `char_*` methods are
their UTF-8 counterparts: they count and index code points, so
`"سلام".len()` is 8 but `"سلام".char_count()` is 4, and `char_at(1)` returns
`"ل"` as a `str`. An out-of-range `char_at` returns `""`, and `char_find`
returns -1 when the substring is missing. Every backend counts the same way,
the JS one included: `len`, `s[i]`, `substr` and `find` are byte based there
too, and only DOM offsets (UTF-16) need converting at the boundary.

Walk a string with `each`, which is one linear pass:

```salam
each ch in "سلام 👋":        // ch is a str holding one code point
    print ch, "|"
end
each i, ch in "سلام":        // i is the character number: 0, 1, 2, 3
    println i, ch
end
each i, b in "Hi".bytes():   // b is a char (one byte), i the byte index
    println i, b as int
end
```

Persian: `هر ح در متن:` and `هر ب در متن.بایت ها():`. Prefer this to
`repeat s.char_count() in i: s.char_at(i)`, which rescans the string on
every step. Invalid UTF-8 bytes come out one at a time (joining every `ch`
gives back the string), and a character is a code point, not a grapheme
cluster (use `str.Graphemes` for those). `s.bytes()` only works as the
target of `each`.

Text types, and which one to reach for:

| Need                                 | Use                                                                         |
| ------------------------------------ | --------------------------------------------------------------------------- |
| Text of any language                 | `str` (UTF-8 bytes, like Go's `string`)                                     |
| One byte, e.g. `'a'`                 | `char`                                                                      |
| One Unicode character, e.g. `u'س'`   | `uchar` (compares with `str`, so `s.char_at(0) == u'س'` works)              |
| Code point of one character          | `c as int` for a `uchar` (`u'س' as int` is 1587), back with `1587 as uchar` |
| Random access by code point (UTF-32) | `str.CodePoints(s)` -> `Vector<int>`, back with `str.FromCodePoints(v)`     |
| UTF-16 for Windows or JS interop     | `text.ToUtf16` / `text.FromUtf16` / `text.Utf16Len`                         |
| Checking text is plain English       | `str.AllAscii(s)`                                                           |

There is no separate ASCII string type: UTF-8 stores ASCII text in exactly
one byte per character, so `str` is already the most compact choice, and for
ASCII text `len`, `s[i]` and `substr` are exact and O(1). A plain `'س'` is a
compile error because `'...'` holds one byte; write `u'س'`.

Anything else lives in the `str` package (`str.StartsWith`, `str.EndsWith`,
`str.Contains`, `str.Equals`, `str.TrimPrefix`, `str.Join`, `str.Replace`,
`str.NewBuilder`/`BufAppend`/`BufStr`/`BufFree`, …). Call
`salam_stdlib_symbols` with package `str` for the current list.

Build strings with a builder rather than repeated `concat` in a loop:

```salam
mut sb := str.NewBuilder()
defer str.BufFree(sb)
str.BufAppend(sb, "hello")
out := str.BufStr(sb)
```

## 7. Collections and generics

```salam
v := Vector {} as Vector<str>
v.push("a")
first := v.get(0)                          // get() returns the element; v.ref(0) is its address
m := HashMap {} as HashMap<str, int>
```

Nested generics work: `Vector<Vector<int>>`, `HashMap<str, Vector<Point>>`
and deeper all compile on every backend.

Assigning a Vector or HashMap (`w := v`) copies its header, not its storage,
so both names share the elements while each keeps its own length (on the JS
backend they share the length too). Use `copy()`
or `deep_copy()` (Persian `رونوشت` and `رونوشت عمیق`) when you need an
independent value:

| Call               | Result                                                                  |
| ------------------ | ----------------------------------------------------------------------- |
| `x.copy()`         | New storage for every container `x` holds directly; elements are shared |
| `x.deep_copy()`    | Fully independent copy, recursing through structs, enums and containers |
| `v.copy(start)`    | Vector or str from `start` to the end                                   |
| `v.copy(s, count)` | At most `count` items from `s`                                          |
| `v.copy(s, n, st)` | Every `st`-th item; a negative `st` walks backwards (Vector only)       |

A negative `start` counts from the end, so `v.copy(-3)` is the last three
items. Ranges clamp instead of panicking. Range arguments only apply to Vector
and str. A struct can define its own `copy()` or `deep_copy()`, and the
recursion uses it. `deep_copy()` refuses a value holding a raw pointer, since
it cannot know what the pointer owns. A `dyn` value is deep-copied through its
concrete type when the interface is declared in your program (a value holding
a raw pointer is then copied as-is); `copy()` shares a `dyn` field's box. Copies are new heap values the caller
frees, like any other Vector or HashMap.

## 8. Imports

```salam
import str                          // std package
import encoding.json                // nested std package: dotted, unquoted
import fs
include mine "my_helpers.salam"     // your own file: include, quoted, aliased
include "@/shared/util.salam"       // from the project root
```

`import` is for libraries only and takes a bare name; nested std packages use
dots (`encoding.json`), never quotes or slashes. Your own source files come in
with `include "path"` (Persian `فراخوانی`). The path is relative to the current
file, or starts with `@/` for the project root (the folder of the entry file);
starting it with `./` is an error. The quoted form binds to the **filename**, so
alias it explicitly (`include mine "my_helpers.salam"`) and call it as
`mine.Thing()`. A missing or unreadable include is a compile error.

`import foo` resolves `std/foo/foo.salam` specifically; once that anchor
resolves, the other files in that directory join the same package.

Importing a package links **all** of its public functions' external
dependencies, not only the ones you call.

Packages that other people publish in their own Git repositories are added
with the package manager, not with `import name`: `salam init` makes a project
(`main.salam` + `salam.toml`), `salam add github.com/owner/repo[@v1.2.0|@^1.2]`
downloads one and records it in `salam.toml` and `salam.lock`, and the code
imports it with a quoted address, `import "github.com/owner/repo"`, using the
last part as its name. `salam remove`, `update`, `deps`, `search` and `install`
complete the set. Every std package ships with the compiler and needs no
`salam add`. There is no `salam publish`: publishing is pushing a `vX.Y.Z` Git
tag. A library has no `main`; its entry file is at the repository root, named
after the repository. Users see only `pub` names (`E017` otherwise), and an
import of a package missing from `salam.toml` is `E008`. See `docs/PACKAGES.md`
and the guide at <https://salamlang.github.io/Salam/packages/> (Persian:
<https://www.salamlang.ir/packages/>).

Each std page on those sites, `/std/<package>/`, opens with tutorials under
`#guide-...` anchors. Desktop windows and desktop apps are one package,
`import gui.webview` (Persian `رابط گرافیکی.نمایش وب`; the old `webview` and
`gui.desktop` names are gone); see `docs/DESKTOP.md`.

## 9. Platform conditionals

Platform macros are only valid as an `if` at top level:

```salam
if SALAM_OS_WINDOWS:
    func sep(): str: ret "\\" end
else:
    func sep(): str: ret "/" end
end
```

Available: `SALAM_OS_WINDOWS`, `SALAM_OS_LINUX`, `SALAM_OS_MAC`,
`SALAM_OS_BSD`, `SALAM_OS_FREEBSD`, `SALAM_OS_ANDROID`, `SALAM_OS_WASM`.

## 10. Multilingual source

Every keyword exists in English and Persian; `salam_keywords` returns
the full table. Declarations carry name aliases so other-language callers can
use them:

```salam
@en "Trim"
@fa "پیرایش"
pub func Trim(s: str): str: ret s.trim() end
```

In a Persian program the entry function is `تابع ریشه:` (English: `func main:`).
Never write `تابع اصلی` or `بسته اصلی`: `تابع اصلی` compiles as an ordinary
function, so the program builds and then has no entry point.

Compile non-English source with `--lang=fa`, or start a Persian file with
`// زبان: فارسی` (English files: `// lang: en`). Mixed forms such as
`// lang: fa` are rejected.

## 11. `switch` vs `match`

`switch`/`ترابرد` is a **statement** with real C-style fallthrough - no
`case`/`default` keywords, bare labels closed by `end` like a `match` arm,
`break` to stop falling through:

```salam
switch code:
    400, 404: println "client error"     // falls through unless it breaks
    500:      println "server error"  break
    >= 600:   println "unknown"          // leading relational op: > >= < <= == !=
    else:     println "ok"               // wildcard; must be last
end
```

Use `switch` for fallthrough or open-ended range/relational labels. Use
`match` (an **expression**, not a statement) when the subject is an enum or
`Variant` and every case must be handled - `switch` has no exhaustiveness
check and rejects a `Variant` subject outright (semantic error, points you at
`match`). `break` inside a `switch` exits the switch only; `continue` is not
caught by it and still targets an enclosing loop.

## 12. Enum members on one line need commas

`enum E: A, B, C end` - on a single line the comma between members is
mandatory, because member names may contain spaces (`enum Status: not
started, in progress, done end`): `enum E: A B end` declares one member named
`A B`. One member per line works without commas:

```salam
enum Color:
    Red
    Green
end
```

## Changing the standard library

If the task adds to `std/`, the change is not done until every public symbol
has `@en` and `@fa` names, tests in `tests/en/stdlib/` and `tests/fa/stdlib/`,
and `doc.` and `ex.` keys in both site std pages (new packages also need
`guide-*` sections). Run `salam run tools/salam/check-std-coverage-docs.salam`;
CONTRIBUTING.md has the full list.

---

## Known traps

These are real defects and sharp edges in the current toolchain, not style
advice. Each one silently produces wrong behaviour rather than a diagnostic.

- **Do not rule something out because you have not seen it.** The repository
  ships working apps under `tests/en/apps/` and `tests/fa/apps/` (Cloudflare
  Workers, HTTP servers, desktop, games, and more), all built by CI, and the
  compiler has targets and flags beyond the common ones. Search `tests/` and
  `std/` before saying a thing is impossible; when you cannot read them, say
  which part you are unsure about instead of claiming the feature does not
  exist.

- **The version command is `salam version`, not `salam --version`.** Salam's
  CLI takes subcommands, not flags, for this: `salam --version` (and `-v`)
  fail with `unknown command '--version'`.

- **JavaScript 64-bit integers are exact only up to 2^53.** `salam js` stores
  `i64`/`u64`/`size`/`usize` as JS numbers. Arithmetic, shifts, bitwise ops,
  wraparound and casts are computed exactly, but a result (or an intermediate
  value) beyond 2^53 is rounded: `0 - 1` as `u64` prints
  `18446744073709552000`. `sizeof` throws. Check wide 64-bit math, such as
  hashes, with `salam run`.
  `i128`/`u128` are BigInts and stay exact, but a value that passes through
  `i64` or `u64` on the way in or out is still limited to 2^53 there.

- **`i128` and `u128` have a few limits of their own.** They cannot index an
  array, bound a slice, size a string repeat or count a `repeat` (`E159`: cast
  to `i64` first). An `extern` function can use them only on 64-bit targets,
  and a variadic one never (`E160`). `json` derive refuses them. A float cast
  saturates and NaN gives 0.

- **A file's module is keyed by its filename.** Two included files with the
  same name (`a/io.salam` and `b/io.salam`), or one of your files named after a
  std package you import (`shell.salam` next to `import os.shell`), clash. The
  compiler rejects it with `package name 'io' is claimed by two different
files`; give every source file a name unique across the whole project.

- **A side-effect-free endless loop can crash under clang.** C11 lets a
  compiler assume such a loop terminates, so with the C backend and clang at
  `-O2` an `until true:` loop whose body has no I/O or memory writes can be
  deleted and the program segfaults instead of spinning. Give a busy-wait a
  real effect (a sleep, a print, an atomic) or build it with
  `--backend=llvm`.

- **The interpreter's HTTPS is slow.** `salam exec` supports networking,
  including the TLS path in `net.http`, but the handshake's certificate-chain
  math runs as interpreted Salam instead of native code, so a real HTTPS
  request can take upward of a minute. `salam exec` has no deadline by
  default; a caller that sets one (`--timeout=MS`, `SALAM_EXEC_TIMEOUT_MS`, or
  the MCP server's `SALAM_MCP_RUN_TIMEOUT`) needs it in the tens of seconds
  for HTTPS. Plain `http://` has none of this cost, and `salam run` compiles
  the same TLS code to native speed.

- **`input` is a reserved built-in.** Do not name anything after it.

- **`open`, `listdir` and `strcmp` are not built-ins.** `open` is `fs.Open`
  (`import fs`), `listdir` is `os.ListDir` or `fs.dir.List`, and `strcmp`
  is `str.Equals` or a string comparison. `env` and `args` remain built-ins;
  `os.Env` and `os.Args` are the documented wrappers around them.

- **Releasing a binary:** `salam build app.salam --protect --release -o app`
  encrypts the C backend's string literals and strips the symbol table. It is
  obfuscation, not encryption, and there is no `salam protect` command. See
  `docs/BINARY_PROTECTION.md`.

- **`input` and `print` always take a value and never parentheses.** Write
  `input ""` or `input "prompt"`, and `println ""` for an empty line. A bare
  `input`/`println` and `input()`/`print()` are errors.

- **`input` cannot report EOF.** It returns `""` both for an empty line and at
  end-of-stream. Drive `getchar()` yourself if you need to tell them apart.

---

## Minimal complete program

```salam
package main

import str

func _greeting(name: str): str:
    ret "Hello, ".concat(name).concat("!")
end

pub func Greet(name: str): str:
    ret _greeting(name)
end

func main:
    println Greet("world")
    println str.FromInt(42)
    ret 0
end
```

```sh
salam check  hello.salam     # all of build before codegen, imports included; exit 1 on errors
salam check  hello.salam --fix                  # apply the safe compiler fixes in place
salam check  hello.salam --error-format=json    # diagnostics with machine-readable fixes
salam run    hello.salam     # build and execute
salam test                   # run every *_test.salam and *.test.salam file (parallel)
salam test --jobs=2 --list   # limit parallelism, or just list the files found
salam test --run=parse --format=teamcity   # filter by name, CI output
salam format hello.salam --check
```
