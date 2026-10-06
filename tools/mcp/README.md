# 🤖 salam-mcp

A [Model Context Protocol](https://modelcontextprotocol.io) server for the
Salam programming language, **written in Salam**.

It gives an AI model two things it otherwise has to guess at: the compiler
(check, build, run, format, inspect, emit IR) and the standard library (what
packages exist, what they actually export, and working examples). Diagnostics
come back as structured data with exact line and column positions, not as
scraped text.

Because it is a single native binary with no runtime dependencies, anyone who
has `salam` can run it. There is no Node, Python or package manager in the
loop.

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

| Variable         | Default                                        | Purpose                              |
| ---------------- | ---------------------------------------------- | ------------------------------------ |
| `SALAM_MCP_BIN`  | `<root>/salam[.exe]`, else `salam` from `PATH` | Which compiler to drive              |
| `SALAM_MCP_ROOT` | `.` (the launcher sets it to the checkout)     | Workspace that paths resolve against |
| `SALAM_STD`      | `<root>/std`, else `std` beside the compiler   | Standard library root                |

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

### 🔧 Compiler

| Tool           | Use it for                                                              |
| -------------- | ----------------------------------------------------------------------- |
| `salam_check`  | Type-check without codegen; the fast loop, run after every edit         |
| `salam_build`  | Full compile including linking; supports `backend`, `release`, `target` |
| `salam_run`    | Build and execute, capturing stdout, stderr and exit code               |
| `salam_exec`   | Interpret without a C toolchain (not trustworthy for unsigned maths)    |
| `salam_format` | Report formatting drift; never rewrites files                           |

### 🔬 Inspection

| Tool            | Use it for                                            |
| --------------- | ----------------------------------------------------- |
| `salam_inspect` | Token, AST or symbol dump as XML                      |
| `salam_llvm_ir` | Textual LLVM IR                                       |
| `salam_js`      | Compile to browser-ready JavaScript                   |
| `salam_version` | Version, commit and build date of the driven compiler |

### 📚 Standard library and examples

| Tool                    | Use it for                                       |
| ----------------------- | ------------------------------------------------ |
| `salam_stdlib_packages` | Every importable `std` package in this checkout  |
| `salam_stdlib_symbols`  | A package's real public declarations, filterable |
| `salam_find_examples`   | Search the test corpus for working usages        |
| `salam_read_source`     | Read a line-numbered slice of any workspace file |
| `salam_keywords`        | English/Persian keyword table                    |

## 📖 Resources

| URI                         | Contents                                                       |
| --------------------------- | -------------------------------------------------------------- |
| `salam://guide/agents.md`   | The language guide (`docs/ai/AGENTS.md`)                       |
| `salam://stdlib/index.json` | Every package mapped to its public declarations, computed live |
| `salam://guide/llms.txt`    | Orientation entry point                                        |

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

## 📡 Protocol

Implements MCP **2026-07-28** and is **dual-era**: a client that opens with the
legacy `initialize` handshake gets legacy semantics (protocol versions
`2025-11-25` back to `2024-11-05`) for the life of the process, while modern
clients declare their version per request in `_meta` and may call
`server/discover`. A request naming an unsupported version gets
`UnsupportedProtocolVersionError` (`-32022`) listing what is available, as the
spec requires.

Transport is stdio: one JSON-RPC message per line, nothing but MCP messages on
stdout, logging on stderr, and the process exits when stdin closes.

## 🧪 Tests

```sh
node tools/mcp/tests/protocol_test.mjs ./salam-mcp "$PWD"
```

Spawns real server processes and asserts on the wire format: framing,
dual-era negotiation, error codes, structured diagnostics, path-traversal
refusal. CI runs this on every change to `tools/mcp/`, `docs/ai/` or `std/`.

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
| `main.salam`          | Entry point and the stdio read loop                        |
| `mcp_rpc.salam`       | JSON-RPC framing, EOF-aware line reads, protocol constants |
| `mcp_server.salam`    | Method dispatch and dual-era version negotiation           |
| `mcp_registry.salam`  | Tool catalog (JSON Schemas) and `tools/call` routing       |
| `mcp_tools.salam`     | Compiler-driving handlers                                  |
| `mcp_docs.salam`      | Stdlib, example-search and keyword handlers                |
| `mcp_resources.salam` | Resource list/read, live stdlib index                      |
| `mcp_exec.salam`      | Subprocess execution and argument validation               |
| `mcp_result.salam`    | The `ToolResult` shape shared by every handler             |
| `mcp_text.salam`      | Backend-safe string helpers                                |

Two implementation notes worth knowing, both documented at the top of the file
that works around them: the server avoids `str.Split` (it can segfault on its
last element on gcc-linked builds) and avoids `os.shell.Run` (it deadlocks when
a child outfills the pipe buffer, which compiler dumps routinely do).

## 🔗 Links

- [Model Context Protocol](https://modelcontextprotocol.io) - protocol specification
- [Claude Code](https://claude.ai/code) - the primary MCP client for Salam development
- [Salam Playground](https://salamlang.github.io/Salam/) - try Salam in your browser
- [Discord](https://discord.gg/HfY3QHDPdv) - real-time community chat
- [Telegram](https://t.me/SalamProgrammingLanguage) - community on Telegram
