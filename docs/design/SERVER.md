# Server rendering and server functions

Status: step 3 and the render part of step 6 are implemented (2026-11-22); the rest is proposed. Builds on `ROUTING.md` and the page language
(`UI_DSL.md`, `UI_BEHAVIOUR.md`).

## Goals

- Next.js style rendering: the server sends finished HTML for the requested
  route, the browser shows it at once, then JavaScript takes over (hydration).
- The front project stays a front project. It holds pages and a few
  `server func` declarations. The backend is a separate project, written in
  Salam or any language, and reached over HTTP. The front project does not
  contain a database, a file store or a business rule.
- One source, three outputs: static web (today, the default), server rendered
  web (`--ssr`), native (no server, same functions through a stub).

## Shape of a project

```
shop/
  front/                  this design
    main.salam
    pages/
      index.salam
      product/[id].salam
    api.salam             server funcs: calls to the backend
  api/                    any language, own repository if you like
```

## `server func`

A normal function with a marker (`server` is a contextual word before `func`,
like `go`, so it stays a legal name elsewhere; the Persian spelling is `سرور تابع`, and it
sits last among the modifiers: `pub server func`). It runs only on the front server. The page,
the browser bundle and the native app never contain its body.

```salam
server func GetProduct(id: int): Product:
    r := http.GetWith(env.Get("SHOP_API") + "/products/" + str.FromInt(id as i64), auth_headers())
    ret json.Decode(r.body) as Product
end
```

A failure (a transport error, a bad status, a body that does not decode) is a
`throw`. The caller handles it with `try`/`catch`; an uncaught throw during
server rendering gives a 500 page, and over the wire becomes `{"ok": false}`.

```salam
pub component ProductPage(id: int):
    p := GetProduct(id)
    heading: content = p.name end
end
```

### Rules

- Top level only. Parameters and result are wire types: `int`, `float`, `bool`,
  `str`, structs and `Vector`s of wire types, optional (`T?`). No pointers,
  references or closures (E285). Enums are allowed. `HashMap` and arrays are
  not wire types yet. E287 is reported for a method; a nested function cannot
  be written at all. E288 counts overloads: the wire name is the function name.
- Callable from a component body or `load(...)`, a handler, and another
  `server func`. On the server a call is a direct call. In the browser and on
  native it is the generated stub (below). The call site looks the same.
- `env.Get(name)` is the existing `std/env` function. It is ordinary on the
  server and in native programs, but it does not exist in a browser bundle.
- A function that is reachable from browser code and uses a browser-missing
  name (`std/env`, a server func body, a server only import) is E286. Reachability is
  computed per output, so a helper shared by both sides is fine until it touches
  a server only name.
- `request()` returns the current request (method, path, headers, cookies, the
  client address) inside a `server func` and during server rendering. Use it to
  forward a session cookie or token to the backend.

### Keeping it a thin layer

The body is free, as decided, so the guard is a warning, not a ban. W280 is
raised when a `server func` imports or calls `db`, `fs`, `os`, `exec`, or the
`net.http` server types (`NewServer`, `Use`, routers), with the message "this
looks like backend code; call your API from here instead". Projects that really
want it set `allow_backend = true` in `salam.cfg`, found by walking up from the
source file (its directory and three parents). The check also covers the db
drivers (`sqlite`, `postgres`, `mysql`, `redis`), `process` and `router`, and
warns once per package per function.

Outbound calls use `std/net/http`. Defaults for a call made from a server func:
10 second timeout, response cap of 8 MB, redirects followed up to 5. All three
are arguments of `http.Do` options already; the server runtime only sets the
defaults.

## Wire protocol (browser and native)

`POST /_salam/fn/<Name>` with `Content-Type: application/json`.

```
request   {"args": {"id": 3}}
response  {"ok": true,  "value": {...}}
          {"ok": false, "error": "text"}
```

- Status is 200 for both `ok` outcomes, 400 for a body that does not decode or
  does not match the signature, 403 for a failed origin or token check, 404 for
  an unknown name, 500 when the function itself fails (the error text is
  generic unless the function returns an error value).
- Same origin only: the server checks `Origin` or `Sec-Fetch-Site` and a
  per session token in `X-Salam-Token`. The rendered page embeds the token. There
  is no CORS.
- Native has no origin. It sends the token issued by `GET /_salam/session` and
  the setting `server = "https://front.example.com"` of the app names the
  server.
- Only functions marked `server func` are registered. The name list is
  generated at build time; nothing is looked up by string at run time.

## Server rendering

1. Match the path with `nav.Match` against the route table (`ROUTING.md`).
   Unknown path: `_404` with status 404.
2. Run the layout and route guards on the server. A failed guard answers
   `302` to its `redirect`.
3. Render the layout chain and the page component to HTML with the same
   lowering the runtime layout (`page Name(params)`, `http.Html`) already uses.
   A `server func` call or a `load(...)` ready inside the render is awaited
   before the response (default budget 3 seconds per request; on timeout a
   `load(...)` renders its loading branch and the client continues; a direct
   `server func` call with no loading branch fails the render with a 504 page,
   so write calls that may be slow through `load`).
4. Write `<head>` (title, meta, CSS link), the body, the embedded state
   (`<script type="application/json" id="__salam">`: variable values and the
   call cache; every `<` is written as `\u003c` and `\u2028`, `\u2029` are escaped,
   so no value can close the tag) and the bundle script.

Streaming and incremental regeneration are out of the first version.

## Generated server

`salam page build --ssr` compiles the project twice. The first run is the web
build (shell HTML with `<!--salam:N--><!--/salam:N-->` regions, CSS, bundle)
into `dist/public`. The second run lowers the same page AST for a native
program: `ExpandPage`, `PrepareWebState`, `PrepareViews`, then
`PrepareServerPage` (`compiler/sal_layout/layout_ssr.salam`), then semantic
analysis and the normal code generator. The driver flag is `opt.ssr_server`.

Contract with the server func lowering (`feat/server-func-codegen`):

- The hook is `func __salam_register_server_funcs()`: top level, package
  main, no parameters, no result. The generated `main` calls it once before it
  listens. It registers every server func with `net/ssr` (`ssr.Register`).
- `PrepareServerPage` runs after `PrepareViews` and before `sm.Run`. It scans
  the main program's top level and appends an empty definition only when no
  function of that name exists. So the server func pass must add its function
  to the AST before semantic analysis (it may run before or after
  `PrepareServerPage`; if it adds one first, nothing is appended).
- The in-render hooks have the same rule and a no-op default:
  `func __salam_ssr_render_begin()` runs before the layout chain and route
  component of a request are rendered, and `func __salam_ssr_render_end(): str`
  runs after and returns the call cache as a JSON object text (`{}` by
  default); the server puts it into the `__salam` state script as `"calls"`.
  A `server func` call inside a component body runs in process between the two.
- Render calls are serialized by one mutex (route and view state are globals),
  so the hooks never run concurrently.
- The generated `main` reads `PORT` (default 8080, `0` picks a free port) and
  `HOST`, serves `dist/public` first (`ssr.SetStaticDir`), and renders every
  other GET through `ssr.SetRender`. It prints `listening on http://HOST:PORT`.

## Hydration

What was built (the smallest design that works):

- The scope is the region, not the element. Every reactive region is already
  wrapped in `<!--salam:N-->` and `<!--/salam:N-->` markers, and the markers are
  the same in the server HTML and in the client render, because both come from
  the same lowering. There are no per element `data-s` ids: event handlers are
  inline (`onclick="__sl_ev_3()"`) and survive in the server HTML, and the
  client finds a region by its marker.
- The bundle reads `<script type="application/json" id="__salam">` when it
  starts (`window.__salam_state`, with `path`, `pattern`, `status`, `calls`) and
  sets a hydrating flag until the first mount of every region is done.
- Mounting a region renders its view from the state into a template and
  patches the nodes that are already there (`__salam_kids`), so matching
  server nodes are kept, not replaced. Test: no element is removed during
  hydration.
- Mismatch: while hydrating, a region whose server nodes differ from its fresh
  render (nested regions are compared empty, each is checked on its own turn)
  logs `salam: hydration mismatch in region N, it is rendered again from the
  state` through `console.warn` and the patch makes it equal. Nothing throws. A
  region the server left empty (`each` rows with per row state, std style
  values: `salamWsCur`, `salamWsSlot`, `salamWsFresh`, `salamWsStyle`) is simply
  filled on load without a warning; the first paint is the server HTML.
- Variable values are not serialized: views compute them from the route and the
  initial values again. A value that only a server func can give travels in
  the call cache.

Call cache for server funcs (the contract with the server func stubs):

- Key: `Name:HASH:N` where `HASH` is the 8 digit lower case hex FNV-1a (32 bit)
  of the UTF-8 bytes of the argument JSON text (`{"id":3}`, the same text the
  wire protocol sends) and `N` counts earlier calls with the same `Name:HASH`
  in this render, from 0. So two identical calls that returned different values
  replay in order.
- Server: `ssrpage.PageCallKey(name, argsJson)` gives the key (call it before
  `PageCallPut` for the same call), `ssrpage.PageCallPut(key, valueJson)` stores
  the result, and the default `__salam_ssr_render_end()` returns
  `ssrpage.PageCalls()`, a JSON object that is embedded as `calls`.
- Browser: `__salam_call_key(name, argsJson)` returns the next key (it counts
  occurrences from page load), and `__salam_call_cache_get(key)` returns the
  stored value or `undefined`. A stub that gets a value returns it at once and
  does not fetch. Keys are not consumed, a later call with a new occurrence
  number goes to the network.
- A difference between the cached and a fresh value is not an error: the next
  state change renders it.

Not built: serializing page variables, per element ids, hydration of `each`
rows with per row state on the server (they are filled by the bundle).

## Build and run

| command | result |
| --- | --- |
| `salam page build` | static site, as today |
| `salam page build --ssr[=DIR] [project]` | `DIR/server` (a native executable made from the page AST lowered for the server: router, guards, renderers) and `DIR/public` (bundle, CSS, assets, prerendered pages for routes with no parameters and no guard); `DIR` defaults to `dist` |
| `salam serve --render [project] [--port=N] [--host=H]` | builds into a scratch directory, runs the server, rebuilds on save and swaps the server; the browser is not reloaded yet |

`public/` of the project is copied into `DIR/public` first. The shell page (the
web build with empty `<!--salam:N--><!--/salam:N-->` regions) is written to the
hidden file `DIR/public/.shell.html`; the server fills the regions per request
and never serves that file. Prerendered `index.html` copies are in `DIR/public`
for CDNs, but the server renders every page itself (`ssr.PageInit` turns the
static index lookup off). A project with `mode = hash` or its own `main`
function is refused. Regions that need browser only state (`salamWsStyle`,
per row state of `each`) are left empty by the server and filled by the bundle.

The generated server reads `PORT` (`0` picks a free port) and `HOST`, serves `dist/public` with
`ETag` and caching headers, and listens behind any reverse proxy. Backend
addresses and secrets come from the environment through `std/env`.

## Static output

The default build (no `--ssr`) has no server to answer `/_salam/fn/<Name>`. A
page that can reach a `server func` is then E289 `'F' needs --ssr`, unless the
entry page sets `server = "https://front.example.com"`, in which case the stub
posts there (the same setting native uses).

## Native

There is no rendering on native. A `server func` call becomes an HTTP call to the
front server named in the app settings, using the wire protocol above, so the
app and the site share one set of functions, and no secret is in the binary.

## Server function lowering

The compiler lowers a `server func F(args): T` three ways, chosen by the
output being built. The choice is made in `compiler/semantic/sema_server_gen.salam`
before function bodies are checked.

| output | what `F` becomes |
| --- | --- |
| server program (default, `--ssr` server) | `F` unchanged, plus a generated wrapper per function and `__salam_register_server_funcs()` |
| web (`salam page build`, `salam web`, `salam js`) | `F` keeps its signature; its body is replaced by a stub that posts to the server |
| native page (`ui_target` native) | same stub, using `net/ssrclient` (HTTP client) |

Server side. For every server func the compiler adds, as ordinary checked
Salam code, `__salam_srv_<n>(c: i64)`: it reads the JSON `args` object with
`ssr.Args(c)`, decodes each parameter by name with `json.Unmarshal` (a missing
required key, a wrong type or an unknown enum member answers 400 through
`ssr.Reject`; a `T?` parameter may be absent), calls `F`, and answers
`ssr.Ok(c, json.Marshal(result))`. A function declared `throw` runs inside a
`try:`: an error becomes `ssr.Fail(c, "server function failed")` and the real
message is printed on the server only. A call to `F` from server code is an
ordinary in-process call.

The hook `func __salam_register_server_funcs()` registers every wrapper by wire
name with `ssr.Register`. The wire name is the English alias when `F` has one,
else its name. The function exists whenever the program has a server func and
is built as a server program. If a definition with that name is already
present (the `--ssr` generator adds an empty one), its body is replaced, so
there is one definition either way. The imports `net/ssr` and `encoding/json`
are added when the program does not import them. Wire types are the JSON
mapping of `std/encoding/json`: `int` and `float` are numbers, enums are their
member name, structs are objects, `Vector<T>` is an array.

Client side. The stub body builds `{"name": value, ...}` with `json.Marshal`,
calls `ssrweb.Call(name, args)` (web) or `ssrclient.Call(name, args)`
(native), and decodes the value. On failure a function declared `throw`
throws the server's error text; any other function returns the zero value.
The result of a `server func` call is therefore available at once: the web
stub uses a synchronous request, so a handler or a component body that calls
one blocks until the answer arrives. The `load(...)` form of R9 is not part of
the language yet; when it is, it wraps the same stub.

- Token (web): `ssrweb` reads `<script type="application/json" id="__salam">{"token": "..."}</script>`
  (the SSR build embeds it), then `<meta name="salam-token" content="...">`,
  then asks `GET /_salam/session`, and sends it as `X-Salam-Token`. A 403
  fetches a new token once and retries.
- Server address: the page setting `server = "https://front.example.com"`
  (Persian `سرور`) in the `page:` block. Web: written as
  `<meta name="salam-server">`; the stub posts to it, same origin when absent.
  Native: the page calls `ssrclient.SetServer(url)` at start; without the
  setting `SALAM_SERVER` from the environment is used.
- Not in the client output: the body, and any top level function or global that
  only server funcs use, are dropped from the web program after the checks
  (name based reachability from every non server definition). Native programs
  lose them through unreachable function removal, because the stub no longer
  mentions them.
- E286: a definition that stays in the web program and uses a member of `env`
  or `ssr` (`env.Get`, `ssr.Request`) is an error naming that member.
- E289: a static web build (`salam page build` without a `server` setting)
  whose page, handlers or helpers mention a server func is an error. The
  `--ssr` build leaves the check off.

## Diagnostics

| code | kind | message |
| --- | --- | --- |
| E285 | error | `server func 'F' parameter 'x' has type 'T', which cannot cross the network` |
| E286 | error | `'F' runs in the browser but uses 'G', which only exists on the server` |
| E287 | error | `server func 'F' must be declared at the top level` |
| E288 | error | `two server funcs are named 'F'` (after the Persian name is applied) |
| E289 | error | `server func 'F' is reachable from a static build; build with --ssr or set 'server'` |
| W280 | warning | `server func 'F' looks like backend code ('db'); call your API from here instead` |

## Decisions

| # | decision |
| --- | --- |
| SV1 | separate projects: the front has no data layer, only `server func` that call HTTP APIs |
| SV2 | `server func` is a normal function with a marker; W280 warns on backend looking code |
| SV3 | wire is JSON over POST `/_salam/fn/<Name>`, same origin plus a token |
| SV4 | `env` and server bodies never reach a client output (E286) |
| SV5 | SSR is opt in (`--ssr`); static stays the default |
| SV6 | hydration by stable site ids and an embedded state and call cache; mismatch re-renders one scope |
| SV7 | native calls the front server through the same protocol |

## First implementation steps

1. `server func` parse and checks (E285, E287, E288), no output change yet.
2. Server side runtime package `net/ssr`: registry, endpoints, token, `request()`, `env`.
3. `salam page build --ssr`: generated server, render per route, guards, 404 (done).
4. Stubs: JS stub and native stub; `load(...)` integration; E286, W280.
5. Hydration: embedded state, call cache keys, per scope warning (done).
6. `salam serve --render` with rebuild (done, no browser reload); docs and website pages (en and fa).
