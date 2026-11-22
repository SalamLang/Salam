# Server rendering and server functions

Status: proposed (2026-11-22). Builds on `ROUTING.md` and the page language
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

A normal function with a marker. It runs only on the front server. The page,
the browser bundle and the native app never contain its body.

```salam
server func GetProduct(id: int): Product:
    r := http.GetWith(env("SHOP_API") + "/products/" + id, auth_headers())
    ret json.Decode(r.body) as Product
end
```

```salam
pub component ProductPage(id: int):
    p := GetProduct(id)
    heading: content = p.name end
end
```

### Rules

- Top level only. Parameters and result are wire types: `int`, `float`, `bool`,
  `str`, structs and `Vector`s of wire types, optional (`T?`). No pointers,
  references or closures (E285).
- Callable from a component body or `load(...)`, a handler, and another
  `server func`. On the server a call is a direct call. In the browser and on
  native it is the generated stub (below). The call site looks the same.
- `env(name)` reads a server environment variable. It is allowed only inside a
  `server func` (E286), so a secret cannot reach a bundle.
- A function that is reachable from client code and uses a server only name
  (`env`, a server func body, a server only import) is E287. Reachability is
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
want it set `allow_backend = true` in `salam.cfg`.

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
   before the response (default budget 3 seconds per request; on timeout the
   loading branch is rendered and the client continues).
4. Write `<head>` (title, meta, CSS link), the body, the embedded state
   (`<script type="application/json" id="__salam">`: variable values and the
   call cache) and the bundle script.

Streaming and incremental regeneration are out of the first version.

## Hydration

- Every element that has an event, a condition or a bound value carries a stable
  id attribute (`data-s`). Ids come from the same site numbering the web
  runtime uses for its scopes, so server and client agree without a walk.
- The bundle reads `__salam`, restores the state, and attaches handlers and
  scope tracking to the existing nodes. It does not render again.
- The call cache is keyed by function name and a hash of the arguments. A
  `server func` call during hydration that hits the cache returns at once and
  does not fetch.
- If an id is missing or a node differs from what the scope expects, that scope
  re-renders from state and a line goes to the console (and to the trace with
  `SALAM_UI_TRACE=1`). A mismatch is never fatal.
- R1 to R12 (`UI_DSL.md`) apply unchanged. The R11 tests run in a third mode,
  server render then hydrate, with the same expected output.

## Build and run

| command | result |
| --- | --- |
| `salam page build` | static site, as today |
| `salam page build --ssr` | `dist/server` (a native executable made from a generated Salam program: router, guards, renderers, function endpoints) and `dist/public` (bundle, CSS, assets, prerendered pages for routes with no parameters) |
| `salam serve --render` | builds into a scratch directory, runs the server, rebuilds on save, reloads the browser |

The generated server reads `PORT` and `HOST`, serves `dist/public` with
`ETag` and caching headers, and listens behind any reverse proxy. Backend
addresses and secrets come from the environment through `env`.

## Native

There is no rendering on native. A `server func` call becomes an HTTP call to the
front server named in the app settings, using the wire protocol above, so the
app and the site share one set of functions, and no secret is in the binary.

## Diagnostics

| code | kind | message |
| --- | --- | --- |
| E285 | error | `server func 'F' parameter 'x' has type 'T', which cannot cross the network` |
| E286 | error | `'env' can only be used inside a server func` |
| E287 | error | `'F' runs in the browser but uses 'G', which only exists on the server` |
| E288 | error | `server func 'F' must be declared at the top level` |
| E289 | error | `two server funcs are named 'F'` (after the Persian name is applied) |
| W280 | warning | `server func 'F' looks like backend code ('db'); call your API from here instead` |

## Decisions

| # | decision |
| --- | --- |
| SV1 | separate projects: the front has no data layer, only `server func` that call HTTP APIs |
| SV2 | `server func` is a normal function with a marker; W280 warns on backend looking code |
| SV3 | wire is JSON over POST `/_salam/fn/<Name>`, same origin plus a token |
| SV4 | `env` and server bodies never reach a client output (E286, E287) |
| SV5 | SSR is opt in (`--ssr`); static stays the default |
| SV6 | hydration by stable site ids and an embedded state and call cache; mismatch re-renders one scope |
| SV7 | native calls the front server through the same protocol |

## First implementation steps

1. `server func` parse and checks (E285, E288, E289), no output change yet.
2. Server side runtime package `net/ssr`: registry, endpoints, token, `request()`, `env`.
3. `salam page build --ssr`: generated server, render per route, guards, 404.
4. Stubs: JS stub and native stub; `load(...)` integration; E286, E287, W280.
5. Hydration: ids, embedded state, call cache, per scope fallback.
6. `salam serve --render` with rebuild; docs and website pages (en and fa).
