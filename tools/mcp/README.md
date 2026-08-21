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

## 🔨 Build

```sh
tools/mcp/build.sh            # -> ./salam-mcp
```

Build it with the compiler from this checkout, not an installed one. An older
`salam` on `PATH` parses some flags differently and the server then fails in
ways that look like server bugs.

## 🔌 Wire it up

`.mcp.json` in the repository root already configures it for anything that
reads project-scoped MCP config (Claude Code included), so building the binary
is enough.

For a Claude Code **plugin** (the server plus a language skill and
`/salam-check`, `/salam-api`, `/salam-run` shortcuts), install
`tools/mcp/claude-plugin`, which expects `salam-mcp` on `PATH`.

Any other MCP client:

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
| `SALAM_MCP_ROOT` | `.`                                            | Workspace that paths resolve against |
| `SALAM_STD`      | `<root>/std`                                   | Standard library root                |

The compiler default prefers the binary in the workspace over `PATH` on
purpose: an installed `salam` is often older than the checkout, and the
resulting flag-parsing differences surface as confusing tool failures rather
than as a version error.

On Windows, point `command` at `salam-mcp.exe`. Process spawning does not add
the extension for you.

### 🖥️ Platform-specific setup

**macOS / Linux** — build and add to PATH:

```sh
tools/mcp/build.sh
export PATH="$PWD:$PATH"   # add salam-mcp to PATH for Claude Code plugin
```

**Windows** — build from the repository root (requires `salam` on PATH):

```bat
tools\mcp\build.bat
```

Then point `command` in your MCP config at the full path to `salam-mcp.exe`.

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
| `salam_keywords`        | English/Persian/Arabic keyword table             |

## 📖 Resources

| URI                         | Contents                                                       |
| --------------------------- | -------------------------------------------------------------- |
| `salam://guide/agents.md`   | The language guide (`docs/ai/AGENTS.md`)                       |
| `salam://stdlib/index.json` | Every package mapped to its public declarations, computed live |
| `salam://guide/llms.txt`    | Orientation entry point                                        |

## 🔒 Safety

The server is **read-only**: it never writes to the workspace. Build artifacts
go to temp paths and are deleted, and `salam_format` always runs with
`--check`.

Paths from a tool call are validated rather than escaped; anything containing
shell metacharacters or `..` is refused outright.

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

CI fails if the committed copy has drifted.

## 📁 Layout

| File                  | Role                                                       |
| --------------------- | ---------------------------------------------------------- |
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

- [Model Context Protocol](https://modelcontextprotocol.io) — protocol specification
- [Claude Code](https://claude.ai/code) — the primary MCP client for Salam development
- [Salam Playground](https://salamlang.github.io/Salam/) — try Salam in your browser
- [Discord](https://discord.gg/HfY3QHDPdv) — real-time community chat
- [Telegram](https://t.me/SalamProgrammingLanguage) — community on Telegram
