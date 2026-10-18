# 🤖 salam-mcp

A [Model Context Protocol](https://modelcontextprotocol.io) server for the
Salam programming language, **written in Salam**.

It gives an AI model the three things it otherwise has to guess at: the
language itself (the full reference, searchable by topic or error code), the
standard library (what exists, its exact signatures, and working examples),
and the compiler (check, build, run, format, translate, inspect, emit IR).
Diagnostics come back as structured data with exact line and column
positions, the offending source line, the compiler's help, and a hint when the
code uses syntax borrowed from another language.

Because it is a single native binary with no runtime dependencies, anyone who
has `salam` can run it. There is no Node, Python or package manager in the
loop.

## 🌐 Hosted server

The server is also online at **`https://mcp.salamlang.ir/mcp`**, so any MCP
client can use it with nothing installed:

```sh
claude mcp add --transport http salam https://mcp.salamlang.ir/mcp
```

```json
{
  "mcpServers": {
    "salam": { "type": "http", "url": "https://mcp.salamlang.ir/mcp" }
  }
}
```

In claude.ai or Claude Desktop, add the same URL under Settings, Connectors,
Add custom connector.

The first connection opens a browser window to sign in with a Salam account
from [ai.salamlang.ir](https://ai.salamlang.ir) and allow the app. The client
then keeps an access token and refreshes it on its own. Each tool call is
recorded against the account and the app, with its arguments, so people can
review what their apps did under
[Connected apps](https://ai.salamlang.ir/apps) and disconnect them there.

The hosted server cannot see your files, so its compiler tools take the
program text in a `source` argument (plus `files` for a multi-file program)
instead of a `path`. It never runs code:
`salam_run`, `salam_exec` and `salam_build` exist only when you run the server
yourself. Everything else, including the stdlib, example and keyword tools,
works the same.

To host your own, build the image from the repository root and put it behind
a TLS proxy:

```sh
docker build -f tools/mcp/Dockerfile -t salam-mcp .
docker run -d -p 127.0.0.1:8080:8080 --read-only --tmpfs /tmp:size=256m salam-mcp
```

Or run `salam-mcp --http` directly; it listens on `SALAM_MCP_HOST:SALAM_MCP_PORT`
and serves MCP at `/mcp`, a setup page at `/` and `GET /health` for probes.
`salam-mcp --health` exits 0 when a local server answers, for container
health checks.

## 🔨 Install

You need a `salam` compiler from this checkout. If `./salam` is missing,
build it first:

```sh
tools/bash/build-selfhost.sh --output "$PWD/salam"
```

Then build the server:

```sh
tools/mcp/build.sh            # -> ./salam-mcp
```

Build it with the compiler from this checkout, not an installed one. An older
`salam` on `PATH` parses some flags differently and the server then fails in
ways that look like server bugs. `build.sh` picks `$SALAM`, then `./salam`,
then `salam` on `PATH`, and compiles in a temporary directory so it never
leaves a `.salam-build` behind. All build output goes to stderr.

## 🔌 Wire it up

### Claude Code in this repository

Nothing to do beyond having a compiler. `.mcp.json` in the repository root
starts the server through `tools/mcp/run.sh`:

```json
{ "mcpServers": { "salam": { "command": "sh", "args": ["tools/mcp/run.sh"] } } }
```

The launcher finds the checkout from its own path, rebuilds `salam-mcp` when
the binary is missing or older than any `tools/mcp/*.salam` file, sets
`SALAM_MCP_ROOT` to the checkout unless it is already set, and then `exec`s
the server. A fresh clone or a new worktree therefore works on first use;
the first start takes a few seconds longer while it builds.

Claude Code starts stdio servers in the project root, so the relative path
is enough. The config deliberately avoids `${CLAUDE_PROJECT_DIR}`: Claude
Code does not always define it when it expands `.mcp.json`, and an unset
variable is passed through literally, which used to fail with
`ENOENT ... posix_spawn '${CLAUDE_PROJECT_DIR}/salam-mcp'`.

Approve the project server the first time Claude Code asks (or via `/mcp`),
then check it:

```sh
claude mcp list               # salam: sh tools/mcp/run.sh - Connected
```

If it does not connect, run the launcher by hand and read stderr:

```sh
printf '%s\n' '{"jsonrpc":"2.0","id":1,"method":"ping"}' | sh tools/mcp/run.sh
```

### Claude Code plugin

`tools/mcp/claude-plugin` bundles the server with a language skill and the
`/salam-check`, `/salam-api` and `/salam-run` shortcuts. The plugin starts
`salam-mcp` from `PATH`, so build it and put it there first:

```sh
tools/mcp/build.sh "$HOME/.local/bin/salam-mcp"
```

Outside a Salam checkout the server uses the project directory as its
workspace and looks for `std/` next to the compiler, so point
`SALAM_MCP_BIN` or `SALAM_STD` at a checkout if the stdlib tools report that
`std` is missing.

### Any other MCP client

```json
{
  "mcpServers": {
    "salam": {
      "command": "/path/to/salam-mcp",
      "env": { "SALAM_MCP_ROOT": "/path/to/your/salam/project" }
    }
  }
}
```

### 🌍 Environment Variables

| Variable                | Default                                        | Purpose                                             |
| ----------------------- | ---------------------------------------------- | --------------------------------------------------- |
| `SALAM_MCP_BIN`         | `<root>/salam[.exe]`, else `salam` from `PATH` | Which compiler to drive                             |
| `SALAM_MCP_ROOT`        | `.` (the launcher sets it to the checkout)     | Workspace that paths resolve against                |
| `SALAM_STD`             | `<root>/std`, else `std` beside the compiler   | Standard library root                               |
| `SALAM_MCP_RUN_TIMEOUT` | `60`                                           | Seconds before `salam_run`/`salam_exec` are stopped |

With `--http` the server also reads:

| Variable                   | Default                        | Purpose                                                                                                                                    |
| -------------------------- | ------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------ |
| `SALAM_MCP_HOST`           | `0.0.0.0`                      | Address to listen on                                                                                                                       |
| `SALAM_MCP_PORT`           | `8080`                         | Port to listen on                                                                                                                          |
| `SALAM_MCP_JOBS`           | `4`                            | Compiler processes allowed at once; extra calls wait                                                                                       |
| `SALAM_MCP_TIMEOUT`        | `600`                          | Seconds before a compiler process is killed                                                                                                |
| `SALAM_MCP_MEMORY_SOFT_MB` | `1500`                         | Restart to give memory back above this, once no call is in flight                                                                          |
| `SALAM_MCP_MEMORY_HARD_MB` | `3200`                         | Restart above this even with calls in flight                                                                                               |
| `SALAM_MCP_ORIGINS`        | empty                          | Comma separated browser origins to allow; empty allows all                                                                                 |
| `SALAM_MCP_AUTH_URL`       | empty                          | Base URL of the authorization server's API; setting it turns sign-in on                                                                    |
| `SALAM_MCP_AUTH_SECRET`    | empty                          | Shared secret for token introspection and activity reports                                                                                 |
| `SALAM_MCP_ISSUER`         | `https://ai-api.salamlang.ir`  | Issuer listed in the protected resource metadata                                                                                           |
| `SALAM_MCP_RESOURCES`      | `https://mcp.salamlang.ir/mcp` | URIs this server answers to; tokens must be issued for one of them                                                                         |
| `SALAM_MCP_SERVICE_TOKEN`  | empty                          | A bearer token (32 characters or more) for a trusted internal caller such as the Salam AI assistant; its calls are reported as the service |

### 🔑 Sign-in

With `SALAM_MCP_AUTH_URL` set, the server is an OAuth 2.1 resource server as
the MCP authorization spec describes. The authorization server is the Salam AI
backend in [`ai/backend`](../../ai/backend), which owns the accounts.

- `GET /.well-known/oauth-protected-resource` (and `/mcp` appended) is the
  RFC 9728 metadata: the resource URI and its authorization server.
- A request without a valid token gets `401` with
  `WWW-Authenticate: Bearer resource_metadata="...", scope="mcp"`, plus
  `error="invalid_token"` when a token was sent; a token without the `mcp`
  scope gets `403` with `error="insufficient_scope"`.
- Tokens are checked by RFC 7662 introspection against
  `SALAM_MCP_AUTH_URL/oauth/introspect`, cached for at most 30 seconds, and
  accepted only when their audience is one of `SALAM_MCP_RESOURCES`. A
  disconnected app therefore stops working within 30 seconds.
- Every `tools/call` and `resources/read` is reported to
  `SALAM_MCP_AUTH_URL/internal/mcp/events` from a background queue, so a slow
  report never delays a reply. Reporting is best effort: if the 1024-event
  queue is full, the event is dropped, logged, and counted in
  `dropped_events` on `GET /health`.

Without `SALAM_MCP_AUTH_URL` (and always over stdio) nothing changes: no
sign-in, no reports.

The compiler default prefers the binary in the workspace over `PATH` on
purpose: an installed `salam` is often older than the checkout, and the
resulting flag-parsing differences surface as confusing tool failures rather
than as a version error.

On Windows, point `command` at `salam-mcp.exe`. Process spawning does not add
the extension for you.

### 🖥️ Windows

There is no launcher script for Windows yet. Build `salam-mcp.exe` with
`salam build tools/mcp/main.salam --output=salam-mcp.exe` and point
`command` in your MCP config at its full path.

## 🛠️ Tools

Every compiler tool takes either a workspace `path` or the program text as
`source`, plus `files` (`{"util.salam": "..."}`) for the files it includes.
The hosted server takes only `source` and `files`.

### 🔧 Compiler

| Tool              | Use it for                                                                      |
| ----------------- | ------------------------------------------------------------------------------- |
| `salam_check`     | Type-check without codegen; the fast loop, run after every edit                 |
| `salam_build`     | Full compile including linking; supports `backend`, `release`, `target`         |
| `salam_run`       | Build and execute with optional `stdin`, capturing stdout, stderr and exit code |
| `salam_exec`      | Run on the interpreter without a C toolchain, with optional `stdin`             |
| `salam_format`    | Return the canonical formatting of a file as text; never rewrites files         |
| `salam_translate` | Return a program rewritten between English and Persian Salam                    |

### 🔬 Inspection

| Tool            | Use it for                                            |
| --------------- | ----------------------------------------------------- |
| `salam_inspect` | Token, AST or symbol dump as XML                      |
| `salam_llvm_ir` | Textual LLVM IR                                       |
| `salam_js`      | Compile to browser-ready JavaScript                   |
| `salam_version` | Version, commit and build date of the driven compiler |

### 📚 Language, standard library and examples

| Tool                    | Use it for                                                                      |
| ----------------------- | ------------------------------------------------------------------------------- |
| `salam_docs`            | The reference section for a topic ("lambda", "loops") or an error code ("E085") |
| `salam_stdlib_search`   | Find std functions, methods and types by what they do, across every package     |
| `salam_stdlib_packages` | Every importable `std` package in this checkout                                 |
| `salam_stdlib_symbols`  | One package's types, fields, methods, functions and constants with signatures   |
| `salam_find_examples`   | Search the test corpus (then std) for working usages, with context lines        |
| `salam_read_source`     | Read a line-numbered slice of any checkout file                                 |
| `salam_keywords`        | English/Persian keyword table                                                   |

Every tool carries MCP annotations: all of them are read-only and idempotent
except `salam_run` and `salam_exec`, which execute the program.

### 🩺 Diagnostics

A failed check opens with a one-line summary (`salam check main.salam: 3
errors`) and lists each diagnostic with:

- the code, `file:line:column` and message;
- the source line with a caret under the column;
- the compiler's `help`, when it gives one;
- a `hint` when the line uses syntax from another language (`for x in`,
  `while`, `func (...)` lambdas, `elif`, `return`, braces, `.length`, `self`,
  `True`/`None`, foreign type names) or when a keyword error really comes from
  an unfinished line above;
- for a coded error, the minimal failing program in `tests/en/errors` that
  `salam_docs` shows for that code.

When the first error is a parse error the result says so, since the rest are
usually knock-on effects. The same data is in `structuredContent.diagnostics`.

## 💬 Prompts

| Prompt        | Arguments         | What it sets up                                      |
| ------------- | ----------------- | ---------------------------------------------------- |
| `salam-write` | `task`, `lang`    | Write a new program with the check-and-fix workflow  |
| `salam-port`  | `code`, `from`    | Port code from another language into idiomatic Salam |
| `salam-fix`   | `code`, `problem` | Diagnose and fix a program that fails or misbehaves  |

Each one carries the workflow and the short list of rules that differ from
other languages. The same list is in the server's `instructions`, so a client
that injects them gives the model the rules before its first line of Salam.

## 📖 Resources

| URI                         | Contents                                                       |
| --------------------------- | -------------------------------------------------------------- |
| `salam://guide/skill.md`    | The complete language reference (`SKILL.md`)                   |
| `salam://guide/agents.md`   | The short guide for agents (`docs/ai/AGENTS.md`)               |
| `salam://stdlib/index.json` | Every package mapped to its public declarations, computed live |
| `salam://guide/llms.txt`    | Orientation entry point                                        |

Two templates: `salam://stdlib/{package}` is one package's API (as
`salam_stdlib_symbols` shows it) and `salam://docs/{topic}` is the reference
section for a topic or error code.

## 🔒 Safety

The server is **read-only**: it never writes to the workspace. The compiler
runs inside a private temporary directory (so its `.salam-build` cache lands
there, not in your checkout), build artifacts go to temp paths, the directory
is removed when the server exits, and `salam_format` always runs with
`--check`.

Paths from a tool call resolve against `SALAM_MCP_ROOT`, never against the
server's own working directory. They are validated rather than escaped:
anything containing shell metacharacters or `..`, or an absolute path outside
the workspace, is refused outright.

`salam_run` and `salam_exec` read stdin from the `stdin` argument (empty by
default, never the server's own stdin) and are killed after
`SALAM_MCP_RUN_TIMEOUT` seconds (default 60; 0 turns the limit off), so an
endless loop is reported as a timeout instead of hanging the server.

In HTTP mode each call writes its `source` and `files` to a fresh temporary directory,
compiles it there and deletes the directory afterwards, so concurrent callers
never share files. Compiler processes are capped at `SALAM_MCP_JOBS` and
killed after `SALAM_MCP_TIMEOUT` seconds, request bodies are limited to 4 MB
and sources to 256 KB, and the tools that execute a program are not offered.
Run the container read-only with a size-limited `/tmp`, as above.

## 📡 Protocol

Implements MCP **2026-07-28** and is **dual-era**: a client that opens with the
legacy `initialize` handshake gets legacy semantics (protocol versions
`2025-11-25` back to `2024-11-05`) for the life of the process, while modern
clients declare their version per request in `_meta` and may call
`server/discover`. A request naming an unsupported version gets
`UnsupportedProtocolVersionError` (`-32022`) listing what is available, as the
spec requires.

Two transports:

- **stdio** (default): one JSON-RPC message per line, nothing but MCP messages
  on stdout, logging on stderr, and the process exits when stdin closes.
- **Streamable HTTP** (`--http`): `POST /mcp` with a JSON-RPC message returns
  the response as `application/json`; notifications get `202 Accepted`. The
  server is stateless, so it issues no `Mcp-Session-Id` and answers `GET` and
  `DELETE` on `/mcp` with `405`. An unsupported `MCP-Protocol-Version` header
  gets `400` and a body that is not JSON gets `415`. Each HTTP request is
  negotiated on its own: one that carries the protocol version in `_meta`
  gets modern semantics, any other gets legacy semantics.

## 🧪 Tests

```sh
node tools/mcp/tests/protocol_test.mjs ./salam-mcp "$PWD"
```

Spawns real server processes and asserts on the wire format: framing,
dual-era negotiation, error codes, structured diagnostics and their hints,
docs and stdlib search, multi-file and stdin inputs, run timeouts, prompts,
resource templates, path-traversal refusal, and the sign-in responses
against a mock introspection endpoint. CI runs this on every change to `tools/mcp/`, `docs/ai/` or `std/`.

## 🔄 Regenerating the committed stdlib index

`docs/ai/stdlib-index.json` is generated from `std/`. After changing the
standard library:

```sh
tools/mcp/gen-index.sh
```

CI fails if the committed copy has drifted. The `stdlib-index` prek hook
runs this for you whenever a commit touches `std/` or `tools/mcp/`, and
fails when it had to rewrite the index, so the drift shows up locally
rather than on the pull request. A checkout without a built `salam-mcp`
cannot regenerate anything; there the hook reports that and passes.

## 📁 Layout

| File                  | Role                                                       |
| --------------------- | ---------------------------------------------------------- |
| `run.sh`              | Launcher: rebuilds the binary when stale, then execs it    |
| `build.sh`            | Builds `salam-mcp` in a scratch directory                  |
| `main.salam`          | Entry point: stdio loop, `--http` and `--health`           |
| `mcp_http.salam`      | Streamable HTTP transport, CORS and the setup page         |
| `mcp_auth.salam`      | Bearer checks, protected resource metadata, activity queue |
| `Dockerfile`          | Image for the hosted server                                |
| `mcp_rpc.salam`       | JSON-RPC framing, EOF-aware line reads, protocol constants |
| `mcp_server.salam`    | Method dispatch and dual-era version negotiation           |
| `mcp_registry.salam`  | Tool catalog (JSON Schemas, annotations) and routing       |
| `mcp_tools.salam`     | Compiler-driving handlers and diagnostic enrichment        |
| `mcp_hints.salam`     | Hints for syntax borrowed from other languages             |
| `mcp_docs.salam`      | Stdlib, search, example, docs and keyword handlers         |
| `mcp_api.salam`       | Parses `std/` into an in-memory declaration index          |
| `mcp_guide.salam`     | Splits the reference into sections; error-code examples    |
| `mcp_decl.salam`      | The declaration and section records                        |
| `mcp_prompts.salam`   | Prompts, the language crib and the server instructions     |
| `mcp_resources.salam` | Resources, templates, live stdlib index                    |
| `mcp_exec.salam`      | Subprocess execution and argument validation               |
| `mcp_result.salam`    | The `ToolResult` shape shared by every handler             |
| `mcp_text.salam`      | Backend-safe string helpers                                |

Compiler processes are started through `os.Run` with stdin, stdout and stderr
redirected to temporary files, so a child that writes a lot of output (a full
AST dump, say) can never block on a full pipe.

## 🔗 Links

- [Model Context Protocol](https://modelcontextprotocol.io) - protocol specification
- [Claude Code](https://claude.ai/code) - the primary MCP client for Salam development
- [Salam Playground](https://editor.salamlang.ir/) - try Salam in your browser
- [Discord](https://discord.gg/HfY3QHDPdv) - real-time community chat
- [Telegram](https://t.me/SalamProgrammingLanguage) - community on Telegram
